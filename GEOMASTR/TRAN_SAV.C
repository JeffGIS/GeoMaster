#include "graphint.h"
#include "mapl.h"
#include "p_tol.h"
#include "hash.h"    
#include <direct.h>   
#include <sys\types.h>
#include <sys\stat.h>         
#include <fcntl.h>
#include <dibapi.h>  
#include <ctype.h>
#include <math.h> 
#include <float.h>   
#include "winexec.h" 
static  BYTE    Mask[8] = {128, 64, 32, 16, 8, 4, 2, 1}; 

#include "gmextern.h"
 
HANDLE STRAN2 (double X1[], double Y1[],double X2[],double Y2[],short N, LPFLOAT RSQMIN, short Type,LPMNMXCORD pBounds)
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

{
     double    ANGLE1, ANGLE2, ANGLE, DEN,
               SINANG, COSANG, DX1, DX2, DY1, DY2, SCLRAT, 
               ANGLED, BASX1, BASX2, BASY1, BASY2;
     LPDOUBLE  XT1, YT1, XT2, YT2;
     float     RSQ1, RSQ2;
     short     NUM, I,J;
     LPTRANDATA TranPtr;
     HANDLE     hTran, hTemp;
    
    if (!N || N > MAXTRANPOINTS)
    	return 0;  
    hTemp = GSSiGlobAlloc (1281,GMEM_MOVEABLE,(long)MAXTRANPOINTS*16*2);
    XT1 = (LPDOUBLE)GlobalLock (hTemp);
    XT2 = XT1 + MAXTRANPOINTS;
    YT1 = XT2 + MAXTRANPOINTS;
    YT2 = YT1 + MAXTRANPOINTS;
    hTran = GSSiGlobAlloc (1282,GHND,sizeof(TRANDATA));
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
      	TranPtr->TriHandle = TRFTRI_SET (XT1,YT1,XT2,YT2,N,BASX1,BASY1,BASX2,BASY2,hTran,&TranPtr->TriBounds);
      GlobalUnlock (hTran);  
      GSSiGlobUlFree (&hTemp);
      return (hTran);

/******* SET-UP WITH N EQ 2.*/
 S10: NUM    = 0;
      ANGLE  = 0;
      SCLRAT = 0;
      for (I=0,J=1;I< N - 1;I++,J++)
      {
          DX1    = X1[J] - X1[I];
          DX2    = X2[J] - X2[I];
          DY1    = Y1[J] - Y1[I];
          DY2    = Y2[J] - Y2[I];
          DEN = pow(DX1,2) + pow (DY1,2);
          if (fabs(DEN) < 0.0001) goto S1000;
          SCLRAT = SCLRAT + sqrt ((pow (DX2,2) + pow (DY2,2))/ DEN);
          ANGLE1 = atan2 (DY2, DX2);
          ANGLE2 = atan2 (DY1, DX1);
          ANGLED  = LTWOPI (ANGLE2 - ANGLE1);
          if (ANGLED > PY) ANGLED =  ANGLED - TWOPI;
          ANGLE  = ANGLE + ANGLED;
          NUM = NUM + 1;
      }
      ANGLE  = ANGLE / NUM;
      SCLRAT = SCLRAT / NUM;
      COSANG = cos (ANGLE);
      SINANG = sin (ANGLE);
      TranPtr->B1 = COSANG * SCLRAT;
      TranPtr->C1 = SINANG * SCLRAT;
      TranPtr->B2 = COSANG * SCLRAT;
      TranPtr->C2 =-SINANG * SCLRAT;
      TranPtr->A1 = 0;
      TranPtr->A2 = 0;
      for (I=0; I<N; I++)
      {
          TranPtr->A1 = TranPtr->A1 + X2[I] - (COSANG * X1[I] +SINANG * Y1[I]) * SCLRAT;
          TranPtr->A2 = TranPtr->A2 + Y2[I] - (COSANG * Y1[I] -SINANG * X1[I]) * SCLRAT;
      }
      TranPtr->A1 = TranPtr->A1 / N;
      TranPtr->A2 = TranPtr->A2 / N;
      *RSQMIN = (float)1.;
      TranPtr->BASX = 0;
      TranPtr->BASY = 0;
      TranPtr->NSETPT = N; 
      GlobalUnlock(hTran);
      GSSiGlobUlFree (&hTemp);
      return (hTran);
S1000:
      GSSiGlobUlFree (&hTemp);
	  return (0);
} 

