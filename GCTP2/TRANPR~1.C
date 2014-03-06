#include "shr.h"
#include "proj.h" //{/umsc/include/projections.ftn}
//#include "cnstnt.h" //{/umsc/include/cnstnt.ftn}
#define MFT  3.280833333333333e0
#define FTM 3.04800609601219e-1
#define COUNTY 50
#define GEO 0

long  GET_COUNTY_PARMS (long COUNTYID,long *CNO,long *CPART);
   double  CVTA (double V);
   double  CVTB (double V);
int ConvertCtyCoord(long inCountyno, 
                  long Inputcoor,  long WhatPartOfCounty,
                  double *Lat, double *Long,
                  double *spX, double *spY, 
                  double *cpX, double *cpY);
long TranProjection2(long ID_FROM, long ID_TO, double *X, double *Y);

                  
long TranProjection (long ID_FROM, long ID_TO, double *X, double *Y)
{   
      long CNO, CPART, SAVE_PRJ_TYPE, SAVE_PRJ_SPHEROID, SAVE_PRJ_UNITS,
               SAVE_PRJ_ZONE, IRC;
      double LAT, LONGITUDE, SPX, SPY, SAVEXBIAS, SAVEYBIAS, SAVEGTOG,
             X2, Y2 ;
     // char str[120];
      
        IRC = 0;
      
      	if (ID_FROM == 1 && IS_BASE[ID_TO])
   			goto NoTrans;
      	if (ID_TO == 1 && IS_BASE[ID_FROM])
   			goto NoTrans;
   		if (ID_FROM == 1 && PRJ_TYPE[ID_TO] == 200)
   		{
           TRANS2 (*X,*Y,&X2,&Y2,(HANDLE) PRJ_TRAN[ID_TO][1]);
           *X = X2;
           *Y = Y2; 
           return IRC;
        }       
   		if (ID_TO == 1 && PRJ_TYPE[ID_FROM] == 200)
   		{
           TRANS2 (*X,*Y,&X2,&Y2,(HANDLE) PRJ_TRAN[ID_FROM][2]);
           *X = X2;
           *Y = Y2; 
           return IRC;
        }       
        if(PRJ_TRAN[ID_FROM][1] > 0) 
        {  
           TRANS2 (*X,*Y,&X2,&Y2,(HANDLE) PRJ_TRAN[ID_FROM][1]);
          *X = X2;
          *Y = Y2;
        }       
		if (PRJ_TYPE[ID_FROM] == SphericalMercatorPROJECTION || PRJ_TYPE[ID_FROM] == PROJ4PROJECTION)
        {
			int id = PRJ_ZONE[ID_FROM];

			if (PRJ_TYPE[ID_FROM] == SphericalMercatorPROJECTION)
				id = GOOGLEMAPSPROJECTION;
			IRC = pj_transform(PRJ_PROJ4DEF[ID_FROM], PRJ_PROJ4DEF[2], 1, 1, X, Y, NULL);
			if (IRC)
           		return IRC;                   
			*X *= RAD_TO_DEG;
			*Y *= RAD_TO_DEG;
			SAVE_PRJ_TYPE = PRJ_TYPE[ID_FROM];
			SAVEXBIAS = PRJ_X_BIAS[ID_FROM];
			SAVEYBIAS = PRJ_Y_BIAS[ID_FROM];
			SAVEGTOG = PRJ_GRND_TO_GRID[ID_FROM];
			SAVE_PRJ_UNITS = PRJ_UNITS[ID_FROM];
			SAVE_PRJ_ZONE = PRJ_ZONE[ID_FROM];
			PRJ_TYPE[ID_FROM] = GEO;
			PRJ_X_BIAS[ID_FROM] = 0;
			PRJ_Y_BIAS[ID_FROM] = 0;
			PRJ_UNITS[ID_FROM] = 4;
			PRJ_GRND_TO_GRID[ID_FROM] = 1e0;
			//*Y = LAT; //CVTA(LAT);
			//*X = (-1.0 * LONGITUDE); //CVTA(-LONGITUDE);
			   
			IRC = TranProjection2 (ID_FROM,ID_TO,X,Y);
			   
			PRJ_ZONE[ID_FROM] = SAVE_PRJ_ZONE;
			PRJ_TYPE[ID_FROM] = SAVE_PRJ_TYPE;
			PRJ_X_BIAS[ID_FROM] = SAVEXBIAS;
			PRJ_Y_BIAS[ID_FROM] = SAVEYBIAS;
			PRJ_GRND_TO_GRID[ID_FROM] = SAVEGTOG;
			PRJ_UNITS[ID_FROM] = SAVE_PRJ_UNITS;
			//if (IRC)
           		return IRC;                   
		}
		if (PRJ_TYPE[ID_TO] == SphericalMercatorPROJECTION || PRJ_TYPE[ID_TO] == PROJ4PROJECTION)
        {
			SAVE_PRJ_TYPE = PRJ_TYPE[ID_TO];
			SAVEXBIAS = PRJ_X_BIAS[ID_TO];
			SAVEYBIAS = PRJ_Y_BIAS[ID_TO];
			SAVEGTOG = PRJ_GRND_TO_GRID[ID_TO];
			SAVE_PRJ_UNITS = PRJ_UNITS[ID_TO];
			SAVE_PRJ_ZONE = PRJ_ZONE[ID_TO];
			PRJ_TYPE[ID_TO] = GEO;
			PRJ_X_BIAS[ID_TO] = 0;
			PRJ_Y_BIAS[ID_TO] = 0;
			PRJ_UNITS[ID_TO] = 4;
			PRJ_GRND_TO_GRID[ID_TO] = 1e0;
			//*Y = LAT; //CVTA(LAT);
			//*X = (-1.0 * LONGITUDE); //CVTA(-LONGITUDE);
			   
			IRC = TranProjection2 (ID_FROM,ID_TO,X,Y);
			   
			PRJ_ZONE[ID_TO] = SAVE_PRJ_ZONE;
			PRJ_TYPE[ID_TO] = SAVE_PRJ_TYPE;
			PRJ_X_BIAS[ID_TO] = SAVEXBIAS;
			PRJ_Y_BIAS[ID_TO] = SAVEYBIAS;
			PRJ_GRND_TO_GRID[ID_TO] = SAVEGTOG;
			PRJ_UNITS[ID_TO] = SAVE_PRJ_UNITS;
			if (IRC)
           		return IRC;                   
			*X *= DEG_TO_RAD;
			*Y *= DEG_TO_RAD;
			IRC = pj_transform(PRJ_PROJ4DEF[2], PRJ_PROJ4DEF[ID_TO], 1, 1, X, Y, NULL);
			if (IRC)
           		return IRC;                   
		}
		else if(PRJ_TYPE[ID_FROM] == COUNTY)
        {
           PRJ_SPHEROID[ID_FROM] = 8;
           *X = (*X+PRJ_X_BIAS[ID_FROM])*PRJ_GRND_TO_GRID[ID_FROM];
           *Y = (*Y+PRJ_Y_BIAS[ID_FROM])*PRJ_GRND_TO_GRID[ID_FROM]  ;
            if(PRJ_UNITS[ID_FROM] == 2) 
            { 
               *X = *X * MFT;
               *Y = *Y * MFT;
            }
           IRC = GET_COUNTY_PARMS (PRJ_ZONE[ID_FROM],&CNO,&CPART); 
           if (IRC)
           	return IRC;                   
                
           IRC = ConvertCtyCoord (CNO, 3, CPART,
                              &LAT, &LONGITUDE, &SPX, &SPY, X, Y);
           if (IRC)
           	return IRC;                   
           if(PRJ_TYPE[ID_TO] == COUNTY)  goto S100;
           SAVE_PRJ_TYPE = PRJ_TYPE[ID_FROM];
           SAVEXBIAS = PRJ_X_BIAS[ID_FROM];
           SAVEYBIAS = PRJ_Y_BIAS[ID_FROM];
           SAVEGTOG = PRJ_GRND_TO_GRID[ID_FROM];
           SAVE_PRJ_UNITS = PRJ_UNITS[ID_FROM];
           SAVE_PRJ_ZONE = PRJ_ZONE[ID_FROM];
           PRJ_TYPE[ID_FROM] = GEO;
           PRJ_X_BIAS[ID_FROM] = 0;
           PRJ_Y_BIAS[ID_FROM] = 0;
           PRJ_UNITS[ID_FROM] = 4;
           PRJ_GRND_TO_GRID[ID_FROM] = 1e0;
           *Y = LAT; //CVTA(LAT);
           *X = (-1.0 * LONGITUDE); //CVTA(-LONGITUDE);
               
           IRC = TranProjection2 (ID_FROM,ID_TO,X,Y);
               
           PRJ_ZONE[ID_FROM] = SAVE_PRJ_ZONE;
           PRJ_TYPE[ID_FROM] = SAVE_PRJ_TYPE   ;
           PRJ_X_BIAS[ID_FROM] = SAVEXBIAS;
           PRJ_Y_BIAS[ID_FROM] = SAVEYBIAS;
           PRJ_GRND_TO_GRID[ID_FROM] = SAVEGTOG;
           PRJ_UNITS[ID_FROM] = SAVE_PRJ_UNITS;
           if (IRC)
           	return IRC;                   
       } 
       else 
       { 
         if (PRJ_TYPE[ID_TO] == COUNTY)
         {
            if(PRJ_TYPE[ID_FROM] != GEO || PRJ_SPHEROID[ID_FROM] != 8)
            {
               SAVE_PRJ_TYPE = PRJ_TYPE[ID_TO]  ;
               SAVE_PRJ_UNITS = PRJ_UNITS[ID_TO];
               SAVE_PRJ_SPHEROID = PRJ_SPHEROID[ID_TO];
               PRJ_TYPE[ID_TO] = GEO;
               PRJ_SPHEROID[ID_TO] = 8  ;
               PRJ_UNITS[ID_TO] = 4;
               SAVE_PRJ_ZONE = PRJ_ZONE[ID_TO];
                   
               IRC = TranProjection2 (ID_FROM,ID_TO,X,Y); 
                   
               PRJ_ZONE[ID_TO] = SAVE_PRJ_ZONE;
               PRJ_TYPE[ID_TO] = SAVE_PRJ_TYPE;
               PRJ_UNITS[ID_TO] = SAVE_PRJ_UNITS;
               if (IRC)
               	return IRC;                   
           }
           LAT = *Y; //LAT = CVTB (*Y);
           LONGITUDE = (-1E0*(*X)); //LONGITUDE = CVTB (-1*(*X));
S100:          IRC = GET_COUNTY_PARMS (PRJ_ZONE[ID_TO],&CNO,&CPART);
           if (IRC)
           	return IRC;                   
           IRC = ConvertCtyCoord (CNO, 1, CPART,
                              &LAT, &LONGITUDE, &SPX, &SPY, X, Y)  ;
           if (IRC)
           		return IRC;                   
            if(PRJ_UNITS[ID_TO] ==2) 
            { 
               *X = *X * FTM;
               *Y = *Y * FTM;
            }
            *X =(*X/PRJ_GRND_TO_GRID[ID_TO])-PRJ_X_BIAS[ID_TO];
            *Y =(*Y/PRJ_GRND_TO_GRID[ID_TO])-PRJ_Y_BIAS[ID_TO];
         }
         else 
         {
           IRC = TranProjection2 (ID_FROM,ID_TO,X,Y);
           if (IRC)
           		return IRC;
         }                                       
       }
       if(PRJ_TRAN[ID_TO][2] > 0) 
       { 
         TRANS2 (*X,*Y,&X2,&Y2,(HANDLE)PRJ_TRAN[ID_TO][2]);
         *X = X2;
         *Y = Y2;
       }     
//c        MessageBox( NULL, 'Made It'c,'Tranprojection2'c, MB_OK)

   return IRC;  
       
NoTrans:
	if (PRJ_UNITS[ID_FROM] == 1 && PRJ_UNITS[ID_TO] == 2)
	{
		*X *= FTM;
		*Y *= FTM; 
	}
	else if (PRJ_UNITS[ID_FROM] == 2 && PRJ_UNITS[ID_TO] == 1)
	{
		*X *= MFT;
		*Y *= MFT; 
	}
	return IRC;
       
 }           

  double  CVTA (double V)
 {
       double DEG, MIN, SEC, intval;
       
       DEG = IDNINT (V/1e4);
       MIN = IDNINT (V/1e2) - DEG * 1e2;
       SEC = modf (V/1e2, &intval);
                                  
       return (DEG + MIN/6e1 + SEC/36e2);
 }

   double  CVTB (double V)
   {
       double DEG, MIN, SEC, REM;
          
       DEG = IDNINT (V);
       REM = V - DEG;
       MIN = IDNINT(REM * 6e1);
       REM = V - DEG - MIN/6e1;
       SEC = REM * 36e2;
       return (DEG * 1e4 + MIN * 1e2 + SEC);
    }                                                                
    
