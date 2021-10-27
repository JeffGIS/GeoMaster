#include "shr.h"
#include "proj.h" //{/umsc/include/projections.ftn} 
#include "gmextern.h"

#include "..\gctpc\proj.h"
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
                     
#define COUNTY 50
//{Geographic Coordinate System
#define GEOx 0
#define DEG 4
#define DEG_UNITS 4
#define PRJ_GEO 0
//{Units code for degrees of arc

static	char	NADCONInitFile[256]={"[%DL]conus.bin"};     
static double     PRJ_PARMS[4][16];        //{Reference system parameters
static	double     PRJ_IN_COR[2];         // {Input reference system coordinates
                               // {   (1) is the X coordinate value
                               // {   (2) is the Y coordinate value
static	double     PRJ_OUT_COR[2];        // {Output reference system coordinates
                               // {   (1) is the X coordinate value
                               // {   (2) is the Y coordinate value



LPSTR GetDistAndUnits (LPSTR pParm,LPDOUBLE pDist,LPSHORT pCurDistUnits,BOOL UseDefault);

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
		long	ErrOutCode=0, ParmOutCode=0;
		char	ParmFileName[16]="";
		char	ErrFileName[16]="";  
		char	NAD27ParmFile[16]="nad27sp", NAD83ParmFile[16]="nad83sp";


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
             _fmemcpy(&OutParms[1],PRJ_PARMS[ID_FROM],120); 
              
//             irc =  GCTPZ0 (lpGctp,&PRJ_IN_COR[0], PRJ_PARMS[ID_FROM], 
//                      &TwoCoords[0],&OutParms[1]);
			gctp(PRJ_IN_COR, &lpGctp->FROM_PRJ_TYPE, &lpGctp->FROM_PRJ_ZONE, PRJ_PARMS[ID_FROM],
				 &lpGctp->FROM_PRJ_UNITS, &lpGctp->PRJ_SPHEROID, &ErrOutCode, ErrFileName,&ParmOutCode,ParmFileName,
				 TwoCoords,  &lpGctp->TO_PRJ_TYPE, &lpGctp->TO_PRJ_ZONE, &OutParms[1],
				 &lpGctp->TO_PRJ_UNITS, &lpGctp->PRJ_SPHEROID, 
				 NAD27ParmFile,NAD83ParmFile,
				 &irc);
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
                          
                if(irc !=0 )
                  goto S90;
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
            irc = 0;
            {
            	DPOINT	p,p2;   
            	int	dir=0;  
            	double	xdif, ydif,dst;
            	static	BOOL First=TRUE;  
            	static	int	st; 
            	static	double	maxdist=0;
            	
            	if (First)
            	{
            		ExpandText (NADCONInitFile);
            		if (!GM32NADCONINIT (NADCONInitFile))
            		{   
            			MessageBox (0,"File conus.bin not found","Error in NADCON conversion",MB_ICONEXCLAMATION);
            			irc = 1;
            			goto S90;
            		} 
            		First = FALSE;
            		
            	}
            	
            	if (KEY < 0)
            		dir = 1;
            	
            	p.x = TMP_CORDS[0];
            	p.y = TMP_CORDS[1];
            	if (!GMNADCON (&p,dir))
            		irc = 2;
            	TMP_CORDS[2] = p.x;
            	TMP_CORDS[3] = p.y;
/*            	irc = NADCON (&KEY, &TMP_CORDS[0]); 
            	xdif = TMP_CORDS[2] - p.x;
            	ydif = TMP_CORDS[3] - p.y;  
            	p2.x =TMP_CORDS[2];
            	p2.y =TMP_CORDS[3];   
            	dst = ArcDistance (p,p2); 
            	maxdist = max (maxdist,dst);
            	p.x =TMP_CORDS[0];
            	p.y =TMP_CORDS[1];   
            	dst = ArcDistance (p,p2);
            	dir = 0;  */
            }
            if(irc != 0)
             goto S90;
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

