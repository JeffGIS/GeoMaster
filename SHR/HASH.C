#include "graphint.h"
#include "mapl.h"
#include "hash.h" 
            
// extern char *ReturnFileAddress (short id);
/*CD      INTEGER*2 ID;     */
/*C D     CALL HASHS (ID,10,10,1000,' ',0,ST);*/
/*C D     CALL HASHP (ID,'HI THERE  ',LOC); */
/*C D     PRINT *,LOC;    */
/*C D     CALL HASHC (ID);  */
/*C D     CALL HASHS (ID,10,10,991,' ',0,ST);  */
/*C D     CALL HASHP (ID,'HI THERE  ',LOC);  */
/*C D     PRINT *,LOC;  */
/*C D     CALL HASHC (ID);   */
/*C D     END;*/

static   short NID = 0, ID ;                                                                                     
    //  FILE  *file; 
static   long   ISTAT,  BEGLOC,  ARRAY_LN, I ;
static   char  zero[101];   
   //   BOOL  LFILE[51]; 
     union {
              double RVAL;
              char CRI[9];
            }LDA1;
      union {
              short IEQ2[2];
              char  CI[2][2];
            }LDA2;
      union {
              short IEQ[4];
              double RVAL;
            }LDA3;   

  //    char far STORAGE[50][10]  ;
  
  //    POINTER /POINTER/ ELLEN, KEYLEN, NELEM, NUMBER, ARRAY_LEN, ARRAY; 

     //POINTER /IPOINT/ NADDS, MAX, HALF;

  
//      DATA      zero /100*0/, NID/0/, IOFF/32/, PNTRS/50*0/; 


   //   EQUIVALENCE (RVAL, CR) , (IEQ2, CI);
   //   EQUIVALENCE (IEQ, RVAL);
 const long IOFF = 24;
 typedef struct 
   {
     long ELLEN, KEYLEN, NELEM, NUMBER, ARRAY_LEN, SPACER;
#if WIN32
     char  *Array ;
     char  *start;
#else
     char huge *Array ;
     char huge *start;
#endif
     BOOL16 in_use;        
   }   PT ;
 #if WIN32
static    PT  *POINTER;
static    PT  PNTRS[51]; 
#else
static    PT huge *POINTER;
static    PT huge PNTRS[51]; 
 #endif
static struct  from_ihrans
      {
        short NADDS;                     
        long MAX;
        BOOL16 HALF;
      } iPOINT, IPNTRS[51] ;
      
    
      
                   