HANDLE ReadTranData (HFILE Fid)
{
    HANDLE  hTran;
    LPTRANDATA TranPtr;    
    long	ii;
    
    hTran = GSSiGlobAlloc (1283,GHND,sizeof(TRANDATA));
    TranPtr = (LPTRANDATA) GlobalLock (hTran); 
    ii=_lread (Fid,TranPtr,sizeof(TRANDATA));
	if (TranPtr->TriHandle)
	{   
		TRANTRI		TranTri;
	    HPTRANTRI	Tri;
	    short		NumTri;
	    long		Loc = _llseek (Fid,0,1);
        
        _lread (Fid,&TranTri,sizeof(TRANTRI)); 
        _llseek (Fid,Loc,0);
        NumTri = TranTri.NumTri;
        TranPtr->TriHandle = GSSiGlobAlloc (1284,GMEM_MOVEABLE,(long)NumTri*sizeof(TRANTRI));
        Tri=(HPTRANTRI)GlobalLock (TranPtr->TriHandle);
	    while (NumTri--)
	    {
	        _lread (Fid,Tri,sizeof(TRANTRI)); 
	        Tri->hFromTran = 0;
	        Tri->hToTran = 0;
		    Tri++;
	    } 
	    GlobalUnlock (TranPtr->TriHandle);
	}
    GlobalUnlock (hTran);
    return (hTran);
}  