//              irc = GCTPZ0 (lpGctp, &TMP_CORDS[2], PRJ_PARMS[ID_FROM], 
//                      &PRJ_OUT_COR[0],PRJ_PARMS[ID_TO]);
			gctp(&TMP_CORDS[2], &lpGctp->FROM_PRJ_TYPE, &lpGctp->FROM_PRJ_ZONE, PRJ_PARMS[ID_FROM],
				 &lpGctp->FROM_PRJ_UNITS, &lpGctp->PRJ_SPHEROID, &ErrOutCode, ErrFileName,&ParmOutCode,ParmFileName,
				 PRJ_OUT_COR, &lpGctp->TO_PRJ_TYPE, &lpGctp->TO_PRJ_ZONE, PRJ_PARMS[ID_TO],
				 &lpGctp->TO_PRJ_UNITS, &PRJ_SPHEROID[ID_TO], 
				 NAD27ParmFile,NAD83ParmFile,
				 &irc);
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
               if (lpGctp->FROM_PRJ_TYPE == lpGctp->TO_PRJ_TYPE &&
               	   lpGctp->FROM_PRJ_ZONE == lpGctp->TO_PRJ_ZONE &&
               	   (lpGctp->FROM_PRJ_TYPE == 1 || lpGctp->FROM_PRJ_TYPE == 2))
               {    
               		double factor=1;
               		
               		if (lpGctp->FROM_PRJ_UNITS == 1 && lpGctp->TO_PRJ_UNITS == 2)
               			factor = FTM;
               		else if (lpGctp->FROM_PRJ_UNITS == 2 && lpGctp->TO_PRJ_UNITS == 1)
               			factor = MFT;
               		PRJ_OUT_COR[0] = PRJ_IN_COR[0] * factor;
               		PRJ_OUT_COR[1] = PRJ_IN_COR[1] * factor;  
               		irc = 0;
               }
               else
//             irc =  GCTPZ0 (lpGctp, &PRJ_IN_COR[0], PRJ_PARMS[ID_FROM], 
//                      &PRJ_OUT_COR[0], PRJ_PARMS[ID_TO]); 
			gctp(PRJ_IN_COR, &PRJ_TYPE[ID_FROM], &PRJ_ZONE[ID_FROM], PRJ_PARMS[ID_FROM],
				 &PRJ_UNITS[ID_FROM], &PRJ_SPHEROID[ID_FROM], &ErrOutCode, ErrFileName,&ParmOutCode,ParmFileName,
				 PRJ_OUT_COR, &PRJ_TYPE[ID_TO], &PRJ_ZONE[ID_TO], PRJ_PARMS[ID_TO],
				 &PRJ_UNITS[ID_TO], &PRJ_SPHEROID[ID_FROM], 
				 NAD27ParmFile,NAD83ParmFile,
				 &irc);
                      
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
          //  MessageBox(NULL,"Error in GCTP transformation" ,
          //          "TranProj2 Error",MB_ICONSTOP);
            return irc;
         } 
         else 
         {
//C          Good status - remove bias and conversion factor.
           *X =(PRJ_OUT_COR[0] /PRJ_GRND_TO_GRID[ID_TO])-PRJ_X_BIAS[ID_TO];
           *Y =(PRJ_OUT_COR[1] /PRJ_GRND_TO_GRID[ID_TO])-PRJ_Y_BIAS[ID_TO];
         }
S99:    return 0;

}