/*C******* FIND AN AVAILABLE ID*/  

 void HASHS  (int *id, long ITLN, long KEYLN, long NITM, char *FNAME, long NLEN,char *STATUS)
 {      
 
/*C******* SPECIFICATIONS ************************************************/
/*C*                                                                    **/
/*C*       PROGRAM SUMMARY                                              **/
/*C*       ------- -------                                              **/
/*C*    THIS ROUTINE HANDLES A HASH ARRAY OF VARIABLE LENGTH ITEMS.     **/
/*C*    EACH ITEM IS PLACED INTO THE ARRAY BY HASHING THE KEY WHICH     **/
/*C*    IS THE FIRST KEYLN BYTES OF THE ITEM.                           **/
/*C*                                                                    **/
/*C*    HASHS IS CALLED ONCE BEFORE ENTERING ANY ITEMS INTO THE ARRAY.  **/
/*C*    THE STATUS PARAMETER DETERMINES WHETHER THE ARRAY IS TO BE      **/
/*C*    CREATED NEW, UPDATED OR READ. IF NEW NLEN MAY BE 0 IN WHICH     **/
/*C*    CASE THE ARRAY IS TEMPORARY AND DISSAPEARS WHEN A CALL IS       **/
/*C*    MADE TO HASHC OR HASHCA. IN ALL OTHER CASES THE ARRAY IS        **/
/*C*    SAVED IN THE FILE SPECIFIED BY FNAME.                           **/
/*C*                                                                    **/
/*C*    HASHP ENTERS AN ITEM INTO THE ARRAY AND RETURNS THE LOCATION    **/
/*C*    AT WHICH IT WAS STORED (IF THE ITEM ALREADY EXISTED THE         **/
/*C*    LOCATION AT WHICH IT WAS FOUND IS RETURNED AND THE ITEM IS      **/
/*C*    REPLACED IN THE ARRAY).                                         **/
/*C*                                                                    **/
/*C*    HASHF SEARCHES THE ARRAY FOR AN ITEM AND RETURNS ITS LOCATION   **/
/*C*    IF IT IS FOUND OR zero IF IT IS NOT FOUND.                      **/
/*C*                                                                    **/
/*C*    HASHG IS CALLED TO GET THE ITEM STORED IN A GIVEN LOCATION.     **/
/*C*                                                                    **/
/*C*    HASHI IS CALLED TO INSERT A VALUE INTO A SPECIFIC LOCATION.     **/
/*C*    THIS SHOULD NOT BE USED FOR NORMAL HASH ARRAY PROCESSING BUT IS **/
/*C*    USEFUL IF YOU WISH TO USE THE ARRAY SIMPLY AS A HANDY           **/
/*C*    DYNAMICALLY ALLOCATED ARRAY.                                    **/
/*C*                                                                    **/
/*C*    HASHC IS CALLED TO RELEASE THE SPACE TAKEN BY THE ARRAY WHEN IT **/
/*C*    IS NO LONGER NEEDED AND CLOSE THE FILE IT IS STORED IN IF NLEN  **/
/*C*    WAS NOT 0 ON THE CALL TO HASHS.                                 **/
/*C*                                                                    **/
/*C*    HASHCA CLEARS ALL CURRENTLY ALLOCATED HASH ARRAYS.              **/
/*C*                                                                    **/
/*C*    HASHE ERASES THE CONTENTS OF THE SPECIFIED HASH ARRAY.          **/
/*C*                                                                    **/
/*C*    HASHN RETURNS THE NUMBER OF ELEMENTS CURRENTLY IN THE ARRAY.    *                         **/
/*C*                                                                    **/
/*C*    HASHRA PLACES HASH DATA INTO 'DATA', BEGINNING AT BYTE 'IGET'   **/
/*C*    FOR LENGTH 'LEN'.                                               **/
/*C*                                                                    **/
/*C*    HASHPA PLACES HASH DATA FROM 'DATA' INTO HASH ARRAY BEGINNING   **/
/*C*    AT BYTE 'IPUT' FOR LENGTH 'LEN'.                                **/
/*C*                                                                    **/
/*C*    NOTE: ALWAYS MAKE THE HASH ARRAY SIZE AT LEAST 20% LARGER       **/
/*C*          THAN THE NUMBER OF ITEMS IT IS TO HOLD. THE HASHING       **/
/*C*          ALGORITHM WORKS BEST WHEN NITM IS A PRIME NUMBER.         **/
/*C*                                                                    **/
/*C*    NOTE: THE ITEM WHICH CONTAINS ALL BINARY zeroS IS NOT ALLOWED.  **/
/*C*                                                                    **/
/*C*       ARGUMENT DESCRIPTION                                         **/
/*C*       -------- -----------                                         **/
/*C*    ID     I*2  THE HASH ARRAY IDENTIFIER                           **/
/*C*    ITLN   I*4  THE SIZE OF AN ITEM IN THE HASH ARRAY IN BYTES      **/
/*C*    KEYLN  I*4  THE SIZE OF AN KEY PORTION OF THE ELEMENT IN BYTES  **/
/*C*    NITM   I*4  THE SIZE OF THE HASH ARRAY IN ITEMS                 **/
/*C*    FNAME  CHR  THE PATH NAME FOR THE HASH ARRAY.                   **/
/*C*    NLEN   I*4  THE LENGTH OF FNAME IN BYTES - 0 IF THE HASH        **/
/*C*                ARRAY IS NOT TO BE SAVED.                           **/
/*C*    STATUS CHR  'NEW', 'UPDATE', OR 'READ'                          **/
/*C*    ITEM   ADD  THE ITEM TO BE INSERTED OR SEARCHED FOR             **/
/*C*    LOC    I*4  THE LOCATION IN THE HASH ARRAY AT WHICH AN ITEM     **/
/*C*                IS INSERTED OR FOUND. ON A CALL TO HASHG THIS       **/
/*C*                IS RETURNED AS A zero IF THE ITEM IS NOT FOUND.     **/
/*C*    NUMBER I*4  THE NUMBER OF ELEMENTS CURRENTLY IN THE ARRAY.      **/
/*C*    DATA   ADD  AN ADDRESS AT WHICH THE HASH ARRAY IS TO BE         **/
/*C*                TRANSFERED FROM OR TO.                              **/
/*C*    IGET   I*4  THE HASH ARRAY ITEM NUMBER AT WHICH TO BEGIN        **/
/*C*                GETTING DATA.                                       **/
/*C*    IPUT   I*4  THE HASH ARRAY ITEM NUMBER AT WHICH TO BEGIN        **/
/*C*                PUTTING DATA.                                       **/
/*C*    LEN    I*4  THE NUMBER OF BYTES OF DATA TO BE TRANSFERED FROM   **/
/*C*                OR TO THE HASH ARRAY.                               **/
/*C*    IPNTR  I*4  A POINTER TO THE BEGINNING OF THE HASH ARRAY        **/
/*C*                                                                    **/
/*C*                                                                    **/
/*C*       ERROR HANDLING                                               **/
/*C*       --------------                                               **/
/*C*     THIS ROUTINE WILL ABEND WITH AN APPROPRIATE MESSAGE IF THE     **/
/*C*     FILLS UP.                                                      **/
/*C*     THIS ROUTINE WILL ABEND IF STATUS = 'READ' OR 'UPDATE' AND     **/
/*C*     THE FILE DOES NOT EXIST.                                       **/
/*C*     THIS ROUTINE WILL ABEND IF STATUS IS 'READ' AND ANY UPDATES    **/
/*C*     ARE ATTEMPTED THROUGH CALLS TO HASHP, HASHI,  OR HASHPA.       **/
/*C*                                                                    **/
/*C*       AUTHOR                                                       **/
/*C*       ------                                                       **/
/*C*     JEFF SMITH                                                     **/
/*C**********************************************************************/
//%include 'joint_copyright.ftn'  {'/umsc/include/joint_copyright.ftn'}

//%include '/sys/ins/base.ins.ftn'
//%include '/sys/ins/ms.ins.ftn'  
 static BOOL first = TRUE;
     
     short NMLEN, ID;
     long LM, ItemLen=ITLN, Spacer; 
     
     ID = 0; 
       if(first)
       {
          for (I = 0; I < 101;  zero[I] = 0, I++);
          first = FALSE;
      }      
         for (I=1;I<=NID;I++)
        {//  DO I = 1, NID;
          if (PNTRS[I].in_use  == FALSE) goto S2;
        }//  ENDDO;
      if (NID >= 50)
      {
       GSSiMsgBox(0, "HASHS out of HASH POINTERS",
		   "hash.c",MB_ICONSTOP,0);
        return;
      }  
      NID++;
      I        = NID;
 S2: ID     =(short) I;
       *id = ID; 
    //  PNTR =  PNTRS[ID];
      NMLEN  = (short)NLEN;
      if (KEYLN > ITLN)
       {  
         GSSiMsgBox(0, " ERROR in HASHS - KEYLEN greater than ELLEN ",
		   "hash.c",MB_ICONSTOP,0);

          return;
       }  
          if (NMLEN == 0)goto S10;
      //    LFILE[ID] = TRUE;
          if (_fstricmp(STATUS, "NEW") == 0)
          { 
              Spacer = (long)fmod(ITLN,16);
              ARRAY_LN = (NITM+2) * (ITLN+1);
              if(ARRAY_LN + IOFF < 65528 || ItemLen == 2 ||
                 ItemLen == 4 || ItemLen == 8 || ItemLen == 16)
                Spacer = 0;
              else
                if(ITLN > 16) Spacer = 16  - Spacer;
                if(ITLN > 32) Spacer = 32  - Spacer;
                if(ITLN > 64) Spacer = 64  - Spacer;
                if(ITLN > 128)Spacer = 128 - Spacer;
                if(ITLN > 256)Spacer = 256 - Spacer;
                ARRAY_LN = (NITM+2) * (ITLN+1+Spacer);
              
              PNTRS[ID].start  =  MS_CRMAPL (FNAME, NMLEN, 0,  (long) (ARRAY_LN+IOFF),
                                               MS_NR_XOR_1W,  MS_WR, &ISTAT);
               if(PNTRS[ID].start == NULL)                                                        
               {  
                  GSSiMsgBox(0, " ERROR in HASHS -Insuffient Memory " ,
		   "hash.c",MB_ICONSTOP,0);

                  return;
               }  
                PNTRS[ID].Array     = (PNTRS[ID].start + IOFF);
                PNTRS[ID].ELLEN     = ITLN;
                PNTRS[ID].KEYLEN    = KEYLN;
                PNTRS[ID].NELEM     = NITM;
                PNTRS[ID].ARRAY_LEN = ARRAY_LN; 
                PNTRS[ID].NUMBER    = 0; 
                PNTRS[ID].in_use    = TRUE;
                PNTRS[ID].SPACER    = Spacer;
                //MS_UNMAP(PNTRS[ID].start,LM,&ISTAT);
               goto S11;                                                      
           }
           if( _fstricmp(STATUS,"UPDATE")==0)          
           {   
               POINTER =  (PT far *)MS_MAPL (FNAME,NMLEN, 0 ,IOFF, MS_NR_XOR_1W,MS_WR, FALSE,&LM,&ISTAT);
               if(POINTER == NULL)                                                        
               {  
                  GSSiMsgBox(0,  " ERROR in HASHS -Insuffient Memory " ,
		   "hash.c",MB_ICONSTOP,0);

                  return;
               }  
               PNTRS[ID].ELLEN     = POINTER->ELLEN;
               PNTRS[ID].KEYLEN    = POINTER->KEYLEN;
               PNTRS[ID].NELEM     = POINTER->NELEM;
               PNTRS[ID].ARRAY_LEN = POINTER->ARRAY_LEN; 
               PNTRS[ID].NUMBER    = POINTER->NUMBER; 
               PNTRS[ID].in_use    = TRUE;
               PNTRS[ID].SPACER    = POINTER->SPACER;
               ARRAY_LN = PNTRS[ID].ARRAY_LEN; 
               LM = 0;  // 
               MS_UNMAP ((char far *)POINTER, LM, &ISTAT);
               PNTRS[ID].start  = MS_MAPL (FNAME, NMLEN, 0, (long) (ARRAY_LN+IOFF),  MS_NR_XOR_1W,
                                                                     MS_WR,  FALSE, &LM, &ISTAT); 
               if( PNTRS[ID].start == NULL)                                                        
               {  
                  GSSiMsgBox(0,  " ERROR in HASHS -Insuffient Memory " ,
		   "hash.c",MB_ICONSTOP,0);

                  return;
               }  
               PNTRS[ID].Array  =  ( PNTRS[ID].start + IOFF);
               goto S11;                                                      
            }
            if ( _fstricmp(STATUS,"READ")== 0)
            {                                      
               POINTER = (PT far *)MS_MAPL (FNAME,NMLEN, 0 , IOFF, MS_NR_XOR_1W,MS_R, FALSE,&LM,&ISTAT);
               if(POINTER == NULL)                                                        
               {  
                  GSSiMsgBox(0,  " ERROR in HASHS -Insuffient Memory " ,
		   "hash.c",MB_ICONSTOP,0);
 
                  *id = -1;
                  return;
               }  
               PNTRS[ID].ELLEN     = POINTER->ELLEN;
               PNTRS[ID].KEYLEN    = POINTER->KEYLEN;
               PNTRS[ID].NELEM     = POINTER->NELEM;
               PNTRS[ID].ARRAY_LEN = POINTER->ARRAY_LEN; 
               PNTRS[ID].NUMBER    = POINTER->NUMBER; 
               PNTRS[ID].in_use    = TRUE;
               PNTRS[ID].SPACER    = POINTER->SPACER;
               ARRAY_LN = PNTRS[ID].ARRAY_LEN;
               MS_UNMAP ((char far *)POINTER, LM, &ISTAT);
               PNTRS[ID].start = MS_MAPL (FNAME, NMLEN, 0,  (long) (ARRAY_LN+IOFF),
                                           MS_NR_XOR_1W,  MS_R,  FALSE, &LM, &ISTAT);
               PNTRS[ID].Array  =  (PNTRS[ID].start + IOFF);

               if(PNTRS[ID].start == NULL)                                                        
               {  
                  GSSiMsgBox(0,  " ERROR in HASHS -Insuffient Memory " ,
		   "hash.c",MB_ICONSTOP,0);

                  return;
               }  
            } 
            else 
            {
               GSSiMsgBox(0, " ERROR in HASHS: invalid STATUS parameter"  ,
		   "hash.c",MB_ICONSTOP,0);

               return;
            } 
            goto S11;
            
// gets here if the user didn't supply a filename - meaning they want a temporary hash array created                                        
//S10:     continue; //  LFILE[ID] = FALSE;                     
S10:          Spacer = (long)fmod(ITLN,16);
              ARRAY_LN = (NITM+2) * (ITLN+1);
              if(ARRAY_LN + IOFF < 65528 || ItemLen == 2 || ItemLen == 4 || ItemLen == 8)
                Spacer = 0;
              else
                if(ITLN > 16)Spacer  = 16 - Spacer;
                if(ITLN > 32)Spacer  = 32 - Spacer;
                if(ITLN > 64)Spacer  = 64 - Spacer;
                if(ITLN > 128)Spacer = 128 - Spacer;
                if(ITLN > 256)Spacer = 256 - Spacer;
                ARRAY_LN = (NITM+2) * (ITLN+1+Spacer);

               PNTRS[ID].Array  =  MS_CRMAPL (FNAME, NMLEN, 0,  (long) (ARRAY_LN ),
                                   MS_NR_XOR_1W, MS_R, &ISTAT);
               if(PNTRS[ID].Array == NULL)                                                        
               {  
                  GSSiMsgBox(0, " ERROR in HASHS -Insuffient Memory " ,
		   "hash.c",MB_ICONSTOP,0);

                  *id = -1;
                  return;
               }  
             
              PNTRS[ID].ELLEN     = ITLN;
              PNTRS[ID].KEYLEN    = KEYLN;
              PNTRS[ID].NELEM     = NITM;
              PNTRS[ID].ARRAY_LEN = ARRAY_LN;
              PNTRS[ID].NUMBER    = 0;
              PNTRS[ID].in_use    = TRUE;
              PNTRS[ID].start     = PNTRS[ID].Array ; 
              PNTRS[ID].SPACER    = Spacer;
              
//S11:       PNTRS[ID] = PNTR;

/*C******* SET UP THE RANDOMIZING ROUTINE  */
S11:      I =  IHRANS ( ID, PNTRS[ID].KEYLEN, PNTRS[ID].NELEM);
      return;

}
/*C******* CLEAR A HASH ARRAY ID FOR REUSE - CLOSE FILE if ALLOCATED  */
 void    HASHC (int ID)
 {  
    long LM=0 ;
      
      if (ID == 0) return;
      if (PNTRS[ID].start == 0) return;
//      PNTR = PNTRS[ID];
//      if (FILE[ID])
       _fmemmove(PNTRS[ID].start,     &PNTRS[ID].ELLEN,   4);
       _fmemmove(&PNTRS[ID].start[4], &PNTRS[ID].KEYLEN,  4);
       _fmemmove(&PNTRS[ID].start[8], &PNTRS[ID].NELEM,   4);
       _fmemmove(&PNTRS[ID].start[12], &PNTRS[ID].NUMBER, 4);
       _fmemmove(&PNTRS[ID].start[16], &PNTRS[ID].ARRAY_LEN, 4);
       _fmemmove(&PNTRS[ID].start[20], &PNTRS[ID].SPACER, 4);
       MS_UNMAP(PNTRS[ID].start, LM, &ISTAT); 
       PNTRS[ID].start = 0;
       PNTRS[ID].in_use = FALSE;
      return;
 }
 void      HASH_RESET( )
{       
      NID = 0;
      return;
 }
