#include <windows.h>
#include <stdio.h>
#include <stdlib.h> 
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "shr.h"
#include "proj.h" //{/umsc/include/projections.ftn}
//#include "cnstnt.h" //{/umsc/include/cnstnt.ftn}
typedef  struct 
        {
           long   FROM_PRJ_TYPE, FROM_PRJ_ZONE,
                  FROM_PRJ_UNITS, PRJ_SPHEROID,
                  TO_PRJ_TYPE, TO_PRJ_ZONE,
                  TO_PRJ_UNITS, IRC ;
        } GCTPARGS;
typedef GCTPARGS far *lpGctpArgs;
 
extern long FAR PASCAL  GCTPZ0 (lpGctpArgs lpGctp, double *PRJ_IN_COR, double *FROM_PRJ_PARMS, 
                     double *OUT_COR, double *TO_PRJ_PARMS);
                      
extern long FAR PASCAL NADCON (long *KEY, double *TMP_CORDS);
                     
#define MFT  3.280833333333333e0
#define FTM 3.04800609601219e-1
#define COUNTY 50
//{Geographic Coordinate System
#define GEO 0
#define DEG 4
#define DEG_UNITS 4
#define PRJ_GEO 0
//{Units code for degrees of arc


long TranProjection2(long ID_FROM, long ID_TO, double *X, double *Y)
{

//C******* SPECIFICATIONS ********************************************************
//C*        This routine 
//C*
//C*        ARGUMENTS
//C*        ---------
//C*        ID_FROM    I*4   (IN)       Projection of input coordinates
//C*                                       1 = Base Projection
//C*                                       2 = Input Projection
//C*                                       3 = Output Projection
//C*
//C*        ID_TO      I*4   (IN)       Projection to convert coordinates to
//C*                                       1 = Base Projection
//C*                                       2 = Input Projection
//C*                                       3 = Output Projection
//C*                                        
//C*        X          R*8   (IN/OUT)   Unbiased X coordinate.
//C*
//C*        Y          R*8   (IN/OUT)   Unbiased Y coordinate.
//C*                              
//C*******************************************************************************
//C

//C-------------------------------
//C   GLOBAL VARIABLE DECLARATION  
//C-------------------------------


//C-------------------------------
//C   LOCAL VARIABLE DECLARATION  
//C-------------------------------
       double   TwoCoords[2],   TMP_CORDS[4] ,OutParms[16]   ; //{Working storage for coord. values
       long   KEY, irc; //{Direction of datum conversion -1=NAD27, 1=NAD83
static BOOL FirstTime = TRUE;


GCTPARGS GCTP;        
lpGctpArgs lpGctp = &GCTP;

//C---------------------
//C   START
//C---------------------
//C
//C      Set up names of state plane grid files the first time ed.
//C      
        
        
//C
//C      Apply bias and conversion factor to input coordinate values.
//C
       PRJ_IN_COR[0]     = (*X+PRJ_X_BIAS[ID_FROM])*PRJ_GRND_TO_GRID[ID_FROM];
       PRJ_IN_COR[1]     = (*Y+PRJ_Y_BIAS[ID_FROM])*PRJ_GRND_TO_GRID[ID_FROM];
       lpGctp->FROM_PRJ_TYPE     = PRJ_TYPE[ID_FROM];
       lpGctp->FROM_PRJ_ZONE     = PRJ_ZONE[ID_FROM];

//C      Check if output coordinates in same datum as input coordinates.
//C      If not the coordinates must be][
//C             Transformed to geographic coord. system (latitude and longitude).
//C             Converted the other datum via NADCON subroutine.
//C             Transformed to desired projection.
//C      
        if(PRJ_SPHEROID[ID_FROM] != PRJ_SPHEROID[ID_TO])
        {
            if(PRJ_TYPE[ID_FROM] != GEO)
            { //{If input not Geo. Coord. System
//C              Transform coordinates to latitude and longitude.
               lpGctp->FROM_PRJ_UNITS    = PRJ_UNITS[ID_FROM];
               lpGctp->PRJ_SPHEROID      = PRJ_SPHEROID[ID_FROM];
               lpGctp->TO_PRJ_UNITS      = DEG_UNITS;
               lpGctp->TO_PRJ_TYPE       = PRJ_GEO;   
               lpGctp->TO_PRJ_ZONE       = PRJ_ZONE[ID_FROM];                                                                                                                                                              
             _fmemcpy(&OutParms[1],&PRJ_PARMS[ID_FROM][1],120); 
              
             irc =  GCTPZ0 (lpGctp,&PRJ_IN_COR[0], &PRJ_PARMS[ID_FROM][1], 
                      &TwoCoords[0],&OutParms[1]);
         /*     CALL GTPZ0 (PRJ_IN_COR,         PRJ_TYPE(ID_FROM), ! {Tranform to lat. long.
     +                      PRJ_ZONE(ID_FROM),  PRJ_PARMS(1,ID_FROM),
     +                      PRJ_UNITS(ID_FROM), PRJ_SPHEROID(ID_FROM),
     +                      PRT_ERMES_FLG,      PRT_PRJP_FLG,
     +                      LU_ERMSG,           LU_PRJP,
     +                      TMP1_COR,           PRJ_GEO,
     +                      PRJ_ZONE(ID_FROM),  PRJ_PARMS(1,ID_FROM),
     +                      DEG_UNITS,
     +                      LU27,               LU83,     
     +                      FIL_NAD27,          FIL_NAD83,
     +                      REC_LNG,            IRC)*/                             
                          
                if(irc !=0 )  goto S90;
                TMP_CORDS[0] = TwoCoords[0];
                TMP_CORDS[1] = TwoCoords[1];
//C      
//C              Restore output projection parameters.
//C      
//c               write(str,'(2(a,f24.12),a)')
//c     3        ' lat = ',tmp1_cor[1] ,' long = ',tmp1_cor[2] ,char(0)
//c                messagebox(NULL,str,'TranProj2'c,MB_OK)          
           } 
           else 
           {
               TMP_CORDS[0] = PRJ_IN_COR[0] ; //{Get input latitude longitude
               TMP_CORDS[1] = PRJ_IN_COR[1] ;
               TMP_CORDS[2] = PRJ_IN_COR[0] ; //{Get input latitude longitude
               TMP_CORDS[3] = PRJ_IN_COR[1] ;
           }
//C      
//C          Shift coordinates to the other datum
//C      
            if(PRJ_SPHEROID[ID_TO] == 0) 
               KEY = -1    ; //{Yes Convert to NAD 1927
            else 
               KEY = 1     ; //{No, convert to NAD 1983
            irc = NADCON (&KEY, &TMP_CORDS[0]);
            if(irc != 0)
            { 
             if(irc == 1)
             { 
                MessageBox(NULL,
                "*** Error in NADCON. Unable to open grids", 
                    "Convert Coord Error", MB_ICONSTOP);
             } 
             else 
             {
                MessageBox(NULL,
                "*** Error in NADCON. Coords out of Range", 
                    "Convert Coord Error", MB_ICONSTOP);
             } // 
             goto S99;
           }
//C      
//C          If output coordinates to be in Geographic Coord. System, we are done.
//C      
            if(PRJ_TYPE[ID_TO] == GEO)
            {
               PRJ_OUT_COR[0]  = TMP_CORDS[2] ;
               PRJ_OUT_COR[1]  = TMP_CORDS[3] ;
               irc = 0;
            } 
            else 
            {
 
               lpGctp->FROM_PRJ_UNITS    = DEG_UNITS;
               lpGctp->FROM_PRJ_TYPE     = PRJ_GEO;
               lpGctp->PRJ_SPHEROID      = PRJ_SPHEROID[ID_TO];
               lpGctp->TO_PRJ_ZONE       = PRJ_ZONE[ID_TO];
               lpGctp->TO_PRJ_UNITS      = PRJ_UNITS[ID_TO];
               lpGctp->TO_PRJ_TYPE       = PRJ_TYPE[ID_TO];   

              irc = GCTPZ0 (lpGctp, &TMP_CORDS[2], &PRJ_PARMS[ID_FROM][1], 
                      &PRJ_OUT_COR[0],&PRJ_PARMS[ID_TO][1]);
               /*   GTPZ0 (TMP2_COR,           PRJ_GEO,  ; //{Tranform from lat. long.
                           PRJ_ZONE[ID_FROM],  PRJ_PARMS(1,ID_FROM),
                           DEG_UNITS,          PRJ_SPHEROID[ID_TO],
                           PRT_ERMES_FLG,      PRT_PRJP_FLG,
                           LU_ERMSG,           LU_PRJP,
                           PRJ_OUT_COR,        PRJ_TYPE[ID_TO],
                           PRJ_ZONE[ID_TO],    PRJ_PARMS(1,ID_TO),
                           PRJ_UNITS[ID_TO],
                           LU27,               LU83,     
                           FIL_NAD27,          FIL_NAD83,
                           REC_LNG,            IRC);   */
           }
       } 
       else 
       {
       
//C
//C      Transform coordinates to another projection in same Datum.
//C      
//c         write(str,'(a,i8,a)')
//c     *  'ing GTPZ0 with PRJ_ZONE[ID_FROM] = ',
//c     *   PRJ_ZONE[ID_FROM],char(0)
//c          messagebox(NULL,STR,'Nad2'c,MB_OK)  

               lpGctp->FROM_PRJ_UNITS    = PRJ_UNITS[ID_FROM];
               lpGctp->PRJ_SPHEROID      = PRJ_SPHEROID[ID_TO];
               lpGctp->TO_PRJ_ZONE       = PRJ_ZONE[ID_TO];
               lpGctp->TO_PRJ_UNITS      = PRJ_UNITS[ID_TO];   
               lpGctp->TO_PRJ_TYPE       = PRJ_TYPE[ID_TO];   

             irc =  GCTPZ0 (lpGctp, &PRJ_IN_COR[0], &PRJ_PARMS[ID_FROM][1], 
                      &PRJ_OUT_COR[0], &PRJ_PARMS[ID_TO][1]); 
                      
          /*      GTPZ0 (PRJ_IN_COR,         PRJ_TYPE[ID_FROM],  ; //{Tranform projection.
                       PRJ_ZONE[ID_FROM],  PRJ_PARMS(1,ID_FROM),
                       PRJ_UNITS[ID_FROM], PRJ_SPHEROID[ID_TO],
                       PRT_ERMES_FLG,      PRT_PRJP_FLG,
                       LU_ERMSG,           LU_PRJP,
                       PRJ_OUT_COR,        PRJ_TYPE[ID_TO],
                       PRJ_ZONE[ID_TO],    PRJ_PARMS(1,ID_TO),
                       PRJ_UNITS[ID_TO],
                       LU27,               LU83,     
                       FIL_NAD27,          FIL_NAD83,
                       REC_LNG,            IRC); */
       }
//C
//C      
//C
        if(PRJ_ZONE[ID_FROM] == 61) PRJ_ZONE[ID_FROM] = 62;
        if(PRJ_ZONE[ID_TO] == 61) PRJ_ZONE[ID_TO] = 62;
//C      
//C      Check status from transformation subroutine.
//C      
S90:     if(irc !=0)
         {
            MessageBox(NULL,"Error in GCTP transformation" ,
                    "TranProj2 Error",MB_ICONSTOP);
         } 
         else 
         {
//C          Good status - remove bias and conversion factor.
           *X =(PRJ_OUT_COR[0] /PRJ_GRND_TO_GRID[ID_TO])-PRJ_X_BIAS[ID_TO];
           *Y =(PRJ_OUT_COR[1] /PRJ_GRND_TO_GRID[ID_TO])-PRJ_Y_BIAS[ID_TO];
         }
S99:    return irc;

}