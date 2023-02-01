#include "graphint.h"

#include "gmextern.h"

static RECT	EnlargeRectOrig;
static RECT	EnlargeRectNew;
static HANDLE	hTranEnlargeScreen=0;
static THEMEHIGHLIGHTKEY	ThemeHighlightKey;
static THEMEHIGHLIGHTDATA	ThemeHighlightData;
static int	nGridElevPt=0;

short	LastDayOfMonth (long systime)
#if ENABLETRACE
{GSSiEnterProg (1078);
#endif
{   
	time_t	testtime;
	struct	tm	tmtime;
	short	wantmonth, iday=32;
	time_t	t = systime;
	
	tmtime = *localtime (&t);
	wantmonth = tmtime.tm_mon;
	do
	{
		tmtime = *localtime (&t);
		iday--;
		tmtime.tm_mday = iday; 
		tmtime.tm_isdst = -1;
		testtime = mktime (&tmtime); 
		if (testtime < 0)
{
#if ENABLETRACE
GSSiExitProg (1078);
#endif
			return iday;
}
		tmtime = *localtime (&testtime);
	} while (tmtime.tm_mon != wantmonth); 
{
#if ENABLETRACE
GSSiExitProg (1078);
#endif
	return iday;
}
#if ENABLETRACE
}
#endif
}