/*C******* return DESCRIPTION OF HASH ARRAY     */
 void  HASH_DESC (int ID, long *ITLN, long *KEYLN, long *NITM)
{
//      PNTR = PNTRS[ID];
      *ITLN  = PNTRS[ID].ELLEN;
      *KEYLN = PNTRS[ID].KEYLEN;
      *NITM  = PNTRS[ID].NELEM;
      return;
}

/*C******* ERASE THE HASH ARRAY - REMAINS IN USE BUT EMPTY*/
  void    HASHE (int ID)
{      
    //  PNTR = PNTRS[ID];
      _fstrset((char *) PNTRS[ID].Array, '0' );
      /*STRING (IDUM,0,0,ARRAY[1],1,ARRAY_LEN,0,1);*/
      PNTRS[ID].NUMBER = 0;
      return;
}
/*C******* CLEAR ALL HASH ARRAYS*/
void       HASHCA( )
{  
  long LM =0;     
         for (I=1 ; I < NID ; I++)
         { // DO I = 1, NID;
          if (PNTRS[I].start != 0)
           { 
            //  PNTR = PNTRS[I]; ELLEN, KEYLEN, NELEM, NUMBER, ARRAY_LEN
             _fmemcpy(PNTRS[I].start,     &PNTRS[I].ELLEN,   4);
             _fmemcpy(&PNTRS[I].start[4], &PNTRS[I].KEYLEN,  4);
             _fmemcpy(&PNTRS[I].start[8], &PNTRS[I].NELEM,   4);
             _fmemcpy(&PNTRS[I].start[12], &PNTRS[I].NUMBER, 4);
             _fmemcpy(&PNTRS[I].start[16], &PNTRS[I].ARRAY_LEN, 4);
             _fmemcpy(&PNTRS[ID].start[20], &PNTRS[ID].SPACER, 4);
             
             MS_UNMAP(PNTRS[I].start, LM, &ISTAT);
             PNTRS[ID].start = 0;
             PNTRS[ID].in_use = FALSE;

           }   
       }//   ENDDO;
      NID = 0;
      return ;
}