long    LoadProjection (long ID,LPSTR InName) 
{

/*C
C******* SPECIFICATIONS ********************************************************
C*        This routine reads a projection file and initializes the projections
C*        common block.
C*
C*        ARGUMENTS
C*        ---------
C*        ID         I*4   (IN)   1 = System base projection
C*                                2 = Input projection
C*                                3 = Output projection
C*
C*        INNAME     C*(*) (IN)   Name of the projection file
C*
C*                               Return status code
C*                                  0 = OK
C*                                  1 = Can't open projection file
C*                                  2 = Error reading projection file
C*                                  3 = Invalid units in projection
C*
C*******************************************************************************
C

C-------------------------------
C   GLOBAL VARIABLE DECLARATION  
C-------------------------------        */


//C-------------------------------
//C   LOCAL VARIABLE DECLARATION  
//C-------------------------------

     //  long     ID;             //{Projection code 1=base, 2=input, 3=output
       long     NTP=0;             
       double XFROM[100], YFROM[100], XTO[100], YTO[100];
       float RSQ;
       char *Ptr;
       HFILE lpFile1; 
       OFSTRUCTGM  OBF;
       OFSTRUCTGM FAR *lpOBF = &OBF;
       short       I, Units;
       HGLOBAL Handle;             
       BOOL       END;
       //char BASEPATH[]="c:\\windows\\";             
       char   TRANTYPE[9],   NAME2[MAX_PATH],
         NAME[MAX_PATH],   PNAME[MAX_PATH], XYUNITS[32],ZUNITS[7];
       char str[260], Suffix[6]=".CVT"; 
       int  n=0, SETUNITS;   
	   char INNAME[MAX_PATH * 4];
       LPSTR    lpPtr, lpCVT, pBeg, lpGoogle;
	   BOOL fromOtherDL = FALSE;

	   if (!ID)
		   ii = 1;

	   strcpy(INNAME, InName);
//C---------------------
//C   SUBROUTINE START
//C---------------------
	   Top:
	   FreePROJ(ID);
	   if (*INNAME == '+')
	   {
		   if (!(PRJ_PROJ4DEF[ID] = pj_init_plus(INNAME)))
		   {
			   PRJ_PROJ4DEF[ID] = 0;
			   return 4;
		   }
		   IS_BASE[ID] = FALSE;
		   PRJ_TYPE[ID] = PROJ4PROJECTION;
		   PRJ_SPHEROID[ID] = PRJ_SPHEROID[1];
		   PRJ_ZONE[ID] = PRJ_ZONE[1];
		   strlwr(INNAME);
		   if (strstr(INNAME, "units=m"))
			   PRJ_UNITS[ID] = PRJ_UNITS_METERS;
		   if (strstr(INNAME, "units=f"))
			   PRJ_UNITS[ID] = PRJ_UNITS_FEET;
		   if (strstr(INNAME, "proj=longlat"))
			   PRJ_UNITS[ID] = PRJ_UNITS_LATLON;
		   return 0;
	   }
	   else if (IsProjectionFile(INNAME))
	   {
		   LPSTR def = SHPGetNVP(INNAME,0);
		   strcpy(INNAME, def);
		   free(def);
		   goto Top;
	   }

       strcpy(NAME,INNAME); 
	   ExpandText (NAME);
       strupr (NAME);   
	   if (ExistFile(NAME))
	   {
		   char DL[MAX_PATH] = "[%DL]";
		   ExpandText(DL);
		   strupr(DL);
		   if (strncmp(NAME, DL, strlen(DL)))
			   fromOtherDL = TRUE;
	   }
       if ((lpCVT = _fstrstr (NAME,".CVT")))
       		*lpCVT = 0;
       XYUNITS[0] = '\0';
       ZUNITS[0]  = '\0';  
	   if ((pBeg = strrchr (NAME,'\\')))
		   pBeg++;
	   else
		   pBeg = NAME;
       if (!fromOtherDL && !_fstricmp(pBeg,"BASEPROJ") && ID != 1)
       {     
       		IS_BASE[ID]=TRUE; 
       		PRJ_TYPE[ID]=PRJ_TYPE[1];
			PRJ_PROJ4DEF[ID] = PRJ_PROJ4DEF[1];
			PRJ_SPHEROID[ID]=PRJ_SPHEROID[1];
			PRJ_ZONE[ID]=PRJ_ZONE[1];
			PRJ_GRND_TO_GRID[ID]=PRJ_GRND_TO_GRID[1];
			PRJ_X_BIAS[ID]=PRJ_X_BIAS[1];
			PRJ_Y_BIAS[ID]=PRJ_Y_BIAS[1];
       		PRJ_UNITS[ID]=PRJ_UNITS[1];
       		PRJ_BASEUNITS[ID]=PRJ_BASEUNITS[1];
           _fstrcpy (PROJECTION_ID[ID],"BASEPROJ");
           return 0L;
       }  
	   if (_fstrstr(NAME, ".CPT"))
	   {
		   char	OppNAME[MAX_PATH];

		   PRJ_TYPE[ID] = 0;
		   PRJ_UNITS[ID] = 0;
		   PRJ_BASEUNITS[ID] = 0;
		   PRJ_TRAN[ID][2] = 0;
		   PRJ_TRAN[ID][1] = LoadTranFileWithDandT(NAME);
		   if (!PRJ_TRAN[ID][1])
		   {
			   GSSiMessageBox(0, "Error loading transformation file", NAME, MB_ICONEXCLAMATION, 0);
			   return -1;
		   }
		   IS_BASE[ID] = FALSE;
		   PRJ_TYPE[ID] = 200;
		   _fstrcpy(OppNAME, "|OPP|");
		   _fstrcat(OppNAME, NAME);
		   PRJ_TRAN[ID][2] = LoadTranFileWithDandT(OppNAME);

		   return 0L;
	   }
	   lpGoogle = strstr(NAME, "GOOGLE");
	   if (!lpCVT && lpGoogle)
	   {
		   char	OppNAME[MAX_PATH];

		   PRJ_TYPE[ID] = 1000 + atoi (lpGoogle+6);
		   PRJ_UNITS[ID] = 0;
		   PRJ_BASEUNITS[ID] = 0;
		   PRJ_SPHEROID[ID] = 8;
		   PRJ_TRAN[ID][2] = 0;
		   IS_BASE[ID] = FALSE;

		   return 0L;
	   }

       switch (ID)
       {
			case 1:
			{
			   PRJ_UNITS[ID] = 2; 
			   _fstrcpy (NAME2,"BASEPROJ"); 
			   IS_BASE[ID]=TRUE;
			   break;
			}
			default :
			{  
				IS_BASE[ID]=FALSE; 
			   _fstrcpy(NAME2,NAME);
			}
      }// end of the switch  
       _fstrupr (NAME2);
       _fstrcpy(PROJECTION_ID[ID] ,NAME2);
       if (_fstricmp(NAME,"PLOT") == 0)
       {
           PRJ_TYPE[ID] = 100;
           PRJ_UNITS[ID] = 0;
           _fstrcpy(PROJECTION_UNITS[ID],"INCHES");
           return 0L;
       } 
       if (_fstrstr (NAME2,".CVT"))
       		*Suffix=0;
	   if (strchr (NAME2,'\\'))
			sprintf (PNAME,"%s%s",NAME2,Suffix);
	   else
			sprintf (PNAME,"%s%s%s","[%DL]",NAME2,Suffix);
//       SetWindowText(hWndMain,PNAME);
	   if (!ExistFile(PNAME))
		   sprintf(PNAME, "%s%s%s", "[%DL]projections\\", NAME2, Suffix);

	   if (!ExistFile(PNAME))
	   {
           sprintf (PNAME,"%s%s",NAME2,Suffix);
           lpFile1=GSSiOpenFile (PNAME,lpOBF,OF_READ);
           if(lpFile1 == HFILE_ERROR)
           
           {  
              char  mess[1024];
              setDoPaint( FALSE);
              sprintf (mess,"Error opening projection definition file %s",PNAME);
              MessageBox(NULL,mess,"Load Projection",MB_ICONEXCLAMATION|MB_TASKMODAL);
              return -1;
           } 
        }
        else 
        	lpFile1=GSSiOpenFile (PNAME,lpOBF,OF_READ);       

        _fstrcpy(PROJECTION_ID[ID],NAME2);
        str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}   
        SETUNITS = 0;
        if(!_fstricmp(str,"FEET"))
        {
           SETUNITS = 1;
        } 
        else if (!_fstricmp(str,"METERS"))
        {
            SETUNITS = 2;
        } 
        else
        	goto GotFirstLine;