BOOL GoToNextControl (HWND hWndDlg,UINT FromControl,UINT ToControl)
#if ENABLETRACE
{GSSiEnterProg (1079);
#endif
{
	SetFocus (GetDlgItem(hWndDlg,ToControl));
	SendMessage(GetDlgItem(hWndDlg, ToControl),EM_SETSEL, 0, -1);
{
#if ENABLETRACE
GSSiExitProg (1079);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

 

HANDLE	LinfitHLTItems (LPINT pNumPoints,short Function)
#if ENABLETRACE
{GSSiEnterProg (1081);
#endif
{
	HPDPOINT	pPoints;
	short	pos=BT_FIRST;
	HANDLE	hPoints, hWeights,RtnhPoints=GSSiGlobAlloc (1052,GMEM_MOVEABLE,2*sizeof(DPOINT));   
	HPDPOINT	pRtnPoints=(HPDPOINT)GlobalLock (RtnhPoints), pBegPoint;
	HPDOUBLE	pWeights;
	HIGHLIGHTDATA	HighlightData; 
	DPOINT	OpenEnd;  
	long	Refno, NumPoints=0, LOFMDF, i,NumNewPoints; 
	BOOL	First=TRUE, Reverse1=FALSE, Reverse2=FALSE;  
	double	D1, D2, MinD, A, B, MAXDIF, TotWeight=0, Dist;

    if (!TotHLTPoints)
{
#if ENABLETRACE
GSSiExitProg (1081);
#endif
    	return 0;
}
    hPoints = GSSiGlobAlloc (1053,GMEM_MOVEABLE,(long)TotHLTPoints*sizeof(DPOINT)); 
    pPoints = (HPDPOINT)GlobalLock (hPoints);
    hWeights = GSSiGlobAlloc (1054,GMEM_MOVEABLE,(long)TotHLTPoints*sizeof(double)); 
    pWeights = (HPDOUBLE)GlobalLock (hWeights);
	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))
	{   
		pos = BT_NEXT;
		PickList[0] = HighlightData.PD; 
		pBegPoint = pPoints;
		NumNewPoints = GetPickItemPoints (0,FALSE,&pPoints);
		if (NumNewPoints == 1)
			pPoints--;
		else
		{  
			NumPoints += NumNewPoints; 
			Dist = ldistp (*pBegPoint++,*pBegPoint);
			*pWeights++ = Dist;
			for (i=1;i<NumNewPoints-1;i++)
			{
				Dist = ldistp (*pBegPoint++,*pBegPoint);
				*pWeights = (*(pWeights-1) + Dist)/2;
				pWeights++;
			}             
			*pWeights++ = Dist;
		}
	}
	GlobalUnlock (hPoints);
	pPoints = (HPDPOINT)GlobalLock (hPoints); 
	GlobalUnlock (hWeights);
	pWeights = (HPDOUBLE)GlobalLock (hWeights); 
	*pNumPoints = 2;  
	switch (Function)
	{
		case GF_LINFIT:
		if (LINFIT (pPoints,NumPoints,&A,&B,&MAXDIF,&LOFMDF))
		{ 
			if (fabs(B) < 1)
			{
				pRtnPoints[0].x = HLTBounds.xmn;
				pRtnPoints[0].y = A + pRtnPoints[0].x * B;
				pRtnPoints[1].x = HLTBounds.xmx;
				pRtnPoints[1].y = A + pRtnPoints[1].x * B;
			}
			else
			{                                        
				pRtnPoints[0].y = HLTBounds.ymn;
				pRtnPoints[0].x = (pRtnPoints[0].y - A)/B;
				pRtnPoints[1].y = HLTBounds.ymx;
				pRtnPoints[1].x = (pRtnPoints[1].y - A)/B;
			}
		}
		else
		{
				pRtnPoints[0].y = HLTBounds.ymn;
				pRtnPoints[0].x = pPoints->x;
				pRtnPoints[1].y = HLTBounds.ymx;
				pRtnPoints[1].x = pPoints->x;
		} 
		break;
		
		default:
		{
			double AveX=0,AveY=0;
			
			for (i=0;i<NumPoints;i++,pPoints++,pWeights++)
			{
				AveX += pPoints->x * *pWeights;
				AveY += pPoints->y * *pWeights;
				TotWeight += *pWeights;
			}
			AveX /= TotWeight;
			AveY /= TotWeight;
			if (Function == GF_HORZFIT) 
			{
				pRtnPoints[0].x = HLTBounds.xmn;
				pRtnPoints[0].y = AveY;
				pRtnPoints[1].x = HLTBounds.xmx;
				pRtnPoints[1].y = AveY;
			}
			else
			{                                        
				pRtnPoints[0].y = HLTBounds.ymn;
				pRtnPoints[0].x = AveX;
				pRtnPoints[1].y = HLTBounds.ymx;
				pRtnPoints[1].x = AveX;
			}
		}
	}  
	GSSiGlobUlFree (&hWeights);
	GSSiGlobUlFree (&hPoints);
	GlobalUnlock (RtnhPoints);
{
#if ENABLETRACE
GSSiExitProg (1081);
#endif
	return RtnhPoints;
}
#if ENABLETRACE
}
#endif
} 

double DistAlongCurve (LPDPOINT PC,LPDPOINT PT,LPDOUBLE pRPX,LPDOUBLE pRPY,LPDOUBLE pCLEN)
{ 
	double d, az1, az2,R, rpx,rpy,newclen, azdif;    
	DPOINT	RP={*pRPX,*pRPY}, POC;
	
	R = ldistp (RP,*PC);
	az1 = getazd (&RP,PC);
	az2 = getazd (&RP,PT);
	azdif = DeltaAZ (az1,az2);
	POC = dnewpt (RP,LTWOPI(az1+azdif/2),R);
   	RCURVE(&PC->x,&PC->y,&POC.x,&POC.y,&PT->x,&PT->y,&rpx,&rpy,&newclen);
   	if (Signof (*pCLEN) == Signof (newclen))
   		return (fabs (newclen));
   	azdif = TWOPI - fabs (azdif); 
	POC = dnewpt (RP,LTWOPI(az1+azdif/2),R);
   	RCURVE(&PC->x,&PC->y,&POC.x,&POC.y,&PT->x,&PT->y,&rpx,&rpy,&newclen);
	d = fabs (newclen);
	
	return d;
}

BOOL IntersectPolys1 (short Type1,short Type2,long nPnts1,HPDPOINT pPolyPoints1,HPDOUBLE pPolyAZ,
								 long nPnts2,HPDPOINT pPolyPoints2,int nPoly2,HANDLE hPolyPartLen2,
								 LPDPOINT pNearPoint,LPDPOINT pIntPoint,LPDOUBLE pD1,LPDOUBLE pD2,LPHANDLE pMaskAccelerators)
#if ENABLETRACE
{GSSiEnterProg (1082);
#endif
{
#define	NUMININDEX	32
    double   A1, A2, XP, YP, XMID1, XMID2, YMID1, YMID2, AMID,
             PAX1, PAY1, PAX2, PAY2, MNX, MNY, MXY,
             XMIDMN, XMIDMX, YMIDMN, YMIDMX, XMID2_NEED, YMID2_NEED;  
    double	PCX,PCY,RPX,RPY,CLEN, POCX, POCY, PTX, PTY;  
    double	XINT[3],YINT[3];
    HPDPOINT	lpAreaPoints=pPolyPoints2, lpPoint;
    DPOINT	EndPoint, EndAreaPoint, BeginPoint, BeginAreaPoint=*lpAreaPoints, IntPt, IntPoint[2];
    DWORD i, iline, iend1, iend2, index, numindex,indexbegin,indexend,j;
    long IRC ;
	MNMXCORD Bounds1, Bounds2; 
	LPMNMXCORD	pBounds2;
	HPMNMXCORD	ACBounds;
	BOOL	HavePoint = FALSE;  
	double	d, MinDist=DBL_MAX, dist1=0, dist2;
	double	gap;
	short	nc,nl,rc;  
	LPINT	pParts2;
	
	if (nPoly2 > 1 && hPolyPartLen2)
		pParts2 = (LPINT)GlobalLock (hPolyPartLen2);  //    pParts2[1]     pPolyPoints2[35]
	Type1 = GetHighlightType (Type1);
	Type2 = GetHighlightType (Type2);
	if (Type1 == 3)
	{   
		iend1 = nPnts1;
    }
	else
	{
		iend1 = nPnts1-1;    
    }
	if (Type2 == 3)
	{   
		iend2 = nPnts2;
    }
	else
	{
		iend2 = nPnts2-1;    
    }  
    if (Type1 == 5 && Type2 == 5)
    { 
	    double	PCX1,PCY1,RPX1,RPY1,CLEN1, POCX1, POCY1, PTX1, PTY1;  
	    double	PCX2,PCY2,RPX2,RPY2,CLEN2, POCX2, POCY2, PTX2, PTY2;
	      
    	PCX1 = pPolyPoints1[0].x;
    	PCY1 = pPolyPoints1[0].y;
    	POCX1 = pPolyPoints1[1].x;
    	POCY1 = pPolyPoints1[1].y;
    	PTX1 = pPolyPoints1[2].x;
    	PTY1 = pPolyPoints1[2].y;
     	RCURVE(&PCX1,&PCY1,&POCX1,&POCY1,&PTX1,&PTY1,&RPX1,&RPY1,&CLEN1); 
    	PCX2 = pPolyPoints2[0].x;
    	PCY2 = pPolyPoints2[0].y;
    	POCX2 = pPolyPoints2[1].x;
    	POCY2 = pPolyPoints2[1].y;
    	PTX2 = pPolyPoints2[2].x;
    	PTY2 = pPolyPoints2[2].y;
     	RCURVE(&PCX2,&PCY2,&POCX2,&POCY2,&PTX2,&PTY2,&RPX2,&RPY2,&CLEN2); 
		XCC(&PCX1,&PCY1,&RPX1,&RPY1,&CLEN1,
			&PCX2,&PCY2,&RPX2,&RPY2,&CLEN2,
			&XINT[0],&YINT[0],&XINT[1],&YINT[1],&XINT[2],&YINT[2],
			&nc,    &nl,     &gap,     &rc);
		if (rc)
		{
			IntPt.x = XINT[2];
			IntPt.y = YINT[2];
		  	HavePoint = TRUE; 
		  	d = ldistp (IntPt,*pNearPoint);
		  	if (d < MinDist)
		  	{
	  	  		*pIntPoint = IntPt;
		  		MinDist = d;  
		  		if (pD1)
		  		{
	  				*pD1 = DistAlongCurve (&pPolyPoints1[0],&IntPt,&RPX1,&RPY1,&CLEN1);
	  				*pD2 = DistAlongCurve (&pPolyPoints2[0],&IntPt,&RPX2,&RPY2,&CLEN2);
		  		}
		  	}
		}
    	goto Exit;
    }
    if (Type1 == 5)
    {   
    	PCX = pPolyPoints1[0].x;
    	PCY = pPolyPoints1[0].y;
    	POCX = pPolyPoints1[1].x;
    	POCY = pPolyPoints1[1].y;
    	PTX = pPolyPoints1[2].x;
    	PTY = pPolyPoints1[2].y;
     	RCURVE(&PCX,&PCY,&POCX,&POCY,&PTX,&PTY,&RPX,&RPY,&CLEN); 
     	iend1 = 1;
    }
    if (Type2 == 5)
    {
    	PCX = pPolyPoints2[0].x;
    	PCY = pPolyPoints2[0].y;
    	POCX = pPolyPoints2[1].x;
    	POCY = pPolyPoints2[1].y;
    	PTX = pPolyPoints2[2].x;
    	PTY = pPolyPoints2[2].y;
     	RCURVE(&PCX,&PCY,&POCX,&POCY,&PTX,&PTY,&RPX,&RPY,&CLEN); 
     	iend2 = 1;
    } 
	if (pMaskAccelerators)
	{ 
	    numindex = (iend2-1)/32+1;     
		if (!pMaskAccelerators[1])
		{   
			DWORD	j=0;
			
			pMaskAccelerators[1] = GSSiGlobAlloc (1777,GMEM_MOVEABLE,(iend2+numindex)*sizeof(MNMXCORD));
			ACBounds = (HPMNMXCORD)GlobalLock (pMaskAccelerators[1]);
			lpPoint = lpAreaPoints;
			dist2 = 0;
			for (i = 0; i <iend2; i++,lpPoint++)  
			{
				if (i == iend2-1)
					EndAreaPoint = BeginAreaPoint;
				else
					EndAreaPoint = *(lpPoint + 1);
				ACBounds[i].xmn = min (EndAreaPoint.x,lpPoint->x) - P_TOL;
				ACBounds[i].xmx = max (EndAreaPoint.x,lpPoint->x) + P_TOL;
				ACBounds[i].ymn = min (EndAreaPoint.y,lpPoint->y) - P_TOL;
				ACBounds[i].ymx = max (EndAreaPoint.y,lpPoint->y) + P_TOL;
			}
			while (j < iend2)
			{   
				UINT	k;
				
				ACBounds[i] = ACBounds[j++];
				for (k=0;k<31;k++)
				{
					if (j >= iend2)
						break;
					AddMinMaxD (&ACBounds[i],&ACBounds[j++]);
				}
				i++;
			}
		}
		else
			ACBounds = (HPMNMXCORD)GlobalLock (pMaskAccelerators[1]);
	} 
	else
		numindex = 1;
    
    BeginPoint = *pPolyPoints1;
    for (iline = 0; iline < iend1; iline++)
    { 
      if (iline == nPnts1-1)
      	EndPoint = BeginPoint;
      else
      	EndPoint = pPolyPoints1[iline+1]; 
      if (ldistpp (&pPolyPoints1[iline],&EndPoint) < P_TOL)//pPolyPoints1[0] pPolyPoints1[1] pPolyPoints1[2] pPolyPoints2[0] pPolyPoints2[1]
      	goto S200;  
      if (pPolyAZ)
      	A2 = pPolyAZ[iline];
      else
      	A2 = getazd (&pPolyPoints1[iline],&EndPoint);
      DBoundsInit (&Bounds1);
	  AddDPointToMinMax (&EndPoint,&Bounds1);
	  AddDPointToMinMax (&pPolyPoints1[iline],&Bounds1);
	  ExpandBounds (&Bounds1,P_TOL);

      lpPoint = lpAreaPoints;
      dist2 = 0;  
      indexbegin = 0;
      indexend   = NUMININDEX;
      if (!pMaskAccelerators)
      	indexend = iend2; 
      for (index = 0,j=iend2;index < numindex;index++,j++)
      {
	      indexend = min (iend2,indexend);
		  if (!pMaskAccelerators || BoundsInBounds (&Bounds1,&ACBounds[j],1))
		      for (i = indexbegin; i <indexend; i++)
		      {
			      if (i == nPnts2-1)
			      	EndAreaPoint = BeginAreaPoint;
			      else
			      	EndAreaPoint = lpPoint[i+1];    //lpPoint[i]     lpPoint[178]
			      if (nPoly2>1)
			      { 
			      	int	ipoly;
			      	long	loc=0;
			      	 
			      	if (i == nPnts2-2)
			      		goto S100;
			      	for (ipoly = 0;ipoly < nPoly2;ipoly++)
			      	{   
			      		loc += pParts2[ipoly];  //pParts2[1]
			      		if (i+1 == loc)
			      			goto S100;
			      		if (ipoly)
			      			loc++;
			      		if (i+1 == loc)
			      			goto S100;
			      	}
			      }
			      if (Type1 == 5 || Type2 == 5)
			      {     
			      		double	x1,y1,x2,y2;
			      		
			      		if (Type1 == 5)
			      		{
			      			x1 = lpPoint[i].x;
			      			y1 = lpPoint[i].y;
			      			x2 = EndAreaPoint.x;
			      			y2 = EndAreaPoint.y;
			      		}
			      		else
			      		{
			      			x1 = pPolyPoints1[iline].x;
			      			y1 = pPolyPoints1[iline].y;
			      			x2 = EndPoint.x;
			      			y2 = EndPoint.y;
			      		}
			      		
						XLC(&PCX,&PCY,&RPX,&RPY,&CLEN,&x1,&y1,&x2,&y2,  
							&XINT[0],&YINT[0],&XINT[1],&YINT[1],&XINT[2],&YINT[2],
							&nc,    &nl,     &gap,     &rc);
						  	if (!pIntPoint) 
						  	{
								switch (rc)
								{
				               		case 0:			//  NO INTERSECTION
				               		case -1:		//	1 TANGENT PT,ON CURVE ONLY 
				               		case -2:        //	2 INTERSECTION PTS,ON CURVE ONLY    
									case -4:		//  1ST PT ON CURVE ONLY
									case 6:			//  BOTH PTS ON EXT. OF BOTH
									case 7:			//  BOTH PTS ON EXT. OF CURVE,ONE OF THEM ON LN
									case 8:			//  BOTH PTS ON LINE,BOTH PTS ON EXT OF CURVE  
				               			goto S100;
				               		case 1:			//	1 TANGENT PT,ON LINE & CURVE
									case 5:			//  IN CONTINUITY   
				               		case 2: 		//	2 INTERSECTION PTS,ON LINE & CURVE
				               		case 3:   		//	1ST PT ON BOTH,2ND PT ON CURVE ONLY 
				               		case -3:		//	2ND PT ON BOTH,1ST PT ON CURVE ONLY  
				               		case 4:       	//	1ST PT ON BOTH,2ND PT ON EXT. OF BOTH  
				               		break;
								} 
								HavePoint = TRUE;
								goto Exit;
						}
						switch (rc)
						{
		               		case 0:			//   NO INTERSECTION
		               			goto S100;
		               		case 1:			//	1 TANGENT PT,ON LINE & CURVE
		               		case -1:		//	1 TANGENT PT,ON CURVE ONLY 
							case 5:			//  IN CONTINUITY 
		               		case 2: 		//	2 INTERSECTION PTS,ON LINE & CURVE
		               		case -2:        //	2 INTERSECTION PTS,ON CURVE ONLY    
		               		case 3:   		//	1ST PT ON BOTH,2ND PT ON CURVE ONLY 
		               		case -3:		//	2ND PT ON BOTH,1ST PT ON CURVE ONLY  
		               		case 4:       	//	1ST PT ON BOTH,2ND PT ON EXT. OF BOTH 
							case -4:		//  1ST PT ON CURVE ONLY
							case 6:			//  BOTH PTS ON EXT. OF BOTH
							case 7:			//  BOTH PTS ON EXT. OF CURVE,ONE OF THEM ON LN
							case 8:			//  BOTH PTS ON LINE,BOTH PTS ON EXT OF CURVE
								IntPt.x = XINT[2];
								IntPt.y = YINT[2];
								break;  
		/*					{
								double	d1,d2;
								  
							  	IntPoint[0].x = XINT[0];
							  	IntPoint[0].y = YINT[0];
							  	IntPoint[1].x = XINT[1];
							  	IntPoint[1].y = YINT[1];
							  	d1 = ldistp (IntPoint[0],*pNearPoint); 
							  	d2 = ldistp (IntPoint[1],*pNearPoint);
							  	if (d1 < d2)
							  		IntPt = IntPoint[0];
							  	else
							  		IntPt = IntPoint[1]; 
							}
								break;*/
						}
					  	HavePoint = TRUE; 
					  	d = ldistp (IntPt,*pNearPoint);
					  	if (d < MinDist)
					  	{
		          	  		*pIntPoint = IntPt;
					  		MinDist = d;  
					  		if (pD1)
					  		{   
					  			if (Type1 == 5) 
					  				*pD1 = DistAlongCurve (&pPolyPoints1[iline],&IntPt,&RPX,&RPY,&CLEN);
					  			else
					  				*pD1 = dist1 + ldistpp (&pPolyPoints1[iline],&IntPt);
					  			if (Type2 == 5) 
					  				*pD2 = DistAlongCurve (&lpPoint[i],&IntPt,&RPX,&RPY,&CLEN);
					  			else
					  				*pD2 = dist2 + ldistpp (&lpPoint[i],&IntPt);
					  		}
					  	}
			      }
			      else
			      {   	
/*			          POINT	ptemp[2];
			          
			          ptemp[0] = DPointToPoint (lpPoint[i]);   
			          ptemp[1] = DPointToPoint (EndAreaPoint);
					  SetDisplayMode (CurView->hDC, GF_TEXTMODE);
					  SelectClipRgn (CurView->hDC,0);  
					  SelectObject (CurView->hDC,GetStockObject(BLACK_PEN));
			          Polyline (CurView->hDC,ptemp,2);  */
		//		      DBoundsInit (&Bounds2);
		//			  AddDPointToMinMax (&EndAreaPoint,&Bounds2);
		//			  AddDPointToMinMax (lpPoint,&Bounds2); 
		//			  ExpandBounds (&Bounds2,P_TOL); 
					  if (!pMaskAccelerators)
					  {
						  Bounds2.xmn = min (EndAreaPoint.x,lpPoint[i].x) - P_TOL;
						  Bounds2.xmx = max (EndAreaPoint.x,lpPoint[i].x) + P_TOL;
						  Bounds2.ymn = min (EndAreaPoint.y,lpPoint[i].y) - P_TOL;
						  Bounds2.ymx = max (EndAreaPoint.y,lpPoint[i].y) + P_TOL;
						  pBounds2 = &Bounds2;
					  }
					  else
					  	pBounds2 = &ACBounds[i];
					  if (BoundsInBounds (&Bounds1,pBounds2,1))
					  {
				          A1 = getazd (&lpPoint[i],&EndAreaPoint);
				          IRC = LIN_SEC (lpPoint[i].x,lpPoint[i].y,A1,
				          				 pPolyPoints1[iline].x,pPolyPoints1[iline].y, A2,
				          				 &IntPt.x, &IntPt.y);
			/*C******* IGNORE PARALLEL LINES */
			          	  if (IRC != 0)
			          	  	goto S100; 
						  if (DPointInBounds (&IntPt,&Bounds1) && DPointInBounds (&IntPt,pBounds2)) 
						  {
						  	HavePoint = TRUE;
						  	if (!pIntPoint)
						  		goto Exit;
						  	d = ldistp (IntPt,*pNearPoint);
						  	if (d < MinDist)
						  	{
			          	  		*pIntPoint = IntPt;
						  		MinDist = d;  
						  		if (pD1)
						  		{
						  			*pD1 = dist1 + ldistpp (&pPolyPoints1[iline],&IntPt);
						  			*pD2 = dist2 + ldistpp (&lpPoint[i],&IntPt);
						  		}
						  	}
						  }
					  }	
				  }
		S100:     if (pD2)
					dist2 += ldistpp (&EndAreaPoint,&lpPoint[i]);
		      }
	      indexbegin += NUMININDEX;
	      indexend   += NUMININDEX; 
	  }
S200:   
		if (pD1)
			dist1 += ldistpp (&EndPoint,&pPolyPoints1[iline]);
	}
Exit: 
	if (nPoly2 > 1)
		GlobalUnlock (hPolyPartLen2);
	if (pMaskAccelerators) 
		GlobalUnlock (pMaskAccelerators[1]);
{
#if ENABLETRACE
GSSiExitProg (1082);
#endif
	return HavePoint;
}
#if ENABLETRACE
}
#endif
} 

short IntersectPolys2 (long nPnts1,HPDPOINT pPolyPoints1,
					  long nPnts2,HPDPOINT pPolyPoints2,
					  double FromDist1,LPDOUBLE pIntDist,LPDPOINT pIntPoint,
					  LPDOUBLE	pInAZ, LPDOUBLE OutAZ,LPBOOL OutReverseDir,LPSHORT WhichPoly,BOOL GetNextInt)
#if ENABLETRACE
{GSSiEnterProg (1083);
#endif
{
    double   A1, A2, XP, YP, XMID1, XMID2, YMID1, YMID2, AMID,
             PAX1, PAY1, PAX2, PAY2, MNX, MNY, MXY,
             XMIDMN, XMIDMX, YMIDMN, YMIDMX, XMID2_NEED, YMID2_NEED;
    HPDPOINT	lpPoint;
    DPOINT	BeginPoint, BeginPoint2, IntPt;
    long i, iline, ibeg=0, IRC, NumLines=nPnts1-1 ;
	MNMXCORD Bounds1, Bounds2;
	BOOL	HavePoint = FALSE;  
	double	d, MinDist=DBL_MAX, d1=0, d2=0,seglen;
	short	nOutAZ=0;
	
	pIntDist[0] = DBL_MAX;
	BeginPoint = *pPolyPoints1++;
    for (iline = 0; iline < NumLines; iline++)
    { 
      seglen = ldistp (BeginPoint,*pPolyPoints1);	
      if (d1 + seglen <= FromDist1)
      	goto S200;
      if (LDIST (BeginPoint.x,BeginPoint.y,pPolyPoints1->x,pPolyPoints1->y) < P_TOL)
      	goto S200; 
      A2 = getazd (&BeginPoint,pPolyPoints1);
      DBoundsInit (&Bounds1);
	  AddDPointToMinMax (&BeginPoint,&Bounds1);
	  AddDPointToMinMax (pPolyPoints1,&Bounds1);
	  ExpandBounds (&Bounds1,P_TOL);

      lpPoint = pPolyPoints2;
      BeginPoint2 = *lpPoint++;
      d2 = 0;
      for (i = 0; i <nPnts2-1; i++)
      {
	      DBoundsInit (&Bounds2);
		  AddDPointToMinMax (&BeginPoint2,&Bounds2);
		  AddDPointToMinMax (&lpPoint[i],&Bounds2); 
		  ExpandBounds (&Bounds2,P_TOL);
		  if (BoundsInBounds (&Bounds1,&Bounds2,1))
		  {
	          A1 = getazd (&BeginPoint2,&lpPoint[i]);
	          IRC = LIN_SEC (BeginPoint2.x,BeginPoint2.y,A1,
	          				 BeginPoint.x,BeginPoint.y, A2,
	          				 &IntPt.x, &IntPt.y);
			  if (IRC == 2)
			  { 
			  	IRC = 0;
			  	if (ldistp (BeginPoint,BeginPoint2) < P_TOL)
			  		IntPt = BeginPoint;
			  	else if (ldistp (BeginPoint,lpPoint[i]) < P_TOL)
			  		IntPt = BeginPoint;	
			  	else if (ldistp (*pPolyPoints1,BeginPoint2) < P_TOL)
			  		IntPt = *pPolyPoints1;
			  	else if (ldistp (*pPolyPoints1,lpPoint[i]) < P_TOL)
			  		IntPt = *pPolyPoints1;	
			  	else
			  		IRC = 2;
			  }
			  		
          	  if (IRC != 0)  goto S100; 
			  if (DPointInBounds (&IntPt,&Bounds1) && DPointInBounds (&IntPt,&Bounds2)) 
			  {
			  	HavePoint = TRUE;
			  	d = ldistp (IntPt,BeginPoint);
			  	if (d + d1 >FromDist1+P_TOL && d + d1 + P_TOL <= pIntDist[0])
			  	{   
			  		*pInAZ = getazd (&BeginPoint,&IntPt);
			  		nOutAZ = 0;
          	  		*pIntPoint = IntPt;
			  		pIntDist[0] = d + d1;
			  		pIntDist[1] = ldistp (IntPt,BeginPoint2) + d2;
			  		if (ldistp (IntPt,BeginPoint2) > P_TOL)
			  		{
			  			OutAZ[nOutAZ] = getazd (&IntPt,&BeginPoint2);
			  			WhichPoly[nOutAZ] = 1;
			  			OutReverseDir[nOutAZ++] = TRUE;   
			  		}
			  		if (ldistp (IntPt,lpPoint[i]) > P_TOL)
			  		{
			  			OutAZ[nOutAZ] = getazd (&IntPt,&lpPoint[i]);
			  			WhichPoly[nOutAZ] = 1;
			  			OutReverseDir[nOutAZ++] = FALSE;
			  		} 
			  		else if (i+1 <nPnts2-1)
			  		{
			  			OutAZ[nOutAZ] = getazd (&IntPt,(HPDPOINT)&lpPoint[i+1]);
			  			WhichPoly[nOutAZ] = 1;
			  			OutReverseDir[nOutAZ++] = FALSE;
			  		}
			  		if (ldistp (IntPt,*pPolyPoints1) > P_TOL)
			  		{
			  			OutAZ[nOutAZ] = A2;
			  			WhichPoly[nOutAZ] = 0;
			  			OutReverseDir[nOutAZ++] = FALSE;
			  		}
			  		else if (iline+1 < NumLines)
			  		{
			  			OutAZ[nOutAZ] = getazd (pPolyPoints1,(HPDPOINT)(pPolyPoints1+1));
			  			WhichPoly[nOutAZ] = 0;
			  			OutReverseDir[nOutAZ++] = FALSE;
			  		}
			  	}
			  }	
		  }
S100:     d2 += ldistp (BeginPoint2,lpPoint[i]);
		  BeginPoint2 = lpPoint[i];
		  if (nOutAZ && !GetNextInt)
{
#if ENABLETRACE
GSSiExitProg (1083);
#endif
		  	return nOutAZ;
}
      } 
S200:   
		d1 += ldistp (BeginPoint,*pPolyPoints1);
      	BeginPoint = *pPolyPoints1++; 
	} 
{
#if ENABLETRACE
GSSiExitProg (1083);
#endif
	return nOutAZ;
}
#if ENABLETRACE
}
#endif
} 

void SnapPolyPoints (long nPnts,HPDPOINT pPoints, double SnapTol)
#if ENABLETRACE
{GSSiEnterProg (1084);
#endif
{   
	DWORD	i,j;
	
	if (!SnapTol)
		return;
	for (i=1;i<nPnts;i++)
		for (j=0;j<i;j++)
		if (ldistp (pPoints[i],pPoints[j]) <= SnapTol)
		{
			pPoints[i] = pPoints[j];
			break;
		}
{
#if ENABLETRACE
GSSiExitProg (1084);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL RemoveDupPolyPoints (LPLONG pnPnts,HPDPOINT pPoints, double Tol)
#if ENABLETRACE
{GSSiEnterProg (1085);
#endif
{   
	DWORD	i,j;
	long	NewnPnts=0,nRemoved=0;
	
	for (i=1;i<*pnPnts;i++)
		if (ldistp (pPoints[i],pPoints[NewnPnts]) > Tol) //pPoints[4]
		{   
			NewnPnts++;
			pPoints[NewnPnts] = pPoints[i];
		}
		else
			nRemoved++;
	NewnPnts++; 
	if (*pnPnts == NewnPnts)
{
#if ENABLETRACE
GSSiExitProg (1085);
#endif
		return FALSE;
}
	*pnPnts = NewnPnts;
{
#if ENABLETRACE
GSSiExitProg (1085);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

short PolyCrossesItself (int Type,short wantcross,long nPnts,HPDPOINT pPoints,LPDPOINT pIntPoint)
#if ENABLETRACE
{GSSiEnterProg (1086);
#endif
{   
	// returns
	//	0 - no cross
	//	1 - line intersection found
	//	2 - line on top of another but not equeal
	//	3 - dup lines (probably link line)
	//	4 - dup points in line (in sequence)
    double   A1, A2, XP, YP, XMID1, XMID2, YMID1, YMID2, AMID,
             PAX1, PAY1, PAX2, PAY2, MNX, MNY, MXY,
             XMIDMN, XMIDMX, YMIDMN, YMIDMX, XMID2_NEED, YMID2_NEED;
    HPDPOINT	pPolyPoints2, lpPoint, pPointNew;
    DPOINT	BeginPoint, BeginAreaPoint, IntPt;
    long i, j, iline, ibeg=0, IRC ;
	MNMXCORD Bounds1, Bounds2;
	double	d, MinDist=DBL_MAX;  
	short	nCross=0;   
	HANDLE	hTemp=0;
	
    for (i = 0; i < nPnts; i++) //pPoints[7]
    {
		for (j = 0; j < nPnts; j++)
		{
			if (i != j && SameDPoint (&pPoints[i], &pPoints[j]))
			{
				*pIntPoint = pPoints[i];
				GSSiGlobUlFree (&hTemp);
{
#if ENABLETRACE
GSSiExitProg (1086);
#endif
	  			return 4;
}
			}
		}
	}
	SnapPolyPoints (nPnts,pPoints,GetGlobalDVal2 ("[%SNAPPOLYTOL]",0));
	if (Type == GF_AREA && ldistp (pPoints[0],pPoints[nPnts-1]) > P_TOL)
	{
		hTemp = GSSiGlobAlloc (1055,GMEM_MOVEABLE,(nPnts+1)*sizeof(DPOINT));
		pPointNew = (HPDPOINT)GlobalLock (hTemp);
		hmemmove ((HPSTR)pPointNew,(HPSTR)pPoints,nPnts*sizeof(DPOINT));
		pPointNew[nPnts++] = *pPoints;
		pPoints = pPointNew;
	}
    for (iline = 0; iline < nPnts-2; iline++)
    {
		if (LDIST (pPoints[iline].x,pPoints[iline].y,pPoints[iline+1].x,pPoints[iline+1].y) < P_TOL)
			goto S200; 
		A2 = getazd (&pPoints[iline],&pPoints[iline+1]);
		DBoundsInit (&Bounds1);
		AddDPointToMinMax (&pPoints[iline],&Bounds1);
		AddDPointToMinMax (&pPoints[iline+1],&Bounds1);
		ExpandBounds (&Bounds1,P_TOL);

		for (i = iline+1; i <nPnts-1; i++)
		{
	      DBoundsInit (&Bounds2);
		  AddDPointToMinMax (&pPoints[i],&Bounds2);
		  AddDPointToMinMax (&pPoints[i+1],&Bounds2);
		  ExpandBounds (&Bounds2,P_TOL);
		  if (BoundsInBounds (&Bounds1,&Bounds2,1))
		  {
				if (LinesEqual (pPoints,iline,iline+1,i,i+1)) //pPoints[37]
				{
			  		nCross++;
          			*pIntPoint = MidPointD (pPoints[i],pPoints[i+1]);
			  		if (nCross >= wantcross) 
			  		{
			  			GSSiGlobUlFree (&hTemp);
{
#if ENABLETRACE
GSSiExitProg (1086);
#endif
			  			return 3;               
}
			  		}
			 	}
			  	else
			  	{
		        	A1 = getazd (&pPoints[i],&pPoints[i+1]);
		        	IRC = LIN_SEC (pPoints[i].x,pPoints[i].y,A1,
		         				   pPoints[iline].x,pPoints[iline].y,A2,
		          				   &IntPt.x, &IntPt.y);

	          		if (IRC ==  2)
	          		{
						MNMXCORD Bounds1Tol=Bounds1, Bounds2Tol = Bounds2;

						ExpandBounds (&Bounds1Tol, -P_TOL*2);
						ExpandBounds (&Bounds2Tol, -P_TOL*2);
						if (IntersectBounds (&Bounds1Tol,&Bounds2Tol,0))
						{
							if (getazd (&pPoints[iline],&pPoints[i+1]) == A1)
							{
/*				 		if ( ((i == iline + 1) && !DPointInBounds (&pPoints[i+1],&Bounds1) && !DPointInBounds (&pPoints[iline],&Bounds2)  ||
				 			 ((i== nPnts -2) && !iline) && !DPointInBounds (&pPoints[i],&Bounds1) && !DPointInBounds (&pPoints[iline+1],&Bounds2)) )
				 			 ;
				 		else
				 		{*/
		          				nCross++; 
		          				*pIntPoint = MidPointD (pPoints[i],pPoints[i+1]);
					  			if (nCross >= wantcross)
					  			{
					  				GSSiGlobUlFree (&hTemp);
{
#if ENABLETRACE
GSSiExitProg (1086);
#endif
						  			return 2;
}
						  		}
							}
					  	}
				  	} 
				  	else if (!IRC)
					 	if (DPointInBounds (&IntPt,&Bounds1) && DPointInBounds (&IntPt,&Bounds2)) 
					 	{   
					 		if (!( (ldistp (IntPt,pPoints[i]) < P_TOL || ldistp (IntPt,pPoints[i+1]) < P_TOL) &&
					 			   (ldistp (IntPt,pPoints[iline]) < P_TOL || ldistp (IntPt,pPoints[iline+1]) < P_TOL)))
					 		{   
					 			BOOL	IntIsOK=FALSE;
					 			
					 			if (hPolyPartLen)
					 			{
					 				LPWORD	pPartLen = (LPWORD)GlobalLock (hPolyPartLen); 
					 				long	PointID=-1;
					 				short	ip;
					 				
					 				for (ip = 0; ip < nPoly; ip++,pPartLen++)  
					 				{   
					 					PointID += *pPartLen;
					 					if (ip>1)
					 						PointID++;
					 					if (PointID == i || PointID == iline)
					 						IntIsOK = TRUE;
					 				}
									GlobalUnlock (hPolyPartLen);
                                } 
                                if (!IntIsOK)
                                {
				          			nCross++; 
							  		if (nCross >= wantcross)  
							  		{
			          	  				*pIntPoint = IntPt;
								  		GSSiGlobUlFree (&hTemp);
{
#if ENABLETRACE
GSSiExitProg (1086);
#endif
						  				return 1; 
}
						  			}
						  		}
					 	 	}
						}
				}
				
		  }
      } 
S200:;
	} 
	GSSiGlobUlFree (&hTemp);
{
#if ENABLETRACE
GSSiExitProg (1086);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
} 



  
BOOL UpdateDateRange (HPSTR buf, long len, LPLONG MinTime, LPLONG MaxTime)
#if ENABLETRACE
{GSSiEnterProg (1088);
#endif
{   
	HPSHORT	ipnt;
	long	Offset;
	short	i, SymNum=0;
	LPBYTE	Pcode; 
	HPSTR	EndLoc;
	LPSHORT	pItemLen; 

    EndLoc = buf+len;
    ipnt = (HPSHORT)EndLoc;
    *ipnt = 0;
    ipnt = (HPSHORT)buf;  
    Pcode = (LPBYTE)ipnt;
    if (*Pcode != 12)
{
#if ENABLETRACE
GSSiExitProg (1088);
#endif
    	return FALSE; 
}
	CurElementType = 0;
    while (*ipnt != 0)
    {   
    	Pcode = (LPBYTE)ipnt;
    	ipnt++;

	    switch (*Pcode)
        {   
            case 8:	/*	description */
	 		{   
	 		    ConvertSymbol (ipnt); 
	 		    CreateFileSymList (ipnt);
	 			SymNum = *ipnt;
	 			ipnt++;
	 		}
	 		break;
	 		
		    case 9:	/*	refno	*/
		    {   
		    	short ii, ltag;   
		    	static	long	debugrefno=1591435;
	           	LPLONG	pRefno =(LPLONG) ipnt;

	           	if (*pRefno == debugrefno)
	           		ii=1;  
	           	if (ReReference)
	           		*pRefno = GetNewRefno(PltName,0,0,0,0); 
	           	CurrentRefno = LastRef = *pRefno; 
	        	ipnt += 2; 
	        	ltag = *++Pcode;
	        	if (ltag) 
	        	{   LPSTR	lpTAG;
	                	
	        		lpTAG = (LPSTR)ipnt;
					ipnt = (LPSHORT) (lpTAG + ltag + ltag%2);
				}  
		    }
		    break;
	
			case 37: 
			{
				long	StartTime, EndTime;
					
				StartTime = *(LPLONG)ipnt;
				ipnt+=2;
				EndTime = *(LPLONG)ipnt;
				ipnt+=2; 
				*MinTime = min (StartTime,*MinTime);
				*MaxTime = max (EndTime,*MaxTime);
			}
			break;
            
            default: 
				SkipSubRec (Pcode,&ipnt,0);
            break;

        } 
    } 
    if (hSetNewSymType)
    {   
    	LPSHORT	pNum=(LPSHORT)GlobalLock (hSetNewSymType);
    	
    	while (*pNum)
    	{    
    		if (*pNum == SymNum)
    		{
		    	switch (CurElementType)
		    	{
		    		case GF_POINT: 
		    			pNum++;
		    			(*pNum)++;
		    		break;
		    		
		    		case GF_LINE:
		    		case GF_POLYLINE:
		    		case GF_CURVE:
		    			pNum += 2;
		    			(*pNum)++;
		    		break;
		    		
		    		case GF_AREA:
		    			pNum += 3;
		    			(*pNum)++;
		    		break;
		    	}  
		    	break;
		    }         
		    pNum += 4;
	    }
    	GlobalUnlock (hSetNewSymType);
    }
{
#if ENABLETRACE
GSSiExitProg (1088);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

DPOINT dnewptproj (DPOINT OldPoint, double AZM, double DIS)
{   
	DPOINT	Pt;
	
	if (!DIS)
		return OldPoint;
	if (PRJ_UNITS[1] == 4)
		Pt = NewLatLong(OldPoint.y,OldPoint.x,DIS,AZM); 
	else
		Pt = dnewpt (OldPoint,AZM,DIS);
	return Pt;
}  

BOOL CreateAreaGridLatLon (LPSTR File,LPSTR SymName,LPSTR PrefixIN,double Width,double Height,double Overlap,double XAdjust,double YAdjust,BOOL UseStatusWnd)
{
	HIGHLIGHTDATA	HighlightData; 
	DPOINT			MidPoint, Pt;    
	double			HeightInDeg, WidthInDeg, OverlapInDegX, OverlapInDegY,XAdjustInDeg,YAdjustInDeg;
	MNMXCORD		Area;
	long	Refno, nAreaPnts, TotAreas, nLoaded=0, MaxAreas, MemSize; 
	HPDPOINT pAreaPoints;  
	HANDLE	hAreaPoints;
	long	nRows,nCols, nAreas=0, iRow, iCol, iArea=0, i,j;  
	BOOL	InArea; 
	DPOINT	Point, IntPoint;
	short	AreaSymbol;
	HANDLE	hSymDesc=0;
	short	NumSyms=0;    
	long	NewRefno=1,nRecs; 
	char	Prefix[10], UDI[64]=""; 
	HANDLE	hAccel=0;  
	double	NorthAZ=PY/2, EastAZ=0; 
	MNMXCORD	Bounds;
	HPDPOINT	pPolyPoints;
	HANDLE	hPoly=0; 
	double	D[4]; 
	HANDLE	hP;
	HPDPOINT pPoints; 
	long	npnts;  
	int		np=4;    
	short	ii;
    
    if (PRJ_UNITS[1] == 4)
	{
		NorthAZ = 0;
		EastAZ = PY/2;
	}
    Width *= (5280 * FTM);        	
    Height *= (5280 * FTM);        	
    Overlap *= (5280 * FTM);   
	XAdjust *= (5280 * FTM);
	YAdjust *= (5280 * FTM);
	AreaSymbol=GetDictSymbolNumber (SymName);
	if (BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData))
		return FALSE;  
	hP = GSSiGlobAlloc (1138,GMEM_MOVEABLE,sizeof(DPOINT)*4);
	pPoints  = (HPDPOINT)GlobalLock (hP);
	PickList[0] = HighlightData.PD; 
    hAreaPoints = GSSiGlobAlloc (1136,GMEM_MOVEABLE,(long)PickList[0].NumPoints*sizeof(DPOINT)); 
    pAreaPoints = (HPDPOINT)GlobalLock (hAreaPoints);
	nAreaPnts = GetPickItemPoints (0, FALSE, &pAreaPoints); 
	GlobalUnlock (hAreaPoints);
    pAreaPoints = (HPDPOINT)GlobalLock (hAreaPoints);   
	nAreas = 0; 
	Bounds = HighlightData.PD.Rect;
	MidPoint.x = HighlightData.PD.Rect.xmn;
	MidPoint.y = HighlightData.PD.Rect.ymn;  
	nRecs = 1000 * (HighlightData.PD.Rect.ymx - HighlightData.PD.Rect.ymn);
	Pt = dnewptproj (MidPoint,NorthAZ,Height);  
	HeightInDeg = Pt.y - MidPoint.y;
	Pt = dnewptproj (MidPoint,NorthAZ,YAdjust);  
	YAdjustInDeg = Pt.y - MidPoint.y;
	Pt = dnewptproj (MidPoint,NorthAZ,Overlap);  
	OverlapInDegY = Pt.y - MidPoint.y;
	Pt = dnewptproj (MidPoint,EastAZ,Width);  
	WidthInDeg = Pt.x - MidPoint.x; 
	GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts,&hPoly);
	pPolyPoints = (HPDPOINT)GlobalLock (hPoly); 
	InflateBounds (&Bounds, max (HeightInDeg*2,WidthInDeg*2));
	strcpy (PltName,File); 
	strcpy (EditName,File);
	CreateNewMap (PltName,&Bounds,0,0,0,0,0,0,FALSE);
	EditBounds = CurView->FileMNMX;
	MidPoint.y += YAdjustInDeg;
	if (UseStatusWnd)
		CreateStatusWind (hWndMain,1,"Create Area Grid");
	
	do
	{
		Pt = dnewptproj (MidPoint,EastAZ,Width);  
		WidthInDeg = Pt.x - MidPoint.x; 
		Pt = dnewptproj (MidPoint,EastAZ,Overlap);  
		OverlapInDegX = Pt.x - MidPoint.x; 
		Pt = dnewptproj (MidPoint,EastAZ,XAdjust);  
		XAdjustInDeg = Pt.x - MidPoint.x; 
		MidPoint.x = HighlightData.PD.Rect.xmn + WidthInDeg + XAdjustInDeg;
		do
		{
			InArea = FALSE;
			DBoundsInit (&Area); 
			Point.x = MidPoint.x - WidthInDeg - OverlapInDegX;
			Point.y = MidPoint.y - HeightInDeg - OverlapInDegY;
			if (POINT_IN_AREAD (Point, nAreaPnts,pAreaPoints,1,0,0,&hAccel))
				InArea = TRUE;  
			AddDPointToMinMax (&Point,&Area);
			Point.y = MidPoint.y + HeightInDeg + OverlapInDegY;
			if (POINT_IN_AREAD (Point, nAreaPnts,pAreaPoints,1,0,0,&hAccel))
				InArea = TRUE;  
			AddDPointToMinMax (&Point,&Area);
			Point.x = MidPoint.x + WidthInDeg + OverlapInDegX;
			if (POINT_IN_AREAD (Point, nAreaPnts,pAreaPoints,1,0,0,&hAccel))
				InArea = TRUE;  
			AddDPointToMinMax (&Point,&Area);
			Point.y = MidPoint.y - HeightInDeg - OverlapInDegY;
			if (POINT_IN_AREAD (Point, nAreaPnts,pAreaPoints,1,0,0,&hAccel))
				InArea = TRUE;  
			AddDPointToMinMax (&Point,&Area); 
			if (POINT_IN_AREAD (MinMaxMidPointD (&Area),nAreaPnts,pAreaPoints,1,0,0,&hAccel))
				InArea = TRUE; 
			BoundsToPoints (&Area,pPoints,0);
			if (!InArea)
			{   
				InArea = IntersectPolys1 ( HighlightData.PD.Type,GF_AREA,npnts,pPolyPoints,0,4,pPoints,0,0,
											&HighlightData.PD.PickedPoint,&IntPoint,&D[3],&D[0],0);
			}
			if (InArea) 
			{
				ltoa (1000+iArea+2,UDI,10);
				NewRefno = GetNewRefno (EditName,0,0,0,0);
				if (*PrefixIN)
				{
					strcpy (Prefix,PrefixIN);
					itoa (nAreas+1,UDI,10);
				}
				if (AddPolyToMap (1,&np, &hP,0,NewRefno++,0,2,AreaSymbol,0,Prefix,UDI,-1,-1,-1,0,0,0,0,TRUE,0))
					nAreas++;   
				else
					ii = 1;
				iArea++;
			}
			MidPoint.x += WidthInDeg * 2;     
		}
		while (MidPoint.x - WidthInDeg < HighlightData.PD.Rect.xmx);
		MidPoint.y += HeightInDeg * 2;
		if (UseStatusWnd)
			StatusWindowUpdate (0,0, nRecs, (long)(1000*(MidPoint.y-HighlightData.PD.Rect.ymn)));
	}                                     
	while (MidPoint.y - HeightInDeg < HighlightData.PD.Rect.ymx);
	GSSiGlobUlFree (&hP); 
	GSSiGlobFree (&hAccel);
	GSSiGlobUlFree (&hPoly);
	AddToSymList (AreaSymbol,&NumSyms,&hSymDesc); 
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,0); 
	DestroySymList (&NumSyms,&hSymDesc); 
	if (UseStatusWnd)
		DestroyStatusWindow(0);  

	GSSiGlobUlFree (&hAreaPoints);  
	return TRUE;
}

BOOL SaveContourElevPoints (HDC hDC,LPSTR SymName,HPFPOINT lpPoints,int nPnts)
{
	double LineLen = GetPolyLengthD (lpPoints,nPnts);
	double Spacing = (CurView->ScreenRect.right - CurView->ScreenRect.left)/2;
	int	nElev = max (1,IDNINT (LineLen / Spacing));
	int	Elev = atoi (&SymName[3]);
	static int	CharWidth = 12, CharHeight=18;
	double	DisBetweenPt = LineLen / (nElev + 1);
	double	dist=Spacing/3 + (Elev%3)*Spacing/3, d[2], dmid, az[3], azmid;
	int	inc[]={0,1,-1,2,-2,3,-3,4,-4,5,-5,6,-6,7,-7,8,-8,9,-9,10,-10,11,-11,12,-12,13,-13};
	int	ninc = sizeof(inc)/sizeof(int);
	DPOINT	p[3], pmid;
	int	i,j,k,n,start,ii;
	static	double	azdif=0.25;
	char	celev[8];
	BOOL	HaveFlat;
	static	int	dbelev=50;
	int		jstart;
	BOOL UNIFIED = TRUE;

	if (Elev == dbelev)
		ii=1;
	for (i=0;i<nElev;i++)
	{
		HaveFlat = FALSE;
		dist += DisBetweenPt;
		jstart = 3 * (Elev % 3);
		for (j=jstart;j<ninc;j++)
		{
			dmid = dist + inc[j] * CharWidth/3;
			if (dmid > 0 && dmid < LineLen)
			{
				p[1] = PointAtDistOnPoly (lpPoints,nPnts,dmid,&az[1],0);
				d[0] = dmid - CharWidth/2;
				p[0] = PointAtDistOnPoly (lpPoints,nPnts,d[0],&az[0],0);
				d[1] = dmid + CharWidth/2;
				p[2] = PointAtDistOnPoly (lpPoints,nPnts,d[1],&az[2],0);
				if (fabs(DeltaAZ (az[1],az[0])) < azdif && fabs(DeltaAZ (az[1],az[2])) < azdif)
				{
					HaveFlat=TRUE;
					dist = dmid;
					break;
				}
			}
		}

		if (!HaveFlat)
			continue;
		itoa (Elev,celev,10);
		n = strlen (celev);
		if (UNIFIED)
		{
			start = 1;
			n = 1;
		}
		else if (n < 3)
			start = 1;
		else
			start = 0;
		for (k=start;k<n+start;k++)
		{
			DisplayCharAtPoint (hDC,DPointToPoint(p[k]),az[k],CharHeight,celev[k-start],k-start,0,Elev);
		}
		nGridElevPt++;
	}

	return TRUE;
}

void SplitCoordFile (HANDLE hBTIn,LPHANDLE phBTOut1,LPHANDLE phBTOut2)
{
	int nRecs = BT_NUM_IN_INDEX (hBTIn);
	int pos=BT_FIRST;
	char	File[MAX_PATH];
	BTVARDESC	BTVar[2];
	HANDLE	hBT1;
	long	save,y;
	int		n=0;
	struct	{long x,GridCellID;} key;

	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;

	GSSiGetTempFileName (0,"gm",0,File);
	BT_CREATE (File, 4, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	*phBTOut1 = BT_OPEN (File, 0, BT_WRITE, 0);
	GSSiGetTempFileName (0,"gm",0,File);
	BT_CREATE (File, 4, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	*phBTOut2 = BT_OPEN (File, 0, BT_WRITE, 0);
	
	nRecs /= 2;
	while (!BT_FIND (hBTIn,(LPSTR)&key,pos,BT_ANY,(LPSTR)&y))
	{
		pos = BT_NEXT;
		save = key.x;
		key.x = y;
		y = save;
		if (n++ < nRecs)
			BT_PUT (*phBTOut1,(LPSTR)&key,(LPSTR)&y);
		else
			BT_PUT (*phBTOut2,(LPSTR)&key,(LPSTR)&y);
	}
	BT_CLOSEANDDELETE (&hBTIn);
	return;
}

BOOL SplitGridFile (LPSTR INFile,LPSTR OutFile,int nLevels)
{
	HANDLE hBTIn = BT_OPEN (INFile,0, BT_READ, 0);
	long	GridCellID, Offset, inc=0;
	int		pos=BT_FIRST;
	POINT	pt;
	char	File[MAX_PATH];
	BTVARDESC	BTVar[2];
	HANDLE	hBT1;
	long	x,y;
	int		i,j,k,nfiles,nfileslast;
	HANDLE	hLev[32];
	LPHANDLE	ph1, ph2;
	struct	{long x,GridCellID;} key;
	char	SubFileID;

	nLevels++;

	if (!hBTIn)
		return FALSE;
		
	GSSiGetTempFileName (0,"gm",0,File);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BT_CREATE (File, 4, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hLev[0] = GSSiGlobAlloc (0,GHND,sizeof(HANDLE));
	ph1 = GlobalLock (hLev[0]);
	*ph1 = BT_OPEN (File, 0, BT_WRITE, 0);
	while (!BT_FIND (hBTIn,(LPSTR)&GridCellID,pos,BT_ANY,(LPSTR)&Offset))
	{
		pos = BT_NEXT;
		pt = GetGridPoint (GridCellID);
		key.x = pt.x;
		key.GridCellID = GridCellID;
		y = pt.y;
		BT_PUT (*ph1,(LPSTR)&key,(LPSTR)&y);
	}
	BT_CLOSE (hBTIn);
	nfileslast = 1;
	nfiles = 2;
	for (i=1,k=1;i<nLevels;i++,k*=2)
	{
		hLev[i] = GSSiGlobAlloc (0,GHND,nfiles*sizeof(HANDLE));
		ph2 = GlobalLock (hLev[i]);
		for (j=0;j<nfileslast;j++,ph2+=2,ph1++)
			SplitCoordFile (*ph1,ph2,(ph2+1));
		GSSiGlobUlFree (&hLev[i-1]);
		GlobalUnlock (hLev[i]);
		ph1 = GlobalLock (hLev[i]);
		nfileslast = nfiles;
		nfiles *= 2;
	}

	SubFileID = 'A';
	for (i=0;i<nfileslast;i++,ph1++,SubFileID++)
	{
		HFILE	FidOut;
		char	str[32];
		char	OFile[MAX_PATH];

		sprintf (OFile,"%s%c.txt",OutFile,SubFileID);
		FidOut = GSSiOpenFile (OFile,0,OF_CREATE);
		
		fputstring ("GRIDCELLID",FidOut);
		pos = BT_FIRST;
		while (!BT_FIND (*ph1,(LPSTR)&key,pos,BT_ANY,(LPSTR)&y))
		{
			pos = BT_NEXT;
			itoa (key.GridCellID,str,10);
			fputstring (str,FidOut);
		}
		BT_CLOSEANDDELETE (ph1);
		GSSiClose2 (&FidOut);
	}
	GSSiGlobUlFree (&hLev[nLevels-1]);
	return TRUE;
}

DPOINT GridCellPCT (LPDPOINT pDPoint)
{
	DPOINT	Point;

	Point.x = fmod ((pDPoint->x - GridMinX),GridWidth)/GridWidth;
	Point.y = fmod ((pDPoint->y - GridMinY),GridHeight)/GridHeight;
	return Point;
}

BOOL HaveNonBKColor (LPRECT rect,HDIB32 hDib32)
{
	int y=max (CurView->ScreenRect.top+1,rect->top);
	int	right = min (CurView->ScreenRect.right-1,rect->right);
	int	bottom = min (CurView->ScreenRect.bottom-1,rect->bottom);
	int	bpp = FreeImage_GetBPP (hDib32);
	int	height = FreeImage_GetHeight (hDib32);

	switch (bpp)
	{
	case 24:

		while (y <= bottom)
		{
			int	x=max (CurView->ScreenRect.left+1,rect->left);
			RGBTRIPLE * p24Bit = (RGBTRIPLE	*)FreeImage_GetScanLine (hDib32,height-y);

			while (x <= right)
			{
	//			COLORREF Color = SetPixel (CurView->hDC,x++,y,RGB(255,255,255));
	//			COLORREF Color = GetPixel (CurView->hDC,x++,y);
				COLORREF Color = RGBTRIPLEToCOLORREF (p24Bit[x++]);
				if (Color != (COLORREF)-1 && Color != CurView->BackGroundColor)
					return TRUE;
			}
			y++;
		}
		return FALSE;

	case 32:
		while (y <= bottom)
		{
			int	x=max (CurView->ScreenRect.left+1,rect->left);
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

BOOL HaveNonBKColorInGoogleBounds (LPMNMXCORD pGoogleBounds,HDIB32 hDib32)
{
	BOOL rtn;
	RECT screenRect;
	int	i;
	DPOINT	points[4];
	POINT	screenPt;


	RectInit (&screenRect);
	BoundsToPoints (pGoogleBounds,points,0);
	for (i=0;i<4;i++)
	{
		ConvertCoord(&points[i], GOOGLEMAPSPROJECTION, 1);
		screenPt = BasePtToScreenPt (&points[i]);
		AddPointToRect (screenPt,&screenRect);
	}
	rtn = HaveNonBKColor (&screenRect,hDib32);  
	return rtn;
}

BOOL CreateGoogleMapTiles (LPSTR InFile,int MaxZoom,LPMNMXCORD pBounds)
{
	BOOL rtn=FALSE;
	int	 zoom, tilex, tiley;
	int	 pos = BT_FIRST;
	double scale;
    char    tempFile[MAX_PATH];
    char    tempFile2[MAX_PATH];
    BTVARDESC   BTVar[3];
	HANDLE	hBT, hBT2;
	struct	{int zm,y,x;} key;	
	MNMXCORD	bounds, nxtbounds;
	MNMXCORD	BoundsInGoogleProjection, gTileBounds;
	HFILE	fidOut;
	char	str[256];
	HDIB32	hDib32=0;
	double boundsSize = BoundsWidth (pBounds) * BoundsHeight (pBounds);
	DPOINT	midPt = MinMaxMidPointD  (pBounds);
	double	meterPerPix;
	RECT	ScreenRect;
	HBITMAP	hBitmap;

	ConvertCoord (&midPt,1,2);
	//find the first zoom level with grid large enough to encompass bounds
	for (zoom = MaxZoom;zoom > 0;zoom--)
	{
		double res = GroundResolution(midPt.y,zoom);
		double gridSize = 256 * 256 * res * res;
		
		if (gridSize > boundsSize)
			break;
	}
	if (!ConvertRectCoord (&BoundsInGoogleProjection,pBounds, 1,GOOGLEMAPSPROJECTION))
		return FALSE;
	AddjustSphericalMercatorBounds (&BoundsInGoogleProjection);
	if (!GetGoogleZoomAndTileFromBounds (pBounds,0,&zoom,&tilex,&tiley,&scale,0))
		return rtn;
	fidOut = GSSiOpenFile (InFile,0,OF_CREATE);
	if (fidOut == HFILE_ERROR)
		return rtn;
	SetCurView (pViewports[*pCommandViewport-1]);
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	GetClientRect (CurView->hWnd,&ScreenRect);
	hBitmap = SaveScreen (CurView->hDC,ScreenRect);
	hDib32 = BitmapToDIB32 (hBitmap);  
	DeleteObject (hBitmap);
    BTVar[0].BT_VARTYP=BT_INTEGER;
    BTVar[0].BT_VARLEN=4;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=4;
    BTVar[1].BT_VAROFF=4;
    BTVar[2].BT_VARTYP=BT_INTEGER;
    BTVar[2].BT_VARLEN=4;
    BTVar[2].BT_VAROFF=8;
	GSSiGetTempFileName (0,"gm",0,(LPSTR)tempFile); 
	BT_CREATE (tempFile, sizeof(MNMXCORD), FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    hBT = BT_OPEN (tempFile,0,BT_WRITE,0); 
	GSSiGetTempFileName (0,"gm",0,(LPSTR)tempFile2); 
	BT_CREATE (tempFile2, sizeof(MNMXCORD), FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    hBT2 = BT_OPEN (tempFile2,0,BT_WRITE,0); 
  
//	GetGoogleTileBounds (zoom,tiley,tilex,&bounds);
	key.zm = -zoom--;
	key.x = 0;
	key.y = 0;
	if (zoom <= MaxZoom)
	{
		BT_PUT (hBT,(LPSTR)&key,(LPSTR)&BoundsInGoogleProjection);
//		BT_PUT (hBT2,(LPSTR)&key,(LPSTR)&bounds);
	}
	while (!BT_FIND (hBT,(LPSTR)&key,BT_FIRST,BT_ANY,(LPSTR)&bounds))
	{
		DPOINT	point;
		int		nxtzoom = -key.zm + 1;
		
		BT_DELETE (hBT,(LPSTR)&key,(LPSTR)&bounds,FALSE);
		if (nxtzoom <= MaxZoom)
		{
			int	nxtrow, nxtcol, googleRow;

			point.x = bounds.xmn + (bounds.xmx - bounds.xmn) / 4;
			point.y = bounds.ymn + (bounds.ymx - bounds.ymn) / 4;
			GetGoogleTileBoundsFromPointAndZoom (nxtzoom,point,&nxtbounds,&nxtrow,&nxtcol,&googleRow);
			if (HaveNonBKColorInGoogleBounds (&nxtbounds,hDib32))
			{
				key.zm = -nxtzoom;
				key.x = nxtcol;
				key.y = googleRow;
				BT_PUT (hBT2,(LPSTR)&key,(LPSTR)&nxtbounds);
				BT_PUT(hBT, (LPSTR)&key, (LPSTR)&nxtbounds);
				if (key.zm == -18 && key.x == 63121 && key.y == 94253)
					ii = 1;

			}
			point.x = bounds.xmn + (bounds.xmx - bounds.xmn) / 4;
			point.y = bounds.ymn + 3 * (bounds.ymx - bounds.ymn) / 4;
			GetGoogleTileBoundsFromPointAndZoom (nxtzoom,point,&nxtbounds,&nxtrow,&nxtcol,&googleRow);
			if (HaveNonBKColorInGoogleBounds (&nxtbounds,hDib32))
			{
				key.zm = -nxtzoom;
				key.x = nxtcol;
				key.y = googleRow;
				BT_PUT (hBT2,(LPSTR)&key,(LPSTR)&nxtbounds);
				BT_PUT (hBT,(LPSTR)&key,(LPSTR)&nxtbounds);
				if (key.zm == -18 && key.x == 63121 && key.y == 94253)
					ii = 1;
			}
			point.x = bounds.xmn + 3 * (bounds.xmx - bounds.xmn) / 4;
			point.y = bounds.ymn + (bounds.ymx - bounds.ymn) / 4;
			GetGoogleTileBoundsFromPointAndZoom (nxtzoom,point,&nxtbounds,&nxtrow,&nxtcol,&googleRow);
			if (HaveNonBKColorInGoogleBounds (&nxtbounds,hDib32))
			{
				key.zm = -nxtzoom;
				key.x = nxtcol;
				key.y = googleRow;
				BT_PUT (hBT2,(LPSTR)&key,(LPSTR)&nxtbounds);
				BT_PUT (hBT,(LPSTR)&key,(LPSTR)&nxtbounds);
				if (key.zm == -18 && key.x == 63121 && key.y == 94253)
					ii = 1;
			}
			point.x = bounds.xmn + 3 * (bounds.xmx - bounds.xmn) / 4;
			point.y = bounds.ymn + 3 * (bounds.ymx - bounds.ymn) / 4;
			GetGoogleTileBoundsFromPointAndZoom (nxtzoom,point,&nxtbounds,&nxtrow,&nxtcol,&googleRow);
			if (HaveNonBKColorInGoogleBounds (&nxtbounds,hDib32))
			{
				key.zm = -nxtzoom;
				key.x = nxtcol;
				key.y = googleRow;
				BT_PUT (hBT2,(LPSTR)&key,(LPSTR)&nxtbounds);
				BT_PUT (hBT,(LPSTR)&key,(LPSTR)&nxtbounds);
				if (key.zm == -18 && key.x == 63121 && key.y == 94253)
					ii = 1;
			}
/*			point = MinMaxMidPointD (&bounds);
			GetGoogleTileBoundsFromPointAndZoom (nxtzoom,point,&nxtbounds,&nxtrow,&nxtcol,&googleRow);
			if (HaveNonBKColorInGoogleBounds (&nxtbounds,hDib32))
			{
				key.zm = -nxtzoom;
				key.x = nxtcol;
				key.y = googleRow;
				BT_PUT (hBT2,(LPSTR)&key,(LPSTR)&nxtbounds);
				BT_PUT (hBT,(LPSTR)&key,(LPSTR)&nxtbounds);
				if (key.zm == -18 && key.x == 63121 && key.y == 94253)
					ii = 1;
			}*/
		}
	}

	BT_CLOSEANDDELETE (&hBT); 
	sprintf (str,"GZOOM\tGROW\tGCOL\tGBOUNDS");
	fputstring (str,fidOut);
/*	key.zm = -18;
	key.x = 63121;
	key.y = 94253;
	BT_FIND(hBT2, (LPSTR)&key, BT_FIRST,BT_EQ, (LPSTR)&bounds);*/
	while (!BT_FIND (hBT2,(LPSTR)&key,pos,BT_ANY,(LPSTR)&bounds))
	{
		pos = BT_NEXT;
		sprintf (str,"%i\t%i\t%i\t%f %f %f %f",-key.zm,key.y,key.x,bounds.xmn,bounds.ymn,bounds.xmx,bounds.ymx);
		fputstring (str,fidOut);
	}
	GSSiClose2 (&fidOut);
	BT_CLOSEANDDELETE (&hBT2);  
	GMDestroyDIB32 (hDib32);

	return rtn;
}

BOOL CreateAreaGridFromGrid (LPSTR InFile,LPSTR SymName,LPSTR Prefix,int MaxZoom,LPMNMXCORD pBounds,BOOL UseStatusWnd)
{
	HIGHLIGHTDATA	HighlightData;
	MNMXCORL		GridBounds;
	MNMXCORD		Area;
	long	Refno, nAreaPnts, TotAreas, nLoaded=0, MaxAreas, MemSize; 
	HPDPOINT pAreaPoints;  
	HANDLE	hAreaPoints;
	long	nRows,nCols, nAreas=0, iRow, iCol, iArea=0, i,j;  
	BOOL	InArea; 
	POINT	Point, Point1, Point2, WPoint;
	DPOINT	IntPoint;
	short	AreaSymbol;
	HANDLE	hSymDesc=0;
	short	NumSyms=0;    
	long	NewRefno=1,nRecs; 
	char	UDI[64]=""; 
	HANDLE	hAccel=0;  
	double	NorthAZ=PY/2, EastAZ=0; 
	HPDPOINT	pPolyPoints;
	HANDLE	hPoly=0; 
	double	D[4]; 
	HANDLE	hP;
	HPDPOINT pPoints; 
	MNMXCORD	Bounds;
	long	npnts;  
	int		np=4, nCol, nRow, GridID, irow, icol, Done=0;    
	short	ii;
	char	File[MAX_PATH];
	char	str[256];
	HFILE	Fid;

	strcpy (File,InFile);
	strlwr (File);
    
	if (!stricmp (Prefix,"GOOGLE"))
		return CreateGoogleMapTiles (InFile,MaxZoom,pBounds);
    if (PRJ_UNITS[1] == 4)
	{
		NorthAZ = 0;
		EastAZ = PY/2;
	}
	if (!GetGridDef (Prefix))
		return FALSE;
	if (strstr (File,".plt"))
	{
		AreaSymbol=GetDictSymbolNumber (SymName);
		if (BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData))
			return FALSE;  
		hP = GSSiGlobAlloc (1138,GMEM_MOVEABLE,sizeof(DPOINT)*4);
		pPoints  = (HPDPOINT)GlobalLock (hP);
		PickList[0] = HighlightData.PD; 
		hAreaPoints = GSSiGlobAlloc (1136,GMEM_MOVEABLE,(long)PickList[0].NumPoints*sizeof(DPOINT)); 
		pAreaPoints = (HPDPOINT)GlobalLock (hAreaPoints);
		nAreaPnts = GetPickItemPoints (0, FALSE, &pAreaPoints); 
		GlobalUnlock (hAreaPoints);
		pAreaPoints = (HPDPOINT)GlobalLock (hAreaPoints);   
		nAreas = 0;
		Bounds = HighlightData.PD.Rect;
		Point1.x = HighlightData.PD.Rect.xmn;
		Point1.y = HighlightData.PD.Rect.ymn;
		Point2.x = HighlightData.PD.Rect.xmx;
		Point2.y = HighlightData.PD.Rect.ymn;
		nCol = GridCellID (&Point2) - GridCellID (&Point1) + 1;
		Point2.x = HighlightData.PD.Rect.xmn;
		Point2.y = HighlightData.PD.Rect.ymx;
		nRow = GridCellRow (&Point2) - GridCellRow (&Point1) + 1;
		nRecs = nRow * nCol;
		GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts,&hPoly);
		pPolyPoints = (HPDPOINT)GlobalLock (hPoly); 
		InflateBounds (&Bounds, max (GridWidth,GridHeight));
		strcpy (PltName,File); 
		strcpy (EditName,File);
		CreateNewMap (PltName,&Bounds,0,0,0,0,0,0,FALSE);
		EditBounds = CurView->FileMNMX;
		if (UseStatusWnd)
			CreateStatusWind (hWndMain,1,"Create Area Grid");
		
		WPoint.y = HighlightData.PD.Rect.ymn;
		for (irow = 0;irow < nRow;irow++)
		{
			WPoint.x = HighlightData.PD.Rect.xmn;
			GridID = GridCellID (&WPoint);

			for (icol = 0;icol < nCol;icol++)
			{
				InArea = FALSE;
				GetGridBounds (GridID,&GridBounds);
				BoundsLToPoints (&GridBounds,pPoints,0);
				for (i=0;i<4;i++)
					if (POINT_IN_AREAD (pPoints[i], nAreaPnts,pAreaPoints,1,0,0,&hAccel))
						InArea = TRUE;  
				if (!InArea)
				{   
					InArea = IntersectPolys1 ( HighlightData.PD.Type,GF_AREA,npnts,pPolyPoints,0,4,pPoints,0,0,
												&HighlightData.PD.PickedPoint,&IntPoint,&D[3],&D[0],0);
				}
				if (!InArea)
				{   
					for (i=0;i<npnts;i++)
						if (PointInBoundsL (pPolyPoints[i],&GridBounds))
							InArea = TRUE;
				}
				if (InArea) 
				{
					ltoa (GridID,UDI,10);
					NewRefno = GetNewRefno (EditName,0,0,0,0);
					if (AddPolyToMap (1,&np, &hP,0,NewRefno++,0,2,AreaSymbol,0,Prefix,UDI,-1,-1,-1,0,0,0,0,TRUE,0))
						nAreas++;   
					else
						ii = 1;
					iArea++;
				}
				WPoint.x += GridWidth;  
				GridID++;
				Done++;
			}
			
			WPoint.y += GridHeight;
			if (UseStatusWnd)
				StatusWindowUpdate (0,0, nRecs, Done);
		}                                     
		 
		GSSiGlobUlFree (&hP); 
		GSSiGlobFree (&hAccel);
		GSSiGlobUlFree (&hPoly);
		AddToSymList (AreaSymbol,&NumSyms,&hSymDesc); 
		CloseMap(TRUE);  
		AddSymToMap (NumSyms,hSymDesc,0,0); 
		DestroySymList (&NumSyms,&hSymDesc); 
		if (UseStatusWnd)
			DestroyStatusWindow(0);  

		GSSiGlobUlFree (&hAreaPoints);
	}
	else
	{
		DPOINT	WDPoint;
		HBITMAP	hBitmap;
		HDIB32 hDib32;
		RECT	ScreenRect;

		SaveDC (CurView->hDC);
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
		GetClientRect (CurView->hWnd,&ScreenRect);
		hBitmap = SaveScreen (CurView->hDC,ScreenRect);
		hDib32 = BitmapToDIB32 (hBitmap);  
		DeleteObject (hBitmap);
		Fid = GSSiOpenFile (File,0,OF_READWRITE);
		if (Fid == HFILE_ERROR)
			return FALSE;
		GSSillseek (Fid,0,2);
		WDPoint.y = pBounds->ymn - GridHeight;
		while (WDPoint.y <= pBounds->ymx)
		{
			WDPoint.x = pBounds->xmn - GridWidth;

			while (WDPoint.x <= pBounds->xmx)
			{
				DPOINT	Points[4];
				DPOINT	MidPoint=WDPoint;
				POINT	MidPointI;

				MidPoint.x += GridWidth/2;
				MidPoint.y += GridHeight/2;
				MidPointI = DPointToPoint (MidPoint);
				GridID = GridCellID (&MidPointI);
				InArea = FALSE;
				GetGridBounds (GridID,&GridBounds);
				BoundsLToPoints (&GridBounds,Points,0);
				RectInit (&ScreenRect);
				for (i=0;i<4;i++)
				{
					POINT	ScreenPt = BasePtToScreenPt (&Points[i]);

					AddPointToRect (ScreenPt,&ScreenRect);
				}
				if (GridID == 458265 || GridID == 458270)
					ii=1;
				InArea = HaveNonBKColor (&ScreenRect,hDib32);  
				if (InArea) 
				{
					RECT	Rect2=ScreenRect;
					static	int	dbid=4911834;

				//	InflateRect (&Rect2,-2,-2);
				//	FillRect (CurView->hDC,&Rect2,GetStockObject (WHITE_BRUSH));
					sprintf (str,"%s\t%i",Prefix,GridID);
				//	if (GridID == dbid)
				//		MessageBox (0,"Found IT",0,MB_OK);
					fputstring (str,Fid);
					iArea++;
				}
				WDPoint.x += GridWidth;  
				Done++;
			}
			
			WDPoint.y += GridHeight;
//			if (UseStatusWnd)
//				StatusWindowUpdate (0,0, nRecs, Done);
		}                                     
		GSSiClose2 (&Fid); 
		RestoreDC (CurView->hDC,-1);
		GMDestroyDIB32 (hDib32);
	}
	return TRUE;
}

BOOL CreateAreaGrid (LPSTR File,LPSTR Symbol,double Dist)
#if ENABLETRACE
{GSSiEnterProg (1089);
#endif
{
	short  NumSyms=0, AreaSym;  
	int		np=5;
	HANDLE hSymDesc=0, hPoly; 
	long	NewRefno; 
	long	nRows, nCols, iRow, iCol; 
	HPDPOINT	pPoly, pPoly0;
    
    _fstrcpy (PltName,File);  
    PltType = 2;
	if (!OpenMap (CurView->hWnd,CurView->hDC))
{
#if ENABLETRACE
GSSiExitProg (1089);
#endif
		return FALSE;
}
	if (!(AreaSym = GetDictSymbolNumber (Symbol)))
{
#if ENABLETRACE
GSSiExitProg (1089);
#endif
		return FALSE;
}
	AddToSymList (AreaSym,&NumSyms,&hSymDesc);
	EditBounds = CurView->FileMNMX; 
	CloseMap (FALSE);
	nCols = 1 + (EditBounds.xmx - EditBounds.xmn) / Dist;
	nRows = 1 + (EditBounds.ymx - EditBounds.ymn) / Dist; 
	hPoly = GSSiGlobAlloc (1056,GMEM_MOVEABLE,5*sizeof(DPOINT));
	for (iRow = 0;iRow < nRows;iRow++)
	{
		for (iCol = 0; iCol < nCols; iCol++)
		{
			pPoly = pPoly0 = (HPDPOINT)GlobalLock (hPoly);  
			pPoly->x = EditBounds.xmn + iCol * Dist;
			pPoly++->y = EditBounds.ymn + iRow * Dist;
			pPoly->x = EditBounds.xmn + iCol * Dist;
			pPoly++->y = min (EditBounds.ymx,EditBounds.ymn + (iRow+1) * Dist);
			pPoly->x = min (EditBounds.xmx,EditBounds.xmn + (iCol+1) * Dist);
			pPoly++->y = min (EditBounds.ymx,EditBounds.ymn + (iRow+1) * Dist);
			pPoly->x = min (EditBounds.xmx,EditBounds.xmn + (iCol+1) * Dist);
			pPoly++->y = EditBounds.ymn + iRow * Dist;
			*pPoly = *pPoly0;
			GlobalUnlock (hPoly);
			NewRefno = GetNewRefno(PltName,0,0,0,0);
            AddPolyToMap (1,&np, &hPoly,0,NewRefno,0,-1,AreaSym,0,0,0,
                            -1,-1,0,0,0,0,0,TRUE,0);
		}
	} 
	CloseMap(TRUE);
	AddSymToMap (NumSyms,hSymDesc,0,0); 
	DestroySymList (&NumSyms,&hSymDesc);    
	GSSiGlobFree (&hPoly);
{
#if ENABLETRACE
GSSiExitProg (1089);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

void OpenDisplayedHighlightedRefs(void)
#if ENABLETRACE
{GSSiEnterProg (1090);
#endif
{	BTVARDESC	BTVar[2]; 
	char	File[MAX_PATH]; 
	UINT	ifile;

	CloseDisplayedHighlightedRefs();
	if (!CurView || ForceRefIndex)
		goto Exit;
    for (ifile=0;ifile<CurView->NumFiles;ifile++)
	    if (CurView->FileType[ifile] == 6)
		{
			if (LayerIsVisible (ifile))
				goto Open;
		}
	goto Exit;
Open:
	GSSiGetTempFileName(0,"gm",0,File);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (File, 2, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hDisplayedHighlightedRefs = BT_OPEN (File, 0, BT_WRITE, 0);
Exit:
{
#if ENABLETRACE
GSSiExitProg (1090);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void CloseDisplayedHighlightedRefs(void)
#if ENABLETRACE
{GSSiEnterProg (1091);
#endif
{   
	BT_CLOSEANDDELETE (&hDisplayedHighlightedRefs);
{
#if ENABLETRACE
GSSiExitProg (1091);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void OpenDisplayedRefs(void)
#if ENABLETRACE
{GSSiEnterProg (1090);
#endif
{	BTVARDESC	BTVar[2]; 
	char	File[MAX_PATH]; 

	CloseDisplayedRefs();
	GSSiGetTempFileName(0,"gm",0,File);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (File, 2, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hDisplayedRefs = BT_OPEN (File, 0, BT_WRITE, 0);
{
#if ENABLETRACE
GSSiExitProg (1090);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void CloseDisplayedRefs(void)
#if ENABLETRACE
{GSSiEnterProg (1091);
#endif
{   
	BT_CLOSEANDDELETE (&hDisplayedRefs);
{
#if ENABLETRACE
GSSiExitProg (1091);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 
int	GetSymbolNum (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (1092);
#endif
{
	LPVISLIST	SaveVis;
	int			l;
	
	if (AlwaysUseSymDict)
{
#if ENABLETRACE
GSSiExitProg (1092);
#endif
		return GetDictSymbolNumber (Name);
}
	CurrentSymNum = 0;
	_fstrcpy(CurrentSymName,Name);
	GetVisList (0,-9999,0,0,-1,HFILE_ERROR);
{
#if ENABLETRACE
GSSiExitProg (1092);
#endif
	return (CurrentSymNum);
}
#if ENABLETRACE
}
#endif
}

short LayerFileType (short Layer)
{   
	
	if (Layer <= 0)
		return 0; 
	return MapFileType(CurView->lpFiles[Layer - 1], 0, 0);
}

short MapFileType (LPSTR InName,LPSTR fileName, LPSTR tableName)
{   
	char	Name[256];  
	short	l; 
	LPSTR	pBS, pPar, pParClose, pDot;
	
	if (fileName)
		*fileName = 0;
	if (tableName)
		*tableName = 0;
	_fstrcpy (Name,InName);
	ExpandText (Name);
	_fstrupr (Name);
	if ((pDot = strrchr (Name,'.')))
	{
		if ((pPar = _fstrrchr(pDot, '(')))
		{
			*pPar++ = 0;
			if ((pParClose = strrchr(pPar, ')')))
			{
				*pParClose = 0;
				if (tableName)
					strcpy(tableName, pPar);
			}
		}
		if (fileName)
			strcpy(fileName, Name);
	}
	else
		pDot = Name;
	l=_fstrlen (Name);
	if (l < 4)
		return 0; 
	if ((pBS = _fstrrchr (Name,'\\')))     
		pBS++;
	else
		pBS = Name;
	if (!stricmp (pBS,"HIGHLIGHT"))
		return MT_HIGHLIGHTLIST;
	if (!_fstricmp (pBS,"FILELIST.TXT"))
		return MT_INDEX;
	if (!_fstricmp (&Name[l-4],".HGF"))
		return MT_HGF; 
	if (!_fstricmp (&Name[l-4],".PLT"))
		return MT_PLT; 
	if (!_fstricmp (&Name[l-4],".SHP"))
		return MT_SHP; 
	if (!_fstricmp (&Name[l-4],".MDB"))
		return MT_PERSONAL_GEO_DB; 
	if (!_fstricmp (&Name[l-4],".GDB"))
		return MT_FILE_GEO_DB; 
	if (!_fstricmp (&Name[l-4],".SID") || !_fstricmp (&Name[l-4],".JP2"))
		return MT_SID; 
	if (!_fstricmp (&Name[l-4],".ORA"))
		return MT_ORA; 
	if (!_fstricmp(&Name[l - 4], ".GMD"))
		return MT_GMD;
	if (!_fstricmp(&Name[l - 4], ".SLT"))
		return MT_SQLITE;
	if (!_fstricmp(&Name[l - 4], ".GPX"))
		return MT_GPX;
	if (!_fstricmp (&Name[l-4],".KML"))
		return MT_KML;
	if (!_fstricmp(&Name[l - 5], ".DGN8") || !_fstricmp(&Name[l - 4], ".COM5"))
		return MT_DGN8;
	if (!_fstricmp(&Name[l - 4], ".DGN") || !_fstricmp(&Name[l - 4], ".COM"))
		return MT_DGN7;
	if (!_fstricmp(&Name[l - 4], ".TXT"))
		return MT_MACRO; 
	if (!_fstricmp(&Name[l - 4], ".BMP") || !_fstricmp(&Name[l - 4], ".JPG") || !_fstricmp(&Name[l - 4], ".TIF") || !_fstricmp(&Name[l - 4], ".PCX"))
		return MT_IMAGE;
	if (!_fstricmp(&Name[l - 4], ".GG1"))
		return MT_GOOGLE_ROADMAP;
	if (!_fstricmp(&Name[l - 4], ".GG2"))
		return MT_GOOGLE_SATELLITE;
	if (!_fstricmp(&Name[l - 4], ".GG3"))
		return MT_GOOGLE_TERRAIN;
	if (!_fstricmp(&Name[l - 4], ".GG4"))
		return MT_GOOGLE_HYBRID;
	if (!_fstricmp(&Name[l - 4], ".DTM") || !_fstricmp(&Name[l - 4], ".DTS") || !_fstricmp(&Name[l - 4], ".LDR") || !_fstricmp(&Name[l - 4], ".TIN") || !_fstricmp(&Name[l - 4], ".LAZ"))
		return MT_DTM; 
	if (!_fstrnicmp (pBS,"INDEX",5))
		return MT_INDEX;
	return 0;
}
 
BOOL GetSymbolName (int Desc, LPSTR Name,LPSHORT pParent, int From, LPBOOL pIsPar)
#if ENABLETRACE
{GSSiEnterProg (1093);
#endif
{
	static		int		LastDesc=-1, LastIsPar, LastParent;
	static		char	LastName[lnCurrentSymName]; 
	OFSTRUCTGM	OFStruct;
    short       Signature,l;   
    long   		PrimeOffset; 
    BOOL		rtn;
	LPSTR		pTab;
	
	*Name = 0;
	if (From != 2 && From != 4 && AlwaysUseSymDict) //from = 2 gets from edit file, 4 always gets from current visibility list files
	{
		HANDLE	hSymbol;
		LPSYMBOL pSymbol; 
		
		hSymbol = GetDictSymDesc (Desc,1);
		if (hSymbol)
		{ 
			pSymbol = (LPSYMBOL)GlobalLock (hSymbol);  
			_fstrcpy (Name,pSymbol->Name);  
			if (pParent)
				*pParent = pSymbol->Parent;
			if (pIsPar) 
			{
				if (!pSymbol->Type)
					*pIsPar = 1; 
				else
					*pIsPar = 0; 
			}
	     	GlobalUnlock (hSymbol); 
			DestroySymbol (hSymbol); 
{
#if ENABLETRACE
GSSiExitProg (1093);
#endif
			return TRUE;
}
		} 
		else
{
#if ENABLETRACE
GSSiExitProg (1093);
#endif
			return FALSE;
}
	} 
	if (From == 3)
	{ 
		rtn = GetOpenFileSymName (Desc,Name);
{
#if ENABLETRACE
GSSiExitProg (1093);
#endif
			return rtn;
}
	}
	if (!CurView)
		SetCurView (pViewports[*pCommandViewport-1]);
	CurrentSymParent = 0; 
	CurSymIsParent = FALSE;
	*CurrentSymName=0;
	if (From == 2) //get from edit file
	{
	    DuplicateDescInit();
        CloseMap (FALSE);  
        _fstrcpy (PltName,EditName);
        FidMap = GSSiOpenFile (PltName,(LPOFSTRUCTGM)&OFStruct,OF_READ); 
        if (FidMap != HFILE_ERROR)
        {
		    GSSillseek(FidMap,(LONG)-(6),2);
		    BigRead (FidMap,(HPSTR)&Signature,2);
		    if (Signature == 32349)
		    {  
		    	BYTE	MV;
		    	
			    BigRead (FidMap,&MV,1);  
			    MapVersion = MV;
                GSSillseek(FidMap,(LONG)-(6+12),2);
                BigRead (FidMap,(HPSTR)&PrimeOffset,4);
                GSSillseek(FidMap,PrimeOffset,0);
                ProcessPrimarySeg (0, 0, 0,FALSE,0,HFILE_ERROR);
                GSSillseek(FidMap,UsedDescOffset,0);
                ProcessPrimarySeg (0,-Desc,0,FALSE,0,HFILE_ERROR); 
            }
            GSSiClose2 (&FidMap); 
        }
        FidMap = HFILE_ERROR;
	    DuplicateDescClose(); 
	    goto Exit;
	}
	if (Desc == LastDesc && LastName[0])
	{
		_fstrcpy (Name,LastName); 
		if (pParent)
			*pParent = LastParent; 
		if (pIsPar)
			*pIsPar = LastIsPar; 
{
#if ENABLETRACE
GSSiExitProg (1093);
#endif
		return TRUE;
}
	}
	{
		HANDLE	hVis=GSSiGlobAlloc (1057,GMEM_MOVEABLE,sizeof(VISLIST));
		LPVISLIST	SaveVis=CurVis;
		
		CurVis = (LPVISLIST)GlobalLock (hVis);
		if (SaveVis)
			*CurVis = *SaveVis;
		else
			InitVis ();
		if (From >= 0)
		{
			SelectVisList (FALSE);
			GetVisList (0,-Desc,0,0,-1,HFILE_ERROR);
		}
		else 
		{   
			CurVis->FileIsVisible[-(From+1)]=TRUE;
			GetVisList (0,-Desc,0,0,-From,HFILE_ERROR);  
		}
		_fstrcpy (Name,CurrentSymName);
		CurVis = SaveVis;
		GSSiGlobUlFree (&hVis);
	} 
Exit:
	if (*CurrentSymName) 
	{
		LastDesc = Desc;  
		LastParent = CurrentSymParent;
		ConvertSymName (CurrentSymName,0,FALSE,0);
		_fstrcpy (LastName,CurrentSymName);
		if ((pTab = strchr (LastName,'\t')))
			*pTab = 0;
		strcpy (Name,LastName);
		if (pParent)
			*pParent = CurrentSymParent;
		if (pIsPar)
			*pIsPar = CurSymIsParent; 
		LastIsPar = CurSymIsParent; 
{
#if ENABLETRACE
GSSiExitProg (1093);
#endif
		return TRUE;
}
	} 
	else if (pParent)
		*pParent = 0;
	LastDesc = -1; 
	LastParent = 0;
{
#if ENABLETRACE
GSSiExitProg (1093);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}
void AddGraphicsCmd (HWND hWnd,LPSTR Cmd,BOOL Keydef, UINT StartPrompt)
#if ENABLETRACE
{GSSiEnterProg (1094);
#endif
{   
	USHORT	len=_fstrlen(Cmd);
	HANDLE	hCmd = GSSiGlobAlloc (1058,GHND,len+sizeof(CMDSTRING)+1);
	LPCMDSTRING    pCmdStrPrev,pCmdStr = (LPCMDSTRING)GlobalLock (hCmd);  
	LPSTR	pPrevCmd; 
	BOOL	SameCmd=FALSE;
	
	if (hWnd)
		SetFocus(hWnd);
	_fstrcpy (pCmdStr->Cmd,Cmd);
	pCmdStr->EndLoc = len;
	pCmdStr->MyHandle = hCmd;  
	pCmdStr->VP = CurView->ID;        
	pCmdStr->Prompt = StartPrompt;
	pCmdStr->PrevHandle = CurView->FunStackHandle;
	GlobalUnlock (hCmd);
	if (CurView->FunStackHandle)
	{
		pCmdStrPrev = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);   
		pPrevCmd = pCmdStrPrev->Cmd;
		if (!_fstrcmp (pCmdStrPrev->Cmd,Cmd) && !_fstrstr (Cmd,"$RESET()"))
		{   
			AddLBUTTON = pCmdStrPrev->AddLBUTTON; 
/*			GSSiGlobFree (&pCmdStrPrev->hError);
	    	GSSiGlobFree (&pCmdStrPrev->hWhile);
			GlobalUnlock (CurView->FunStackHandle);
			GlobalFree (hCmd);
	    	if (AddLBUTTON && HaveCurrentLBUTTON)
	    	{
	    		HaveCurrentLBUTTON = FALSE;
	    		if (Keydef) 
	    			PostMessage (hWndMain,WM_LBUTTONDOWN,0,MAKELONG(CurrentLBUTDOWNLoc.x,CurrentLBUTDOWNLoc.y)); 
	    		IgnoreLbutton = FALSE;
	    		PostMessage (hWndMain,WM_LBUTTONUP,0,MAKELONG(CurrentLBUTDOWNLoc.x,CurrentLBUTDOWNLoc.y));
	    	}
			retrn;  
*/   
			SameCmd = TRUE;
		}
		GlobalUnlock (CurView->FunStackHandle);
	}
	if (SameCmd) 
	{
		GSSiGlobFree (&hCmd);
    	if (AddLBUTTON && HaveCurrentLBUTTON)
    	{
    		HaveCurrentLBUTTON = FALSE;
    		if (Keydef) 
    			PostMessage (hWndMain,WM_LBUTTONDOWN,0,MAKELONG(CurrentLBUTDOWNLoc.x,CurrentLBUTDOWNLoc.y)); 
    		IgnoreLbutton = FALSE;
    		PostMessage (hWndMain,WM_LBUTTONUP,0,MAKELONG(CurrentLBUTDOWNLoc.x,CurrentLBUTDOWNLoc.y));
    	}
    	else
			ProcessGraphicsFunction (hWnd, GF_REINIT, 0, 0);
    }
	else
	{		
		CurView->FunStackHandle = hCmd;
		CurView->CurrentFunction = 0;
		RemoveGraphicsFunction (hWnd,0); 
	}
	if (CurView && CurView->FunStackHandle) 
	{
   		pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle); 
		StartPrompt = pCmdStr->Prompt;
		GlobalUnlock (CurView->FunStackHandle);
	}
	SetPrompt (StartPrompt,FALSE);
{
#if ENABLETRACE
GSSiExitProg (1094);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void AddGraphicsFunction (HWND hWnd, int Function,UINT StartPrompt)
#if ENABLETRACE
{GSSiEnterProg (1095);
#endif
{   
	char	str[32];
    POINT	CursorLoc;
	
//    if (CurView->CurrentFunction==Function) retrn;
	switch (Function)
	{
/*   		case GF_LBUTTON:
			GetCursorPos (&CursorLoc);
			ScreenToClient (hWnd,&CursorLoc);
			PostMessage (hWnd,WM_LBUTTONDOWN,1,MAKELONG(CursorLoc.x,CursorLoc.y)); 
			PostMessage (hWnd,WM_LBUTTONUP,0,MAKELONG(CursorLoc.x,CursorLoc.y)); 
			break;
   		case GF_RBUTTON:
			GetCursorPos (&CursorLoc);
			ScreenToClient (hWnd,&CursorLoc);
			PostMessage (hWnd,WM_RBUTTONDOWN,2,MAKELONG(CursorLoc.x,CursorLoc.y)); 
			PostMessage (hWnd,WM_RBUTTONUP,0,MAKELONG(CursorLoc.x,CursorLoc.y)); 
			break;   */
   		default:
		    itoa (Function,str,10);
		    AddGraphicsCmd (hWnd,str,FALSE,StartPrompt); 
		    break;
	}
{
#if ENABLETRACE
GSSiExitProg (1095);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void RemoveGraphicsFunction (HWND hWnd,int Function)
#if ENABLETRACE
{GSSiEnterProg (1096);
#endif
{
    HANDLE	hLast;
	LPCMDSTRING    pCmdStr;
	LPSTR	pLoc, EndCmd;     
	short	LastFun;
	
	if (InReset)
{
#if ENABLETRACE
GSSiExitProg (1096);
#endif
		return; // called from $RESET()		
}
NextCmd:  
	if (!CurView)
		goto Exit;
   	LastFun = CurView->CurrentFunction; 
   	if (Function && Function != LastFun)
   	{   
   		hLast = CurView->FunStackHandle;
   		while (hLast)
   		{   
   			HANDLE	hCur=hLast;
   			
	    	pCmdStr = (LPCMDSTRING)GlobalLock (hLast);  
	    	if (pCmdStr->CurFun == Function) 
	    	{
	    		pCmdStr->CurFun = 0;
		    	GlobalUnlock (hCur);
		    	goto Exit;
		    }
	    	hLast = pCmdStr->PrevHandle; 
	    	GlobalUnlock (hCur);
   		}
   		goto Exit;
   	}
    if (CurView->FunStackHandle)
    { 
   		if (!AdjustCurLoc (&CurView->FunStackHandle))
   		{   
   			HaveCurrentLBUTTON = FALSE;
   			if (!CurView->FunStackHandle) 
   			{
				GSSiGlobFree (&hEmbeddedGFCommand);
   				goto NextCmd;
   			}
	    	pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);
	    	if (!pCmdStr)
	    	{
	    		CurView->FunStackHandle = 0;
				GSSiGlobFree (&hEmbeddedGFCommand);
	    		goto NextCmd;
	    	} 
	    	hLast = pCmdStr->PrevHandle; 
			GSSiGlobFree (&pCmdStr->hError);
	    	GSSiGlobFree (&pCmdStr->hWhile);
    		GSSiGlobUlFree (&CurView->FunStackHandle);
    		CurView->FunStackHandle = hLast; 
    		if (CurView->FunStackHandle)
    		{
		    	pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle); 
		    	if (!pCmdStr)
		    	{
		    		CurView->FunStackHandle = 0;
					GSSiGlobFree (&hEmbeddedGFCommand);
		    		goto NextCmd;
		    	} 
		    	if (!pCmdStr->CurFun)
		    	{
		    		GlobalUnlock (CurView->FunStackHandle);
					GSSiGlobFree (&hEmbeddedGFCommand);
		    		goto NextCmd;
		    	} 
    			CurView->CurrentFunction = pCmdStr->CurFun; 
    			if (!CursorIsLocked)
    				SetCurs (pCmdStr->hCursor,FALSE);
	    		GlobalUnlock (CurView->FunStackHandle);
			    ProcessGraphicsFunction (hWnd, GF_COMPLETE, LastFun, 0); 
			    NotifyFunction (CurView,GF_ENTER_VIEWPORT);  

    		}
    		else
    		{
    			CurView->CurrentFunction = 0; 
    			if (!CursorIsLocked)
					SetCurs (0,FALSE);
    		}
    	}
    	else
    	{               
    		char	SaveChr;
    		LPSTR	pCmd;
    		HANDLE	hCmd, CurrentHandle, InWhile;
    		short	SaveVPID; 
    		
	    	pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle); 
	    	if (!pCmdStr)
	    	{
	    		CurView->FunStackHandle = 0; 
				GSSiGlobFree (&hEmbeddedGFCommand);
	    		goto NextCmd;
	    	}
    		if (pCmdStr->PrevHandle && CurView->CurrentFunction/*&& 
    			(CurView->CurrentFunction == GF_LBUTTON || 
    			 CurView->CurrentFunction == GF_RBUTTON)*/)
    		{
				LPCMDSTRING    pCmdStrPrev=(LPCMDSTRING)GlobalLock (pCmdStr->PrevHandle);
	    		GlobalUnlock (CurView->FunStackHandle);
		    	if (pCmdStrPrev->CurFun)
		    	{   
	    			CurView->CurrentFunction = pCmdStrPrev->CurFun; 
				    ProcessGraphicsFunction (hWnd, GF_COMPLETE, LastFun, 0);
    			    NotifyFunction (CurView,GF_ENTER_VIEWPORT);  

        			CurView->CurrentFunction = LastFun; 
	    		}
	    		GlobalUnlock (pCmdStr->PrevHandle);
	    		if (CurView->FunStackHandle) 
		    		pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle); 
		    	else 
		    	{
					GSSiGlobFree (&hEmbeddedGFCommand);
		    		goto NextCmd;
		    	}
	    	} 
InsertIfResult:
			pLoc = pCmdStr->Cmd;
			pLoc += pCmdStr->CurLoc;
			EndCmd = MatchLev (pLoc,';');
			if (!EndCmd)
				EndCmd = _fstrchr (pLoc,0);
    		pCmdStr->CurLoc = EndCmd-pCmdStr->Cmd;
    		hCmd = GSSiGlobAlloc (1059,GMEM_MOVEABLE,4096);
    		pCmd = GlobalLock (hCmd);
    		SaveChr = *EndCmd;
    		*EndCmd = 0;
    		_fstrcpy (pCmd,pLoc);
    		*EndCmd = SaveChr;
 			SaveVPID = pCmdStr->VP; 
 			InWhile = pCmdStr->hWhile;
     		GlobalUnlock (CurView->FunStackHandle);  
     		CurrentHandle = CurView->FunStackHandle; 
    CheckIF:
			if (!InWhile && !_fstrnicmp (pCmd,"IF(",3))
			{   
				BOOL	Err;
				HANDLE	hIF = ProcessIF (pCmd,&Err);
                
                if (hIF) 
                {   
                	LPSTR pIF = GlobalLock (hIF);
                	
                	if (*pIF == '{' && *LastChr(pIF) == '}')
                	{
                		_fstrcpy (pCmd,&pIF[1]);
                		*LastChr (pCmd) = 0;
                	}
                	else
                		_fstrcpy (pCmd,pIF);
                	GSSiGlobUlFree (&hIF); 
                	if (!_fstrncmp (pCmd,"IF(",3))
                		goto CheckIF;
                }
                else
                	*pCmd = 0;
			}
			else 
			{ 
				BOOL SaveIGFC = InGRFCmd;
				InGRFCmd = TRUE;
    			ExpandText (pCmd); 
    			InGRFCmd = SaveIGFC;
    		}
    		if (!ConfigLoaded)
    		{	
    			GSSiGlobUlFree (&hCmd);
					goto Exit;
    		}
     		SetViewport (SaveVPID);  
     		if (CurView->FunStackHandle != CurrentHandle) 
     		{
	    		GSSiGlobUlFree (&hCmd);
					goto Exit;
			}
	    	pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle); 
	    	if (!pCmdStr->hWhile && MatchLev (pCmd,';')) //check for commands embedded in IF
	    	{   
	    		short	l = _fstrlen (pCmd);
	    		short	l2;
	    		
				pLoc = pCmdStr->Cmd;
				pLoc += pCmdStr->CurLoc; 
				l2 = pCmdStr->EndLoc - pCmdStr->CurLoc;
	    		_fmemmove (pCmdStr->Cmd+l,pLoc,l2+1); 
	    		_fmemmove (pCmdStr->Cmd,pCmd,l);
	    		pCmdStr->EndLoc = l + l2;
	    		pCmdStr->CurLoc = 0;
	    		GSSiGlobUlFree (&hCmd);
	    		goto InsertIfResult;
	    	}
    		pCmdStr->CurFun = IsGFCmd (pCmd,pCmdStr);
    		GSSiGlobUlFree (&hCmd);
    		if (!ContinueProcessing)
    		{   
		    	hLast = pCmdStr->PrevHandle; 
		    	GSSiGlobFree (&pCmdStr->hWhile);
		    	GSSiGlobFree (&pCmdStr->hError);
	    		GSSiGlobUlFree (&CurView->FunStackHandle);
	    		CurView->FunStackHandle = hLast;
	    		SetContinueProcessing ( TRUE); 
				GSSiGlobFree (&hEmbeddedGFCommand);
    			goto NextCmd;
    		}
    		if (pCmdStr->CurFun) 
    		{
   				USHORT	CmdLim = pCmdStr->CommandLimit;

    			CurView->CurrentFunction = pCmdStr->CurFun;  
   				GlobalUnlock (CurView->FunStackHandle);
    			AddLBUTTON = FALSE;  
    			if (hWnd) 
    			{   
    				if (!ProcessGraphicsFunction (hWnd, GF_INIT, 0, CmdLim))
    				{
						GSSiGlobFree (&hEmbeddedGFCommand);
		    			goto NextCmd; 
		    		}
		    	}
			    else  
			    	ProcessGraphicsFunction2 (CurView->CurrentFunction,hWnd, GF_INIT, 0, 0);
   			    NotifyFunction (CurView,GF_ENTER_VIEWPORT);  
                if (pCmdStr->AddLBUTTON == 2)
			    	ProcessGraphicsFunction2 (CurView->CurrentFunction,hWnd, GF_USEPICKED, 0, 0);
                if (pCmdStr->AddLBUTTON == 3)
				{
					pCmdStr->AddLBUTTON = 0;
					CommandPoint = pCmdStr->Point;
			    	ProcessGraphicsFunction2 (CurView->CurrentFunction,hWnd, GF_USECMDCOR, 0, 0);
					PostMessage(hWnd, GF_CLOSE,0, 0L);	

				}
				else if (CurView->FunStackHandle)
				{
			    	pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle); 
				    pCmdStr->AddLBUTTON = AddLBUTTON; 
		    		GlobalUnlock (CurView->FunStackHandle); 
		    	}
		    	if (AddLBUTTON && HaveCurrentLBUTTON)
		    	{
		    		HaveCurrentLBUTTON=FALSE; 
		    		IgnoreLbutton = FALSE;
		    		PostMessage (hWnd,WM_LBUTTONUP,0,MAKELONG(CurrentLBUTDOWNLoc.x,CurrentLBUTDOWNLoc.y));
		    	}
		    	if (hEmbeddedGFCommand)
		    		PostMessage (hWnd,GF_ADD_EMEBEDDED_CMD,CurView->ID,(LPARAM)hEmbeddedGFCommand);

			}
    		else  
    		{
				GSSiGlobFree (&hEmbeddedGFCommand);
	    		GlobalUnlock (CurView->FunStackHandle);
	   			CurView->CurrentFunction = 0; 
    			goto NextCmd; 
    		}
    	}
    }
	else
		CurView->CurrentFunction = 0; //4/9/07
    DisplayFunctionStack();

Exit:
{
#if ENABLETRACE
GSSiExitProg (1096);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

short IsGFCmd (LPSTR str,LPCMDSTRING pCmdStr)
#if ENABLETRACE
{GSSiEnterProg (1097);
#endif
{   
	short	Cmd=atoi (str);
	LPSTR	pParen, ClosePar, lpComma, EndCmd, pCmd;
	
	pCmdStr->CommandLimit = 0;
	if ((pParen = _fstrchr (str,'(')))
	{   
		*pParen++=0; 
		if ((ClosePar = MatchLev (pParen,')')))
		{
			*ClosePar = 0;
			lpComma = MatchLev (pParen,',');  
			if (lpComma)
			{
				*lpComma++ = 0;
				pCmdStr->Prompt = PRMT_USERPROMPT;
				_fstrcpy (pCmdStr->PromptText,lpComma);
			}
			if (*pParen == '*')
				pCmdStr->CommandLimit = USHRT_MAX;
			else if (*pParen == '(') 
			{   
				pParen++;
				if ((EndCmd = MatchLev (pParen,')')))
				{   
					*EndCmd = 0;
					pCmdStr->CommandLimit = USHRT_MAX-2;
					GSSiGlobFree (&hEmbeddedGFCommand);
					hEmbeddedGFCommand = GSSiGlobAlloc (1060,GMEM_MOVEABLE,1024);
					pCmd = GlobalLock (hEmbeddedGFCommand);
					_fstrcpy (pCmd,pParen); 
					GlobalUnlock (hEmbeddedGFCommand);
				}
			}
			else if (*pParen == 'P')
				pCmdStr->AddLBUTTON = 2;
			else
			{    
				if (sscanf (pParen,"%lf %lf",&CommandPoint.x,&CommandPoint.y) != 2)
					pCmdStr->CommandLimit = atoi (pParen);
				else
				{
					pCmdStr->AddLBUTTON = 3;
					pCmdStr->Point = CommandPoint;
					pCmdStr->CommandLimit = USHRT_MAX-1;
				}
			} 
		}
	} 
	if (!_fstrncmp (str,"GF_",3))
		Cmd = GetGFCmdID (str);
	else
	{
		while (*str)
		{
			if (!isdigit (*str++))
{
#if ENABLETRACE
GSSiExitProg (1097);
#endif
				return 0;
}
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1097);
#endif
	return Cmd;
}
#if ENABLETRACE
}
#endif
}

BOOL AdjustCurLoc (LPHANDLE phCmdStr)
#if ENABLETRACE
{GSSiEnterProg (1098);
#endif
{
// checks to see if at end of block control structure (if then else; while loop)
// if not checks to see if beginning of new loop or at goto 
// returns FALSE if at end of current command string
	LPSTR	pLoc, pMem, pBeg, pWhile, pEnd, pStr;
	short	ii;   
	long	len, lWhile;
	LPCMDSTRING pCmdStr; 
	BOOL	KeepGoin; 
	HANDLE	hMem;
	HANDLE	hCmd;
	
    pCmdStr = (LPCMDSTRING)GlobalLock (*phCmdStr); 
    if (!pCmdStr)
    {
    	*phCmdStr = 0;
{
#if ENABLETRACE
GSSiExitProg (1098);
#endif
    	return FALSE;
}
    }
Begin:
	pLoc = pCmdStr->Cmd; 
	pBeg = pLoc;
	pLoc += pCmdStr->CurLoc; 
Top:
	switch (*pLoc)
	{   
		case 0:
			if (pCmdStr->hWhile)
			{ 
				BOOL	Rtn;
				BOOL	rc;

				pWhile = GlobalLock (pCmdStr->hWhile);
				hMem = GSSiGlobAlloc (1061,GMEM_MOVEABLE,4096);
				pMem = GlobalLock (hMem);
				_fstrcpy (pMem,pWhile);  
				GlobalUnlock (pCmdStr->hWhile);
				Rtn = LogicP (pMem,&rc);
				if (Rtn && !rc)
					KeepGoin = TRUE;  
				else
					KeepGoin = FALSE;
				GSSiGlobUlFree (&hMem); 
				if (KeepGoin)
				{
					pCmdStr->CurLoc = 0;
					pLoc = (LPSTR)pCmdStr;
					goto Begin;
				}
			} 
			GlobalUnlock (*phCmdStr);
{
#if ENABLETRACE
GSSiExitProg (1098);
#endif
			return FALSE; 
}
			
		case ';':
			pCmdStr->CurLoc++;
			pLoc++;
			goto Top;
		default:
			if (!_fstrnicmp (pLoc,"WHILE(",6))
			{
				pLoc += 6;
				pEnd = MatchLev (pLoc,')');
				if (!pEnd)
					goto Exit; 
				pWhile = pLoc;
				lWhile = pEnd++ - pLoc; 
				if (*pEnd != '{')
					goto Exit;   
				pLoc = pEnd;
				pLoc++;
				pEnd = MatchLev (pLoc,'}');
				if (!pEnd)
					goto Exit; 
				len = pEnd++ - pLoc;
				pCmdStr->CurLoc = pEnd - pBeg;
				GlobalUnlock (*phCmdStr);
				hCmd = GSSiGlobAlloc (1062,GHND,len+sizeof(CMDSTRING)+1);
				pCmdStr = (LPCMDSTRING)GlobalLock (hCmd);
				_fstrncpy (pCmdStr->Cmd,pLoc,(size_t)len);
				pCmdStr->EndLoc = len;
				pCmdStr->MyHandle = hCmd;  
				pCmdStr->VP = CurView->ID;
				pCmdStr->PrevHandle = *phCmdStr; 
				pCmdStr->hWhile = GSSiGlobAlloc (1063,GHND,lWhile+1); 
				pStr = GlobalLock (pCmdStr->hWhile);
				_fstrncpy (pStr,pWhile,(size_t)lWhile);
				GlobalUnlock (pCmdStr->hWhile);
				*phCmdStr = hCmd;  
				pLoc = _fstrchr (pCmdStr->Cmd,0);
				goto Top;
			} 
	Exit:
			GlobalUnlock (*phCmdStr);
{
#if ENABLETRACE
GSSiExitProg (1098);
#endif
			return TRUE;
}
	}
#if ENABLETRACE
}
#endif
}

   

BOOL TrimHighlightedItems (double MaxDist)
#if ENABLETRACE
{GSSiEnterProg (1100);
#endif
{
    long	Sequence, Refno, ToRef;
	HIGHLIGHTDATA	HighlightData;
	short	pos=BT_FIRST,type[2];  
	HPDPOINT	pPolyPoints[2];
	HANDLE	hPoly[2];    
	long	npnts[2];  
	DPOINT	DPoint, BP1,EP1;
	double	AZ, OffDist;
	MNMXCORD	Bounds; 
	long	FromPtID; 
	BOOL	rtn=FALSE;
	
	if (BT_NUM_IN_INDEX (hHighlight) < 2) 
{
#if ENABLETRACE
GSSiExitProg (1100);
#endif
		return FALSE;
}
	BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_LAST,BT_ANY,(LPSTR)&ToRef);
	BT_FIND (hHighlight,(LPSTR)&ToRef,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
	WantUnsplinedPoints = TRUE;
	if (!GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts[0],&hPoly[0]))      
		goto Exit;
    type[0] = HighlightData.PD.Type;  
    pPolyPoints[0] = (HPDPOINT)GlobalLock (hPoly[0]);
	if (type[0] == 2)  
		ExtendPoly (npnts[0],pPolyPoints[0],10000);
	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))
	{ 
		pos = BT_NEXT;
		if (Refno != ToRef)
		{ 
			if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts[1],&hPoly[1]))      
			{   
				double	PCT1,PCT2;
				
			    pPolyPoints[1] = (HPDPOINT)GlobalLock (hPoly[1]); 
			    BP1 = *pPolyPoints[1];
			    EP1 = *(pPolyPoints[1] + (npnts[1]-1));
				PickList[0]=HighlightData.PD;
			    type[1] = SysTypeFromPickType(HighlightData.PD.Type);  
		   		if (type[1] != GF_AREA && type[1] != GF_CURVE)  
		   			ExtendPoly (npnts[1],pPolyPoints[1],10000); 
				if (IntersectPolys1 (type[0],type[1],npnts[0],pPolyPoints[0],0,npnts[1],pPolyPoints[1],0,0,&PickPointBase,&SnapPoint,&PCT1,&PCT2,0))
				{   
					double	dist;
/*					double	dist1 = ldistp (PickList[0].BeginPoint,SnapPoint);
					double	dist2 = ldistp (PickList[0].EndPoint,SnapPoint);

			   		if (dist1 < dist2)
						SnapEnd = 1;
					else
						SnapEnd = 2; */  
					SnapType = type[1];
					if (PickList[0].PCT > 0.5)  
					{
						dist = ldistp (PickList[0].EndPoint,SnapPoint);
						SnapEnd = 2;   
					}
					else
					{
						dist = ldistp (PickList[0].BeginPoint,SnapPoint);
						SnapEnd = 1; 
					} 
					*pPolyPoints[1] = BP1;
					*(pPolyPoints[1] + (npnts[1]-1)) = EP1;
					Bounds.xmn = SnapPoint.x-1;
					Bounds.xmx = SnapPoint.x+1;
					Bounds.ymn = SnapPoint.y-1;
					Bounds.ymx = SnapPoint.y+1;
					LineInBoundsD (npnts[1], pPolyPoints[1],GetPolyLengthD(pPolyPoints[1],npnts[1]),
									&SnapPCT,&OffDist,&AZ,&Bounds,&DPoint,&FromPtID);
//					if (min (dist1,dist2) < MaxDist)
					if (dist < MaxDist)
						SnapPickedItem (0); 
				}
				GSSiGlobUlFree (&hPoly[1]); 
				rtn = TRUE;
			}
		}
	}  
Exit:
	GSSiGlobUlFree (&hPoly[0]);
	WantUnsplinedPoints = FALSE;

{
#if ENABLETRACE
GSSiExitProg (1100);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL MovePolyLine (DPOINT SnapPoint)
#if ENABLETRACE
{GSSiEnterProg (1101);
#endif
{	int	iPickAp, xmove, ymove;
	DWORD	i, PickedPoint;
	long	mini;
	HPPOINTS	lpPoints, lpPointsMin;
	float	factor;
	long	RefDist1, RefDist2;
	POINT	RefPoint;
	double	d,mindist;
    
    if (HiPrecis)
{
#if ENABLETRACE
GSSiExitProg (1101);
#endif
    	return (MovePolyLineD (SnapPoint));
}
	PickPoint = BasePtToFilePt(PickPointBase);
    SetPickAp(0);
	iPickAp = PickAp * WidthFactor;

//    xmove = CurTheme->Xmove * WindowToFileFactor;
//    ymove = CurTheme->Ymove * WindowToFileFactor;
    xmove = xmovePoly / BaseDistToWinDist;
    ymove = -ymovePoly / BaseDistToWinDist;

	lpPoints = lpCurPoints; 
	mindist = 33000;
	mini=-1;
	for (i=0;i<nPnts;i++,lpPoints++)
	{
		d=PointInPickArea (POINTStoPOINT(*lpPoints));
		if (d<mindist)
		{         
			mindist=d;
			mini=i;
			lpPointsMin = lpPoints;
		}
	}		
	if (mini>=0)
	{
		lpPoints=lpPointsMin;		
		PickedPoint = mini; 
		RefPoint = POINTStoPOINT(*lpPoints);
		lpPoints =lpCurPoints;
		RefDist1 = idist (RefPoint,POINTStoPOINT(*lpPoints));
		lpPoints += nPnts-1;
		RefDist2 = idist (RefPoint,POINTStoPOINT(*lpPoints));
		goto Found;
	}		
{
#if ENABLETRACE
GSSiExitProg (1101);
#endif
	return(TRUE);	
}
	
Found:lpPoints = lpCurPoints;
	for (i=0;i<nPnts;i++,lpPoints++)
	{   
		if (i == PickedPoint || MoveOnlyPickedPoint==2)
			factor = 1.0;
		else if ((MoveOnlyPickedPoint==1) || !i || i==(nPnts-1))
			factor = 0;
		else if (i <= PickedPoint && RefDist1 > 0)
			factor = 1.0 - idist (POINTStoPOINT(*lpPoints),RefPoint) /(float) RefDist1; 
		else if (i >= PickedPoint && RefDist2 > 0)
			factor = 1.0 - idist (POINTStoPOINT(*lpPoints),RefPoint) /(float) RefDist2;
		else
			factor = 0; 
		if (factor == 1.0)
			*lpPoints = POINTtoPOINTS(BasePtToFilePt (PickPointBase));
		else
		{
			lpPoints->x += xmove * factor;
			lpPoints->y += ymove * factor; 
		}
		NewBounds.xmn = min (NewBounds.xmn,lpPoints->x);
		NewBounds.ymn = min (NewBounds.ymn,lpPoints->y);
		NewBounds.xmx = max (NewBounds.xmx,lpPoints->x);
		NewBounds.ymx = max (NewBounds.ymx,lpPoints->y);
	}
{
#if ENABLETRACE
GSSiExitProg (1101);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL iInList (DWORD i,LPDWORD mini,short nMin)
{
	while (nMin--)
		if (i == mini[nMin])
			return TRUE;
	return FALSE;
}

BOOL MovePolyLineD (DPOINT SnapPoint)
#if ENABLETRACE
{GSSiEnterProg (1102);
#endif
{	int	iPickAp;
	double	xmove, ymove;
	DWORD	i, PickedPoint;
	DWORD	mini[8];
	HPDPOINT	lpPoints, lpPointsMin;
	double	factor;
	long	RefDist1, RefDist2;
	DPOINT	RefPoint;
	double	d,mindist;
	POINT	NewPoint;  
	short	nMin=0;
    
	PickPoint = BasePtToFilePt(PickPointBase);
    SetPickAp(0);
	 

    xmove = SnapPoint.x - PickPointBase.x;
    ymove = SnapPoint.y - PickPointBase.y;

	lpPoints = lpDCurPoints; 
	mindist = DBL_MAX;
	for (i=0;i<nPnts;i++,lpPoints++)
	{
		d=PointInPickAreaD (lpPoints);
		if (d<mindist)
		{         
			mindist=d; 
			nMin = 0;
			mini[nMin++]=i;
			lpPointsMin = lpPoints;
		} 
		else if (d == mindist && nMin && nMin < 8)
			mini[nMin++]=i;
	}		
	if (nMin)
	{
		PickedPoint = mini[0]; 
		RefPoint = lpDCurPoints[mini[0]];
		RefDist1 = ldistp (RefPoint,lpDCurPoints[0]);
		RefDist2 = ldistp (RefPoint,lpDCurPoints[nPnts-1]);
		goto Found;
	}		
{
#if ENABLETRACE
GSSiExitProg (1102);
#endif
	return(TRUE);	
}
	
Found:lpPoints = lpDCurPoints;
	for (i=0;i<nPnts;i++,lpPoints++)
	{   
		if (iInList(i,mini,nMin) || MoveOnlyPickedPoint==2/*move entire polygon*/)
			factor = 1.0;
		else if ((MoveOnlyPickedPoint==1) || !i || i==(nPnts-1))
			factor = 0;
		else if (i <= PickedPoint && RefDist1 > 0)
			factor = 1.0 - ldistp (*lpPoints,RefPoint) /(double) RefDist1; 
		else if (i >= PickedPoint && RefDist2 > 0)
			factor = 1.0 - ldistp (*lpPoints,RefPoint) /(double) RefDist2;
		else
			factor = 0; 
		if (factor == 1.0 && MoveOnlyPickedPoint==1)
			*lpPoints = SnapPoint;
		else
		{
			lpPoints->x += xmove * factor;
			lpPoints->y += ymove * factor; 
		}
		NewPoint = BasePtToFilePt (*lpPoints);
		NewBounds.xmn = min (NewBounds.xmn,NewPoint.x);
		NewBounds.ymn = min (NewBounds.ymn,NewPoint.y);
		NewBounds.xmx = max (NewBounds.xmx,NewPoint.x);
		NewBounds.ymx = max (NewBounds.ymx,NewPoint.y);
	}
{
#if ENABLETRACE
GSSiExitProg (1102);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
} 

DPOINT TranPointReorg (LPDPOINT pPoint,HANDLE hTran)
#if ENABLETRACE
{GSSiEnterProg (1103);
#endif
{
	DPOINT Point;
	
	if (hTran > (HANDLE)1)
		TRANS2 (pPoint->x,pPoint->y, &Point.x,&Point.y,hTran);
	else
		Point = *pPoint;
	if (hTran)
		ConvertCoord(&Point,1,3);
{
#if ENABLETRACE
GSSiExitProg (1103);
#endif
	return Point;
}
#if ENABLETRACE
}
#endif
}

void TranPointReorgFile (HPPOINT pPoint,HANDLE hTran)
#if ENABLETRACE
{GSSiEnterProg (1104);
#endif
{   
	DPOINT	DPoint, DPointNew;
	
	if (hTranCopyFile)
	{
		DPoint = CopyFilePtToBasePt (*pPoint);
		*pPoint = BasePtToFilePt (DPoint);
	}
	else
	{
		if (!hTran)
{
#if ENABLETRACE
GSSiExitProg (1104);
#endif
			return;
}
		DPoint = FilePtToBasePt (*pPoint);
		if (hTran > (HANDLE)1)
			DPointNew = TranPoint (&DPoint,hTran); 
		else
			DPointNew = DPoint;
		ConvertCoord(&DPointNew,1,3);
		*pPoint = BasePtToFilePtReorg (DPointNew);
	}
{
#if ENABLETRACE
GSSiExitProg (1104);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL ConvertFileBounds (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (1105);
#endif
{
	char		TranData[256];
	double 		XINCH[4],YINCH[4],XBASE[4],YBASE[4];
	double		RSQMIN, dist1,dist2;
	LPSTR		lpCor;
	WORD		nRead;
	short		i; 
    double	Minx, Maxx, Miny, Maxy;
	DPOINT DPoint[4],OrigPoint[4];  
	long	PrimeOffset;
    
    if (!hTranReorg)
{
#if ENABLETRACE
GSSiExitProg (1105);
#endif
    	return TRUE;
}
    GSSillseek(Fid,(LONG)-(6+12),2);
    BigRead (Fid,(HPSTR)&PrimeOffset,4);
    GSSillseek(Fid,PrimeOffset,0);
    ProcessPrimarySeg (0, 0, 0,TRUE,0,HFILE_ERROR);   
	GSSillseek(Fid,TranPointOffset,0);
	BigRead (Fid,(HPSTR)&nRead,2);
    BigRead (Fid,(HPSTR)&TranData,nRead-4); 
    lpCor =(LPSTR)  &TranData;
    XINCH[0] = dread (lpCor,16); 
    lpCor+=16;
    XINCH[2] = dread (lpCor,16);
    lpCor+=16;
    YINCH[0] = dread (lpCor,16);
    lpCor+=16;
    YINCH[1] = dread (lpCor,16);
    lpCor+=16;
    XBASE[0] = dread (lpCor,16);
    lpCor+=16;
    XBASE[2] = dread (lpCor,16);
    lpCor+=16;
    YBASE[0] = dread (lpCor,16);
    lpCor+=16;
    YBASE[1] = dread (lpCor,16);
    XINCH[1] = XINCH[0];
    XINCH[3] = XINCH[2];
    YINCH[2] = YINCH[1];
    YINCH[3] = YINCH[0];
    XBASE[1] = XBASE[0];
    XBASE[3] = XBASE[2];
    YBASE[2] = YBASE[1];
    YBASE[3] = YBASE[0];
	                              
	Minx = DBL_MAX;
	Miny = DBL_MAX;
	Maxx = -DBL_MAX;
	Maxy = -DBL_MAX;
	for (i=0;i<4;i++)
	{	                              
        DPoint[i].x = XBASE[i];
        DPoint[i].y = YBASE[i]; 
        OrigPoint[i]=DPoint[i];
		DPoint[i] = TranPointReorg (&DPoint[i],hTranReorg);            
		Minx = min (Minx,DPoint[i].x);
		Maxx = max (Maxx,DPoint[i].x);
		Miny = min (Miny,DPoint[i].y);
		Maxy = max (Maxy,DPoint[i].y);
    } 
    XBASE[0] = Minx;
    YBASE[0] = Miny;
    XBASE[1] = Minx;
    YBASE[1] = Maxy;
    XBASE[2] = Maxx;
    YBASE[2] = Maxy;
    XBASE[3] = Maxx;
    YBASE[3] = Miny;
    dist1=Maxx-Minx;
    dist2=Maxy-Miny;  
    if (dist1 > dist2)
    {  
    	XINCH[0] = -32000;
    	XINCH[2] =  32000; 
    	YINCH[1] =  IDNINT (dist2/dist1 * 32000);
    	YINCH[0] =  -YINCH[1]; 
    }
    else
    {  
    	YINCH[0] = -32000;
    	YINCH[1] =  32000; 
    	XINCH[2] =  IDNINT (dist1/dist2 * 32000);
    	XINCH[0] =  -XINCH[2];
    }
    XINCH[1]=XINCH[0];
    XINCH[3]=XINCH[2];
    YINCH[2]=YINCH[1];
    YINCH[3]=YINCH[0];
   	MinMax.xmn = IDNINT (XINCH[0]);
   	MinMax.xmx = IDNINT (XINCH[2]); 
   	MinMax.ymn = IDNINT (YINCH[0]);
   	MinMax.ymx = IDNINT (YINCH[1]);
    sprintf (&TranData[0],"%16f%16f%16f%16f%16f%16f%16f%16f",
                XINCH[0],  
                XINCH[2],  
                YINCH[0],  
                YINCH[1],  
                XBASE[0],  
                XBASE[2],  
                YBASE[0],  
                YBASE[1]); 
	GSSillseek(Fid,TranPointOffset+2,0);
    BigWrite (Fid,TranData,128,-1);   
     
    GSSillseek(Fid,(LONG)-14,2);
    BigWrite (Fid,(HPSTR)&MinMax,8,-1);
    
{
#if ENABLETRACE
GSSiExitProg (1105);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL ConvertFileCoordinates (LPSTR Name,int From,int To,int StatusID)
#if ENABLETRACE
{GSSiEnterProg (1106);
#endif
{	                                
	char		TranData[1024], SaveAltProj[MAX_PATH];
	double 		XINCH[4],YINCH[4],XBASE[4],YBASE[4];
	double		RSQMIN, dist1,dist2;
	LPSTR		lpCor;
	WORD		nRead; 
	DPOINT DPoint[4],OrigPoint[4]; 
	int	i,ii;
    LPTRANDATA TranPtr, TranPtrOld; 
    double	Minx, Maxx, Miny, Maxy;
    long	TotLen, SegLen, CurLoc;
    long	QuadOff;
    LPQUAD  pQuad;
	HPSTR	pQuadTree; 
	long	SegStart;
	HANDLE	hpltBuf;
	LPSTR	LPpltBuf;
	LPSHORT	ipnt;     
	short	desc;
    LPDESCBLOCK pDescBlock; 
	mnmxCor     MinMax;
	BOOL	rtn=FALSE; 
	BOOL	ProjLoaded=FALSE;
	double	d1, d2;     
	static	long	debugseg=816;
	BOOL	SaveDisplay = Display;
	
	Display = FALSE;
    if (!From && !*MapCopyProjection)
		goto Exit;
    if (!*Name)
		goto Exit;
    CloseMap(FALSE);  
    _fstrcpy (PltName,Name);
    if (!OpenMap(0,0))
		goto Exit;
    TotLen = _filelength (FidMap);
    hTranFileToBaseOld = GSSiGlobAlloc (1065,GHND,sizeof(TRANDATA));
    TranPtr = (LPTRANDATA) GlobalLock (hTranFileToBaseOld);
    TranPtrOld = (LPTRANDATA)GlobalLock (hTranFileToBase);
    *TranPtr = *TranPtrOld;
    GlobalUnlock (hTranFileToBaseOld);
    GlobalUnlock (hTranFileToBase);
    
	GSSillseek(FidMap,TranPointOffset,0);
	BigRead (FidMap,(HPSTR)&nRead,2);
    BigRead (FidMap,(HPSTR)&TranData,nRead-4); 
    lpCor =(LPSTR)  &TranData;
    XINCH[0] = dread (lpCor,16); 
    lpCor+=16;
    XINCH[2] = dread (lpCor,16);
    lpCor+=16;
    YINCH[0] = dread (lpCor,16);
    lpCor+=16;
    YINCH[1] = dread (lpCor,16);
    lpCor+=16;
    XBASE[0] = dread (lpCor,16);
    lpCor+=16;
    XBASE[2] = dread (lpCor,16);
    lpCor+=16;
    YBASE[0] = dread (lpCor,16);
    lpCor+=16;
    YBASE[1] = dread (lpCor,16);
    XINCH[1] = XINCH[0];
    XINCH[3] = XINCH[2];
    YINCH[2] = YINCH[1];
    YINCH[3] = YINCH[0];
    XBASE[1] = XBASE[0];
    XBASE[3] = XBASE[2];
    YBASE[2] = YBASE[1];
    YBASE[3] = YBASE[0];
	                              
	if (!From)
	{
		ProjLoaded = TRUE;
		ConvertCoordClose ();
		GetGlobalCVal ("[%ALT_PROJECTION]",SaveAltProj,0);
		SetGlobalValue("%ALT_PROJECTION",MapCopyProjection);
		if (ConvertCoordInit())
			goto RtnFalse;  
		From = 1;
		To   = 3;
	}
	DPoint[0].x = CurView->FileMNMX.xmn;
	DPoint[0].y = CurView->FileMNMX.ymn;
	DPoint[1].x = CurView->FileMNMX.xmx;
	DPoint[1].y = CurView->FileMNMX.ymx;
	d1 = ldistp (DPoint[0],DPoint[1]);
	ConvertCoord(&DPoint[0],From,To);
	ConvertCoord(&DPoint[1],From,To);
	d2 = ldistp (DPoint[0],DPoint[1]); 
	DistanceConversionFactor = d2/d1;
	Minx = DBL_MAX;
	Miny = DBL_MAX;
	Maxx = -DBL_MAX;
	Maxy = -DBL_MAX;
	for (i=0;i<4;i++)
	{	                              
        DPoint[i].x = XBASE[i];
        DPoint[i].y = YBASE[i]; 
        OrigPoint[i]=DPoint[i];
		ConvertCoord(&DPoint[i],From,To);
		Minx = min (Minx,DPoint[i].x);
		Maxx = max (Maxx,DPoint[i].x);
		Miny = min (Miny,DPoint[i].y);
		Maxy = max (Maxy,DPoint[i].y);
    } 
    XBASE[0] = Minx;
    YBASE[0] = Miny;
    XBASE[1] = Minx;
    YBASE[1] = Maxy;
    XBASE[2] = Maxx;
    YBASE[2] = Maxy;
    XBASE[3] = Maxx;
    YBASE[3] = Miny;
    dist1=Maxx-Minx;
    dist2=Maxy-Miny;  
    if (dist1 > dist2)
    {  
    	XINCH[0] = -32000;
    	XINCH[2] =  32000; 
    	YINCH[1] =  IDNINT (dist2/dist1 * 32000);
    	YINCH[0] =  -YINCH[1]; 
    }
    else
    {  
    	YINCH[0] = -32000;
    	YINCH[1] =  32000; 
    	XINCH[2] =  IDNINT (dist1/dist2 * 32000);
    	XINCH[0] =  -XINCH[2];
    }
    XINCH[1]=XINCH[0];
    XINCH[3]=XINCH[2];
    YINCH[2]=YINCH[1];
    YINCH[3]=YINCH[0];
   	MinMax.xmn = IDNINT (XINCH[0]);
   	MinMax.xmx = IDNINT (XINCH[2]); 
   	MinMax.ymn = IDNINT (YINCH[0]);
   	MinMax.ymx = IDNINT (YINCH[1]);
    sprintf (TranData,"%16f%16f%16f%16f%16f%16f%16f%16f",
                XINCH[0],  
                XINCH[2],  
                YINCH[0],  
                YINCH[1],  
                XBASE[0],  
                XBASE[2],  
                YBASE[0],  
                YBASE[1]); 
	GSSillseek(FidMap,TranPointOffset+2,0);
    BigWrite (FidMap,TranData,128,-1);   
     
    GSSillseek(FidMap,(LONG)-14,2);
    BigWrite (FidMap,(HPSTR)&MinMax,8,-1);
    
    CloseMap(FALSE);
    OpenMap(0,0); 
	CurLoc = 0;    

    pQuadTree = (HPSTR)GlobalLock (hQuadTree);

    QuadOff   = 0; 
    while (QuadOff<NumQuadSegs && ContinueProcessing) 
    {
		pQuad     = (LPQUAD) (pQuadTree + ((long)(QuadOff))*LenQuadSeg);
		ConvertMinMax (&pQuad->MinMax,hTranFileToBaseOld,hTranBaseToFile,From,To);  
		
		for (i=0;i<MaxQuadType;i++)
		{
        	if (pQuad->TypeOffset[i]>=0)
        	{   
        		idescblock = 0;
    NextDescBlock:
        		if (hDescBlock)
        		{
				    pDescBlock = (LPDESCBLOCK)GlobalLock (hDescBlock);
				    pDescBlock += ((long)(QuadOff+1) * (long)MaxQuadType * (long)NumDescBlocks + (long)i* (long)NumDescBlocks + idescblock++); 
				    desc = pDescBlock->Desc;
				    CurrentSeg = pDescBlock->Offset;
				    GlobalUnlock (hDescBlock); 
				    if (desc <= 0)
				    {
				        idescblock=100;  
						if (desc < 0) 
							CurrentSeg = -1; 
				    }
        		}
        		else
		        	CurrentSeg=pQuad->TypeOffset[i]; 
		        while (CurrentSeg >= 0)
		        {   
		        	if (CurrentSeg == debugseg)
		        		ii=1;
				    GSSillseek (FidMap,CurrentSeg,0);    
				    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
				    ContinuationOffset = -1;  
			        if (nBytes)
			        {  
					    SegStart = GSSillseek (FidMap,0,1);
					    hpltBuf = GSSiGlobAlloc (1066,GMEM_MOVEABLE,(DWORD)nBytes);
					    LPpltBuf = GlobalLock (hpltBuf); 
					    nRead = BigRead (FidMap,LPpltBuf,nBytes);  
					    if (nRead != nBytes) 
					    {   
					       	ProcessInvalidRecord (0,0,2);
						    GlobalUnlock (hQuadTree);
					        goto RtnFalse;
					    }  
					    ipnt = (LPSHORT) LPpltBuf; 
					    
/*					    hTranReorg = KeepTranFileToBase; 
						RecomputeMinMax (&MinMax,pBuf,lUpdateBuf,0,0); 
						hTranReorg = 0;  */
					    
					    
						ConvertPoly (ipnt,hTranFileToBaseOld,hTranBaseToFile,LPpltBuf,From,To);
						CurLoc += nBytes+2;
						if (StatusID == 1)
							StatusWindowUpdate ("Map Coordinate Conversion",PltName, TotLen, CurLoc);
						else if (StatusID == 2)
							StatusWindowUpdate2 (PltName, TotLen, CurLoc);
				        GSSillseek (FidMap,SegStart,0);
				        BigWrite (FidMap,LPpltBuf,nBytes-2,-1);
						GSSiGlobUlFree (&hpltBuf);
	    			}
	    			else
	    				ii=1;
			        CurrentSeg=ContinuationOffset;
		        }
		        if (hDescBlock && idescblock < NumDescBlocks)
		        	goto NextDescBlock;
		    }
		}
		QuadOff++;
	} 
	rtn = TRUE;
    GlobalUnlock (hQuadTree);
RtnFalse:
	
	GSSiGlobFree (&hTranFileToBaseOld);
    CloseMap(TRUE);
    
	if (ProjLoaded)
	{
		ConvertCoordClose ();
		SetGlobalValue("%ALT_PROJECTION",SaveAltProj);
	}
Exit:
	Display = SaveDisplay;
{
#if ENABLETRACE
GSSiExitProg (1106);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL EnlargeScreen (short factor,short width)
#if ENABLETRACE
{GSSiEnterProg (1108);
#endif
{   
	HBITMAP	hBM; 
	POINT	Point;  
	float	RSQMIN;
	double	X1[4], Y1[4], X2[4], Y2[4];
	
	if (CurView)
		RestoreScreen2 (CurView->hDC, hEnlargedScreen,0,FALSE);
	DestroySavedScreen (&hEnlargedScreen,0);
    CloseTRANS2 (&hTranEnlargeScreen);
	if (!factor)
{
#if ENABLETRACE
GSSiExitProg (1108);
#endif
		return TRUE;
}
	GetCursorPos (&Point);
	ScreenToClient (CurView->hWnd,&Point);
	EnlargeRectOrig.left = Point.x - width;
	EnlargeRectOrig.right = Point.x + width;
	EnlargeRectOrig.top = Point.y - width;
	EnlargeRectOrig.bottom = Point.y + width; 
	EnlargeRectNew.left = Point.x - width*factor;
	EnlargeRectNew.right = Point.x + width*factor;
	EnlargeRectNew.top = Point.y - width*factor;
	EnlargeRectNew.bottom = Point.y + width*factor; 
	hBM = SaveScreen (CurView->hDC,EnlargeRectOrig);
	hEnlargedScreen = SaveScreen2 (CurView->hWnd,CurView->hDC, EnlargeRectNew,CurView,0);
	RestoreScreenRect (CurView->hDC,hBM,EnlargeRectOrig,EnlargeRectNew);
	DeleteObject (hBM);
	X1[0]=X1[1]=EnlargeRectNew.left;
	X1[2]=X1[3]=EnlargeRectNew.right;
	Y1[0]=Y1[3]=EnlargeRectNew.bottom;
	Y1[1]=Y1[2]=EnlargeRectNew.top;
	X2[0]=X2[1]=EnlargeRectOrig.left;
	X2[2]=X2[3]=EnlargeRectOrig.right;
	Y2[0]=Y2[3]=EnlargeRectOrig.bottom;
	Y2[1]=Y2[2]=EnlargeRectOrig.top;
    hTranEnlargeScreen = STRAN2 (1627,X1,Y1,X2,Y2,4,&RSQMIN,1,0);
{
#if ENABLETRACE
GSSiExitProg (1108);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

DPOINT EnlargedPoint (POINT Point)
#if ENABLETRACE
{GSSiEnterProg (1109);
#endif
{   
	DPOINT	DPoint;
	
	DPoint.x = Point.x;
	DPoint.y = Point.y;
	
	if (!hEnlargedScreen)
{
#if ENABLETRACE
GSSiExitProg (1109);
#endif
		return DPoint;
}
	if (!PtInRect (&EnlargeRectNew,Point))
{
#if ENABLETRACE
GSSiExitProg (1109);
#endif
		return DPoint;
}
    TRANS2 (DPoint.x,DPoint.y,&DPoint.x,&DPoint.y,hTranEnlargeScreen);  
{
#if ENABLETRACE
GSSiExitProg (1109);
#endif
	return DPoint;
}
#if ENABLETRACE
}
#endif
}

 

BOOL LoadNewData (LPSTR Dir)
{
	return FALSE;
}
/*
#if ENABLETRACE
{GSSiEnterProg (1111);
#endif
{
#if WIN32
    struct  _finddata_t FileInfo; 
#else
    struct  _find_t FileInfo; 
#endif
    char    str[128], CmdFile[128];
    short       i,ii, rtn;
    int st;  
    
    
    sprintf (CmdFile,"%s\\first.txt",Dir);   
    SetGlobalValue ("%NEWDATADIR",Dir);
    if (ExistFile (CmdFile))
		rtn = ProcessMacroFile (CmdFile,0,0,0);
    _fstrcpy (str,Dir);
    _fstrcat (str,"\\*.*");
#if WIN32
    st = _findfirst (str,&FileInfo);
#else
    st = _dos_findfirst (str,_A_NORMAL|_A_SUBDIR,&FileInfo);
#endif
    while (!st)
    {   
        if (FileInfo.name[0] != '.')
        {
            if (FileInfo.attrib == _A_SUBDIR) 
            {
                sprintf (CmdFile,"%s\\%s",Dir,FileInfo.name);   
                SetGlobalValue ("%NEWDATADIR",CmdFile);
                _fstrcat (CmdFile,"\\cmd.txt");
                if (ExistFile (CmdFile))
					rtn = ProcessMacroFile (CmdFile,0,0,0);
            }
            else 
            { 
            }
        }
#if WIN32
        st = _findnext (st,&FileInfo);
#else
        st = _dos_findnext (&FileInfo);
#endif
    }
    sprintf (CmdFile,"%s\\last.txt",Dir);   
    SetGlobalValue ("%NEWDATADIR",Dir);
    if (ExistFile (CmdFile))
		rtn = ProcessMacroFile (CmdFile,0,0,0);
     
{
#if ENABLETRACE
GSSiExitProg (1111);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
} 

*/  

  
 

BOOL UpdateEmbeddedStreetNums (short Item,LPLONG StreetNums)
#if ENABLETRACE
{GSSiEnterProg (1115);
#endif
{   
	BOOL	rtn=FALSE;
	
	CurSNamesLoc = 0;
	ProcessPickedItem (0,FALSE);        		
	if (CurSNamesLoc)
   	{   
		if (OpenMap (CurView->hWnd,0))
		{   
			short	l,ID;
						
			GSSillseek (FidMap,CurSNamesLoc,0);
			BigRead (FidMap,(HPSTR)&ID,2);
			if (ID == 10)
			{
				BigWrite (FidMap,(HPSTR)StreetNums,16,-1);
				rtn = TRUE;
			}
			CloseMap (TRUE);
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1115);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

 

   
void SetCurAZ (double AZ)
#if ENABLETRACE
{GSSiEnterProg (1117);
#endif
{
	if (AZLocked)
{
#if ENABLETRACE
GSSiExitProg (1117);
#endif
		return;
}
	CurrentAZ = AZ;
	CreateDigCursor (CurView->hDC);
{
#if ENABLETRACE
GSSiExitProg (1117);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

POINT AdjustPoint (HWND hWnd,POINT StartPoint,POINT LastPoint,short Xmove,short Ymove)
#if ENABLETRACE
{GSSiEnterProg (1118);
#endif
{   POINT   NewPoint;
    RECT    Rect;
    float   rect_width, rect_height, factor; 
    short     ZoomOpt=1; /* if 1 only allow move in top left to bot right and vicaversa*/
             
    
    Rect = CurView->ZBRect;

    rect_width = (float)(Rect.right - Rect.left);
    rect_height = (float)(Rect.bottom - Rect.top); 
    if (!rect_height || !rect_width)
{
#if ENABLETRACE
GSSiExitProg (1118);
#endif
    	return (LastPoint);
}
    factor = rect_width/rect_height;
    if (abs (Xmove) > abs (Ymove))
    {   NewPoint.x = LastPoint.x + Xmove;
        if (NewPoint.x == StartPoint.x)
            NewPoint = StartPoint; 
        else if (ZoomOpt)
            NewPoint.y = (long)(StartPoint.y + (NewPoint.x-StartPoint.x)/factor);
        else
        {   if (LastPoint.y>StartPoint.y)
                NewPoint.y = (long)(StartPoint.y + abs(NewPoint.x-StartPoint.x)/factor);
            else
                NewPoint.y = (long)(StartPoint.y - abs(NewPoint.x-StartPoint.x)/factor);
        }
    }
    else
    {   
        NewPoint.y = LastPoint.y + Ymove;
        if (NewPoint.y == StartPoint.y)
            NewPoint = StartPoint;
        else if (ZoomOpt)
            NewPoint.x = (long)(StartPoint.x + (NewPoint.y-StartPoint.y)*factor);
        else
        {   if (LastPoint.x>StartPoint.x)
                NewPoint.x = (long)(StartPoint.x + abs(NewPoint.y-StartPoint.y)*factor);
            else
                NewPoint.x = (long)(StartPoint.x - abs(NewPoint.y-StartPoint.y)*factor);
        }
    }

{
#if ENABLETRACE
GSSiExitProg (1118);
#endif
    return (NewPoint);
}
#if ENABLETRACE
}
#endif
}

void ConvertIndexV1ToV2(LPSTR InName)
#if ENABLETRACE
{GSSiEnterProg (1119);
#endif
{
    OFSTRUCTGM    OFStruct; 
    short     NumFiles;
    HFILE   FidIndex,FidIndexNew;
    long    IndexLen;
    FILEINDEX   Index;
    LPSTR	lpSlash;
    MNMXCORD    FileBounds; 
    FILEINDEXENTRY FIEntry;
    long    LastHeadOffset, CurOffset,ii, FirstIndexLoc;
    short     ifile=0;
    short     Version=2, OldNameLen, NewNameLen;
    long    Signature=80251; 
    char    NewName[128], OldName[128], Name[128], TempName[128];
    
    _fmemset (&Index,0,STOREDINDEXLENGTH);
    _fstrcpy (Name,InName);        
    ExpandText(Name);
    FidIndex = GSSiOpenFile (Name,(LPOFSTRUCTGM) &OFStruct,OF_READ);
    _fstrcpy (NewName,Name);
    _fstrcat (NewName,".new");
    FidIndexNew = GSSiOpenFile(NewName,&OFStruct,OF_CREATE);
    DBoundsInit (&FileBounds);
    BigWrite (FidIndexNew,(char *)&FileBounds,sizeof(MNMXCORD),-1);  
    FirstIndexLoc = GSSillseek (FidIndexNew,0,1);   
    BigRead (FidIndex,&Index.Type,2);
    BigRead (FidIndex,(HPSTR)&NumFiles,2);
    BigRead (FidIndex,(HPSTR)&IndexLen,4); 
    BigRead (FidIndex,(HPSTR)&Index.OrthoRes,8);  
    if (Index.Type != 5)
        Index.OrthoRes = 0;
Next: 
	DBoundsInit (&Index.Bounds);   
    Index.NumFiles=0;
    Index.Length=0; 
    LastHeadOffset = GSSillseek(FidIndexNew,0,1);
    BigWrite (FidIndexNew,(char *)&Index,STOREDINDEXLENGTH,-1);
    while (Index.Length < MAXINDEXLENGTH)
    {
        BigRead (FidIndex,(HPSTR)&FIEntry.Len,2); 
        BigRead (FidIndex,(HPSTR)&FIEntry.BMWidth,FIEntry.Len-2);
        AddMinMaxD (&Index.Bounds,&FIEntry.Bounds); 
        OldNameLen = _fstrlen (FIEntry.Name);
        lpSlash = _fstrrchr (FIEntry.Name,'\\');
        if (lpSlash)
        {  
        	lpSlash++;
        	_fstrcpy (TempName,lpSlash);
        	NewNameLen = _fstrlen (TempName); 
        	FIEntry.Len -= (OldNameLen-NewNameLen);
        	_fstrcpy (FIEntry.Name,TempName);
        }
        
        Index.Length+=FIEntry.Len;
        BigWrite (FidIndexNew,(char *)&FIEntry,FIEntry.Len,-1); 
        ifile++; 
        Index.NumFiles++;
        if (ifile == NumFiles) goto Exit;
    }
Exit:
    CurOffset = GSSillseek (FidIndexNew,0,1);
   	Index.NextHeaderOffset = CurOffset; 
    GSSillseek (FidIndexNew,LastHeadOffset,0);
    BigWrite (FidIndexNew,(char *)&Index,STOREDINDEXLENGTH,-1);
    GSSillseek (FidIndexNew,CurOffset,0);
	AddMinMaxD (&FileBounds,&Index.Bounds);
    if (ifile < NumFiles)                          
        goto Next; 
    BigWrite(FidIndexNew,(char *)&Signature,4,-1);
    BigWrite(FidIndexNew,(char *)&Version,2,-1);
    GSSillseek (FidIndexNew,0,0);
    BigWrite (FidIndexNew,(char *)&FileBounds,sizeof(MNMXCORD),-1);
    GSSillseek (FidIndexNew,FirstIndexLoc,0);
    BigRead (FidIndexNew,(char *)&Index,STOREDINDEXLENGTH);
    Index.EndOffset = CurOffset;
    GSSillseek (FidIndexNew,FirstIndexLoc,0);
    BigWrite (FidIndexNew,(char *)&Index,STOREDINDEXLENGTH,-1);
    GSSiClose2 (&FidIndexNew);
    GSSiClose2 (&FidIndex);
    _fstrcpy (OldName,Name);
    _fstrcat (OldName,".v01");   
    GSSiRemove (OldName);
    GSSiRename (Name,OldName);
    GSSiRename (NewName,Name);    
{
#if ENABLETRACE
GSSiExitProg (1119);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 
                                                       
BOOL GetTypeBounds (short Item, LPMNMXCORD pBounds,short Type)
#if ENABLETRACE
{GSSiEnterProg (1120);
#endif
{   
	HPSTR	pBuf;
	mnmxCor	MinMax; 
	BOOL	rtn=FALSE; 
	
    if (!GetPickName (Item))
{
#if ENABLETRACE
GSSiExitProg (1120);
#endif
        return FALSE;
}
    _fstrcpy (PltName,PickName);
	CopyRec = TRUE;
	hUpdateBuf = GSSiGlobAlloc (1067,GMEM_MOVEABLE,MAXREORGBUF); 
	lUpdateBuf = 0;
	ProcessPickedItem (Item,FALSE); 
	CopyRec = FALSE;  
	if (!OpenMap ((HWND)1,0))
	{
	    GSSiGlobFree (&hUpdateBuf);
{
#if ENABLETRACE
GSSiExitProg (1120);
#endif
		return(FALSE); 
}
	}
	pBuf = GlobalLock (hUpdateBuf);     		
    pBuf += lUpdateBuf;  
    *pBuf++ = 0;
    *pBuf = 0;
	GlobalUnlock (hUpdateBuf);
    pBuf = GlobalLock (hUpdateBuf);  
	rtn = RecomputeMinMax (&MinMax,pBuf,lUpdateBuf,Type,0); 
	GSSiGlobUlFree (&hUpdateBuf);
	GetItemMinMax (&MinMax,pBounds);   
	CloseMap (FALSE);
{
#if ENABLETRACE
GSSiExitProg (1120);
#endif
 	return rtn;
}
#if ENABLETRACE
}
#endif
}
BOOL SetBackgroundArea (int item,COLORREF color)
{
	BOOL rtn=FALSE;
	int	 nPnts, nPoly;
	HANDLE	hPoly=0, hPolyPartLen=0;
	int	Size;

	nPoly = GetPolyPointsWithParts ((LPPICKDATAHEADER)&PickList[item],&nPnts,&hPoly,&hPolyPartLen);
	if (nPoly)
	{
		LPMNMXCORD	pBounds = (LPMNMXCORD)GlobalLock (hPoly);
		LPDPOINT	pPoints = (LPDPOINT)(pBounds + 1);
		LPDPOINT	pBKA;

		CurView->numBackgroundAreaParts = nPoly;
		CurView->numBackgroundAreaPoints = nPnts;
		CurView->BackgroundAreaColor = color;
		Size = CurView->numBackgroundAreaPoints * sizeof(DPOINT) + (CurView->numBackgroundAreaParts+1) * sizeof(int);   
		CurView->hBackgroundArea = GSSiGlobAlloc (1763,GMEM_MOVEABLE,Size);
		pBKA = GlobalLock (CurView->hBackgroundArea);
		hmemmove (pBKA,pPoints,CurView->numBackgroundAreaPoints * sizeof(DPOINT));
		GSSiGlobUlFree (&hPoly);
		if (CurView->numBackgroundAreaParts>1)
		{
			HPBYTE	pByte = GlobalLock (CurView->hBackgroundArea); 
			LPINT	pPolyParts = GlobalLock (hPolyPartLen);
		
			pByte += CurView->numBackgroundAreaPoints * sizeof(DPOINT);
			hmemmove ((HPSTR)pByte,(HPSTR)pPolyParts,(CurView->numBackgroundAreaParts+1)*sizeof(int));
			GSSiGlobUlFree (&hPolyPartLen);
			GlobalUnlock (CurView->hBackgroundArea); 
		 }
		 GlobalUnlock (CurView->hBackgroundArea); 
		 rtn = TRUE;
	}
	return rtn;
}

BOOL SetMaskArea(int Item,double OffsetDist,short opt)
#if ENABLETRACE
{GSSiEnterProg (1121);
#endif
{   
	LPTHEME		pTheme, SaveTheme; 
	LPVIEWPORT	SaveVP;  
   	HPDPOINT	pPoint1, pPoint2;
   	HPMNMXCORD	pBounds1, pBounds2;
   	short		nParts,nareas; 
	LPINT		pPolyParts; 
	long		Size;
	
	if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (1121);
#endif
		return FALSE;
}
	SaveVP = CurView;
	ClearMaskArea();
	if (!GetMaskArea)
{
#if ENABLETRACE
GSSiExitProg (1121);
#endif
		return FALSE;
}
    if (OffsetDist)
    {   
    	long	np; 
    	HANDLE	hPoints; 
    	DPOINT	CenterPoint=MinMaxMidPointD(&PickList[Item].Rect);
    	
    	CurView->MaskAreaRefno = 0;
    	CurView->NewBounds = PickList[Item].Rect;
	    SetBoundsRect2 (CurView->DrawRect,CurView->hDC);   //to set basetowin tran for CURVPLT
        switch (opt)
        {
        	case 1:
				hPoints=OffsetPickedArea2 (Item,OffsetDist,&np);
				break;
			case 2:
    			hPoints = CreateCirclePoly (CenterPoint,OffsetDist,&np,BaseDistToWinDist/4);   
    			break;
		}
		if (hPoints)
		{
			CurView->NumMaskPoints = np;
			CurView->hMaskArea = GSSiGlobAlloc (1068,GMEM_MOVEABLE,np*sizeof(DPOINT)+sizeof(MNMXCORD)); 
			CurView->NumMaskAreaParts = 0;
			pBounds1 = pBounds2 = (LPMNMXCORD)GlobalLock (CurView->hMaskArea);       
			pPoint1 = (HPDPOINT)GlobalLock (hPoints);
			pBounds2++;
			pPoint2 = (HPDPOINT)pBounds2; 
			DBoundsInit (pBounds1);    
			while (np--)
			{
				*pPoint2 = *pPoint1++;
				AddDPointToMinMax (pPoint2++,pBounds1);
			}
			GlobalUnlock (CurView->hMaskArea);
			GSSiGlobUlFree (&hPoints); 
{
#if ENABLETRACE
GSSiExitProg (1121);
#endif
			return TRUE;                
}
		}
		else
{
#if ENABLETRACE
GSSiExitProg (1121);
#endif
			return FALSE;
}
   	}
   	if (PickList[Item].Type != 3)
{
#if ENABLETRACE
GSSiExitProg (1121);
#endif
   		return FALSE;    
}
	SetConfig (PickList[Item].ConfigID);
    SetViewport (PickList[Item].ViewID);
   	SaveTheme = CurTheme;
	pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME); 
	CurView->PassID = 4;
	IgnoreBounds = TRUE;
	ProcessSelectedTheme = CurView->NumThemes;
	ProcessPickedItem (Item,FALSE); 
	ProcessSelectedTheme = 0;
	IgnoreBounds = FALSE;    		
    DeleteTheme (pTheme);
    CurTheme = SaveTheme;
	SetCurView ( SaveVP);   
	if (!(nParts = GetSavedPolys ()))
{
#if ENABLETRACE
GSSiExitProg (1121);
#endif
    	return FALSE;
}
	    		    
    if (hSavePolyParts)
    {
    	pPolyParts = (LPINT)GlobalLock (hSavePolyParts);
    	nareas = (*pPolyParts++)-1; 
    }
    else
        nareas=0; 
    CurView->NumMaskPoints = nSavePoly;
    CurView->NumMaskAreaParts = nareas;
    Size = sizeof(MNMXCORD) + (long)CurView->NumMaskPoints * sizeof(DPOINT) + (CurView->NumMaskAreaParts+1) * sizeof(int);   
    CurView->hMaskArea = GSSiGlobAlloc (0,GMEM_MOVEABLE,Size);
    CurView->MaskAreaRefno = PickList[Item].Refno;
	pBounds1 = (LPMNMXCORD)GlobalLock (hSavePoly);       
	pBounds2 = (LPMNMXCORD)GlobalLock (CurView->hMaskArea);       
    Size = sizeof(MNMXCORD) + (long)CurView->NumMaskPoints * sizeof(DPOINT);   
	hmemmove ((HPSTR)pBounds2,(HPSTR)pBounds1,Size);  
	if (hSavePolyParts)
	{
		HPBYTE	pByte = (HPBYTE)pBounds2;  
		
		pByte += Size;
		hmemmove ((HPSTR)pByte,(HPSTR)pPolyParts,(nareas+1)*sizeof(int));
		GlobalUnlock (hSavePolyParts);
	}
	GlobalUnlock (CurView->hMaskArea);
    GlobalUnlock (hSavePoly);
	DestroySavedPolys ();
{
#if ENABLETRACE
GSSiExitProg (1121);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

void ClearMaskArea (void)
#if ENABLETRACE
{GSSiEnterProg (1122);
#endif
{   
	if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (1122);
#endif
		return;
}
	GSSiGlobFree (&CurView->hMaskArea);
	GSSiGlobFree (&CurView->hMaskAccelerator[0]);
	GSSiGlobFree (&CurView->hMaskAccelerator[1]);
	GSSiGlobFree (&CurView->hMaskAccelerator[2]);
	CurView->NumMaskPoints = 0;
	CurView->NumMaskAreaParts = 0;
	 
{
#if ENABLETRACE
GSSiExitProg (1122);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void ClearBackgroundArea (void)
#if ENABLETRACE
{GSSiEnterProg (1122);
#endif
{   
	if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (1122);
#endif
		return;
}
	GSSiGlobFree (&CurView->hBackgroundArea);
	CurView->numBackgroundAreaPoints = 0;
	CurView->numBackgroundAreaParts = 0;
{
#if ENABLETRACE
GSSiExitProg (1122);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

double SetMaskSizeVar (void)
#if ENABLETRACE
{GSSiEnterProg (1123);
#endif
{
 int	i;
 int	Nump; 
 double	size,Perim,Area=0;
 HPDPOINT	lpDPoint;
 LPMNMXCORD	lpRect; 
 HANDLE	hPoly;
    
	if (!CurView->hMaskArea) 
{
#if ENABLETRACE
GSSiExitProg (1123);
#endif
    	return 0;
}
    Nump = CurView->NumMaskPoints;  
    lpRect = (LPMNMXCORD) GlobalLock (CurView->hMaskArea);        
    if (!lpRect)
    {
    	CurView->hMaskArea = 0; 
    	CurView->NumMaskPoints = 0;
{
#if ENABLETRACE
GSSiExitProg (1123);
#endif
    	return 0;
}
    }	
	lpRect++;
	lpDPoint = (HPDPOINT) lpRect;
	size = ComputeAreaAreaD (lpDPoint,Nump,&Perim);
	GlobalUnlock (CurView->hMaskArea);
	Area = ConvertArea (size,OutAreaUnits);  
	SetGlobalValueReal ("%MASKSIZE",Area);
{
#if ENABLETRACE
GSSiExitProg (1123);
#endif
	return Area;
}
#if ENABLETRACE
}
#endif
}   

BOOL CreateQuantitiesFile (LPSTR InName,BOOL LoadFromHLT,LPSTR ThemeVPName)
#if ENABLETRACE
{GSSiEnterProg (1126);
#endif
{
	short		pos=BT_FIRST;
    BTVARDESC  *pVars;  
    short       NumFields, Reclen, len;
    GWDHEADER16 GWDHead; 
	GWDHEADER	GWDHead32;
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;
	HFILE       FidData;
	short ibeg,NumVars,i;
    OFSTRUCTGM    OFStruct;
    GWFLDINFO FldInfo; 
    char	PrimeIndex[MAX_PATH], Name[MAX_PATH], SymName[66], ThemeID[80];
    LPSTR	lpDot, lpEnd;
    HIGHLIGHTDATA	HighlightData;  
    LPLONG		SymCountP, SymCountL, SymCountA, SymCountT;
    LPDOUBLE	SymLength, SymArea;   
    HANDLE		hSymCounts,hSymVals, hHighlightDB=0;
    long	Refno, Offset; 
    short	ii;
    double	tol=0.01;
	LPSTR	pData,pHeader;
	LPTHEME	pTheme, SaveTheme=CurTheme;
       
typedef struct	{
					char	SymName[64];
					char	UofM[6];
					long	Count;
					double	Quantity;
				}	QUANTITIES;
typedef QUANTITIES	FAR	*LPQUANTITIES;   
LPQUANTITIES	pQuan;
   
    _fstrcpy (Name,InName); 
    if (!_fstrrchr (Name,'.'))
    	_fstrcat (Name,".gmd");
    ExpandText (Name); 
    _fstrcpy (PrimeIndex,Name);
    lpDot = _fstrrchr (PrimeIndex,'.');
    *lpDot = 0;
    _fstrcat (lpDot,".in1");
    lpGWDHead = &GWDHead32; 
    
    FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE);   
    _fmemset (&GWDHead,0,sizeof(GWDHEADER16));
    GWDHead.NumIndex=1;
    GWDHead.Version=1;
    GWDHead.NumIndexFields[0]=2;
    GWDHead.IndexFields[0][0]=0;
    GWDHead.IndexFields[0][1]=1;
    BigWrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER16),-1);
    ibeg = 0;

    FldInfo.Len = 64;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"SYMBOLNAME");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 6;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"UNITOFMEASURE");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"COUNT");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;
    
    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"QUANTITY");
    BigWrite (FidData,(char *)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;
    
	GWDHead.Reclen=ibeg; 
	GWDHead.TimeStamp = time(0);
	GSSillseek (FidData,0,0);
	BigWrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER16),-1);
	GSSillseek (FidData,0,2);
	                 
	NumVars = 2;
	            
	hVars = LocalAlloc (LHND,NumVars * sizeof(BTVARDESC));
	pVars =(LPBTVARDESC) LocalLock(hVars);
	            
	pVars[0].BT_VARLEN=64;
	pVars[0].BT_VARTYP=BT_CHAR;
	pVars[0].BT_VAROFF=0;
	pVars[1].BT_VARLEN=6;
	pVars[1].BT_VARTYP=BT_CHAR;
	pVars[1].BT_VAROFF=64;
	BT_CREATE (PrimeIndex, 4, FALSE, NumVars, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
	LocalUnlock(hVars);
	LocalFree(hVars); 
	GSSiClose2 (&FidData);
	hDB = OpenGWDatabase (Name,BT_WRITE); 
	CloseGWDatabase (hDB); 
	if (!LoadFromHLT)
{
#if ENABLETRACE
GSSiExitProg (1126);
#endif
		return TRUE;
}
	if (*ThemeVPName)
	{
    	short iview;
    	
	    for (iview=0;iview<*pNumViewports;iview++)
	    {   
        	if (!_fstricmp (ThemeVPName,pViewports[iview]->Name)) 
        	{
        		if (pViewports[iview]->pTheme)
        		{   
        			CurTheme = pViewports[iview]->pTheme;
        			if (OpenThemeHighlightFile (BT_READ))
        			{    
						pTheme = CurTheme;
				    	CurTheme = SaveTheme;
			    		goto Next;
			    	} 
			    	CurTheme = SaveTheme;
        		}
        	}
        }
{
#if ENABLETRACE
GSSiExitProg (1126);
#endif
		return FALSE;
}
	}
	else
		hHighlightDB = hHighlight;
Next:
	hSymVals = GSSiGlobAlloc (1070,GHND,3201*2*sizeof(double));
	hSymCounts = GSSiGlobAlloc (1071,GHND,3201*4*sizeof(long));
	
	SymCountP = (LPLONG)GlobalLock (hSymCounts);
	SymCountL = SymCountP + 3201;
	SymCountA = SymCountL + 3201;
	SymCountT = SymCountA + 3201;
	SymLength = (LPDOUBLE)GlobalLock (hSymVals);
	SymArea   = SymLength + 3201;
	if (*ThemeVPName)
	{
		while (!BT_FIND (pTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,pos,BT_ANY,(LPSTR)&ThemeHighlightData))
		{   
			pos = BT_NEXT; 
	//		if (fabs (HighlightData.PD.Length-GetUMLength(Refno)) > tol)
	//			ii=1;
			switch (PickTypeFromSysType (ThemeHighlightData.Type))
			{
				case 1:
					SymCountP[ThemeHighlightKey.Class]++;  
					break;
				case 2:
				case 5:
					SymCountL[ThemeHighlightKey.Class]++;  
					SymLength[ThemeHighlightKey.Class]+=ThemeHighlightData.Length;
					break;
				case 3:  
					SymCountA[ThemeHighlightKey.Class]++;  
					SymArea[ThemeHighlightKey.Class]+=ThemeHighlightData.Area;
					break;
				case 4:
					SymCountT[ThemeHighlightKey.Class]++;   
					break;
			}
		}
		CurTheme = pTheme;
		CloseThemeHighlightFile ();
    	CurTheme = SaveTheme;
	}
	else
	{
		while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))
		{   
			pos = BT_NEXT; 
	//		if (fabs (HighlightData.PD.Length-GetUMLength(Refno)) > tol)
	//			ii=1;
			switch (HighlightData.PD.Type)
			{
				case 1:
					SymCountP[HighlightData.PD.Desc]++;  
					break;
				case 2:
				case 5:
					SymCountL[HighlightData.PD.Desc]++;  
					SymLength[HighlightData.PD.Desc]+=HighlightData.PD.Length;
					break;
				case 3:  
					SymCountA[HighlightData.PD.Desc]++;  
					SymArea[HighlightData.PD.Desc]+=HighlightData.PD.Area;
					break;
				case 4:
					SymCountT[HighlightData.PD.Desc]++;   
					break;
			}
		}
	}
	
	hDB = OpenGWDatabase (Name,BT_WRITE); 
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
	if (!hDB)
{
#if ENABLETRACE
GSSiExitProg (1126);
#endif
		return (FALSE);  
}
    pQuan = (LPQUANTITIES)&lpGWDHead->GWDData;
	
	for (i=0;i<3201;i++)
	{	
		if (SymCountP[i] || SymCountL[i] || SymCountA[i] || SymCountT[i])
		{
			if (*ThemeVPName)
				strncpy (pQuan->SymName,pTheme->ClassBM[i],64);
			else
			{
				GetSymbolName (i,SymName,0,1,0);
				_fstrncpy (pQuan->SymName,SymName,32);
			}
			if (SymCountP[i])
			{ 
				_fstrncpy (pQuan->UofM,"Point",6);
				pQuan->Count = SymCountP[i];
				pQuan->Quantity = 0;
				GWDAddRecord (lpGWDHead,0,0); 
			}
			if (SymCountL[i])
			{ 
				_fstrncpy (pQuan->UofM,"Line",6);
				pQuan->Count = SymCountL[i];
				pQuan->Quantity = SymLength[i];
				GWDAddRecord (lpGWDHead,0,0); 
			}
			if (SymCountA[i])
			{ 
				_fstrncpy (pQuan->UofM,"Area",6);
				pQuan->Count = SymCountA[i];
				pQuan->Quantity = SymArea[i];
				GWDAddRecord (lpGWDHead,0,0); 
			}
			
		}
	}  
	GlobalUnlock (hDB);
	CloseGWDatabase (hDB); 
	GSSiGlobUlFree (&hSymCounts);
	GSSiGlobUlFree (&hSymVals); 
{
#if ENABLETRACE
GSSiExitProg (1126);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

double AreaFromPolyWithCurves2 (HANDLE hUnSplinedPoly,long nUnSplinedPoints,HANDLE hCurvePoints)
#if ENABLETRACE
{GSSiEnterProg (1127);
#endif
{   
	double	BackAZ, CLEN, R, H, BPAZ, EPAZ, DAZ, PortionOfFullCircle, AreaToUse, CircumOfCircle;
	double	Area = 0, AreaOfCircle, AreaOfTriangle, AreaOfChord;
	LPSHORT	pCurvePoints=(LPSHORT)GlobalLock (hCurvePoints);
	HPDPOINT	pPoints;
	DPOINT	BP, POC, EP, RP, FirstPoint, ChordMidPoint;
	DWORD	i; 
	short	st;
		 		    	

    pPoints = (HPDPOINT)GlobalLock (hUnSplinedPoly); 
    FirstPoint = *pPoints; 
    for (i=0;i<nUnSplinedPoints;i++)
    {
    	if (i == *pCurvePoints)
    	{   
    		POC = *pPoints++; 
    		if (i + 1 >= nUnSplinedPoints)
    			EP = FirstPoint;
    		else
    			EP = *pPoints;
			st = RCURVE(&BP.x,&BP.y,&POC.x,&POC.y,&EP.x,&EP.y,&RP.x,&RP.y,&CLEN); 
			if (!st)
			{ 
				R = ldistp (RP,BP); 
				AreaOfCircle = PY * R * R; 
				CircumOfCircle = TWOPI * R;
				BPAZ = getazd (&RP,&BP); 
				EPAZ = getazd (&RP,&EP);
				DAZ = fabs (DeltaAZ (BPAZ,EPAZ)); 
				if (DAZ > PY)
					DAZ -= PY;
				PortionOfFullCircle = (TWOPI - DAZ)/TWOPI;
				ChordMidPoint = MidPointD (BP,EP);  
				H = ldistp (RP,ChordMidPoint);
				AreaOfTriangle = 0.5 * H * ldistp (BP,EP);
				AreaOfChord = AreaOfCircle - (AreaOfCircle * PortionOfFullCircle + AreaOfTriangle);
				if (fabs (CLEN) > CircumOfCircle/2)
					AreaToUse = AreaOfCircle - AreaOfChord;
				else
					AreaToUse = AreaOfChord;
				if (CLEN < 0)
					Area -= AreaToUse;
				else
					Area += AreaToUse;
			}
            pCurvePoints++;
    	}
    	else
    	{   
    		if (i)
				Area += (BP.y - pPoints->y) * (pPoints->x + BP.x) / 2;
    		BP = *pPoints++; 
    	}
    }          
	Area += (BP.y - FirstPoint.y) * (BP.x + FirstPoint.x) / 2; 
    GlobalUnlock (hCurvePoints); 
    GlobalUnlock (hUnSplinedPoly);
	
{
#if ENABLETRACE
GSSiExitProg (1127);
#endif
	return Area;
}
#if ENABLETRACE
}
#endif
}
double AreaFromPolyWithCurves (HANDLE hUnSplinedPoly,long nUnSplinedPoints,HANDLE hCurvePoints)
{
	if (!hCurvePoints && nUnSplinedPoints == 2)
	{
		double	Area, Radius;
		HPDPOINT	pPoints = GlobalLock (hUnSplinedPoly);

		Radius = ldistp (pPoints[0],pPoints[1]);
		Area = PY * Radius * Radius;
		GlobalUnlock (hUnSplinedPoly);
		return Area;
	}
	else
		return AreaFromPolyWithCurves2 (hUnSplinedPoly,nUnSplinedPoints,hCurvePoints);
}


HANDLE DisplayPointerInVP (int type,DPOINT WPoint,DPOINT WPoint2,short PixelSize,COLORREF Color)
#if ENABLETRACE
{GSSiEnterProg (1128);
#endif
{    
	LPSYMBOL    CurSymbol;
	HANDLE	hSymbol;
	double	Rot,QuaterPY=HALFPI/2.0;
    BOOL	SaveHVFC=HaveVarFillColor;
    HBRUSH	hOldBrush,hBrush;  
    short	symnum;
	POINT	Point, Point2, VPMidPoint;
	HANDLE	hSaveScreen=0;
	HDC		hDC;

	if (!(symnum = GetDictSymbolNumber ("ARROW1")))
	{
		if (!(symnum = GetDictSymbolNumber ("CIRCLE")))
{
#if ENABLETRACE
GSSiExitProg (1128);
#endif
			return 0;
}
	}
	hSymbol = GetDictSymDesc (symnum,0); 
	if (!hSymbol)
{
#if ENABLETRACE
GSSiExitProg (1128);
#endif
		return 0;
}
	hDC = ScreenBufferDC (CurView->hWnd,CurView->hDC);
	SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);
	VPMidPoint = RectMid (&CurView->DrawRect);	
	Point = BasePtToWinPt (&WPoint);
	if (Point.x < VPMidPoint.x)
	{
		if (Point.y < VPMidPoint.y)
			Rot = QuaterPY * 3;
		else
			Rot = QuaterPY * 2;
	} 
	else
	{
		if (Point.y < VPMidPoint.y)
			Rot = QuaterPY;
		else
			Rot = QuaterPY * 4;
	} 
	hBrush = CreateSolidBrush (Color);
	hOldBrush = SelectObject (hDC,hBrush);
    HaveVarFillColor = TRUE;  
    GlobalColors[0]=Color;
	hSaveScreen = DisplayPointSymbol (hSymbol,hDC, PixelSize, PixelSize,Rot, &Point,0,FALSE,0,0,TRUE,FALSE,0,0); 
	if (type == 2)
	{
		Point2 = BasePtToWinPt (&WPoint2);
		DestroySavedScreen (&hSaveScreen,0);
		hSaveScreen = DisplayPointSymbol (hSymbol,hDC, PixelSize, PixelSize,Rot, &Point2,0,FALSE,0,0,TRUE,FALSE,0,0); 
	}
	DestroySymbol (hSymbol); 
	HaveVarFillColor = SaveHVFC;
	SelectObject (hDC,hOldBrush);    
	DeleteObject (hBrush);
	RestoreDC (hDC,-1);
	ShowBufferedScreen (TRUE,TRUE,0,0);
{
#if ENABLETRACE
GSSiExitProg (1128);
#endif
	return hSaveScreen;
}
#if ENABLETRACE
}
#endif
}

void GetParentNameFromTable (short pNum,LPSTR ParentName,LPSTR pTable ,short nbytes)
#if ENABLETRACE
{GSSiEnterProg (1129);
#endif
{   
    LPSHORT		ipnt = (LPSHORT)pTable, pDesc, pParent;
    LPSTR		pStart = (LPSTR)ipnt;
    short		SymNameLen=32, i;  
    BOOL		parent=FALSE;
	LPLONG		pOffset;
	short		idesc, ndesc,iparent;
	char		DescName[34];
	long		StartLoc;
                    
	*ParentName = 0;
    ipnt++;
moredesc:
    ndesc = *ipnt;
	ipnt++;
	for (i=0;i<ndesc;i++)
	{	pDesc = ipnt;
		idesc=abs (*ipnt); 
		ipnt++; 
		pParent = ipnt;
		iparent=*ipnt;
		ipnt++; 
		if (*pDesc == pNum)
		{
	    	_fmemmove (DescName,ipnt,SymNameLen); 
	    	DescName[SymNameLen]=0;
	    	Truncate (DescName);  
	    	_fstrcpy (ParentName,DescName);
{
#if ENABLETRACE
GSSiExitProg (1129);
#endif
	    	return;
}
	    }
    	ipnt+=(SymNameLen/2);   
	}
	parent = TRUE;
	if (ndesc) goto moredesc;
{
#if ENABLETRACE
GSSiExitProg (1129);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}          

BOOL CreateSymConversionTable (HWND hWnd,LPSTR pTable ,short nbytes)
#if ENABLETRACE
{GSSiEnterProg (1130);
#endif
{
    LPSHORT		ipnt = (LPSHORT)pTable, pDesc, pParent,pSymConversionTable;
    LPSTR		pStart = (LPSTR)ipnt;
    short		SymNameLen=32, i,DictSymNum, ii;  
    BOOL		parent=FALSE;
	LPLONG		pOffset;
	short		idesc, ndesc,iparent;
	char		DescName[34], cdesc[8], str[32];
	long		StartLoc; 
	HANDLE		hNotFound=0;   
	short		nNotFound=0, lNotFound=0;  
	LPSTR		pNotFound;
                    
	GSSiGlobFree (&hSymConversionTable);  
	GSSiGlobFree (&hSetNewSymType); 
    if (MapVersion < 8)
{
#if ENABLETRACE
GSSiExitProg (1130);
#endif
    	return FALSE; 
}
	hSymConversionTable = GSSiGlobAlloc (1072,GHND,USHRT_MAX);
	pSymConversionTable = (LPSHORT)GlobalLock (hSymConversionTable);
    ipnt++;
moredesc:
    ndesc = *ipnt;
	ipnt++;
	for (i=0;i<ndesc;i++)
	{	pDesc = ipnt;
		idesc=abs (*ipnt); 
		ipnt++; 
		pParent = ipnt;
		iparent=*ipnt;
		ipnt++;
    	_fmemmove (DescName,ipnt,SymNameLen); 
    	DescName[SymNameLen]=0;
    	Truncate (DescName);
    	ipnt+=(SymNameLen/2);  
    	if ((DictSymNum = GetDictSymbolNumber(DescName))) 
    	{ 
    		*pSymConversionTable++ = idesc;
    		*pSymConversionTable++ = DictSymNum;
		}
		else 
		{
			if (!nNotFound)
			{
				hNotFound = GSSiGlobAlloc (1073,GHND,USHRT_MAX);
				pNotFound = GlobalLock (hNotFound);
			}
			if (parent)
				sprintf (&pNotFound[lNotFound],"%s;p%i;%i",DescName,iparent,idesc);
			else
				sprintf (&pNotFound[lNotFound],"%s;e%i;%i",DescName,iparent,idesc); 
			lNotFound += (_fstrlen (&pNotFound[lNotFound]) + 1);
			nNotFound++;
		}		
	}
	parent = TRUE;
	if (ndesc) goto moredesc;
	if (nNotFound)
	{   
		LPSTR pNewName=pNotFound, pEndName;
		short	lNewName, n, NumNewSym=0;
		char	str[256], ParentName[34];
		
		//the first pass adds missing parents
		while (*pNewName)
		{   
			lNewName = _fstrlen (pNewName);
			pEndName = _fstrchr (pNewName,';');
			*pEndName++ = 0;
			if (*pEndName == 'p')
			{
			    sprintf (str,"$STR(%s,PAR=-none-,DES=Added by reorg)",pNewName);  
				n = GetOrCreateSym (hWnd,str,0,0,1,0);
			}
		    --pEndName;
		    *pEndName = ';';
			pNewName += lNewName+1;
		}
		// the second pass corrects the parent pointer on the new parents
		pNewName=pNotFound;
		while (*pNewName)
		{   
			short	Type=2;
			lNewName = _fstrlen (pNewName);
			pEndName = _fstrchr (pNewName,';');
			*pEndName++ = 0;
			if (*pEndName == 'p') 
				Type = 0;
			{   
				short pNum = atoi ((LPSTR)(pEndName+1));
				LPSTR	pOldNum = _fstrrchr (pEndName,';') + 1;
				short	OldNum = atoi (pOldNum);  
				LPSHORT	pNumNewSym, pSymNum;
				
				GetParentNameFromTable (pNum,ParentName,pTable ,nbytes);
			    sprintf (str,"$STR(%s,PAR=%s,DES=Added by reorg)",pNewName, ParentName);
				DictSymNum = GetOrCreateSym (hWnd,str,0,0,2,Type);
	    		*pSymConversionTable++ = OldNum;
	    		*pSymConversionTable++ = DictSymNum; 
	    		if (Type)
	    		{
	    			if (!hSetNewSymType)
	    				hSetNewSymType = GSSiGlobAlloc (1074,GHND,USHRT_MAX);
	    			pSymNum = (LPSHORT)GlobalLock (hSetNewSymType);
	    			pSymNum += (4 * NumNewSym);
	    			*pSymNum = DictSymNum;
	    			NumNewSym++;
	    			GlobalUnlock (hSetNewSymType);
	    		}
			}
		    --pEndName;
		    *pEndName = ';';
			pNewName += lNewName+1;
		}
		GSSiGlobUlFree (&hNotFound); 
	}
	GlobalUnlock (hSymConversionTable);
{
#if ENABLETRACE
GSSiExitProg (1130);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
 
void CreateFileSymList (LPSHORT pSymNum)
#if ENABLETRACE
{GSSiEnterProg (1131);
#endif
{   
	LPSHORT	pFileSymList;
	
	if (!hFileSymList)
		hFileSymList = GSSiGlobAlloc (1075,GHND,USHRT_MAX); 
	pFileSymList = (LPSHORT)GlobalLock (hFileSymList);
	while (*pFileSymList)
	{
		if (*pSymNum == *pFileSymList++)
			goto Exit;
	}                 
	*pFileSymList = *pSymNum;
Exit:
	GlobalUnlock (hFileSymList);
{
#if ENABLETRACE
GSSiExitProg (1131);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}	

void ConvertSymbol (LPSHORT pSymNum)
#if ENABLETRACE
{GSSiEnterProg (1132);
#endif
{
	HPSTR	pName;
	LPSHORT	pSym,pSymConversionTable;
	short	nSyms, SymNum;
	short	Neg = 1;

	if (!hSymConversionTable)
{
#if ENABLETRACE
GSSiExitProg (1132);
#endif
		return; 
}
	if (*pSymNum < 0)
	{
		Neg = -1;
		SymNum = -*pSymNum;
	}
	else
		SymNum = *pSymNum; 
	pSymConversionTable = (LPSHORT)GlobalLock (hSymConversionTable);    
	while (*pSymConversionTable)
	{
		if (*pSymConversionTable++ == SymNum)
		{   
			*pSymNum = *pSymConversionTable * Neg;
			break;
		}
		pSymConversionTable++;
	}
	GlobalUnlock (hSymConversionTable); 
{
#if ENABLETRACE
GSSiExitProg (1132);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

short ConvertSymTable (LPHANDLE phSpace)
#if ENABLETRACE
{GSSiEnterProg (1133);
#endif
{
	short		NewLen=0;
	HANDLE		hNewTable=GSSiGlobAlloc (1076,GHND,USHRT_MAX);
	LPSHORT		pNewTable=(LPSHORT)GlobalLock (hNewTable), ipnt=(LPSHORT)GlobalLock (*phSpace);
    LPSHORT		pDesc, pParent;
    LPSTR		pStart = (LPSTR)ipnt;
    short		SymNameLen=32, i;  
    BOOL		parent=FALSE;
	LPLONG		pOffset;
	short		idesc, ndesc,iparent;
	char		DescName[34], cdesc[8], str[32];
	long		StartLoc;
                    

	*pNewTable++ = *ipnt++;
	NewLen += 2;
moredesc:
    ndesc = *ipnt++;
	*pNewTable++ = ndesc;
	NewLen += 2;
	for (i=0;i<ndesc;i++)
	{	
		*pNewTable++ = *ipnt++;
		*pNewTable++ = *ipnt++;
		NewLen += 4;
    	_fmemmove (DescName,ipnt,8); 
    	DescName[8]=0;
    	Truncate (DescName);
    	ipnt+=4;   
    	_fstrcpy ((LPSTR)pNewTable,DescName);
    	pNewTable += 16;
		NewLen += 32;
	}
	parent = TRUE;
	if (ndesc)
		goto moredesc; 
	GSSiGlobUlFree (phSpace);
	GlobalUnlock (hNewTable);
	*phSpace = hNewTable; 
	NewLen += 2;
{
#if ENABLETRACE
GSSiExitProg (1133);
#endif
	return NewLen;
}
#if ENABLETRACE
}
#endif
}

BOOL ConvertSymsInSymTable (LPSTR pTable ,short nbytes)
#if ENABLETRACE
{GSSiEnterProg (1134);
#endif
{
    LPSHORT		ipnt = (LPSHORT)pTable, pDesc, pParent;
    LPSTR		pStart = (LPSTR)ipnt;
    short		SymNameLen=32, i, ipar;  
    BOOL		parent=FALSE;
	LPLONG		pOffset;
	short		idesc, ndesc,iparent;
	char		DescName[34], cdesc[8], str[32];
	long		StartLoc;
                    
    if (MapVersion < 8)
{
#if ENABLETRACE
GSSiExitProg (1134);
#endif
    	return FALSE;   
}
    ipnt++;
moredesc:
    ndesc = *ipnt;
	ipnt++;
	for (i=0;i<ndesc;i++)
	{	pDesc = ipnt;
		idesc=abs (*ipnt); 
		ipnt++; 
		pParent = ipnt;
		iparent=*ipnt;
		ipnt++;
    	ipnt+=(SymNameLen/2);   
    	ConvertSymbol (pDesc); 
    	if ((ipar = GetDictSymParent (*pDesc)))
    		*pParent = ipar;
		else    		
    		ConvertSymbol (pParent);
	}
	parent = TRUE;
	if (ndesc) goto moredesc;
{
#if ENABLETRACE
GSSiExitProg (1134);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
 

BOOL WriteMaskArea (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (1137);
#endif
{
	LPMNMXCORD	pRect; 
	HPDPOINT	pPoint;
	short		ID=OB_SAVEMASK, Version=3;
	HANDLE		handle=0; 
	short		nParts=CurView->NumMaskAreaParts;
	
	if (nParts)
		nParts++;
	BigWrite (Fid,(HPSTR)&ID,2,-1);
	BigWrite (Fid,(HPSTR)&handle,2,-1);
	BigWrite (Fid,(HPSTR)&Version,2,-1);
	BigWrite (Fid,(HPSTR)&CurView->NumMaskPoints,sizeof(int),-1);
	BigWrite (Fid,(HPSTR)&CurView->NumMaskAreaParts,sizeof(short),-1);
    pRect = (LPMNMXCORD) GlobalLock (CurView->hMaskArea);
	BigWrite (Fid,(HPSTR)pRect,sizeof(MNMXCORD),-1);
	pRect++;
	pPoint = (HPDPOINT) pRect; 
	BigWrite (Fid,(HPSTR)pPoint,(long)CurView->NumMaskPoints*(long)sizeof(DPOINT)+nParts*sizeof(int),-1); 
	GlobalUnlock (CurView->hMaskArea); 
{
#if ENABLETRACE
GSSiExitProg (1137);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}   
BOOL WriteBackgroundArea (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (1137);
#endif
{
	LPMNMXCORD	pRect; 
	HPDPOINT	pPoint;
	short		ID=OB_SAVEBACKGROUND, Version=1;
	HANDLE		handle=0; 
	short		nParts=CurView->numBackgroundAreaParts;
	
	if (nParts)
		nParts++;
	BigWrite (Fid,(HPSTR)&ID,2,-1);
	BigWrite (Fid,(HPSTR)&handle,2,-1);
	BigWrite (Fid,(HPSTR)&Version,2,-1);
	BigWrite (Fid,(HPSTR)&CurView->numBackgroundAreaPoints,sizeof(int),-1);
	BigWrite (Fid,(HPSTR)&CurView->numBackgroundAreaParts,sizeof(short),-1);
    pPoint = (LPDPOINT) GlobalLock (CurView->hBackgroundArea);
	BigWrite (Fid,(HPSTR)pPoint,(long)CurView->numBackgroundAreaPoints*(long)sizeof(DPOINT)+nParts*sizeof(int),-1); 
	GlobalUnlock (CurView->hBackgroundArea); 
{
#if ENABLETRACE
GSSiExitProg (1137);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}   