/*C******* return HASH ARRAY ADDRESS */
 void       HASHA (int ID , LPSTR *IPNTR)
{         
      *IPNTR  = (LPSTR)PNTRS[ID].Array;
      return;
}

/*C******* RETRIEVE HASH ARRAY INTO DATA*/
 void       HASHRA (int ID,char *DATA,const long IGET,const long LEND) 
{        
    //  PNTR = PNTRS[ID];
      _fmemcpy( DATA,  &PNTRS[ID].Array[PNTRS[ID].ELLEN+IGET-1],(size_t) LEND);
      return;
}

/*C******* PLACE DATA INTO HASH ARRAY*/
 void   HASHPA (int ID,char *DATA,const long IPUT,const long LEND)
{      
     // PNTR = PNTRS[ID];
      _fmemcpy( &PNTRS[ID].Array[ PNTRS[ID].ELLEN+IPUT-1], DATA,(size_t) LEND);
      return;
 }
 
/*C******* PLACE ITEM INTO HASH ARRAY*/
 void      HASHP (int ID, char *ITEM, long *LOC)
{
    long next;
 //    char dump[200];
//     char *cp = dump;   
     //  PNTR = PNTRS[ID];
      *LOC    = IHRAN ( ID, ITEM);
    //  BEGLOC = *LOC ; 
       next = *LOC;
     //   cp = _fstrncpy (cp, &PNTRS[ID].Array[next * PNTRS[ID].ELLEN], PNTRS[ID].ELLEN );
//       starting = next * PNTRS[ID].ELLEN;
//       forlen = PNTRS[ID].ELLEN; 
//S110:  if (ICMP ( PNTR->Array[LOC* PNTR->ELLEN],ITEM, PNTR->KEYLEN) == 0) goto S122;
//          if (ICMP ( PNTR->Array[LOC* PNTR->ELLEN], zero, PNTR->KEYLEN) == 0) goto S120;
S110:  if (_fmemcmp( &PNTRS[ID].Array[next * (PNTRS[ID].ELLEN+PNTRS[ID].SPACER)]  , ITEM, (size_t)PNTRS[ID].KEYLEN) == 0) goto S122;
       if (_fmemcmp( &PNTRS[ID].Array[next * (PNTRS[ID].ELLEN+PNTRS[ID].SPACER)] ,  zero , (size_t) PNTRS[ID].KEYLEN) == 0) goto S120;
       next-- ;
      //  cp = _fstrncpy(cp,&PNTRS[ID].Array[next * PNTRS[ID].ELLEN], PNTRS[ID].ELLEN ); 
       if (next == *LOC) goto S1000;
       if (next == 0) next = PNTRS[ID].NELEM;
       goto S110 ;
 S120: PNTRS[ID].NUMBER++;
 S122: *LOC = next;
       _fmemcpy ( &PNTRS[ID].Array[next * (PNTRS[ID].ELLEN+PNTRS[ID].SPACER)], ITEM, (size_t)  PNTRS[ID].ELLEN);
     //  cp = _fstrncpy(cp,&PNTRS[ID].Array[(next * PNTRS[ID].ELLEN) - 100], 199 ); 
       return;
/*C****** HASH ARRAY FULL. PRINT ERROR AND STOP*/
S1000: GSSiMsgBox(0,"HASH ARRAY FULL","hash.c",MB_ICONSTOP,0);
return;  

}
      

