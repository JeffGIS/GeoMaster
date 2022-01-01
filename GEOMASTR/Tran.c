#include "graphint.h"

static  BYTE    Mask[8] = {128, 64, 32, 16, 8, 4, 2, 1}; 
static	LPPARCELTRAN currentParcelTran = 0;
static  double currentBASX, currentBASY;

#include "gmextern.h"
 
void SetCurrentParcelTran(LPPARCELTRAN pParTran)
{
	currentParcelTran = pParTran;
	return;
}

HANDLE STRAN2 (int ID,LPDOUBLE X1, LPDOUBLE Y1, LPDOUBLE X2, LPDOUBLE Y2,int N, LPFLOAT RSQMIN, int Type,LPMNMXCORD pBounds)
/*    ENTRY      TRNPRO (XIN,YIN,XOUT,YOUT,TRNNUM)
C     ENTRY      TRANS2 (XIN,YIN,XOUT,YOUT,TRNNUM)
C     ENTRY      RESIDS (XRESID,YRESID,MAXXRN,MAXYRN,X1,Y1,X2,Y2,N,
C    +                   TRNNUM)
C********SPECIFICATIONS************************************************
C*                                                                    *
C*       PROGRAM SUMMARY                                              *
C*       ------- -------                                              *
C*    STRAN2 SETS-UP THE VALUES NEEDED BY TRANS2 TO TRANSFORM         *
C*    COORDINATE VALUES FROM ONE COORDINATE SYSTEM TO ANOTHER. ANY    *
C*    NUMBER OF TRANSFORMATION POINTS MAY BE USED. THE EQUATIONS USED *
C*    TO PERFORM THE TRANSFORMATIONS ARE AS FOLLOWS:                  *
C*                                                                    *
C*             XOUT = A1 + B1 * XIN + C1 * YIN                        *
C*             YOUT = A2 + B2 * YIN + C2 * XIN                        *
C*                                                                    *
C*    IF N IS GREATER THAN 2, A1, B1, C1, A2, B2, C2 ARE DETERMINED   *
C*    BY A LEAST SQUARES MULTIPLE REGRESSION TECHNIQUE. IF N IS 2     *
C*    THESE VALUES ARE DETERMINED BY STANDARD TRIGINOMETRY. IF N IS   *
C*    GREATER THAN 2, SEPARATE SCALE FACTORS FOR EACH AXIS MAY BE USED*
C*    AND THE AXIS NEED NOT BE PERPENDICULAR.                         *
C*                                                                    *
C*    SEVERAL TRANSFORMATIONS MAY BE SET-UP AT ONE TIME BY ASSIGNING  *
C*    DIFFERENT VALUES OF TRNNUM FOR EACH TRANSFORMATION WHEN CALLING *
C*    STRAN2 AND BY USING TRNNUM TO IDENTIFY WHICH TRANSFORMATION TO  *
C*    USE WHEN CALLING TRANS2.                                        *
C*                                                                    *
C*    IF STRAN2 IS CALLED WITH A NEGATIVE VALUE OF N A FIXED POINT    *
C*    TRANSFORMATION IS USED IN WHICH THE TRANSFORMATION POINTS       *
C*    ARE FIXED TO THE SPECIFIED COORDINATES. THIS IS DONE BY         *
C*    SETTING UP SEVERAL 3 POINT TRANSFORMATIONS AND TRANSFORMING     *
C*    DATA BASED ON WHICH TRIANGLE IT FALLS WITHIN OR CLOSEST TO.     *
C*                                                                    *
C*    ENTRY TRNPRO CALLED ONLY BY SGLPRO, LOADING PROFILE DATA TEXT   *
C*    CREATED OR CHANGED IN IGS SESSION, TO AVOID STORING FIXEDP.     *
C*                                                                    *
C*    RESIDS MAY BE USED (IF N GT 2) TO DETECT BAD CONTROL POINTS.    *
C*    RESIDS CALCULATES THE RESIDUALS FOR ALL TRANSFORMATION POINTS   *
C*    AND RETURNS THE LOCATION OF THE MAXIMUM RESIDUAL FOR BOTH THE   *
C*    X AND Y TRANSFORMATIONS.                                        *
C*                                                                    *
C*    NOTE: LEAST SQUARES WITH INDEPENDENT X AND Y AXIS IS THE        *
C*          DEFAULT TRANSFORMATION                                    *
C*          TO USE A FIXED TRANSFORMATION TRNNUM SHOULD BE SET TO     *
C*          MINUS TRNNUM ON THE SET UP CALL. TO USE AN AVERAGE        *
C*          TRANSFORMATION ADD 100 TO TRNNUM ON THE SET UP CALL.      *
C*                                                                    *
C*       ARGUMENT DESCRIPTION                                         *
C*       -------- -----------                                         *
C*    X1, Y1   R*8  TRANSFORMATION PO/INTS IN ORIGINAL COORD SYSTEM   *
C*    X2  Y2   R*8  TRANSFORMATION POINTS IN NEW COORDINATE SYSTEM    *
C*    XRESID   R*8  X-RESIDUALS                                       *
C*    YRESID   R*8  Y-RESIDUALS                                       *
C*    N        I*4  NUMBER OF TRANSFORMATION POINTS                   *
C*    TRNNUM   I*4  TRANSFORMATION NUMBER                             *
C*    RSQMIN   R*4  THE MINIMUM RSQ VALUE FOR THE X AND Y REGRESSIONS *
C*    XIN, YIN R*8  COORDINATE VALUES TO BE TRANSFORMED               *
C*    XOUT,    R*8  TRANSFORMED COORDINATE VALUES                     *
C*        YOUT                                                        *
C*    MAXXRN   I*4  ELEMENT NUMBER OF THE MAXIMUM X-RESIDUALS         *
C*    MAXYRN   I*4  ELEMENT NUMBER OF THE MAXIMUM Y-RESIDUALS         *
C*                                                                    *
C*       AUTHOR                                                       *
C*       ------                                                       *
C*     JEFF SMITH                                                     *
C*                                                                    *
C**********************************************************************
C*/
/*%include 'pptran.ftn' {/umsc/include/pptran.ftn}*/
/*%include 'lsystm.ftn' {/umsc/include/lsystm.ftn}*/
/*%include 'trans2_common.ftn' {/umsc/include/trans2_common.ftn}*/
/*%include 'cnstnt.ftn' {/umsc/include/cnstnt.ftn}*/
/*
C
      LOGICAL   FIXEDP(40),  GOTPNT(38), FIRST, ONE_SCALE
      INTEGER*2 VERTEX(3), NTRI, VERTEXES(3,MXT), ITRI
      INTEGER*4 TRNNUM, STRNUM, TRINUM, TNUM, PNTR(38), TRI_PNTR,
     +          TRINUM2
      REAL * 8 X1(N), Y1(N), X2(N), Y2(N), XIOLD(4), YIOLD(4),
     +         XOOLD(4), YOOLD(4),  XIN, YIN, XOUT, YOUT,
     +         MAXXR, MAXYR, ANGLE1, ANGLE2, ANGLE,
     +         SINANG, COSANG, DX1, DX2, DY1, DY2, SCLRAT, DEN,
     +         SX1(3),  SX2(3),  SY1(3),  SY2(3),
     +         BASX1, BASX2, BASY1, BASY2, XI, YI, X
      REAL*8   ANGLED
      REAL * 8 PN, VERTEX_X(3,MXT), VERTEX_Y(3,MXT), LTWOPI
      DATA     GOTPNT/38*.FALSE./, PN/'STRAN2  '/, FIRST/.TRUE./

      IF (FIRST) THEN
          FIRST = .FALSE.
          CALL GET_MEM (TRANCSIZ,PNTR(1))
          GOTPNT(1) = .TRUE.
          ENDIF*/