NextLine:       
        str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}   
GotFirstLine:
        if (str[0]=='[')    
        {
            ExpandText(str);
            goto NextLine;
        }

      // READ (99,*,END=2000) PRJ_TYPE[ID]; 
        PRJ_TYPE[ID]= atol(str);
		if (PRJ_TYPE[ID] == -1)
		{
			FreePROJ(ID);

			PRJ_TYPE[ID] = PROJ4PROJECTION;
			PRJ_UNITS[ID] = 2;
			_fstrcpy(PROJECTION_UNITS[ID], "METERS");
			strcpy(projid[ID], "GoogleMaps");
			PRJ_PROJ4DEF[ID] = pj_init_plus("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs");
			goto S190;
		}
        else if(PRJ_TYPE[ID] == 0)
        { 
           PRJ_UNITS[ID] = 4;
           _fstrcpy(PROJECTION_UNITS[ID],"DEGREE");
        } 
        else if (SETUNITS == 1)
        {
           PRJ_UNITS[ID] = 1;
           _fstrcpy(PROJECTION_UNITS[ID],"FEET");
        }     
        else
        {
           PRJ_UNITS[ID] = 2;
           _fstrcpy(PROJECTION_UNITS[ID],"METERS");
        }     
        if(ID == 1)
        {
            _fstrcpy(XYUNITS,"[%BASE_UNITS]");
            ExpandText(XYUNITS);
            _fstrupr (XYUNITS);
            if(_fstricmp(XYUNITS,"FEET") == 0)
            {
               PRJ_UNITS[ID] = 1;
               _fstrcpy(PROJECTION_UNITS[ID],"FEET");
            } 
            else if (_fstricmp(XYUNITS,"METERS") == 0)
            {
                PRJ_UNITS[ID] = 2;
                _fstrcpy(PROJECTION_UNITS[ID],"METERS"); 
            }
            else if (_fstricmp(XYUNITS,"DEGREE") == 0)
            {
                if(PRJ_TYPE[ID] != 0) goto S220;
                _fstrcpy(PROJECTION_UNITS[ID],"DEGREE");
            } 
            else if (_fstrlen(XYUNITS) != 0)
            {
    S220:   ;//    MessageBox(NULL,"Invalid units in projection",
             //               NAME,MB_OK); 
                GSSiClose2 (&lpFile1);
                return -3;
            } 
        }
        PRJ_BASEUNITS[ID] = PRJ_UNITS[ID];
       // READ (99,*,END=2000) PRJ_ZONE[ID];
        str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
        PRJ_ZONE[ID] = atol(str);

       // READ (99,*,END=2000) PRJ_SPHEROID[ID];
        str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
        PRJ_SPHEROID[ID] = atol(str);
       // READ (99,*,END=2000) PRJ_GRND_TO_GRID[ID];
        str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
        PRJ_GRND_TO_GRID[ID] = atof(str);
       // READ (99,*,END=2000) PRJ_X_BIAS[ID];
        str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;} 
       str[23]=0;
       Truncate (str);
       GetDistAndUnits (str,&PRJ_X_BIAS[ID],&Units,FALSE); 
       Units++;
       if (PRJ_UNITS[ID] == 2 && Units == 1)
       		PRJ_X_BIAS[ID] *= FTM; 
       else if (PRJ_UNITS[ID] == 1 && Units == 2)
       		PRJ_X_BIAS[ID] *= MFT; 
       // READ (99,*,END=2000) PRJ_Y_BIAS[ID];
        str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;} 
       str[23]=0;
       Truncate (str);
       GetDistAndUnits (str,&PRJ_Y_BIAS[ID],&Units,FALSE); 
       Units++;
       if (PRJ_UNITS[ID] == 2 && Units == 1)
       		PRJ_Y_BIAS[ID] *= FTM; 
       else if (PRJ_UNITS[ID] == 1 && Units == 2)
       		PRJ_Y_BIAS[ID] *= MFT; 
       END = FALSE;
       for( I = 0; I < 15;I++)
       {
            str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
            PRJ_PARMS[ID][I] = atof(str);
        } 
