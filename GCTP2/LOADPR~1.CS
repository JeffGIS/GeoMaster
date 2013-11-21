#include <windows.h>
#include <stdio.h>
#include <stdlib.h> 
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "shr.h"
                   
extern	HWND	hWndMain;
#define PROJECTION_INCLUDE
#include "proj.h" 
#undef PROJECTION_INCLUDE

long    LoadProjection (long ID,LPSTR INNAME) 
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
       long     NTP;             
       double XFROM[100], YFROM[100], XTO[100], YTO[100];
       float RSQ;
       char *Ptr;
       HFILE lpFile1; 
       OFSTRUCT  OBF;
       OFSTRUCT FAR *lpOBF = &OBF;
       short       I;
       HGLOBAL Handle;             
       BOOL       END;
       char BASEPATH[]="c:\\windows\\";             
       char   TRANTYPE[9],   NAME2[64],
         NAME[64],   PNAME[128], XYUNITS[32],ZUNITS[7];
       char	str[258]; 
       int	n=0;   
       LPSTR	lpPtr;
          
//C---------------------
//C   SUBROUTINE START
//C---------------------

       _fstrcpy(NAME,INNAME);
       XYUNITS[0] = '\0';
       ZUNITS[0]  = '\0';  
       PRJ_TRAN[ID][1] = 0;
       PRJ_TRAN[ID][2] = 0;
       if (_fstricmp(NAME,"BASE") == 0 && ID != 1)
       {
            if(ID == 2) 
            {
               INPUT_IS_BASE = TRUE;
               INPUT_UNITS_CONV = 1;
               INPUT_UNITS = (short)PRJ_UNITS[1];
            } 
            else 
            { 
              if (ID == 3)
              {           
               OUTPUT_IS_BASE = TRUE;
               OUTPUT_UNITS_CONV = 1;
               OUTPUT_UNITS = (short)PRJ_UNITS[1];
              }
            } // lda questions what the next line really does  
           _fstrcpy (&PROJECTION_ID[0][1],"BASE");
           PRJ_UNITS[ID] = PRJ_UNITS[1];
           return 0L;
       }
      switch (ID)
      {
         case 1:
         {
           HAVE_BASE_PROJ = FALSE;
           PRJ_UNITS[ID] = 2; 
           _fstrcpy (NAME2,"baseproj");
           break;
        }
        case 2:
        { 
           if(_fstricmp(NAME,"BASE") != 0) INPUT_IS_BASE = FALSE;
           _fstrcpy(NAME2,NAME);
           INPUT_UNITS_CONV = 1;
           break;
        }
        default :
        {   
           if(_fstricmp(NAME,"BASE") != 0) OUTPUT_IS_BASE = FALSE;
           _fstrcpy(NAME2,NAME);
           OUTPUT_UNITS_CONV = 1;
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
       sprintf (PNAME,"%s%s.CVT","[%DATA_LOC]",NAME2);
//       SetWindowText(hWndMain,PNAME);
       lpFile1=GSSiOpenFile (PNAME,lpOBF,OF_READ);
       if(lpFile1 == HFILE_ERROR)
       { 
	       sprintf (PNAME,"%s.CVT",NAME2);
	       lpFile1=OpenFile (PNAME,lpOBF,OF_READ|OF_SEARCH);
	       if(lpFile1 == HFILE_ERROR)
	       
	       {  
	       	  char 	mess[128];
	       	  
	       	  sprintf (mess,"Error opening projection definition file %s",PNAME);
	          MessageBox(NULL,mess,"Load Projection",MB_ICONEXCLAMATION);
	          return -1;
	       } 
	    } 

       _fstrcpy(&(PROJECTION_ID[ID][0]),NAME2);
NextLine:       
		str[0]='#';	while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}  
		if (str[0]=='[')    
		{
			ExpandText(str);
			goto NextLine;
		}

      // READ (99,*,END=2000) PRJ_TYPE[ID]; 
        PRJ_TYPE[ID]= atol(str);
        if(PRJ_TYPE[ID] == 0)
        { 
           PRJ_UNITS[ID] = 4;
           _fstrcpy(&(PROJECTION_UNITS[ID][0]),"DEGREE");
        } 
        else 
        {
           PRJ_UNITS[ID] = 2;
           _fstrcpy(&(PROJECTION_UNITS[ID][0]),"METERS");
        }     
        if(ID == 1)
        {
	        _fstrcpy(XYUNITS,"[%BASE_UNITS]");
	        ExpandText(XYUNITS);
	        _fstrupr (XYUNITS);
	        if(_fstricmp(XYUNITS,"FEET") == 0)
	        {
	           PRJ_UNITS[ID] = 1;
	           _fstrcpy(&( PROJECTION_UNITS[ID][0]),"FEET");
	        } 
	        else if (_fstricmp(XYUNITS,"METERS") == 0)
	        {
	            PRJ_UNITS[ID] = 2;
	            _fstrcpy(&(PROJECTION_UNITS[ID][0]),"METERS"); 
	        }
	        else if (_fstricmp(XYUNITS,"DEGREE") == 0)
	        {
	            if(PRJ_TYPE[ID] != 0) goto S220;
	            _fstrcpy(&(PROJECTION_UNITS[ID][0]),"DEGREE");
	        } 
	        else if (_fstrlen(XYUNITS) != 0)
	        {
	S220: 		MessageBox(NULL,"Invalid units in projection",
	                     	NAME,MB_OK); 
				_lclose(lpFile1);
	            return 3;
	        } 
        }
        else if(ID == 2)
           	INPUT_UNITS =(short) PRJ_UNITS[2];
        else if (ID == 3)
        	OUTPUT_UNITS = (short)PRJ_UNITS[3];
       // READ (99,*,END=2000) PRJ_ZONE[ID];
		str[0]='#';	while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
        PRJ_ZONE[ID] = atol(str);

       // READ (99,*,END=2000) PRJ_SPHEROID[ID];
		str[0]='#';	while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
        PRJ_SPHEROID[ID] = atol(str);
       // READ (99,*,END=2000) PRJ_GRND_TO_GRID[ID];
		str[0]='#';	while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
        PRJ_GRND_TO_GRID[ID] = atof(str);
       // READ (99,*,END=2000) PRJ_X_BIAS[ID];
		str[0]='#';	while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
       PRJ_X_BIAS[ID] = atof(str);
       // READ (99,*,END=2000) PRJ_Y_BIAS[ID];
		str[0]='#';	while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
       PRJ_Y_BIAS[ID] = atof(str);
       END = FALSE;
       for( I = 0; I < 15;I++)
       {
			str[0]='#';	while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
            PRJ_PARMS[I][ID] = atof(str);
        } 
        END=TRUE;
        if(END) goto S190; 
        //READ (99,'(A)',END=190) TRANTYPE;
		str[0]='#';	while (str[0]=='#') {str[0]=' ';fgetstring (str,256,lpFile1);n++;}
        _fstrcpy(TRANTYPE,str);
        if(_fstricmp(TRANTYPE,"        ")) goto S190;
        while(fgetstring (str,256,lpFile1))
        {
//S170:   READ (99,*,END=180) XFROM[NTP+1],YFROM[NTP+1],
//                           XTO[NTP+1],YTO[NTP+1];
		  lpPtr = str;
          _fmemcpy(&XFROM[NTP+1],lpPtr,8);
          lpPtr += 8;
          _fmemcpy(&YFROM[NTP+1],lpPtr,8);
          lpPtr += 8;
          _fmemcpy(&XTO[NTP+1],  lpPtr,8);
          lpPtr += 8;
          _fmemcpy(&YTO[NTP+1],  lpPtr,8);
          lpPtr += 8;
          NTP++;
          XFROM[NTP] = XFROM[NTP] * FTM;
          YFROM[NTP] = YFROM[NTP] * FTM;
          XTO[NTP] = XTO[NTP] * FTM;
          YTO[NTP] = YTO[NTP] * FTM;
//       goto S170; 
        }
       if(NTP == -1) goto S190;
       PRJ_TRAN[ID][1] = 31 + ID * 2;
       PRJ_TRAN[ID][2] = 31 + ID * 2 + 1;
       STRAN2 (XFROM,YFROM,XTO,YTO,(int)NTP,&RSQ, (int)PRJ_TRAN[ID][1]);
       STRAN2 (XTO,YTO,XFROM,YFROM,(int)NTP,&RSQ,(int)PRJ_TRAN[ID][2]);
S190:  
       if(ID == 1) HAVE_BASE_PROJ = TRUE;
       if(PRJ_TYPE[ID] == 1 )
           if(PRJ_ZONE[ID] == 0) PRJ_ZONE[ID] = 61;
       else 
           if (PRJ_TYPE[ID] > 2 && PRJ_TYPE[ID] != 50) PRJ_ZONE[ID] = 61;
        _lclose(lpFile1);
       return 0;
}       
