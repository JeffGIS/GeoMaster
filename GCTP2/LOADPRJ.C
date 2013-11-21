#include "shr.h"
 
#include "proj.h" 

long    LoadProjection (long ID,char far *INNAME) 
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
       char   TRANTYPE[9],   NAME2[14],
         NAME[14],   PNAME[128], XYUNITS[7],ZUNITS[7],
          *lpStuff,  * lpPtr,  *lpNext;
          
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
           lpFile1 = OpenFile ("c:\\windows\\BaseProj.cvt",lpOBF,OF_READ|OF_SEARCH|OF_SHARE_COMPAT);
           if(lpFile1 == HFILE_ERROR )
           {
             MessageBox(NULL,"Error opening BaseProj","LoadTransprojection",MB_OK);
             return -1;
           }  
           _lread (lpFile1,NAME2,12); 
           Ptr = _fstrchr(NAME2,' ');
           if(Ptr) *Ptr = '\0';
           _lread (lpFile1,XYUNITS,6);
           XYUNITS[6] = '\0';
           Ptr = _fstrchr(XYUNITS,' ');
           if(Ptr) *Ptr = '\0';
           _lclose (lpFile1);
           break;
        }
        case 2:
        { 
           if(stricmp(NAME,"BASE") != 0) INPUT_IS_BASE = FALSE;
           strcpy(NAME2,NAME);
           INPUT_UNITS_CONV = 1;
           break;
        }
        default :
        {   
           if(stricmp(NAME2,"BASE") != 0) OUTPUT_IS_BASE = FALSE;
           strcpy(NAME2,NAME);
           OUTPUT_UNITS_CONV = 1;
        }
      }// end of the switch  
       _fstrupr (NAME2);
       strcpy(PROJECTION_ID[ID] ,NAME2);
       if (stricmp(NAME,"PLOT") == 0)
       {
           PRJ_TYPE[ID] = 100;
           PRJ_UNITS[ID] = 0;
           strcpy(PROJECTION_UNITS[ID],"INCHES");
           return 0L;
       }
       strcpy(PNAME,BASEPATH);
       strcat(PNAME, NAME2);
       strcat(PNAME,".CVT"); 
       lpFile1=OpenFile (PNAME,lpOBF,OF_READ|OF_SEARCH|OF_SHARE_COMPAT);
       if(lpFile1 == HFILE_ERROR)
       {
          MessageBox(NULL,"Error opening BaseProj","LoadTransprojection",MB_OK);
          return -1;
       }  
        Handle = GlobalAlloc(GPTR,3000); 
        lpStuff = (char *) GlobalLock(Handle);    
        _lread (lpFile1,lpStuff,3000);
        _lclose(lpFile1);

       strcpy(&(PROJECTION_ID[ID][0]),NAME2);
       lpPtr = strrchr(lpStuff,35);   //ascii 35 = # find the last comment line
       if(!lpPtr)lpPtr = lpStuff;
       lpPtr = strchr(lpPtr,'\n');  //find the end of the last comment line
      // READ (99,*,END=2000) PRJ_TYPE[ID]; 
        lpPtr++;
        lpNext = strchr(lpPtr,' ');
        *lpNext = '\0';
        PRJ_TYPE[ID]= atol(lpPtr);
        *lpNext = ' ';
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
        _fstrupr (XYUNITS);
        if(stricmp(XYUNITS,"FEET") == 0)
        {
           PRJ_UNITS[ID] = 1;
           strcpy(&( PROJECTION_UNITS[ID][0]),"FEET");
        } 
        else 
        { 
          if (stricmp(XYUNITS,"METERS") == 0)
          {
            PRJ_UNITS[ID] = 2;
            strcpy(&(PROJECTION_UNITS[ID][0]),"METERS");
          } 
          else 
          { 
            if (stricmp(XYUNITS,"DEGREE") == 0)
            {
              if(PRJ_TYPE[ID] != 0) goto S220;
              strcpy(&(PROJECTION_UNITS[ID][0]),"DEGREE");
            } 
            else 
            { 
               if (strlen(XYUNITS) != 0)
               {
S220:             MessageBox(NULL,"Invalid units in projection",
                     NAME,MB_OK); 
                  GlobalUnlock(Handle);
                  GlobalFree(Handle);
                  return 3;
               }
            }
          }
        }       
        if(ID == 2)
           INPUT_UNITS =(short) PRJ_UNITS[2];
        else 
          if (ID == 3) OUTPUT_UNITS = (short)PRJ_UNITS[3];
       // READ (99,*,END=2000) PRJ_ZONE[ID];
        lpPtr = strchr(lpPtr,'\n'); 
        lpPtr++;   
        lpNext = strchr(lpPtr,' ');
        *lpNext = '\0';
                PRJ_ZONE[ID] = atol(lpPtr);
        *lpNext = ' ';

       // READ (99,*,END=2000) PRJ_SPHEROID[ID];
        lpPtr = strchr(lpPtr,'\n');
        lpPtr++;    
        lpNext = strchr(lpPtr,' ');
        *lpNext = '\0';
                PRJ_SPHEROID[ID] = atol(lpPtr);
        *lpNext = ' ';
       // READ (99,*,END=2000) PRJ_GRND_TO_GRID[ID];
        lpPtr = strchr(lpPtr,'\n');
        lpPtr++;    
        lpNext = strchr(lpPtr,' ');
        *lpNext = '\0';
               PRJ_GRND_TO_GRID[ID] = atof(lpPtr);
        *lpNext = ' ';
       // READ (99,*,END=2000) PRJ_X_BIAS[ID];
        lpPtr = strchr(lpPtr,'\n');
        lpPtr++;    
        lpNext = strchr(lpPtr,' ');
        *lpNext = '\0';
               PRJ_X_BIAS[ID] = atof(lpPtr);
        *lpNext = ' ';
       // READ (99,*,END=2000) PRJ_Y_BIAS[ID];
        lpPtr = strchr(lpPtr,'\n');
        lpPtr++;   
        lpNext = strchr(lpPtr,' ');
        *lpNext = '\0';
              PRJ_Y_BIAS[ID] = atof(lpPtr);
        *lpNext = ' '; 
       END = FALSE;
       for( I = 0; I < 15;I++)
       {
           if(END)
           {
               PRJ_PARMS[I][ID] = 0e0;
           } 
           else 
           {
             lpPtr = strchr(lpPtr,'\n');
             lpPtr++;
             if(!lpPtr)
               END = TRUE;
             else
             {      
               lpNext = strchr(lpPtr,' ');
               *lpNext = '\0';
               PRJ_PARMS[I][ID] = atof(lpPtr);
               *lpNext = ' ';
             }
           }
        }
        if(END) goto S190; 
        lpPtr = strchr(lpPtr,'\n');
        lpPtr++;
        NTP = -1;
        if(!lpPtr || *lpPtr == '\r' || *lpPtr == '\0')goto S190;
        *(lpPtr+8) = '\0';
        //READ (99,'(A)',END=190) TRANTYPE;
        _fstrcpy(TRANTYPE,lpPtr);
        *(lpPtr+8) = ' ';
        
        if(stricmp(TRANTYPE,"        ")) goto S190;
        lpPtr = strchr(lpPtr,'\n');
        lpPtr++;
        while(lpPtr)
        {
//S170:   READ (99,*,END=180) XFROM[NTP+1],YFROM[NTP+1],
//                           XTO[NTP+1],YTO[NTP+1];
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
S190:  GlobalUnlock(Handle);
       GlobalFree(Handle);
       if(ID == 1) HAVE_BASE_PROJ = TRUE;
       if(PRJ_TYPE[ID] == 1 )
           if(PRJ_ZONE[ID] == 0) PRJ_ZONE[ID] = 61;
       else 
           if (PRJ_TYPE[ID] > 2 && PRJ_TYPE[ID] != 50) PRJ_ZONE[ID] = 61;
       return 0;
}       