/*C******* FIND THE LOCATION OF AN ITEM IN THE HASH ARRAY (LOC=0 if NOT*/
/*C        FOUND)*/

 void     HASHF (int ID,char *ITEM, long *LOC)
{  
      long next;
//      char dump[14];    
 //     char *cp = dump; 
 	  if (!ID)
 	  {
 	  	*LOC = 0;
 	  	return;
 	  }
      *LOC    = IHRAN( ID,  ITEM);
     // BEGLOC = *LOC; 
      next = *LOC; 
   //   _fstrncpy(cp, &PNTRS[ID].Array[next * PNTRS[ID].ELLEN],13);
//S210: if ( _fstrncmp (&PNTRS[ID].Array[next * PNTRS[ID].ELLEN] , ITEM , (size_t)  PNTRS[ID].KEYLEN) == 0)
S210: if ( _fmemcmp (&PNTRS[ID].Array[next * (PNTRS[ID].ELLEN+PNTRS[ID].SPACER)] , ITEM, (size_t)PNTRS[ID].KEYLEN) == 0)
          {
           *LOC = next;
           return;
          } 
      //   cp = _fstrncpy(cp,&PNTRS[ID].Array[next * PNTRS[ID].ELLEN], PNTRS[ID].ELLEN ); 
       if ( _fmemicmp ( &PNTRS[ID].Array[next * (PNTRS[ID].ELLEN+PNTRS[ID].SPACER)] , zero , (size_t)  PNTRS[ID].KEYLEN) == 0) goto S220;
      next--;
       if (next == *LOC) goto S1000;
       if (next == 0) next = PNTRS[ID].NELEM;
       goto S210;
  S220: *LOC = 0;
// S210: if ( _fstrncmp (&PNTR->Array[*LOC * PNTR->ELLEN] , ITEM , (size_t)  PNTR->KEYLEN) == 0) return;
//       if ( _fstrncmp (&PNTR->Array[*LOC * PNTR->ELLEN] , zero , (size_t)  PNTR->KEYLEN) == 0) goto S220;
//       *LOC--;
//       if (*LOC == BEGLOC) goto S1000;
//       if (*LOC == 0) *LOC = PNTR->NELEM;
//       goto S210;
//  S220: *LOC = 0;
      return;
/*C****** HASH ARRAY FULL. PRINT ERROR AND STOP*/
S1000: *LOC = 0;
return;  

}      

