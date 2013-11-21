%include 'hennepin_county_copyright.ftn'  {'/umsc/include/hennepin_county_copyright.ftn'}
      FUNCTION N4PNT (MODELN,X,Y)
C******* SPECIFICATIONS ***********************************************
C*                                                                    *
C*       PROGRAM SUMMARY                                              *
C*       ------- -------                                              *
C*       THIS FUNCTION INTERPOLATES A STATE PLANE SUBCELL NODE FROM   *
C*       FOUR SURROUNDING MODEL GRID NODES.                           *
C*                                                                    *
C*       ARGUMENT DESCRIPTION                                         *
C*       -------- -----------                                         *
C*       SUBSUB  I*4  SUBSCRIPT OF THE SP-OUTPUT SUBCELL NODE.        *
C*       MODELN  I*4 (4) ELEVATIONS OF THE SURROUNDING MODEL NODES    *
C*                       BEGINNING WITH THE LOWER-LEFT NODE AND       *
C*                       PROCEEDING IN A CLOCK-WISE DIRECTION.        *
C*       X       R*4  X-COOR OF THE SP NODE MINUS THE XCOOR OF MODEL  *
C*                    NODE #1 IN HALF-METERS.                         *
C*       Y       R*4  Y-COOR OF THE SP NODE MINUS THE YCOOR OF MODEL  *
C*                    NODE #1.                                        *
C*                                                                    *
C*       VARIABLE DESCRIPTION                                         *
C*       -------- -----------                                         *
C*       IELEV   MODEL ELEVATION     (COMMON MODEL) I*2 (128,16,10,40)*
C*       OUTPUT  SP OUTPUT SURFACE   (COMMON NGI)   I*2 (1024)        *
C*       DM2M1   MODEL ELEVATION 1 - MOD. EL. 2     R*4               *
C*       DM3M4   MODEL ELEVATION 3 - MOD. EL. 4     R*4               *
C*       MN1MN2  ELEVATION POINT INTERPOLATED       R*4               *
C*               BETWEEN MOD. EL. 1 AND 2                             *
C*       MN3MN4  ELEVATION POINT INTERPOLATED       R*4               *
C*               BETWEEN MOD. EL. 3 AND 4                             *
C*       RELV    REAL ELEVATION VARIABLE            R*4               *
C*                                                                    *
C*       ALGORITHM                                                    *
C*       ---------                                                    *
C*       THE SP NODE IS SURROUNDED BY 4 MODEL NODES (MN1-4). FIRST    *
C*       THE TWO ELEVATIONS MN1MN2 AND MN3MN4 ARE INTERPOLATED AS     *
C*       FOLLOWS:                                                     *
C*                                                                    *
C*             MN1MN2 = MN1 + Y * (MN2 - MN1)                          *
C*             MN3MN4 = MN4 + Y * (MN3 - MN4)                         *
C*                                                                    *
C*       THEN THE STATE PLANE NODE IS INTERPOLATED BY:                *
C*                                                                    *
C*             SPN = MN1MN2 + X * (MN3MN4 - MN1MN2)                   *
C*                                                                    *
C*       AUTHOR                                                       *
C*       ------                                                       *
C*       JEFF SMITH                                                   *
C*                                                                    *
C**********************************************************************
      INTEGER*4 MODELN(4)
      REAL*4 DM2M1, DM3M4, MN1MN2, MN3MN4
      REAL*4 X, Y
      REAL*8 NAME$$
%include 'subtim.ftn' {/umsc/include/subtim.ftn}
      DATA   NAME$$/'N4PNT'/
      DM2M1 = MODELN(2) - MODELN(1)
      DM3M4 = MODELN(3) - MODELN(4)
      MN1MN2 = FLOAT(MODELN(1)) + Y * DM2M1
      MN3MN4 = FLOAT(MODELN(4)) + Y * DM3M4
C******* INTERPOLATE THE STATE PLANE NODE AND ADD 0.5 TO ROUND.
      RELV = MN1MN2 + X * (MN3MN4 - MN1MN2)
      N4PNT = RELV + SIGN (0.5,RELV)
      RETURN
      END