/*        END=TRUE;
        if(END)
        	goto S190;*/ 
//        READ (99,'(A)',END=190) TRANTYPE;
        str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
        _fstrcpy(TRANTYPE,str);
        if(_fstricmp(TRANTYPE,"LSQ"))
        	goto S190;
        while(fgetstring (str,256,lpFile1))
        {
		  if (sscanf (str,"%lf %lf %lf %lf",&XFROM[NTP], 
									  		  &YFROM[NTP],
									  		  &XTO[NTP],
									  		  &YTO[NTP]) != 4)
		  {
			  if (sscanf (str,"%lf, %lf, %lf, %lf",&XFROM[NTP], 
										  		  &YFROM[NTP],
										  		  &XTO[NTP],
										  		  &YTO[NTP]) != 4)
			  {           
	           	GSSiMessageBox (0,"Error loading transformation file",NAME,MB_ICONEXCLAMATION,0);
			  	goto S190;
			  }  
		  }
          XFROM[NTP] = XFROM[NTP] * FTM;
          YFROM[NTP] = YFROM[NTP] * FTM;
          XTO[NTP] = XTO[NTP] * FTM;
          YTO[NTP] = YTO[NTP] * FTM; 
          NTP++;
        }
       if(NTP == -1) goto S190;
       CloseTRANS2 (&PRJ_TRAN[ID][1]); 
       CloseTRANS2 (&PRJ_TRAN[ID][2]); 
       PRJ_TRAN[ID][1] = STRAN2 (1655,XFROM,YFROM,XTO,YTO,(int)NTP,&RSQ,1,NULL);
       PRJ_TRAN[ID][2] = STRAN2 (1656,XTO,YTO,XFROM,YFROM,(int)NTP,&RSQ,1,NULL);  