/*C******* GET AN ITEM FROM THE HASH ARRAY AT LOCATION LOC*/
 void     HASHG (int ID, char *ITEM,const long LOC)
{      
  //    PNTR = PNTRS[ID];
      _fmemcpy( ITEM, &PNTRS[ID].Array[LOC * (PNTRS[ID].ELLEN+PNTRS[ID].SPACER)],(size_t) PNTRS[ID].ELLEN);
      return;
} 
     
/*C*******  INSERT AN ITEM INTO THE HASH ARRAY AT LOCATION LOC WITHOUT*/
/*C         HASHING.*/
 void     HASHI (int ID, char *ITEM,const long LOC)
{      
    //  PNTR = PNTRS[ID];
      if ( _fmemcmp( &PNTRS[ID].Array[LOC * (PNTRS[ID].ELLEN+PNTRS[ID].SPACER)] , 
        zero , (size_t) PNTRS[ID].KEYLEN) == 0) 
         PNTRS[ID].NUMBER++;
      _fmemcpy (&PNTRS[ID].Array[LOC * (PNTRS[ID].ELLEN+PNTRS[ID].SPACER)],
         ITEM ,(size_t) PNTRS[ID].ELLEN);
      return;
}

/*C******* RETURN THE NUMBER OF ITEMS CURRENTLY IN THE HASH ARRAY.*/
 void     HASHN (int ID, long *NUMBR)
{      
     // PNTR = PNTRS[ID];
      *NUMBR   = PNTRS[ID].NUMBER;
      return;
}

