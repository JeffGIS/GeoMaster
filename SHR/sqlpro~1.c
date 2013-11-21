#include <windows.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "16two32.h"
    double  FloatAP (char *PRSN,short NumDecDigits, short *IRC);
    void     SkipToNextQuote (short IB,short *LOC,short L,char *EXP);
     
      BOOL SQLProcessor (char *INPEXP)
{      
//C******* SPECIFICATIONS ***********************************************
//C*                                                                    *
//C*       PROGRAM SUMMARY                                              *
//C*       ------- -------                                              *
//C*    LOGICP (BOOLPROCESSOR) RETURNS A BOOLVALUE WHICH                *
//C*    REPRESENTS THE EVALUATION OF THE BOOLEXPRESSION IN INPEXP.      *
//C*    INPEXP IS SIMILAR TO A SQL WHERE STATEMENT.                     *
//C*                                                                    *
//C*       ARGUMENT DESCRIPTION                                         *
//C*       -------- -----------                                         *
//C*    INPEXP  I*4(LENGTH) THE INPUT EXPRESSION                        *
//C*    LENGTH  I*4  THE LENGTH IF INPEXP                               *
//C*                                                                    *
//C*       AUTHOR                                                       *
//C*       ------                                                       *
//C*     JEFF SMITH                                                     *
//C**********************************************************************
      BOOL      NUMER, LOGICP;
      char *EXPRSN, CODE[]={'.','(',')'},QUOTE=39, DQUOTE=34, 
           OPRATN[12][8]={"NotUsed",">", "<", "==", "!=", ">=", "<=",
                   "NOT", "AND", "OR", "IN", "="}, NOTBLK='F';
      short OPCOD[100], OPRNDS[2][100], LEV[100], LENGTH,      NEXOP  = 1,
      MAXLEV = 1,      IE     = 0,      LOC    = 0,      NEXVAL = 1, LASTLP,
      IB     = 1,   LEVEL  = 1, I, K, J, L, IOPRND,  LOP,  NumOperators = 11,NOP,
      LC1, LC2, IRC, IB1, IB2, L1,L2, IDUM;
      double      AVAL1,    AVAL2;
      short OPRATL[12]={7,1,1,2,2,2,2,3,3,2,2,1} , VALBG[100], VALLN[100];
      HANDLE hGlob; 
      hGlob = GlobalAlloc(GHND,(long)(_fstrlen(INPEXP)+2));
      EXPRSN = (char *)GlobalLock(hGlob);

       _fstrcpy( EXPRSN,INPEXP);
        _fstrupr(EXPRSN);
      LENGTH = _fstrlen(INPEXP);
//C   *****************************************************************
//C   * DECODE ALL THE OPERATORS AND OPERANDS AND DETERMINE THE ORDER *
//C   * IN WHICH THE EVALUATION IS TO OCCURE.                         *
//C   *****************************************************************
     SkipToNextQuote (IB,&LOC,LENGTH,EXPRSN);
  S100: LOC++;
  S105:  if(LOC > LENGTH) goto S190;
       if(EXPRSN[LOC] == CODE[1]) goto S120;
       if(EXPRSN[LOC] == CODE[2]) goto S150;
       if(EXPRSN[LOC] == CODE[3]) goto S160;
      goto S100;
  S120: for(I = 1;I <= NumOperators;I++)//    DO 130 I = 1, 9;
        {
           if(_fstrnicmp(&(EXPRSN[LOC]),OPRATN[I], (int)OPRATL[I]) == 0)  goto S140;
        }
      goto S100;
  S140: OPCOD[NEXOP] = I;
      LEV[NEXOP]   = LEVEL;
      IOPRND = 1;
      if(I == NumOperators) goto S142;
      if(IE == 0) IE = LOC - 1 ;
      VALBG[NEXVAL] = IB;
      VALLN[NEXVAL] = IE - IB + 1;
       if((EXPRSN[IB] != QUOTE) ||
          (EXPRSN[IE] != QUOTE)||
          (IB == IE)) goto S141;
      VALBG[NEXVAL] = VALBG[NEXVAL] + 1;
      VALLN[NEXVAL] = VALLN[NEXVAL] - 2;
  S141: IE  = 0;
      OPRNDS[1][NEXOP] = NEXVAL++;
      IOPRND = 2;
  S142: OPRNDS[IOPRND][NEXOP] = NEXVAL;
       if(NEXOP == 1 || I < NumOperators) goto S148;
       K = NEXOP - 1;
       for(L = 1;L <= K;L++) //DO 145 L = 1, K;
       {
           LOP = K - L + 1;
           if(LEV[LOP] - LEVEL < 0) goto S146;
           if(LEV[LOP] - LEVEL > 0) goto S145;
           if(OPCOD[LOP] - I > 0)goto S146;
           S145:   ;
       }//S145:     ;
      LOP = 0;
  S146: OPRNDS [1][NEXOP] = OPRNDS[1][LOP+1];
  S148: NEXOP++;
      LOC    = LOC + OPRATL[I];
      IB     = LOC;
       SkipToNextQuote (IB,&LOC,LENGTH,EXPRSN);
      goto S105;
  S150: LEVEL++;
      MAXLEV = max (MAXLEV,LEVEL);
       if(IB == LOC)
       {
          IB++;
          SkipToNextQuote (IB,&LOC,LENGTH,EXPRSN);
       }
      LASTLP = LOC;
      goto S100;
  S160: LEVEL++;
       if(IB > LASTLP && IE ==0) IE = LOC - 1;
      goto S100;
  S190:  if(IE == 0) IE = LOC - 1;
      VALBG[NEXVAL] = IB;
      VALLN[NEXVAL] = IE - IB + 1;
       if(EXPRSN[IB] != QUOTE) goto S195;
       if(EXPRSN[IE] != QUOTE) goto S195;
       if(IB == IE) goto S195;
      VALBG[NEXVAL] = VALBG[NEXVAL] + 1;
      VALLN[NEXVAL] = VALLN[NEXVAL] - 2;
//C                        ***************************
//C                        * EVALUATE THE EXPRESSION *
//C                        ***************************
  S195: NOP = NEXOP - 1;
        if(NOP ==0) goto S310;
        for(I = 1;I <= MAXLEV; I++)//     DO 305 I = 1, MAXLEV;
        { 
          L   = MAXLEV - I + 1;
         for(J = 1;J <= 9; J++)//     DO 300 J = 1, 9;
         {
            for(K = 1;K <= NOP;K++)//    DO 295 K = 1, NOP;
            {
                     if(LEV[K] != L || OPCOD[K] != J) goto S295;
                    IB1 = VALBG[OPRNDS[1][K]];
                    L1  = VALLN[OPRNDS[1][K]] -1;
                    IB2 = VALBG[OPRNDS[2][K]];
                    L2  = VALLN[OPRNDS[2][K]] -1;
               //     goto S(200,200,200,200,200,200,270,280,290), J;
              if(J >=1 && J <=6)
              {
                    NUMER = FALSE;
                    LC1    = max(1,IB1-1);
                    LC2    = max(1,IB2-1);
                     if(EXPRSN[LC1] ==QUOTE ||
                        EXPRSN[LC2] == QUOTE) goto S201;
//C                    IF (ARITHO (EXPRSN(IB1][IB1+L1]) &&
//C     +                  ARITHO (EXPRSN(IB2][IB2+L2])) NUMER = TRUE
//C                    IF (.NOT. NUMER) goto S201
                    IRC = -1;
                    AVAL1 = FloatAP (&(EXPRSN[IB1]),IDUM,&IRC);
                     if(IRC != 0) goto S201;
                    IRC = -1;
                    AVAL2 = FloatAP (&EXPRSN[IB2],IDUM,&IRC);
                     if(IRC != 0) goto S201;
                    NUMER = TRUE;
              }     
  S201:            ;//   goto S(210, 220, 230, 240, 250, 260), J;
             switch(J)
             {
//C******* PERFORM GT OPERATION
              case 1:
                     if(NUMER) goto S215;
                     if(_fstrncmp(&EXPRSN[IB1], &EXPRSN[IB2], L2) < 0)break;
                     if(_fstrncmp(&EXPRSN[IB1], &EXPRSN[IB2], L2) > 0)goto S212;
  S212:              _fstrnset(&EXPRSN[IB1],' ',L1);
                     goto S295;
   S215:             if(AVAL1 > AVAL2) goto S212;
                     break;
//C******* PERFORM LT OPERATION
              case 2:
                     if(NUMER) goto S225;
                     if(_fstrncmp(&EXPRSN[IB1], &EXPRSN[IB2], L2) < 0)goto S212;
                     break;
  S225:                if(AVAL1 < AVAL2) goto S212;
                    break;
//C******* PERFORM EQ OPERATION
              case 3:
              case 11:
                    if(NUMER) goto S235;
                     if(_fstrncmp(&EXPRSN[IB1], &EXPRSN[IB2], L2) == 0)goto S212;
                     break;
  S235:              if(AVAL1 == AVAL2) goto S212;
                     break;
//C******* PERFORM NE OPERATION
              case 4:
                      if(NUMER) goto S245;
                     if(_fstrncmp(&EXPRSN[IB1], &EXPRSN[IB2],L2) != 0)goto S212;
                     break;
  S245:              if(AVAL1 != AVAL2) goto S212;
                     break;
//C******* PERFORM GE OPERATION
              case 5:
                    if(NUMER) goto S255;
                     if(_fstrncmp(&EXPRSN[IB1], &EXPRSN[IB2],L2) < 0)break;
                     if(_fstrncmp(&EXPRSN[IB1], &EXPRSN[IB2],L2) > 0)goto S212;
                     break;
  S255:              if(AVAL1 >= AVAL2) goto S212;
                     break;
//C******* PERFORM "IN" OPERATION
               case 10:
                    if(NUMER) goto S2265;
                    if(_fstrstr(&EXPRSN[IB1], &EXPRSN[IB2]) != 0)goto S212;
                    break;
  S2265:            if(AVAL1 <= AVAL2) goto S212;
                    break;
//C******* PERFORM LE OPERATION
               case 6:
                      if(NUMER) goto S265;
                     if(_fstrncmp(&EXPRSN[IB1], &EXPRSN[IB2], L2) < 0)goto S212;
                     if(_fstrncmp(&EXPRSN[IB1], &EXPRSN[IB2], L2) > 0)break;
  S265:              if(AVAL1 <=AVAL2) goto S212;
                     break;
//C******* PERFORM NOT OPERATION
               case 7:
                   if(_fstrncmp(&EXPRSN[IB1],"                                                ",
                              L1) == 0)   break;
                    _fstrnset(&EXPRSN[IB1],' ',L1);
                    goto S295;
//C******* PERFORM AND OPERATION
               case 8:
                    if(_fstrncmp(&EXPRSN[IB1],"                                                                     ",
                               L1) != 0) break;
                     if(_fstrncmp(&EXPRSN[IB2],"                                                                ",
                                L2) != 0) break;
                     _fstrnset(&EXPRSN[IB1],' ',L1);
                    goto S295;
//C******* PERFORM OR OPERATION
               case 9:
                   if(_fstrncmp(&EXPRSN[IB1],"                                                                       ",
                          L1) == 0 ||
                       (_fstrncmp(&EXPRSN[IB2],"                                                                       ",
                          L2) == 0))goto S291;
                    EXPRSN[IB1] = NOTBLK;
                    goto S295;
  S291:          _fstrnset(&EXPRSN[IB1],' ',L1);
   
             }//end of the switch
                 EXPRSN[IB1] = NOTBLK;
  S295:    ;
            }//end of the K for loop
  
          }//end of the J for loop
                     
        }//end of the I for loop
  S310: LOGICP = TRUE; 
        if(_fstrncmp(&(EXPRSN[VALBG[1]]),"                                                               ",
          (int) VALLN[1]) != 0)
          LOGICP = FALSE;
          GlobalUnlock(hGlob);
          GlobalFree(hGlob);
      return LOGICP;
     }

 void     SkipToNextQuote (short IB,short *LOC,short L,char *EXP)
      {
      char *cptr;
      char QUOTE=39, DQUOTE=34; 
      *LOC = 0;
       if(IB >= L) return;
      if(EXP[IB] != QUOTE)
       {
         if(EXP[IB] != DQUOTE)return;
         cptr = _fstrchr (&(EXP[IB+1]),DQUOTE);
       }
       else
         cptr = _fstrchr (&(EXP[IB+1]),QUOTE);
       if(cptr == 0) return;
       *cptr = '\0';
      *LOC = _fstrlen(&EXP[IB]);
      if(EXP[IB] != QUOTE)
         *cptr = 39;
       else
         *cptr = 34;
      return;
  }
      