HANDLE LoadTranFile (LPSTR Name,short dir,short Type,LPSHORT pNumPoints,LPDOUBLE pRSQ)
{
	HFILE Fid=HFILE_ERROR;
	char	str[260], curproject[64]="", Marker;
	BOOL	First=TRUE, SetTrans=FALSE, HaveLimits=FALSE; 
	HANDLE	handle=0, hcoord=GSSiGlobAlloc (1285,GMEM_MOVEABLE,(long)MAXTRANPOINTS*16*2); 
	float	RSQMIN=0;
	short	N=0, ConvertID=0, i; 
	LPSTR	lpCVT;
	DPOINT	Point;
	LPDOUBLE	XFROM,YFROM,XTO,YTO;
    HCURSOR hcurSave;
    MNMXCORD	TranBounds, TranLimits;
	
	GetGlobalCVal ("[%ALT_PROJECTION]",curproject,NULL); 
// 1/27/2003	SetTrans = TRUE; messes up $MAPCOPY
	TranBounds.xmn=2;
	TranBounds.xmx=1;
	XFROM = (LPDOUBLE)GlobalLock (hcoord);
	YFROM = XFROM+MAXTRANPOINTS;
	XTO = YFROM+MAXTRANPOINTS;
	YTO = XTO+MAXTRANPOINTS;
	
	Fid = GSSiOpenFile (Name,NULL,OF_READ);
	if (Fid == HFILE_ERROR)
		goto Exit;  
	
    hcurSave = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
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
			if (sscanf (str,"%c %Flf %Flf %Flf %Flf",&Marker,&TranLimits.xmn,&TranLimits.ymn,
											  &TranLimits.xmx,&TranLimits.ymx) == 5)
				HaveLimits = TRUE;
		}	
		else if (*str == 'B' || *str == 'b') 
			sscanf (str,"%c %Flf %Flf %Flf %Flf",&Marker,&TranBounds.xmn,&TranBounds.ymn,
											  &TranBounds.xmx,&TranBounds.ymx);
		else if (N<MAXTRANPOINTS)
		{
			if (sscanf (str,"%Flf %Flf %Flf %Flf",XFROM++,YFROM++,XTO++,YTO++) == 4)
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
	if (N > 1 && Type == 3 && HaveLimits)
	{
		handle = STRAN2 (XFROM,YFROM,XTO,YTO,N,&RSQMIN,1,NULL);
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
		handle = STRAN2 (XFROM,YFROM,XTO,YTO,N,&RSQMIN,Type,&TranBounds);  
	else
		handle = STRAN2 (XTO,YTO,XFROM,YFROM,N,&RSQMIN,Type,&TranBounds); 
Exit:
	GSSiGlobUlFree (&hcoord);
	if (Fid != HFILE_ERROR)
		GSSiClose (Fid);  
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
	OFSTRUCT	OFStruct;  
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
		GSSiClose(Fid);
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
	YFROM[2] = FromRect->bottom;
	YFROM[3] = FromRect->top; 
	XTO[0] = ToRect->left;
	XTO[1] = ToRect->left;
	XTO[2] = ToRect->right;
	XTO[3] = ToRect->right; 
	YTO[0] = ToRect->bottom;
	YTO[1] = ToRect->top;
	YTO[2] = ToRect->bottom;
	YTO[3] = ToRect->top; 
	
	hTran = STRAN2 (XFROM,YFROM,XTO,YTO,4,(LPFLOAT)&RSQMIN,1,NULL);
	return hTran;
}

void TRANRect (LPRECT Rect,HANDLE hTran)
{   
	double	Xfrom,Yfrom,Xto,Yto;  
	RECT	OutRect;
	
	Xfrom = Rect->left;
	Yfrom = Rect->bottom;
	TRANS2 (Xfrom,Yfrom,&Xto,&Yto,hTran); 
	OutRect.left = (short)IDNINT(Xto);
	OutRect.bottom = (short)IDNINT(Yto);
	Xfrom = Rect->right;
	Yfrom = Rect->top;
	TRANS2 (Xfrom,Yfrom,&Xto,&Yto,hTran); 
	OutRect.right = (short)IDNINT(Xto);
	OutRect.top = (short)IDNINT(Yto);
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
	
	hTran = STRAN2 (XFROM,YFROM,XTO,YTO,4,(LPFLOAT)&RSQMIN,1,NULL);
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
	
	hTran = STRAN2 (XFROM,YFROM,XTO,YTO,4,(LPFLOAT)&RSQMIN,1,NULL);
	return hTran;
}

POINT TRANDPointToPoint (DPOINT DPoint,HANDLE hTran)
{   
	double	Xto,Yto;
	POINT	Point;  
	
	TRANS2 (DPoint.x,DPoint.y,&Xto,&Yto,hTran); 
	Point.x = (short)IDNINT(Xto);
	Point.y = (short)IDNINT(Yto);
	return Point;
} 

void CloseTRANS2 (LPHANDLE phlpTran)
{    
	LPTRANDATA  TranPtr;

	if (*phlpTran)
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
	return;

}   

DPOINT TranPoint (LPDPOINT pPoint,HANDLE hTran)
{
	DPOINT Point;
	
	TRANS2 (pPoint->x,pPoint->y, &Point.x,&Point.y,hTran);
	return Point;
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

void TRANS2 (double XIN,double YIN, LPDOUBLE XOUT,LPDOUBLE YOUT, HANDLE hlpTran)
{   double XI, YI;
    LPTRANDATA  TranPtr;
	DPOINT		Point;
      
      if (!hlpTran) 
      {
      	*XOUT = XIN;
      	*YOUT = YIN;
      	return;
      }
	  Point.x = XIN;
	  Point.y = YIN;	
      TranPtr = (LPTRANDATA)GlobalLock(hlpTran); 
      if (!PointInBounds (Point,&TranPtr->Bounds))
      { 
      	GlobalUnlock (hlpTran);
      	*XOUT = XIN;
      	*YOUT = YIN;
      	return;
      }
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
					if ((First || TriNum != TranPtr->LastTri) && DPointInBounds (&Point,&Tri->FromMNMX))
					{
						if (POINT_IN_AREAD (Point,4,Tri->FromPT,NULL))
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
								Tri->hFromTran = STRAN2 (X1,Y1,X2,Y2,3,&RSQMIN,1,NULL);
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
      }
      *XOUT   = TranPtr->A1 + TranPtr->B1 * XI + TranPtr->C1 * YI;
      *YOUT   = TranPtr->A2 + TranPtr->B2 * YI + TranPtr->C2 * XI;
Exit:
      GlobalUnlock(hlpTran);
      return;
} 

HANDLE TRFTRI_SET (LPDOUBLE XT,LPDOUBLE YT,LPDOUBLE XT2,LPDOUBLE YT2,
				   short NSETPTin,double BASX1, double BASY1,double BASX2, double BASY2,HANDLE hTran,LPMNMXCORD pTriBounds)   
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
	
typedef struct {short	nlines;
		 		short	lines[31];} POINTINFO;
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
              D = LDIST (XT[I],YT[I],XT[J],YT[J]);
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
          if (PointInfo[IP1].nlines >= 31 ||  
          	  PointInfo[IP2].nlines >= 31)  
          	GSSiMessageBox ("Too many lines at one point - add more interior points",
          					"Transformation Error",MB_ICONEXCLAMATION);
          else
          {
	          PointInfo[IP1].lines[PointInfo[IP1].nlines++]=(short)NLINES;
	          PointInfo[IP2].lines[PointInfo[IP2].nlines++]=(short)NLINES++;
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
		CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	DeleteObject(CurView->hRgn);
		CreateRandomBrushes (NULL,0);
		for (I=0;I<NUMTRI;I++)
		{   
			HBRUSH	hOldBrush;
			HPEN	hOldPen, hPen=GetStockObject (BLACK_PEN);
			
			hOldBrush = SelectRandomBrush (CurView->hDC,I,&hPen,0); 
			hOldPen = SelectObject (CurView->hDC,hPen);
			GWPolygonD (CurView->hDC, Tri[I].FromPT,4, 1, NULL,0,FALSE); 
			SelectObject (CurView->hDC,hOldBrush);
			SelectObject (CurView->hDC,hOldPen);
		}
		RestoreDC (CurView->hDC,-1);
	}
	GlobalUnlock (TriHandle);
 	return TriHandle;
}

BOOL    GetBitH (short ibit, HPSTR lpBytes)
#if ENABLETRACE
{GSSiEnterProg (341);
#endif
{   short       bit, byte;

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

void SetBitH (short ibit, HPSTR lpBytes, BOOL setto)
#if ENABLETRACE
{GSSiEnterProg (342);
#endif
{   short       bit, byte;

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

HANDLE TransformBitmap (HANDLE hBMP,HANDLE hTran,HANDLE hTranBack,LPMNMXCORD pBounds, LPDOUBLE pRes,BOOL DisplayStatus)
{
	LPBITMAPINFOHEADER	pDib=(LPBITMAPINFOHEADER)GlobalLock (hBMP), pDibNew; 
	LPBITMAPINFO	pDibInfo=(LPBITMAPINFO)pDib,pDibNewInfo;
	USHORT	NewWidth, NewHeight, RowOld, ColOld;
	DPOINT	Point, Point2, NewPoint, NewPoint2, OldPoint; 
	HANDLE	hBMPNew; 
	double	Res;
	HPBYTE	pBitsOld, pBitsNew, pBitsRowOld, pBitsRowNew;
	USHORT	row,col, ByteOld, ByteNew, i;
	long	RowLenOld, RowLenNew,Dwords; 
	USHORT	bc=6725,ec=6800,br=3655,er=3700;
	
	DBoundsInit (pBounds);
	
	Point.x = 0;
	Point.y = 0;
	NewPoint = TranPoint (&Point,hTran);
	AddDPointToMinMax (&NewPoint,pBounds);
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
	Dwords = pDib->biWidth / 32;
	if (pDib->biWidth % 32)
		Dwords++; 
	RowLenOld = Dwords * 4;
	Dwords = NewWidth / 32;
	if (NewWidth % 32)
		Dwords++; 
	RowLenNew = Dwords * 4;
	hBMPNew = GSSiGlobAlloc (1296,GHND,sizeof(BITMAPINFOHEADER)+ RowLenNew * (long)NewHeight + pDib->biClrUsed * sizeof(RGBQUAD));
	pDibNew = (LPBITMAPINFOHEADER)GlobalLock (hBMPNew);  
	pDibNewInfo = (LPBITMAPINFO)pDibNew;
	*pDibNew = *pDib;
	for (i=0;i<pDib->biClrUsed;i++)
		pDibNewInfo->bmiColors[i] = pDibInfo->bmiColors[i]; 
	pDibNew->biWidth = NewWidth;
	pDibNew->biHeight = NewHeight;
	pDibNew->biSizeImage = RowLenNew * (long)NewHeight; 
	pBitsOld = (HPBYTE)pDib + (sizeof(BITMAPINFOHEADER) + pDib->biClrUsed * sizeof(RGBQUAD));
	pBitsNew = (HPBYTE)pDibNew + (sizeof(BITMAPINFOHEADER) + pDibNew->biClrUsed * sizeof(RGBQUAD));
	if (DisplayStatus)
		CreateStatusWindow (hWndMain,1,"Converting Bitmap");
	for (row = 0; row < NewHeight;row++) 
//	for (row = br; row < er;row++) 
	{
		NewPoint.y = pBounds->ymn + row / Res;
		pBitsRowNew = pBitsNew + ((long)row * RowLenNew);
		for (col=0;col<NewWidth;col++)
//		for (col=bc;col<ec;col++)
		{   
			NewPoint.x = pBounds->xmn + col / Res;
			OldPoint = TranPoint (&NewPoint,hTranBack);
			RowOld = max (0,min (OldPoint.y,pDib->biHeight-1));
			pBitsRowOld = pBitsOld + ((long)RowOld * RowLenOld);
			ColOld = max (0,min (OldPoint.x,pDib->biWidth-1)); 
//			ByteOld = ColOld/8;
//			ByteNew = col/8; 
//			pBitsRowNew[ByteNew] = pBitsRowOld[ByteOld];
			if (GetBitH (ColOld,pBitsRowOld))
					SetBitH (col,pBitsRowNew,1);
		} 
		if (DisplayStatus)
			StatusWindowUpdate (NULL,NULL,NewHeight,row); 
		if (!ContinueProcessing)
			break;
	} 
	*pRes = Res;
	ContinueProcessing = TRUE;
	GlobalUnlock (hBMP);  
	GlobalUnlock (hBMPNew);
	if (DisplayStatus)
		DestroyStatusWindow();  
	return hBMPNew;
}