//*******************************************************************
long  GET_COUNTY_PARMS (long COUNTYID,long *CNO,long *CPART)
  {
       long  ID;
static short CNOAR[102], CPARTAR[102];
       char Cnum[6], far *lpPtr, far *lpNext, far *lpStuff;
       OFSTRUCT OF ;
       OFSTRUCT FAR* lpOF = &OF;
static  BOOL FIRST = TRUE;
       HFILE lpFile;
       HGLOBAL Handle;
       
        if(FIRST) 
        { 
           FIRST = FALSE  ;
           lpFile = GSSiOpenFile ("[%DL]CTYCOORD.HLP",lpOF,OF_READ);
           if(lpFile==HFILE_ERROR)
           {
             MessageBox(NULL,"Unable to open COUNTY_COORD.HLP" ,
             "GET_COUNTY_PARMS",MB_OK);
             *CNO = -1;
             *CPART = -1;
             return -2;
           }  
        Handle = GSSiGlobAlloc ( 401,GPTR,10000); 
        lpStuff = (char far *) GlobalLock(Handle);    
        BigRead (lpFile,(HPSTR)lpStuff,10000);
        GSSiClose(lpFile);
//S10:        READ (99,11,END=20) LINE;
       lpPtr = _fstrrchr(lpStuff,35);   //ascii 35 = # find the last comment line
       if(!lpPtr)lpPtr = lpStuff; 
       lpPtr++;
       lpPtr = (_fstrchr(lpPtr,'\n') + 1);  //find the end of the last comment line

         while (*lpPtr)
         {
           lpNext = (_fstrchr(lpPtr,'\n') + 1);
       //    READ (LINE,12) ID, CNOAR[ID], CPARTAR[ID];
       //S12: FORMAT (I5,T33,I2,I5);
           Cnum[5] = '\0';
           _fmemcpy(Cnum,lpPtr,5);
           ID = atoi(Cnum);
           lpPtr += 32;
           Cnum[2] = '\0';
           _fmemcpy(Cnum,lpPtr,2);
           CNOAR[ID] = atoi(Cnum);
           Cnum[5] = '\0';
           lpPtr += 2;
           _fmemcpy(Cnum,lpPtr,5);
           CPARTAR[ID] = atoi(Cnum);
           lpPtr = lpNext;
         } 
         GSSiGlobUlFree (&Handle);
       }
    
       *CNO = (long) CNOAR[COUNTYID];
       *CPART = (long) CPARTAR[COUNTYID];
       return 0;
      }
      