/*C***********************************************************************/
  void    HASHWR (int ID, FILE *UNIT)
{ 
 char holder[16], *ptr;
      
    //  PNTR = PNTRS[ID];
      fputs ( " Element len = ", UNIT); 
     ptr =  itoa((int) PNTRS[ID].ELLEN, holder,10);
      fputs (ptr, UNIT);
      fputs(" Key len = ", UNIT);
       ptr = itoa((int) PNTRS[ID].KEYLEN, holder,10);
       fputs(ptr, UNIT);
      fputs(" num elements ", UNIT);
      ptr = itoa((int) PNTRS[ID].NELEM,  holder,10);
      fputs(ptr, UNIT);
      ptr = itoa((int) PNTRS[ID].NUMBER,  holder,10);
      fputc('\n', UNIT); 
      fputs (  " element NUM = ", UNIT);
      fputs(ptr, UNIT);
      fputc('\n', UNIT); 
       
     // FBLKRW (PNTR->Array , PNTR->ARRAY_LEN , 2 , UNIT);
      return;
}     
/*C*********************************************************************************/
long   IHRANS (int ID,long  ILEN,long  IMAX)
{      
/*C******* THIS ROUTINE RANDOMIZES A VARIABLE LENGTH STRING TO AN INTEGER*/
/*C        BETWEEN 1 AND MAX.*/
   
      IPNTRS[ID].NADDS  = (short)( ILEN / 2);
      IPNTRS[ID].HALF = FALSE;
      if ( (ILEN % 2) != 0) IPNTRS[ID].HALF = TRUE;
      IPNTRS[ID].MAX = IMAX;
      return  0;
}      

/*C***********************************************************************/
   long  IHRAN (int ID,char ITEM[])
{       
long   INTTOT = 0, I;
        iPOINT = IPNTRS [ID];
      for (I=1; I < iPOINT.NADDS; I++)
       {// DO I = 1, NADDS;
          INTTOT += ITEM[I] * I;
       }// ENDDO;                  >>
      if (iPOINT.HALF) INTTOT += (iPOINT.NADDS+1) *  ( ITEM[iPOINT.NADDS+1] >> 8 );  // destroys ITEM!!!!
      LDA3.RVAL = 1907719e0 * INTTOT + 907633963e0;
/*C      CI(1) = CRI(1) // CRI(7)*/
/*C      CI(2) = CRI(5) // CRI(3)*/
/*C      IVAL = (long)(IEQ2(1)) * (long)(IEQ2(2))*/
/*C      IHRAN = MOD(IABS(IVAL),MAX) + 1*/
 //     IHRAN = MOD (IABS((long)(IEQ[1])*(long)(IEQ[2])*(long)(IEQ[3])),MAX) + 1;
      return (labs((long)LDA3.IEQ[1]*(long)LDA3.IEQ[2]*(long)LDA3.IEQ[3])  % iPOINT.MAX) + 1;
} 