#if ENABLETRACE
{GSSiEnterProg (1436);
#endif
{
	double    ANGLE1, ANGLE2, ANGLE, DEN,
               SINANG, COSANG, DX1, DX2, DY1, DY2, SCLRAT, 
               ANGLED, BASX1, BASX2, BASY1, BASY2, D,TOTD;
     LPDOUBLE  XT1, YT1, XT2, YT2;
     float     RSQ1, RSQ2;
     short     NUM, I,J,i,ii;
     LPTRANDATA TranPtr;
     HANDLE     hTran=0, hTemp=0, hPoints=0;
	 LPDPOINT	pPoints;
	 double		A,B, xmin, xmax;
	 BOOL		noOffset = FALSE;
    
	 if (N < 0)
	 {
		noOffset = TRUE;
		N = -N;
	 }
    if (!N || N > MAXTRANPOINTS)
    	goto Exit;   
    for (i=0;i<N;i++)
    {
    	if (X1[i] != X2[i] || Y1[i] != Y2[i])
    		goto Start;
    }
    	
Start:
    hTemp = GSSiGlobAlloc (1281,GMEM_MOVEABLE,(long)MAXTRANPOINTS*16*2);
    XT1 = (LPDOUBLE)GlobalLock (hTemp);
    XT2 = XT1 + MAXTRANPOINTS;
    YT1 = XT2 + MAXTRANPOINTS;
    YT2 = YT1 + MAXTRANPOINTS;
    hTran = GSSiGlobAlloc (ID,GHND,sizeof(TRANDATA));
	if (ID == 1628)
		ii=1;
    TranPtr = (LPTRANDATA) GlobalLock (hTran);
    
    if (pBounds) 
    {
    	if (pBounds->xmn > pBounds->xmx)
	    {
	    	TranPtr->Bounds.xmn = -DBL_MAX;
	    	TranPtr->Bounds.xmx = DBL_MAX;
	    	TranPtr->Bounds.ymn = -DBL_MAX;
	    	TranPtr->Bounds.ymx = DBL_MAX;
	    }
    	else
    		TranPtr->Bounds = *pBounds; 
    } 
    else
    {
    	TranPtr->Bounds.xmn = -DBL_MAX;
    	TranPtr->Bounds.xmx = DBL_MAX;
    	TranPtr->Bounds.ymn = -DBL_MAX;
    	TranPtr->Bounds.ymx = DBL_MAX;
    }
    TranPtr->FIXEDP = FALSE;

    TranPtr->ONE_SCALE = FALSE;
    if (Type == 2)
	    TranPtr->ONE_SCALE = TRUE;
    	
    if (N == 2 || TranPtr->ONE_SCALE) goto S10;
    if (Type == 3)
    	TranPtr->FIXEDP = TRUE;
/*      IF (STRNUM .GT. 100) ONE_SCALE = .TRUE.
      TRNNUM = MOD (IABS (STRNUM), 100)
      IF (N .GT. MXP) GO TO 1300
C
C                ****** TRANSFORMATION SET-UP ******
C
      FIXEDP(TRNNUM) = .FALSE.
      IF (N .EQ. 2 .OR. ONE_SCALE) GO TO 10
      IF (STRNUM .LE. 0 .AND. N .GT. 3) FIXEDP(TRNNUM) = .TRUE.
      IF (FIXEDP(TRNNUM) .AND. .NOT. GOTPNT(TRNNUM)) THEN
          CALL GET_MEM (TRANCSIZ,PNTR(TRNNUM))
          GOTPNT(TRNNUM) = .TRUE.
          ENDIF
      IF (FIXEDP(TRNNUM)) THEN
              TRIPNT = PNTR(TRNNUM)
          ELSE
              TRIPNT = PNTR(1)
          ENDIF*/
/******* SET-UP WITH N GT 2.*/
      BASX1 = DBL_MAX;
      BASX2 = DBL_MAX;
      BASY1 = DBL_MAX;
      BASY2 = DBL_MAX;
      for (I=0; I<N; I++)
      {
           BASX1  = min (BASX1, X1[I]);
           BASX2  = min (BASX2, X2[I]);
           BASY1  = min (BASY1, Y1[I]);
           BASY2  = min (BASY2, Y2[I]);
      }
	  if (noOffset)
	  {
           BASX1  = 0;
           BASX2  = 0;
           BASY1  = 0;
           BASY2  = 0;
	  }
      for (I=0; I<N; I++)
      {
           XT1[I] = X1[I] - BASX1;
           XT2[I] = X2[I] - BASX2;
           YT1[I] = Y1[I] - BASY1;
           YT2[I] = Y2[I] - BASY2;
      }
      RSQ1 = MULREG (XT2,XT1,YT1,N,&TranPtr->A1,&TranPtr->B1,&TranPtr->C1);
      RSQ2 = MULREG (YT2,YT1,XT1,N,&TranPtr->A2,&TranPtr->B2,&TranPtr->C2);
      TranPtr->A1 = TranPtr->A1 + BASX2;
      TranPtr->A2 = TranPtr->A2 + BASY2;
      TranPtr->BASX = BASX1;
      TranPtr->BASY = BASY1;
      *RSQMIN = min (RSQ1,RSQ2);
      TranPtr->NSETPT = N; 
      TranPtr->LastTri = 0;
	  if (TranPtr->FIXEDP)
	  {
		  currentBASX = BASX1;
		  currentBASY = BASY1;
		  TranPtr->TriHandle = TRFTRI_SET(XT1, YT1, XT2, YT2, N, BASX1, BASY1, BASX2, BASY2, hTran, &TranPtr->TriBounds);
	  }
      else if (!*RSQMIN || *RSQMIN > 1.05)
      {
	    TranPtr->ONE_SCALE = TRUE;
    	goto S10;
      }
	  GSSiGlobUlFree(&hTemp);
	  GlobalUnlock (hTran);
      goto Exit;

/******* SET-UP WITH N EQ 2.*/
 S10: NUM    = 0;
      ANGLE  = 0;
      SCLRAT = 0;
      TOTD=0;
      for (I=0;I< N - 1;I++)
      {   
      	  for (J=I+1;J<N;J++)
      	  {
	          DX1    = X1[J] - X1[I];
	          DX2    = X2[J] - X2[I];
	          DY1    = Y1[J] - Y1[I];
	          DY2    = Y2[J] - Y2[I];
	          DEN = pow(DX1,2) + pow (DY1,2); 
	          if (!DEN)
	          	continue;
	          D = sqrt (DEN);
	          TOTD += D;
	          SCLRAT = SCLRAT + (sqrt ((pow (DX2,2) + pow (DY2,2))/ DEN)) * D;
	          ANGLE1 = atan2 (DY2, DX2);
	          ANGLE2 = atan2 (DY1, DX1);
	          ANGLED  = LTWOPI (ANGLE2 - ANGLE1);
	          if (ANGLED > PY)
	          	ANGLED =  ANGLED - TWOPI;
	          ANGLE  = ANGLE + ANGLED * D;
	          NUM = NUM + 1; 
          }
      } 
      if (!NUM)
      	goto S1000;
      ANGLE  = ANGLE / TOTD;
      SCLRAT = SCLRAT / TOTD;
      COSANG = cos (ANGLE);
      SINANG = sin (ANGLE);
      TranPtr->B1 = COSANG * SCLRAT;
      TranPtr->C1 = SINANG * SCLRAT;
      TranPtr->B2 = COSANG * SCLRAT;
      TranPtr->C2 =-SINANG * SCLRAT;
      TranPtr->A1 = 0;
      TranPtr->A2 = 0;
	  hPoints = GSSiGlobAlloc (1827,GMEM_MOVEABLE,sizeof(DPOINT)*N);
	  pPoints = GlobalLock (hPoints);
	  xmin = DBL_MAX;
	  xmax = -DBL_MAX;
      for (I=0; I<N; I++)
      {
          TranPtr->A1 = TranPtr->A1 + X2[I] - (COSANG * X1[I] +SINANG * Y1[I]) * SCLRAT;
          TranPtr->A2 = TranPtr->A2 + Y2[I] - (COSANG * Y1[I] -SINANG * X1[I]) * SCLRAT;
		  pPoints[I].x = X1[I];
		  pPoints[I].y = Y1[I];
		  xmin = min (xmin, X2[I]);
		  xmax = max (xmax, X2[I]);
      }
	  if (LINFIT (pPoints,N,&A,&B,0,0))
	  {
		   pPoints[0].x = xmin;
		   pPoints[0].y = A + pPoints[0].x * B;
		   pPoints[1].x = xmax;
		   pPoints[1].y = A + pPoints[1].x * B;
		   TranPtr->FitPointAZFrom = getazd (&pPoints[0],&pPoints[1]);
	  }
	  xmin = DBL_MAX;
	  xmax = -DBL_MAX;
      for (I=0; I<N; I++)
      {
		  pPoints[I].x = X2[I];
		  pPoints[I].y = Y2[I];
		  xmin = min (xmin, X2[I]);
		  xmax = max (xmax, X2[I]);
      }
	  if (LINFIT (pPoints,N,&A,&B,0,0))
	  {
		   pPoints[0].x = xmin;
		   pPoints[0].y = A + pPoints[0].x * B;
		   pPoints[1].x = xmax;
		   pPoints[1].y = A + pPoints[1].x * B;
		   TranPtr->FitPointAZTo = getazd (&pPoints[0],&pPoints[1]);
	  }
	  GSSiGlobUlFree (&hPoints);
      TranPtr->A1 = TranPtr->A1 / N;
      TranPtr->A2 = TranPtr->A2 / N;
      *RSQMIN = (float)1.;
      TranPtr->BASX = 0;
      TranPtr->BASY = 0;
      TranPtr->NSETPT = N; 
      GlobalUnlock(hTran);
      GSSiGlobUlFree (&hTemp);
	  goto Exit;
S1000:
      GSSiGlobUlFree (&hTemp);
      GSSiGlobUlFree (&hTran);
Exit:
	  currentParcelTran = 0;
{
#if ENABLETRACE
GSSiExitProg (1436);
#endif
	TranPtr = GlobalLock(hTran);
	GlobalUnlock(hTran);

	return (hTran);
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayTranTriangles(HANDLE hTran,LPVIEWPORT CurView)
{
	BOOL rtn = FALSE;
	LPTRANDATA pTran;
	int I;
	LPTRANDATA  TranPtr;
	HANDLE hTranp;

	if (!hTran)
		return FALSE;
	pTran = GlobalLock(hTran);
	if (!strncmp((LPSTR)pTran, "PARCELTRAN", 10))
	{
		LPPARCELTRAN pParTran = (LPPARCELTRAN)pTran;

		hTranp = pParTran->hTran;
		GlobalUnlock(hTran);
		hTran = hTranp;
		pTran = GlobalLock(hTran);
		if (pTran->TriHandle)
		{
			HPTRANTRI	Tri = (HPTRANTRI)GlobalLock(pTran->TriHandle);

			rtn = TRUE;
			SaveDC(CurView->hDC);
			SetDisplayMode(CurView->hDC, GF_TEXTMODE);
			GSSiDeleteObject(&CurView->hRgn);
			CurView->hRgn = CreateVPRgn(FALSE, FALSE);
			SelectClipRgn(CurView->hDC, CurView->hRgn);
			GSSiDeleteObject(&CurView->hRgn);
			CreateRandomBrushes(NULL, 0);
			for (I = 0; I < Tri->NumTri; I++)
			{
				HBRUSH	hOldBrush;
				HPEN	hOldPen, hPen = GetStockObject(BLACK_PEN);

				hOldBrush = SelectRandomBrush(CurView->hDC, I, &hPen, 0);
				hOldPen = SelectObject(CurView->hDC, hPen);
				GWPolygonD(CurView->hDC, Tri[I].FromPT, 4, 1, NULL, 0, FALSE, TRUE, 0);
				SelectObject(CurView->hDC, hOldBrush);
				SelectObject(CurView->hDC, hOldPen);
			}
			RestoreDC(CurView->hDC, -1);
			GlobalUnlock(pTran->TriHandle);
		}
	}
	GlobalUnlock(hTran);
	return rtn;
}

HANDLE ReadTranData (HFILE Fid)
{
    HANDLE  hTran;
    LPTRANDATA TranPtr;    
    long	ii;
    
    hTran = GSSiGlobAlloc (1283,GHND,sizeof(TRANDATA));
    TranPtr = (LPTRANDATA) GlobalLock (hTran); 
    ii=BigRead (Fid,(HPSTR)TranPtr,sizeof(TRANDATA));
	if (TranPtr->TriHandle)
	{   
		TRANTRI		TranTri;
	    HPTRANTRI	Tri;
	    short		NumTri;
	    long		Loc = GSSillseek (Fid,0,1);
        
        BigRead (Fid,(HPSTR)&TranTri,sizeof(TRANTRI)); 
        GSSillseek (Fid,Loc,0);
        NumTri = TranTri.NumTri;
        TranPtr->TriHandle = GSSiGlobAlloc (1284,GMEM_MOVEABLE,(long)NumTri*sizeof(TRANTRI));
        Tri=(HPTRANTRI)GlobalLock (TranPtr->TriHandle);
	    while (NumTri--)
	    {
	        BigRead (Fid,(HPSTR)Tri,sizeof(TRANTRI)); 
	        Tri->hFromTran = 0;
	        Tri->hToTran = 0;
		    Tri++;
	    } 
	    GlobalUnlock (TranPtr->TriHandle);
	}
    GlobalUnlock (hTran);
    return (hTran);
}  

HANDLE LoadTranFromBPW (LPSTR Name)
{
	HFILE Fid=GSSiOpenFile (Name,0,OF_READ);
	HANDLE handle=0;
	double XFROM[4],YFROM[4],XTO[4],YTO[4];
	double xres, yres, bpx, bpy, wx, wy;
	char str[66];
    MNMXCORD	TranBounds;
	float	RSQMIN=0;

	if (Fid != HFILE_ERROR)
	{
		fgetstring (str,64,Fid);
		xres = atof (str);
		fgetstring (str,64,Fid);
		bpx = atof (str);
		fgetstring (str,64,Fid);
		bpy = atof (str);
		fgetstring (str,64,Fid);
		yres = atof (str);
		fgetstring (str,64,Fid);
		wx = atof (str);
		fgetstring (str,64,Fid);
		wy = atof (str);
		XFROM[0] = bpx;
		YFROM[0] = bpy;
		XFROM[1] = bpx + 100;
		YFROM[1] = bpy;
		XFROM[2] = bpx + 100;
		YFROM[2] = bpy + 100;
		XFROM[3] = bpx;
		YFROM[3] = bpy + 100;
		XTO[0] = wx;
		YTO[0] = wy;
		XTO[1] = wx + 100 * xres;
		YTO[1] = wy;
		XTO[2] = wx + 100 * xres;
		YTO[2] = wy + 100 * yres;
		XTO[3] = wx;
		YTO[3] = wy + 100 * yres;
		handle = STRAN2 (1660,XFROM,YFROM,XTO,YTO,4,&RSQMIN,1,&TranBounds); 
		GSSiClose2 (&Fid);
	}
	return handle;
}

HANDLE LoadTranFile (LPSTR Name,int dir,int InType,LPSHORT pNumPoints,LPDOUBLE pRSQ)
{//type=1 is LSQ, type=2 is One Scale, type=3 is fixed point, type=102 is One Scale corrected to window coord
	HFILE Fid=HFILE_ERROR;
	char	str[260], curproject[64]="", Marker;
	BOOL	First=TRUE, SetTrans=FALSE, HaveLimits=FALSE, HaveBounds=FALSE; 
	HANDLE	handle=0, hcoord=GSSiGlobAlloc (1285,GMEM_MOVEABLE,(long)MAXTRANPOINTS*16*2); 
	float	RSQMIN=0;
	short	N=0, ConvertID=0, i; 
	LPSTR	lpCVT;
	DPOINT	Point;
	LPDOUBLE	XFROM,YFROM,XTO,YTO;
    HCURSOR hcurSave;
    MNMXCORD	TranBounds, TranLimits;
	int		Type=InType % 100;
	
	GetGlobalCVal ("[%ALT_PROJECTION]",curproject,NULL); 
// 1/27/2003	SetTrans = TRUE; messes up $MAPCOPY
	TranBounds.xmn=2;
	TranBounds.xmx=1;
	DBoundsInit (&TranBounds);
	XFROM = (LPDOUBLE)GlobalLock (hcoord);
	YFROM = XFROM+MAXTRANPOINTS;
	XTO = YFROM+MAXTRANPOINTS;
	YTO = XTO+MAXTRANPOINTS;
	
	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	Fid = GSSiOpenFile (Name,NULL,OF_READ);
	if (Fid == HFILE_ERROR)
		goto Exit;  
	
	while (fgetstring (str,256,Fid))
	{   
		Truncate (str); 
		if (!*str || *str == '#')
			goto Next;
		ExpandText (str);
		if (First)
		{   
			First = FALSE;
			_fstrlwr (str);
			if ((lpCVT = _fstrstr (str,".cvt")))
			{   
				lpCVT += 4;
				*lpCVT++ = 0;
				SetGlobalValue("%ALT_PROJECTION",str); 
				ConvertID = atoi (lpCVT);
				ConvertCoordClose ();
				ConvertCoordInit(); 
				SetTrans = TRUE;
				goto Next;
			}
		}
		if (*str == 'L' || *str == 'l') 
		{
			if (sscanf (str,"%c %lf %lf %lf %lf",&Marker,&TranLimits.xmn,&TranLimits.ymn,
											  &TranLimits.xmx,&TranLimits.ymx) == 5)
				HaveLimits = TRUE;
		}	
		else if (*str == 'B' || *str == 'b') 
		{
			sscanf (str,"%c %lf %lf %lf %lf",&Marker,&TranBounds.xmn,&TranBounds.ymn,
											  &TranBounds.xmx,&TranBounds.ymx);
			HaveBounds = TRUE;
		}
		else if (N<MAXTRANPOINTS)
		{
			if (sscanf (str,"%lf %lf %lf %lf",XFROM++,YFROM++,XTO++,YTO++) == 4)
			N++;
		}
		else
			goto Exit;
Next:;
	} 
	if (N < 2)
		goto Exit;
	GlobalUnlock (hcoord);
	XFROM = (LPDOUBLE)GlobalLock (hcoord);
	YFROM = XFROM+MAXTRANPOINTS;
	XTO = YFROM+MAXTRANPOINTS;
	YTO = XTO+MAXTRANPOINTS;
	if (ConvertID == 1)
	{
		for (i=0;i<N;i++)
		{
			Point.x = XFROM[i];
			Point.y = YFROM[i];
			ConvertCoord(&Point,3,1);
			XFROM[i] = Point.x;
			YFROM[i] = Point.y;
		}
	}
	else if (ConvertID == 2)
	{
		for (i=0;i<N;i++)
		{
			Point.x = XTO[i];
			Point.y = YTO[i];
			ConvertCoord(&Point,3,1);
			XTO[i] = Point.x;
			YTO[i] = Point.y;
		}
	}
	if (InType == 102)
	{
		DPOINT	WPoint, BPoint;

		for (i=0;i<N;i++)
		{
			BPoint.x = XTO[i];
			BPoint.y = YTO[i];
		//	WPoint = BasePtToWinPtD (&BPoint);
			WPoint = BasePtToFilePtD (BPoint);
			XTO[i] = WPoint.x;
			YTO[i] = WPoint.y;
		}
	}
	if (N > 1 && Type == 3 && HaveLimits)
	{
		handle = STRAN2 (1659,XFROM,YFROM,XTO,YTO,N,&RSQMIN,1,NULL);
		XFROM[N] = TranLimits.xmn;
		YFROM[N] = TranLimits.ymn;
		TRANS2 (XFROM[N],YFROM[N],&XTO[N],&YTO[N],handle);   
		N++;
		XFROM[N] = TranLimits.xmn;
		YFROM[N] = TranLimits.ymx;
		TRANS2 (XFROM[N],YFROM[N],&XTO[N],&YTO[N],handle);   
		N++;
		XFROM[N] = TranLimits.xmx;
		YFROM[N] = TranLimits.ymx;
		TRANS2 (XFROM[N],YFROM[N],&XTO[N],&YTO[N],handle);   
		N++;
		XFROM[N] = TranLimits.xmx;
		YFROM[N] = TranLimits.ymn;
		TRANS2 (XFROM[N],YFROM[N],&XTO[N],&YTO[N],handle);   
		N++;
		CloseTRANS2 (&handle);
	}
	if (dir == 1)
	{
		if (!HaveBounds)
			for (i=0;i<N;i++)
			{
				Point.x = XFROM[i];
				Point.y = YFROM[i];
				AddDPointToMinMax (&Point,&TranBounds);
			}
		handle = STRAN2 (1660,XFROM,YFROM,XTO,YTO,N,&RSQMIN,Type,&TranBounds); 
	}
	else
	{
		if (!HaveBounds)
			for (i=0;i<N;i++)
			{
				Point.x = XTO[i];
				Point.y = YTO[i];
				AddDPointToMinMax (&Point,&TranBounds);
			}
		handle = STRAN2 (1661,XTO,YTO,XFROM,YFROM,N,&RSQMIN,Type,&TranBounds); 
	}
Exit:
	GSSiGlobUlFree (&hcoord);
	if (Fid != HFILE_ERROR)
		GSSiClose2 (&Fid);  
	if (SetTrans)
	{
		SetGlobalValue("%ALT_PROJECTION",curproject);
		ConvertCoordClose ();
	}
    GSSiSetCursor (hcurSave);
    if (pNumPoints)
    	*pNumPoints = N;
    if (pRSQ)
    	*pRSQ = RSQMIN; 
	return handle;
}  

HANDLE LoadTranFileWithDandT (LPSTR Name)
{
	short	Dir=1, Type=1;
	LPSTR	lpPar; 
	char	Name2[256];
	HANDLE	handle;   
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
	BOOL	Invert=FALSE;
	
	if (!_fstrncmp (Name,"|OPP|",5))
	{
		Name += 5;
		Invert = TRUE;
	}
	_fstrcpy (Name2,Name); 
	_fstrlwr (Name2);
	if (_fstrstr (Name2,".ctr"))
	{
		Fid = GSSiOpenFile (Name2,&OFStruct,OF_READ);
		handle =  ReadTranData (Fid);
		GSSiClose2 (&Fid);
	}
	else
	{               		
		if ((lpPar = _fstrchr (Name2,'(')))
		{
			*lpPar++ = 0;
			if (*lpPar == 'R' || *lpPar == 'r')
				Dir = 2;
			lpPar++;
			if (*lpPar == ',')
			{
				lpPar++;
				Type = atoi(lpPar);
			}
		}
		if (Invert)
		{
			if (Dir == 1)
				Dir = 2;
			else
				Dir = 1;
		}
		handle = LoadTranFile (Name2,Dir,Type,NULL,NULL);  
	}
	return handle;
}

void WriteTranData (HFILE Fid,HANDLE hTran)
{
    LPTRANDATA TranPtr;
    
    TranPtr = (LPTRANDATA) GlobalLock (hTran); 
    BigWrite (Fid,(HPSTR)TranPtr,sizeof(TRANDATA),-1);
	if (TranPtr->TriHandle)
	{
	    HPTRANTRI	Tri=(HPTRANTRI)GlobalLock (TranPtr->TriHandle);
	    short		NumTri = Tri->NumTri;

	    while (NumTri--)
		    BigWrite (Fid,(HPSTR)Tri++,sizeof(TRANTRI),-1); 
		GlobalUnlock (TranPtr->TriHandle);
	}

    GlobalUnlock (hTran); 
    return;
} 

double GetTranAZ (HANDLE hTrans)
{
	DPOINT FromPt[2], ToPt[2];
	double AZ;
	LPTRANDATA	TranPtr;

    if (!hTrans)
		return 0;
	
	TranPtr = (LPTRANDATA)GlobalLock(hTrans); 
	FromPt[0].x = TranPtr->Bounds.xmn;
	FromPt[0].y = TranPtr->Bounds.ymn;
	FromPt[1].x = TranPtr->Bounds.xmx+1;
	FromPt[1].y = TranPtr->Bounds.ymn;
	GlobalUnlock (hTrans);
	ToPt[0] = TranPoint (&FromPt[0],hTrans);
	ToPt[1] = TranPoint (&FromPt[1],hTrans);
	AZ = getazd (&ToPt[0],&ToPt[1]);
	return AZ;
}

MNMXCORD GetTranFileBounds (LPSTR Name,LPSTR Direction)
{
	HFILE Fid;
	char	str[260]; 
	MNMXCORD	Bounds;
	DPOINT	Point[2];
	short	dir=0;
	
	if (*Direction == 'R' || *Direction == 'r')
		dir=1;	

	DBoundsInit (&Bounds);	
	Fid = GSSiOpenFile (Name,NULL,OF_READ);
	if (Fid == HFILE_ERROR)
		return Bounds;
	while (fgetstring (str,256,Fid))
	{ 
		if (!*str || *str == '#')
			continue;
		if (sscanf (str,"%lf %lf %lf %lf",&Point[0].x,&Point[0].y,&Point[1].x,&Point[1].y) == 4) 
			AddDPointToMinMax (&Point[dir],&Bounds);
	}  
	GSSiClose2 (&Fid);
	return Bounds;
}
                
HANDLE STRANRect (LPRECT FromRect,LPRECT ToRect)
{
	HANDLE hTran; 
	double	XFROM[4],YFROM[4],XTO[4],YTO[4];  
	float	RSQMIN;
	
	XFROM[0] = FromRect->left;
	XFROM[1] = FromRect->left;
	XFROM[2] = FromRect->right;
	XFROM[3] = FromRect->right; 
	YFROM[0] = FromRect->bottom;
	YFROM[1] = FromRect->top;
	YFROM[2] = FromRect->top;
	YFROM[3] = FromRect->bottom; 
	XTO[0] = ToRect->left;
	XTO[1] = ToRect->left;
	XTO[2] = ToRect->right;
	XTO[3] = ToRect->right; 
	YTO[0] = ToRect->bottom;
	YTO[1] = ToRect->top;
	YTO[2] = ToRect->top;
	YTO[3] = ToRect->bottom; 
	
	hTran = STRAN2 (1662,XFROM,YFROM,XTO,YTO,4,(LPFLOAT)&RSQMIN,1,NULL);
	return hTran;
}

void TRANRect (LPRECT Rect,HANDLE hTran)
{   
	double	Xfrom,Yfrom,Xto,Yto;  
	RECT	OutRect;
	POINT	Point,OutPoint;
	
	RectInit (&OutRect); 
	Point.x = Rect->left;
	Point.y = Rect->bottom;
	OutPoint = TranPoint16 (Point,hTran);
	AddPointToRect (OutPoint,&OutRect);
	Point.y = Rect->top;
	OutPoint = TranPoint16 (Point,hTran);
	AddPointToRect (OutPoint,&OutRect);
	Point.x = Rect->right;
	OutPoint = TranPoint16 (Point,hTran);
	AddPointToRect (OutPoint,&OutRect);
	Point.y = Rect->bottom;
	OutPoint = TranPoint16 (Point,hTran);
	AddPointToRect (OutPoint,&OutRect);
	*Rect = OutRect;
	return;
}

HANDLE STRANRectToBounds (LPRECT FromRect,LPMNMXCORD ToRect)
{
	HANDLE hTran; 
	double	XFROM[4],YFROM[4],XTO[4],YTO[4];  
	float	RSQMIN;
	
	XTO[0] = ToRect->xmn;
	XTO[1] = ToRect->xmn;
	XTO[2] = ToRect->xmx;
	XTO[3] = ToRect->xmx; 
	YTO[0] = ToRect->ymn;
	YTO[1] = ToRect->ymx;
	YTO[2] = ToRect->ymn;
	YTO[3] = ToRect->ymx;   
	
	XFROM[0] = FromRect->left;
	XFROM[1] = FromRect->left;
	XFROM[2] = FromRect->right;
	XFROM[3] = FromRect->right; 
	YFROM[0] = FromRect->bottom;
	YFROM[1] = FromRect->top;
	YFROM[2] = FromRect->bottom;
	YFROM[3] = FromRect->top; 
	
	hTran = STRAN2 (1663,XFROM,YFROM,XTO,YTO,4,(LPFLOAT)&RSQMIN,1,NULL);
	return hTran;
}

HANDLE STRANBoundsToRect (LPMNMXCORD FromRect,LPRECT ToRect)
{
	HANDLE hTran; 
	double	XFROM[4],YFROM[4],XTO[4],YTO[4];  
	float	RSQMIN;
	
	XFROM[0] = FromRect->xmn;
	XFROM[1] = FromRect->xmn;
	XFROM[2] = FromRect->xmx;
	XFROM[3] = FromRect->xmx; 
	YFROM[0] = FromRect->ymn;
	YFROM[1] = FromRect->ymx;
	YFROM[2] = FromRect->ymn;
	YFROM[3] = FromRect->ymx; 
	XTO[0] = ToRect->left;
	XTO[1] = ToRect->left;
	XTO[2] = ToRect->right;
	XTO[3] = ToRect->right; 
	YTO[0] = ToRect->bottom;
	YTO[1] = ToRect->top;
	YTO[2] = ToRect->bottom;
	YTO[3] = ToRect->top; 
	
	hTran = STRAN2 (1664,XFROM,YFROM,XTO,YTO,4,(LPFLOAT)&RSQMIN,1,NULL);
	return hTran;
}

HANDLE STRANBoundsToBounds (LPMNMXCORD FromRect,LPMNMXCORD ToRect)
{
	HANDLE hTran; 
	double	XFROM[4],YFROM[4],XTO[4],YTO[4];  
	float	RSQMIN;
	
	XFROM[0] = FromRect->xmn;
	XFROM[1] = FromRect->xmn;
	XFROM[2] = FromRect->xmx;
	XFROM[3] = FromRect->xmx; 
	YFROM[0] = FromRect->ymn;
	YFROM[1] = FromRect->ymx;
	YFROM[2] = FromRect->ymn;
	YFROM[3] = FromRect->ymx; 
	XTO[0] = ToRect->xmn;
	XTO[1] = ToRect->xmn;
	XTO[2] = ToRect->xmx;
	XTO[3] = ToRect->xmx; 
	YTO[0] = ToRect->ymn;
	YTO[1] = ToRect->ymx;
	YTO[2] = ToRect->ymn;
	YTO[3] = ToRect->ymx;   
	
	hTran = STRAN2 (1665,XFROM,YFROM,XTO,YTO,4,(LPFLOAT)&RSQMIN,1,NULL);
	return hTran;
}

void TranPoints(LPDPOINT FromPt, LPDPOINT ToPt, int nPt, HANDLE hTran)
{
	for (int i = 0; i < nPt; i++)
	{
		ToPt[i] = TranPoint(&FromPt[i], hTran);
	}
}

HANDLE STRANPoints (int id,LPDPOINT FromPt,LPDPOINT ToPt,int nPt,LPFLOAT pRSQMIN,int Type,LPMNMXCORD pBounds)
{
	HANDLE hTran; 
	//double	XFROM[4],YFROM[4],XTO[4],YTO[4]; 
	HANDLE hFromTo = GSSiGlobAlloc(0, GMEM_MOVEABLE, nPt * 4 * sizeof(double));
	LPDOUBLE XFROM = GlobalLock(hFromTo);
	LPDOUBLE YFROM = &XFROM[nPt];
	LPDOUBLE XTO = &YFROM[nPt];
	LPDOUBLE YTO = &XTO[nPt];
	float	RSQMIN;
	int	i;

	for (i=0;i<nPt;i++)
	{
		XFROM[i] = FromPt[i].x;
		YFROM[i] = FromPt[i].y;
		XTO[i] = ToPt[i].x;
		YTO[i] = ToPt[i].y;
	}
	
	hTran = STRAN2 (id,XFROM,YFROM,XTO,YTO,nPt,(LPFLOAT)&RSQMIN,Type,pBounds);
	if (pRSQMIN)
		*pRSQMIN = RSQMIN;
	GSSiGlobUlFree(&hFromTo);
	return hTran;
}

POINT TRANDPointToPoint (HPDPOINT pDPoint,HANDLE hTran)
{   
	double	Xto,Yto;
	POINT	Point;  
	
	TRANS2 (pDPoint->x,pDPoint->y,&Xto,&Yto,hTran); 
	Point.x = IDNINT(Xto);
	Point.y = IDNINT(Yto);
	return Point;
} 

HANDLE DPolyToPPoly (LPINT pnpnts, HANDLE hDPoly,HANDLE hTran)
{
	HANDLE handle = GSSiGlobAlloc ( 761,GMEM_MOVEABLE,(*pnpnts)*sizeof(POINT));
	HPPOINT	pPoints = (HPPOINT)GlobalLock (handle);
	HPDPOINT	pDPoints = GlobalLock(hDPoly);
	DWORD	i,np=*pnpnts, nnewpnts=1;
	
	pPoints[0] = TRANDPointToPoint (pDPoints++,hTran);
	for (i=1;i<np;i++,pDPoints++)
	{
		pPoints[nnewpnts] = TRANDPointToPoint (pDPoints,hTran);
		if (!SamePoint (pPoints[nnewpnts-1],pPoints[nnewpnts]))
			i++;
	}
	*pnpnts = nnewpnts;
	GlobalUnlock (handle);
	return handle;
}

void CloseTRANS2 (LPHANDLE phlpTran)
{    
	LPTRANDATA  TranPtr;

	if (*phlpTran > (HANDLE)1)
	{   
		TranPtr = (LPTRANDATA)GlobalLock(*phlpTran);
		if (!TranPtr)
		{
			*phlpTran = 0;	
			return;
		}
		if (TranPtr->TriHandle)
		{
		    HPTRANTRI	Tri=(HPTRANTRI)GlobalLock (TranPtr->TriHandle);
		    short		NumTri = Tri->NumTri;

		    while (NumTri--)
		    {
		    	CloseTRANS2 (&Tri->hFromTran);
		    	CloseTRANS2 (&Tri++->hToTran);
		    } 
		    GSSiGlobUlFree (&TranPtr->TriHandle);
		}
		GSSiGlobUlFree (phlpTran);
	}
	*phlpTran = 0;
	return;

}   

DPOINT TranPoint (LPDPOINT pPoint,HANDLE hTran)
{
	DPOINT Point;
	
	TRANS2 (pPoint->x,pPoint->y, &Point.x,&Point.y,hTran);
	return Point;
}

POINT TranPoint16 (POINT InPoint,HANDLE hTran)
{
	DPOINT	Point,OutPoint;
	POINT	RtnPoint;
	
	Point.x = InPoint.x;
	Point.y = InPoint.y;
	TRANS2 (Point.x,Point.y, &OutPoint.x,&OutPoint.y,hTran);    
	RtnPoint.x = IDNINT(OutPoint.x);
	RtnPoint.y = IDNINT(OutPoint.y);
	return RtnPoint;
}

void TRNPRO (double XIN,double YIN, LPDOUBLE XOUT,LPDOUBLE YOUT, HANDLE hlpTran)
{   double XI, YI;
    LPTRANDATA  TranPtr;

      TranPtr = (LPTRANDATA)GlobalLock(hlpTran);
      XI     = XIN - TranPtr->BASX;
      YI     = YIN - TranPtr->BASY;
      *XOUT   = TranPtr->A1 + TranPtr->B1 * XI + TranPtr->C1 * YI;
      *YOUT   = TranPtr->A2 + TranPtr->B2 * YI + TranPtr->C2 * XI;
      GlobalUnlock(hlpTran);
      return;
}

void TRITRAN (double XIN,double YIN, LPDOUBLE XOUT,LPDOUBLE YOUT,HPTRANTRI Tri)
{
	return;
}

BOOL WriteTranCHeader (HANDLE hTran,HFILE Fid)
{
    BOOL rtn = FALSE; 
	char str[256];

	if (hTran)
	{
		LPTRANDATA	TranPtr = (LPTRANDATA)GlobalLock(hTran);

		sprintf (str,"\tdouble BASX = %.16lg, BASY = %.16lg;",TranPtr->BASX,TranPtr->BASY);
		fputstring (str,Fid);
		sprintf (str,"\tdouble A1 = %.16lg, B1 = %.16lg, C1 = %.16lg;",TranPtr->A1,TranPtr->B1,TranPtr->C1);
		fputstring (str,Fid);
		sprintf (str,"\tdouble A2 = %.16lg, B2 = %.16lg, C2 = %.16lg;",TranPtr->A2,TranPtr->B2,TranPtr->C2);
		fputstring (str,Fid);
		rtn = TRUE;
		GlobalUnlock (hTran);
	}
	return rtn;
}


void TRANS2 (double XIN,double YIN, LPDOUBLE XOUT,LPDOUBLE YOUT, HANDLE hlpTran)
#if ENABLETRACE
{GSSiEnterProg (1435);
#endif
{    DPOINT WinPointD, Intmod;

	double XI, YI;
    LPTRANDATA  TranPtr;
	DPOINT		Point; 
	short	ii;
      
      if (!hlpTran || hlpTran == (HANDLE)1) 
      {
      	*XOUT = XIN;
      	*YOUT = YIN;
      	goto Exit2;
      }
	  Point.x = XIN;
	  Point.y = YIN;	
      TranPtr = (LPTRANDATA)GlobalLock(hlpTran); 
	  if (!TranPtr)
	  {
		  *XOUT = XIN;
		  *YOUT = YIN;
		  goto Exit2;
	  }

	  if (!strncmp((LPSTR)TranPtr, "PARCELTRAN", 10))
	  {
		  TranToParcelPoints(&XIN, &YIN, XOUT, YOUT, (LPPARCELTRAN)TranPtr);
		  goto Exit;
	  }
/* 3/15/07 not sure why you would want to do this - messes up using bounds for az computation ($GETTRANDATA)
		   seems better to use not fixed tran if out of bounds
      if (!PointInBounds (Point,&TranPtr->Bounds))
      { 
      	GlobalUnlock (hlpTran);
      	*XOUT = XIN;
      	*YOUT = YIN;
      	return;
      }
*/
      XI     = XIN - TranPtr->BASX;
      YI     = YIN - TranPtr->BASY ;
      if (TranPtr->FIXEDP)
      {     
			if (DPointInBounds (&Point,&TranPtr->TriBounds))   
			{
				HPTRANTRI	Tri, TriBeg=(HPTRANTRI)GlobalLock (TranPtr->TriHandle);
				short		NumTri = TriBeg->NumTri, TriNum=TranPtr->LastTri;  
				BOOL		First;
				
				Tri = TriBeg;
				if (TranPtr->LastTri < NumTri)
				{
					First = TRUE;
					Tri += TranPtr->LastTri;
					NumTri = 1;
				}
				else
					First = FALSE;
		AllTri:
				while (NumTri--)
				{
					USHORT	ip;
						
					for (ip=0;ip<3;ip++)
					{
						if (fabs (Point.x - Tri->FromPT[ip].x) < P_TOL && fabs (Point.y - Tri->FromPT[ip].y) <P_TOL)
						{   
							*XOUT = Tri->ToPT[ip].x;
							*YOUT = Tri->ToPT[ip].y;
							GlobalUnlock (TranPtr->TriHandle);
					      	goto Exit;
						      
						}
					}
					if ((First || TriNum != TranPtr->LastTri) && DPointInBounds (&Point,&Tri->FromMNMX))
					{   
						if (POINT_IN_AREAD (Point,4,Tri->FromPT,1,0,NULL,NULL))
						{   
							UINT	i;
	/*						for (i=0;i<3;i++)
								DebugShowLine (&Tri->FromPT[i],&Tri->FromPT[i+1]); */
							
							if (!Tri->hFromTran)
							{
								double X1[3],Y1[3],X2[3],Y2[3];
								float	RSQMIN;
								
								for (i=0;i<3;i++)
								{
									X1[i] = Tri->FromPT[i].x;	
									Y1[i] = Tri->FromPT[i].y;	
									X2[i] = Tri->ToPT[i].x;	
									Y2[i] = Tri->ToPT[i].y;	
								}
								Tri->hFromTran = STRAN2 (1666,X1,Y1,X2,Y2,3,&RSQMIN,1,NULL);
							}
							//TRITRAN (XIN,YIN,XOUT,YOUT,Tri);
							TRANS2 (XIN,YIN,XOUT,YOUT,Tri->hFromTran);
							GlobalUnlock (TranPtr->TriHandle);
							TranPtr->LastTri = TriNum;
							goto Exit;
						}
					} 
					if (First)
					{
						First = FALSE;
						NumTri = TriBeg->NumTri;
						TriNum = 0;
						Tri = TriBeg;
						goto AllTri;
					}
					Tri++;
					TriNum++;
				}
				GlobalUnlock (TranPtr->TriHandle);
			}
			else
				ii=1;
      }
      if (TranPtr->FIXEDP)
      	ii=1;
      *XOUT   = TranPtr->A1 + TranPtr->B1 * XI + TranPtr->C1 * YI;
      *YOUT   = TranPtr->A2 + TranPtr->B2 * YI + TranPtr->C2 * XI;
Exit:
      GlobalUnlock(hlpTran);
Exit2:
{
#if ENABLETRACE
GSSiExitProg (1435);
#endif
     return;
}
#if ENABLETRACE
}
#endif
}

XFORM SetXFORMFromTRANS (HANDLE hlpTran)
{   
	XFORM XForm;
    LPTRANDATA  TranPtr;

	if (hlpTran)
	{
		TranPtr = (LPTRANDATA)GlobalLock(hlpTran); 
		XForm.eDx = TranPtr->A1;
		XForm.eDy = TranPtr->A2;
		XForm.eM11 = TranPtr->B1;
		XForm.eM21 = TranPtr->C1;
		XForm.eM12 = TranPtr->C2;
		XForm.eM22 = TranPtr->B2;
		GlobalUnlock(hlpTran);
	}
	else
	{
		XForm.eDx = 1;
		XForm.eDy = 0;
		XForm.eM11 = 0;
		XForm.eM21 = 1;
		XForm.eM12 = 0;
		XForm.eM22 = 0;
	}
    return XForm;
} 

int priorParPoint(int i, int nParPnt)
{
	if (!i)
		i = nParPnt - 1;
	else
		i--;
	return i;
}
int nextParPoint(int i, int nParPnt)
{
	if (i == nParPnt - 1)
		i = 0;
	else
		i++;
	return i;
}

BOOL pointsAreParcelBoundaryLine(double X1, double Y1, double X2, double Y2, int nParPnt, LPDPOINT parPoints)
{
	int i;
	DPOINT p1,p2;

	p1.x = X1 + currentBASX;
	p1.y = Y1 + currentBASY;
	p2.x = X2 + currentBASX;
	p2.y = Y2 + currentBASY;

	for (i = 0; i < nParPnt; i++)
	{
		if (SameDPoint(&p1, &parPoints[i]))
		{
			if (SameDPoint(&p2, &parPoints[priorParPoint(i, nParPnt)]) ||
				SameDPoint(&p2, &parPoints[nextParPoint(i, nParPnt)]))
				return TRUE;
		}
	}
	return FALSE;
}

double LDISTtran(double X1,double Y1,double X2, double Y2)
{
	double d;

	if (currentParcelTran)
	{
		int i;
		LPINT pnp = GlobalLock(currentParcelTran->hParNumPt);
		LPHANDLE phParPoints = GlobalLock(currentParcelTran->hpParPnts);
		for (i = 0; i < currentParcelTran->nParcels; i++,pnp++)
		{
			LPDPOINT pParPoints = GlobalLock(phParPoints[i]);

			if (pointsAreParcelBoundaryLine(X1, Y1, X2, Y2,*pnp,pParPoints))
			{
				GlobalUnlock(currentParcelTran->hParNumPt);
				GlobalUnlock(phParPoints[i]);
				GlobalUnlock(currentParcelTran->hpParPnts);
				return 0;
			}
			GlobalUnlock(phParPoints[i]);
		}
		GlobalUnlock(currentParcelTran->hpParPnts);
		GlobalUnlock(currentParcelTran->hParNumPt);
	}

	d = LDIST(X1, Y1, X2, Y2);
	return d;
}

HANDLE TRFTRI_SET (LPDOUBLE XT,LPDOUBLE YT,LPDOUBLE XT2,LPDOUBLE YT2,
				   int NSETPTin,double BASX1, double BASY1,double BASX2, double BASY2,HANDLE hTran,LPMNMXCORD pTriBounds)   
{
/*    ENTRY TRFTRI (X,Y,TRINUM,VERTEX,TRNNUM)
C*    ENTRY GET_TRAN_TRI_2 (NTRI,VERTEXES,TRNNUM)
C*    ENTRY TRANS2_WHICH_TRI (WHICH_TRI)
C********SPECIFICATIONS************************************************
C*                                                                    *
C*       PROGRAM SUMMARY                                              *
C*       ------- -------                                              *
C*    TRFTRI (TRANSFORMATION: FIND TRIANGLE) FINDS A TRIANGLE FORMED  *
C*    FROM THREE TRANSFORMATION POINTS WHICH ENCOMPASSES THE POINT    *
C*    X,Y. IF X,Y LIES OUTSIDE ALL TRIANGLES FORMED BY THE CLOSEST    *
C*    TRAN POINT TO X,Y AND ANY OTHER TWO TRAN POINTS THEN ONE OF     *
C*    THE TRIANGLES CONTAINING THE NEAREST POINT IS PICKED.           *
C*                                                                    *
C*       ARGUMENT DESCRIPTION                                         *
C*       -------- -----------                                         *
C*    X, Y     R*8  COORDINATES OF THE POINT TO BE TRANSFORMED IN     *
C*                  THE ORIGINAL SYSTEM COORD SYSTEM                  *
C*    XT, YT   R*8  COORDINATES OF THE TRANSFORMATION POINTS IN THE   *
C*                  ORIGINAL SYSTEM SYSTEM (DIMENSIONED TO NSETPT)    *
C*    NSETPT   I*4  THE NUMBER OF TRANSFORMATION POINTS USED          *
C*    TRINUM   I*4  THE NUMBER OF THE TRIANGLE NUMBER FOUND           *
C*    VERTEX   I*4(3) THE VERTEXES OF THE TRIANGLE FOUND              *
C*    TRNNUM   I*4   TRANSFORMATION NUMBER                            *
C*                                                                    *
C*       AUTHOR                                                       *
C*       ------                                                       *
C*     JEFF SMITH                                                     *
C*                                                                    *
C**********************************************************************
C*
%include 'trans2_common.ftn' {/umsc/include/trans2_common.ftn} */
	short	NSETPT = NSETPTin + 8; 
	short	NSPM1, I,J,K,IPLUS1, AtPt, ToPt, OpEnd;
	double	InAZ, OutAZ, Daz, MinAZ;
	long	MXT = NSETPT * 6, MXPF = Factorial (NSETPT)+1;
	long	NDIST;
	MNMXCORD MinMaxCoord;
	DPOINT	Point1,Point2;
	double	width, height;  
#define MAX_LINES_AT_POINT	255	
typedef struct {short	nlines;
				short	lines[MAX_LINES_AT_POINT];
} POINTINFO;
typedef POINTINFO	HUGE	*HPPOINTINFO;
typedef struct {char	left,right;} TRACK;
typedef TRACK	FAR	*LPTRACK;

    HANDLE	TriHandle = GSSiGlobAlloc (1286,GHND,MXT*(long)sizeof(TRANTRI));    
    HPTRANTRI	Tri=(HPTRANTRI)GlobalLock (TriHandle);
	HANDLE	hNLATPT=GSSiGlobAlloc (1287,GHND,MXT*2);
	LPSHORT	NLATPT = (LPSHORT)GlobalLock (hNLATPT);
	HANDLE	hPointInfo=GSSiGlobAlloc (1288,GHND,(long)NSETPT*sizeof(POINTINFO));
	HPPOINTINFO PointInfo = (HPPOINTINFO)GlobalLock (hPointInfo);
	HANDLE	hORDERD=GSSiGlobAlloc (1289,GMEM_MOVEABLE,MXPF*4);
	HPLONG	ORDERD = (HPLONG)GlobalLock (hORDERD);
	HANDLE	hDIST=GSSiGlobAlloc (1290,GMEM_MOVEABLE,MXPF*8);
	HPDOUBLE DIST = (HPDOUBLE)GlobalLock (hDIST);
	HANDLE	hP1=GSSiGlobAlloc (1291,GMEM_MOVEABLE,MXPF*2);
	HPSHORT P1 = (HPSHORT)GlobalLock (hP1);
	HANDLE	hP2=GSSiGlobAlloc (1292,GMEM_MOVEABLE,MXPF*2);
	HPSHORT P2 = (HPSHORT)GlobalLock (hP2);
	HANDLE	hLP1=GSSiGlobAlloc (1293,GMEM_MOVEABLE,MXPF*2);
	HPSHORT LP1 = (HPSHORT)GlobalLock (hLP1);
	HANDLE	hLP2=GSSiGlobAlloc (1294,GMEM_MOVEABLE,MXPF*2);
	HPSHORT LP2 = (HPSHORT)GlobalLock (hLP2);
	HANDLE	hTrack=GSSiGlobAlloc (1295,GHND,MXPF*sizeof(TRACK));
	LPTRACK Track = (LPTRACK)GlobalLock (hTrack);
	
	double	D;
	short	FirstNotDone=0, NUMTRI=0, PT3, L2;
	BOOL	GotOne=TRUE, Reverse; 
	long	iord, iord2, IP1, IP2, NLINES; 
	double	Offset=0.5;
/*	
     +         VERTX(3), VERTY(3), XN, YN, CLOSE, DST
      REAL * 8 PN            , NORMX(3)               , AREA(MXT),
     +                         NORMY(3)               , AZ1, AZ2,
     +                         TGETAZ, TTWOPI, TOOSML,
     +                         TWOPI
      INTEGER*4 TRNID, SETID(MXT), TRINUM, SORVER(3),
     +          ORDERD(MXPF)
      INTEGER*4 TRNNUM
      INTEGER*2 P1(MXPF), P2(MXPF), LP1(MXPF), LP2(MXPF), VERTEX(3),
     +          LATPT(24,MXT), NLATPT(MXT), NTRI, VERTEXES(3,MXT),
     +           ROT(MXT), WHICH_TRI
      LOGICAL LININT
      DATA     PN/'TRFTRI  '/, NORMX   /0D0, 0D0, 1D0/,
     +                         NORMY   /0D0, 1D0, 0D0/,
     +                                         TOOSML/0.01D0/,
     +                         TWOPI/6.283185307179586D0/   
*/
		DBoundsInit (&MinMaxCoord);
		for (I=0;I<NSETPTin;I++)
		{ 
			Point1.x = XT[I];
			Point1.y = YT[I];
			AddDPointToMinMax (&Point1,&MinMaxCoord);
        } 
        *pTriBounds = MinMaxCoord;    
        width = MinMaxCoord.xmx - MinMaxCoord.xmn;
        height = MinMaxCoord.ymx - MinMaxCoord.ymn;
        pTriBounds->xmn -= width * Offset; 
        pTriBounds->ymn -= height* Offset; 
        pTriBounds->xmx += width * Offset; 
        pTriBounds->ymx += height* Offset; 
        pTriBounds->xmn += BASX1; 
        pTriBounds->xmx += BASX1;  
        pTriBounds->ymn += BASY1;
        pTriBounds->ymx += BASY1;
        XT[I] = MinMaxCoord.xmn -  width*Offset;
        YT[I] = MinMaxCoord.ymn - height*Offset;
		TRNPRO (XT[I]+BASX1,YT[I]+BASY1,&XT2[I],&YT2[I],hTran);
		XT2[I]-=BASX2; 
		YT2[I]-=BASY2;
		I++;
        XT[I] = MinMaxCoord.xmn -  width*Offset;
        YT[I] = (MinMaxCoord.ymn + MinMaxCoord.ymx)/2;
		TRNPRO (XT[I]+BASX1,YT[I]+BASY1,&XT2[I],&YT2[I],hTran);
		XT2[I]-=BASX2; 
		YT2[I]-=BASY2;
		I++;
        XT[I] = MinMaxCoord.xmn -  width*Offset;
        YT[I] = MinMaxCoord.ymx + height*Offset;
		TRNPRO (XT[I]+BASX1,YT[I]+BASY1,&XT2[I],&YT2[I],hTran);
		XT2[I]-=BASX2; 
		YT2[I]-=BASY2;
		I++;
        XT[I] = (MinMaxCoord.xmn + MinMaxCoord.xmx)/2;
        YT[I] = MinMaxCoord.ymx + height*Offset;
		TRNPRO (XT[I]+BASX1,YT[I]+BASY1,&XT2[I],&YT2[I],hTran);
		XT2[I]-=BASX2; 
		YT2[I]-=BASY2;
		I++;
        XT[I] = MinMaxCoord.xmx +  width*Offset;
        YT[I] = MinMaxCoord.ymx + height*Offset;
		TRNPRO (XT[I]+BASX1,YT[I]+BASY1,&XT2[I],&YT2[I],hTran);
		XT2[I]-=BASX2; 
		YT2[I]-=BASY2;
		I++;
        XT[I] = MinMaxCoord.xmx +  width*Offset;
        YT[I] = (MinMaxCoord.ymn + MinMaxCoord.ymx)/2;
		TRNPRO (XT[I]+BASX1,YT[I]+BASY1,&XT2[I],&YT2[I],hTran);
		XT2[I]-=BASX2; 
		YT2[I]-=BASY2;
		I++;
        XT[I] = MinMaxCoord.xmx +  width*Offset;
        YT[I] = MinMaxCoord.ymn - height*Offset;
		TRNPRO (XT[I]+BASX1,YT[I]+BASY1,&XT2[I],&YT2[I],hTran);
		XT2[I]-=BASX2; 
		YT2[I]-=BASY2;
		I++;
        XT[I] = (MinMaxCoord.xmn + MinMaxCoord.xmx)/2;
        YT[I] = MinMaxCoord.ymn - height*Offset;
		TRNPRO (XT[I]+BASX1,YT[I]+BASY1,&XT2[I],&YT2[I],hTran);
		XT2[I]-=BASX2; 
		YT2[I]-=BASY2;
//     ************************************************************
//     *  FIND THE SET OF TRIANGLES WITH THE SHORTEST SIDES WHICH *
//     *  COVERS THE SPACE.                                       *
//     ************************************************************
//
//******* FIND THE DISTANCES BETWEEN ALL PAIRS OF SET UP POINTS
//        AND SORT THEM.

      NDIST  = 0;
      NSPM1  = NSETPT - 1;  
      for (I=0;I<NSPM1;I++)
      {
          IPLUS1 = I + 1;
          for (J=IPLUS1;J<NSETPT;J++)
          {
              P1[NDIST] = I;
              P2[NDIST] = J;
              D = LDISTtran (XT[I],YT[I],XT[J],YT[J]);
              for (iord = 0;iord<NDIST;iord++)
              {
              	if (D < DIST[ORDERD[iord]]) 
              	{
              		for (iord2=NDIST;iord2>iord;iord2--)
              			ORDERD[iord2]=ORDERD[iord2-1];
              		ORDERD[iord] = NDIST;
              		goto S10;
              	}
              }
              ORDERD[NDIST] = NDIST;
          S10:
              DIST[NDIST++] = D;
  		  }
  	  }
//******* FIND ALL SHORTEST NON-INTERSECTING LINES.
      NLINES = 0;
      for (iord=0;iord<NDIST;iord++)
      {
          IP1    = P1[ORDERD[iord]];
          IP2    = P2[ORDERD[iord]];
          for (J=0;J<NLINES;J++)
          {
	          if (LININT (XT[IP1],YT[IP1],XT[IP2],YT[IP2],
                     	  XT[LP1[J]],YT[LP1[J]],
                     	  XT[LP2[J]],YT[LP2[J]])) goto S200;
		  }
          LP1[NLINES] = (short)IP1;
          LP2[NLINES] = (short)IP2;  
          if (PointInfo[IP1].nlines >= MAX_LINES_AT_POINT ||  
			  PointInfo[IP2].nlines >= MAX_LINES_AT_POINT)
          	GSSiMessageBox (0,"Too many lines at one point - add more interior points",
          					"Transformation Error",MB_ICONEXCLAMATION,0);
		  else if (NLINES < MXPF)
          {
	          PointInfo[IP1].lines[PointInfo[IP1].nlines++]=(short)NLINES;
	          PointInfo[IP2].lines[PointInfo[IP2].nlines++]=(short)NLINES++;
	      }
		  else
		  {
			  GSSiMessageBox (0,"Too many points",
				  "Transformation Error", MB_ICONEXCLAMATION, 0);
		  }
	  S200:;
	} 
	
		DBoundsInit (&MinMaxCoord);
		for (J=0;J<NLINES;J++)
		{   
			short	k;
			
			Point1.x = BASX1+XT[LP1[J]];
			Point1.y = BASY1+YT[LP1[J]];
			Point2.x = BASX1+XT[LP2[J]];
			Point2.y = BASY1+YT[LP2[J]];
			AddDPointToMinMax (&Point1,&MinMaxCoord);
			AddDPointToMinMax (&Point2,&MinMaxCoord);
			DebugShowLine (&Point1,&Point2);
			for (k=NSETPTin;k<NSETPT-1;k++)  
			{
				if (LP1[J] == k && LP2[J] == k+1)
					Track[J].left = TRUE;	
				else if (LP1[J] == k+1 && LP2[J] == k)
					Track[J].right = TRUE;
			}	
			if (LP1[J] == NSETPTin && LP2[J] == NSETPT-1)
				Track[J].right = TRUE;	
			else if (LP1[J] == NSETPT-1 && LP2[J] == NSETPTin)
				Track[J].left = TRUE;
		}

//******* DEFINE THE TRIANGLES.
		NUMTRI = 0;  
		while (GotOne)
		{
	      GotOne = FALSE;
	      for (I=FirstNotDone;I<NLINES;I++)
	      {
	      	if (Track[I].left && Track[I].right)
	      	{
	      		if (I == FirstNotDone)
	      			FirstNotDone++;
	      	}
	      	else
	      	{   
	      		GotOne = TRUE;
	      		if (Track[I].right)
	      		{
	      			Reverse = TRUE;  
	      			Track[I].left = TRUE; 
	      			AtPt = LP2[I];
	      			ToPt = LP1[I];
	      		}
	      		else
	      		{
	      			Reverse = FALSE;  
	      			Track[I].right = TRUE;
	      			AtPt = LP1[I];   
	      			ToPt = LP2[I];
	      		}
      			Tri[NUMTRI].FromPT[0].x = XT[AtPt]+BASX1;
      			Tri[NUMTRI].FromPT[0].y = YT[AtPt]+BASY1;
      			Tri[NUMTRI].FromPT[1].x = XT[ToPt]+BASX1;
      			Tri[NUMTRI].FromPT[1].y = YT[ToPt]+BASY1; 
      			Tri[NUMTRI].ToPT[0].x = XT2[AtPt]+BASX2;
      			Tri[NUMTRI].ToPT[0].y = YT2[AtPt]+BASY2;
      			Tri[NUMTRI].ToPT[1].x = XT2[ToPt]+BASX2;
      			Tri[NUMTRI].ToPT[1].y = YT2[ToPt]+BASY2; 
      			InAZ = getazd (&Tri[NUMTRI].FromPT[1], &Tri[NUMTRI].FromPT[0]); 
      			MinAZ = 9999;
      			for (J=0;J<PointInfo[ToPt].nlines;J++)
      			{   
      				K = PointInfo[ToPt].lines[J];
      				if (K != I)
      				{   
      					if (LP1[K] == ToPt)
      						OpEnd = LP2[K];
      					else
      						OpEnd = LP1[K];
   						OutAZ = LGETAZ (XT[ToPt],YT[ToPt],XT[OpEnd],YT[OpEnd]);
						Daz = MinAngleToTheRight (InAZ, OutAZ);
						if (Daz < MinAZ)
						{
							PT3 = OpEnd;
							L2 = K;
							MinAZ = Daz;
						}
					}
				}
      			Tri[NUMTRI].FromPT[2].x = XT[PT3]+BASX1;
      			Tri[NUMTRI].FromPT[2].y = YT[PT3]+BASY1;
      			Tri[NUMTRI].ToPT[2].x = XT2[PT3]+BASX2;
      			Tri[NUMTRI].ToPT[2].y = YT2[PT3]+BASY2;
      			Tri[NUMTRI].FromPT[3] = Tri[NUMTRI].FromPT[0];
      			Tri[NUMTRI].ToPT[3] = Tri[NUMTRI].ToPT[0];
      			DBoundsInit (&Tri[NUMTRI].FromMNMX);
      			DBoundsInit (&Tri[NUMTRI].ToMNMX);
				AddDPointToMinMax (&Tri[NUMTRI].FromPT[0],&Tri[NUMTRI].FromMNMX);
				AddDPointToMinMax (&Tri[NUMTRI].FromPT[1],&Tri[NUMTRI].FromMNMX);
				AddDPointToMinMax (&Tri[NUMTRI].FromPT[2],&Tri[NUMTRI].FromMNMX);
				AddDPointToMinMax (&Tri[NUMTRI].ToPT[0],&Tri[NUMTRI].ToMNMX);
				AddDPointToMinMax (&Tri[NUMTRI].ToPT[1],&Tri[NUMTRI].ToMNMX);
				AddDPointToMinMax (&Tri[NUMTRI].ToPT[2],&Tri[NUMTRI].ToMNMX);
      			if (LP1[L2] == ToPt) 
	      			Track[L2].right = TRUE; 
	      		else
	      			Track[L2].left = TRUE;
      			for (J=0;J<PointInfo[PT3].nlines;J++)
      			{   
      				K = PointInfo[PT3].lines[J];
   					if (LP1[K] == PT3 && LP2[K] == AtPt)
		      			Track[K].right = TRUE; 
   					else if (LP1[K] == AtPt && LP2[K] == PT3)
	      				Track[K].left = TRUE;
	      		}
	      		NUMTRI++;
	      	}
	      }
		} 
	GSSiGlobUlFree (&hNLATPT);
	GSSiGlobUlFree (&hPointInfo);
	GSSiGlobUlFree (&hORDERD);
	GSSiGlobUlFree (&hDIST);
	GSSiGlobUlFree (&hP1);
	GSSiGlobUlFree (&hP2);
	GSSiGlobUlFree (&hLP1);
	GSSiGlobUlFree (&hLP2);
	GSSiGlobUlFree (&hTrack);
	Tri[0].NumTri = NUMTRI;   
	if (GetGlobalBVal ("[%SHOWTIN]"))
	{
	    SaveDC (CurView->hDC);
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	    GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	GSSiDeleteObject(&CurView->hRgn);
		CreateRandomBrushes (NULL,0);
		for (I=0;I<NUMTRI;I++)
		{   
			HBRUSH	hOldBrush;
			HPEN	hOldPen, hPen=GetStockObject (BLACK_PEN);
			
			hOldBrush = SelectRandomBrush (CurView->hDC,I,&hPen,0); 
			hOldPen = SelectObject (CurView->hDC,hPen);
			GWPolygonD (CurView->hDC, Tri[I].FromPT,4, 1, NULL,0,FALSE,TRUE,0); 
			SelectObject (CurView->hDC,hOldBrush);
			SelectObject (CurView->hDC,hOldPen);
		}
		RestoreDC (CurView->hDC,-1);
	}
	GlobalUnlock (TriHandle);
 	return TriHandle;
}

BOOL    GetBitH (long ibit, HPSTR lpBytes)
#if ENABLETRACE
{GSSiEnterProg (341);
#endif
{   int       bit, byte;

    byte = ibit/8;
    bit  = ibit%8;
    lpBytes += byte;
    if (Mask[bit] & *lpBytes)
{
#if ENABLETRACE
GSSiExitProg (341);
#endif
        return 1;
}
    else
{
#if ENABLETRACE
GSSiExitProg (341);
#endif
        return 0;
}
#if ENABLETRACE
}
#endif
}

void SetBitH (long ibit, HPSTR lpBytes, BOOL setto)
#if ENABLETRACE
{GSSiEnterProg (342);
#endif
{   int       bit, byte;

    byte = ibit/8;
    bit  = ibit%8;
    lpBytes += byte;
    if (setto)
        *lpBytes = Mask[bit] | *lpBytes;
    else
        if (Mask[bit] & *lpBytes) *lpBytes = Mask[bit] ^ *lpBytes;

{
#if ENABLETRACE
GSSiExitProg (342);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

HANDLE TransformBitmap (HANDLE hBMP,HANDLE hTran,HANDLE hTranBack,LPMNMXCORD pBounds, LPDOUBLE pRes,LPSTR Name,long nAreaPts,HANDLE hAreaPts,BOOL DisplayStatus)
{
	HPSTR 	pMem;
	LPBITMAPINFOHEADER	pDib=(LPBITMAPINFOHEADER)GlobalLock (hBMP), pDibNew; 
	LPBITMAPINFO	pDibInfo=(LPBITMAPINFO)pDib,pDibNewInfo;
	USHORT	NewWidth, NewHeight, RowOld, ColOld, OldWidth, OldHeight;
	long	RowNew, ColNew, col;
	DPOINT	Point, Point2, NewPoint, NewPoint2, OldPoint; 
	HANDLE	hBMPNew; 
	double	Res;
	HPBYTE	pBitsOld, pBitsNew, pBitsRowOld, pBitsRowNew;
	USHORT	row,ByteOld, ByteNew, i;
	long	RowLenOld, RowLenNew, RowLenNewBits; 
	USHORT	bc=6725,ec=6800,br=3655,er=3700; 
	long	NewSize,ii, dbrow=1858,dbcol=3370; 
	BYTE	AllOn=0xFF; 
	HPDPOINT	pAreaPoints; 
	HANDLE	hPIAAccelerator=0;
	int		nBytesPerPel,SizeImageNew;
	
	if (hAreaPts)
		pAreaPoints = (HPDPOINT)GlobalLock (hAreaPts);
	DBoundsInit (pBounds);
	
	Point.x = 0;
	Point.y = 0;
	NewPoint = TranPoint (&Point,hTran);
	AddDPointToMinMax (&NewPoint,pBounds); 
	OldWidth = pDib->biWidth;
	OldHeight = pDib->biHeight;
	Point2.x = pDib->biWidth-1;
	Point2.y = pDib->biHeight-1;
	NewPoint2 = TranPoint (&Point2,hTran); 
	Res = ldistp (Point,Point2) / ldistp (NewPoint,NewPoint2);
	Point.y = pDib->biHeight-1;
	NewPoint = TranPoint (&Point,hTran);
	AddDPointToMinMax (&NewPoint,pBounds);
	Point.x = pDib->biWidth-1;
	NewPoint = TranPoint (&Point,hTran);
	AddDPointToMinMax (&NewPoint,pBounds);
	Point.y = 0;
	NewPoint = TranPoint (&Point,hTran);
	AddDPointToMinMax (&NewPoint,pBounds);  
	NewWidth = IDNINT (Res * (pBounds->xmx - pBounds->xmn)); 
	NewHeight = IDNINT (Res * (pBounds->ymx - pBounds->ymn));
    nBytesPerPel = pDib->biBitCount/8;
	RowLenNew = NewWidth *(long)nBytesPerPel;  
	if (RowLenNew%4)
		RowLenNew += 4-RowLenNew%4;
	SizeImageNew = NewHeight * RowLenNew;
/*	Dwords = pDib->biWidth / 32;
	if (pDib->biWidth % 32)
		Dwords++;
	RowLenOld = Dwords * 4;
	Dwords = NewWidth / 32;
	if (NewWidth % 32)
		Dwords++; 
	RowLenNew = Dwords * 4; */  
	NewSize = sizeof(BITMAPINFOHEADER)+ RowLenNew * (long)NewHeight + pDib->biClrUsed * sizeof(RGBQUAD);
	if (NewSize > 16L * 1024L * 1024L) 
	{
		GlobalUnlock (hBMP);  
		if (hAreaPts)
			GlobalUnlock (hAreaPts);
		return 0;
	}
	hBMPNew = GSSiGlobAlloc (1296,GHND,NewSize);
	pDibNew = (LPBITMAPINFOHEADER)GlobalLock (hBMPNew); 
	pDibNewInfo = (LPBITMAPINFO)pDibNew;
	*pDibNew = *pDib;
	if (pDibNew->biBitCount == 1)
		pDibNew->biClrUsed = 0;
	for (i=0;i<pDibNew->biClrUsed;i++)
		pDibNewInfo->bmiColors[i] = pDibInfo->bmiColors[i]; 
	pDibNew->biWidth = NewWidth;
	pDibNew->biHeight = NewHeight;
	pDibNew->biSizeImage = RowLenNew * (long)NewHeight; 
	pBitsOld = (HPBYTE)pDib + (sizeof(BITMAPINFOHEADER) + pDib->biClrUsed * sizeof(RGBQUAD));
	pBitsNew = (HPBYTE)pDibNew + (sizeof(BITMAPINFOHEADER) + pDibNew->biClrUsed * sizeof(RGBQUAD)); 
	hmemset (pBitsNew,AllOn,pDibNew->biSizeImage);
	if (DisplayStatus)
	{
		CreateStatusWind (hWndMain,1,"Converting Bitmap");
		StatusWindowUpdate (NULL,Name,1,0); 
	}
	RowLenNewBits = RowLenNew * 8;
	for (row = 0; row < OldHeight;row++)  
//	for (row = br; row < er;row++) 
	{
		pBitsRowOld = pBitsOld + ((long)row * RowLenOld);
		OldPoint.y = row;
		for (col=0;col<OldWidth;col++)
//		for (col=bc;col<ec;col++)
		{   
if (row > dbrow && col > dbcol)
ii=1;
			OldPoint.x = col;
			if (pDibNew->biBitCount == 1)
			{
				if (!GetBitH (col,pBitsRowOld)) 
				{   
					if (!hAreaPts || POINT_IN_AREAD (OldPoint,nAreaPts,pAreaPoints,1,0,NULL,&hPIAAccelerator))  
					{
						NewPoint = TranPoint (&OldPoint,hTran);
						RowNew = (NewPoint.y - pBounds->ymn) * Res + 0.5;
						if (RowNew >= 0 && RowNew < NewHeight)
						{   
							ColNew = (NewPoint.x - pBounds->xmn) * Res + 0.5;
							pBitsRowNew = pBitsNew + ((long)RowNew * RowLenNew);
							if (ColNew >= 0 && ColNew < RowLenNewBits)
								SetBitH (ColNew,pBitsRowNew,0); 
						}
					}
				}
			}
			else
			{
				pBitsRowOld += nBytesPerPel;
				pBitsRowNew += nBytesPerPel;
			}
		} 
		if (DisplayStatus)
			StatusWindowUpdate (NULL,NULL,OldHeight,row); 
		if (!ContinueProcessing)
			break;
	} 
	GSSiGlobFree (&hPIAAccelerator);
	*pRes = Res;
	SetContinueProcessing ( TRUE);
	if (DisplayStatus)
		DestroyStatusWindow(0);  
	GlobalUnlock (hBMPNew);
	GlobalUnlock (hBMP);
	if (hAreaPts)
		GlobalUnlock (hAreaPts);
  
	return hBMPNew;
}
void testxx(void)
{
//#include "c:\temp\header.h"

		return;
}
void Create256RotationHeaders (void)
{
	DPOINT FromPt[2], ToPt[2], ZeroPt={0,0};
	float	RSQMIN;
	HANDLE hTran;
	int		i;
	char	str[256];
	LPTRANDATA	TranPtr;
	double A1[256],A2[256],B1[256],B2[256],C1[256],C2[256];

	HFILE	Fid=GSSiOpenFile ("c:\\temp\\header.h",0,OF_CREATE);

	FromPt[0].x = -1;
	FromPt[0].y = FromPt[1].y= 0;
	FromPt[1].x = 1;
	for (i=0;i<256;i++)
	{
		ToPt[0] = dnewpt (ZeroPt,(i*TWOPI)/255,-1);
		ToPt[1] = dnewpt (ZeroPt,(i*TWOPI)/255,1);
		hTran = STRANPoints (0,FromPt,ToPt,2,&RSQMIN,1,NULL); 
		TranPtr = (LPTRANDATA)GlobalLock(hTran);
		A1[i] = TranPtr->A1;
		A2[i] = TranPtr->A2;
		B1[i] = TranPtr->B1;
		B2[i] = TranPtr->B2;
		C1[i] = TranPtr->C1;
		C2[i] = TranPtr->C2;
		GlobalUnlock (hTran);
		CloseTRANS2 (&hTran); 
	}
	sprintf (str,"\tfloat A1[256] = {");
	fputstring (str,Fid);
	for (i=0;i<255;i++)
	{
		//sprintf (str,"\t%.16lg,",A1[i]);
		sprintf (str,"\t%ff,",A1[i]);
		fputstring (str,Fid);
	}
	sprintf (str,"\t%ff};",A1[255]);
	fputstring (str,Fid);

	sprintf (str,"\tfloat A2[256] = {");
	fputstring (str,Fid);
	for (i=0;i<255;i++)
	{
		sprintf (str,"\t%ff,",A2[i]);
		fputstring (str,Fid);
	}
	sprintf (str,"\t%ff};",A2[255]);
	fputstring (str,Fid);

	sprintf (str,"\tfloat B1[256] = {");
	fputstring (str,Fid);
	for (i=0;i<255;i++)
	{
		sprintf (str,"\t%ff,",B1[i]);
		fputstring (str,Fid);
	}
	sprintf (str,"\t%ff};",B1[255]);
	fputstring (str,Fid);

	sprintf (str,"\tfloat B2[256] = {");
	fputstring (str,Fid);
	for (i=0;i<255;i++)
	{
		sprintf (str,"\t%ff,",B2[i]);
		fputstring (str,Fid);
	} 
	sprintf (str,"\t%ff};",B2[255]);
	fputstring (str,Fid);

	sprintf (str,"\tfloat C1[256] = {");
	fputstring (str,Fid);
	for (i=0;i<255;i++)
	{
		sprintf (str,"\t%ff,",C1[i]);
		fputstring (str,Fid);
	}
	sprintf (str,"\t%ff};",C1[255]);
	fputstring (str,Fid);

	sprintf (str,"\tfloat C2[256] = {");
	fputstring (str,Fid);
	for (i=0;i<255;i++)
	{
		sprintf (str,"\t%ff,",C2[i]);
		fputstring (str,Fid);
	}
	sprintf (str,"\t%ff};",C2[255]);
	fputstring (str,Fid); 

	GSSiClose2 (&Fid); 
	return;
}