S190:  
       if(PRJ_TYPE[ID] == 1 )
           if(PRJ_ZONE[ID] == 0) PRJ_ZONE[ID] = 61;
       else 
		   if (PRJ_TYPE[ID] > 2 && PRJ_TYPE[ID] != COUNTY && PRJ_TYPE[ID] != PROJ4PROJECTION) PRJ_ZONE[ID] = 61;
        GSSiClose2 (&lpFile1);
	   if (ID != 1 && !IS_BASE[ID])
	   {
		   if (PRJ_TYPE[1] != PROJ4PROJECTION && PRJ_TYPE[ID] != PROJ4PROJECTION)
		   {
			   IS_BASE[ID] = TRUE;
			   if (PRJ_GRND_TO_GRID[1] != PRJ_GRND_TO_GRID[ID])
				   IS_BASE[ID] = FALSE;
			   if (PRJ_X_BIAS[1] != PRJ_X_BIAS[ID])
				   IS_BASE[ID] = FALSE;
			   if (PRJ_Y_BIAS[1] != PRJ_Y_BIAS[ID])
				   IS_BASE[ID] = FALSE;
			   if (PRJ_SPHEROID[1] != PRJ_SPHEROID[ID])
				   IS_BASE[ID] = FALSE;
			   if (PRJ_TYPE[1] != PRJ_TYPE[ID])
				   IS_BASE[ID] = FALSE;
			   if (PRJ_UNITS[1] != PRJ_UNITS[ID])
				   IS_BASE[ID] = FALSE;
			   if (PRJ_SPHEROID[1] != PRJ_SPHEROID[ID])
				   IS_BASE[ID] = FALSE;
			   for (I = 0; I < 15; I++)
				   if (PRJ_PARMS[1][I] != PRJ_PARMS[ID][I])
					   IS_BASE[ID] = FALSE;
		   }
	   }
       return 0;
}       
