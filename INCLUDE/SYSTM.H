/*C
C       **************
C        SYSTM COMMON - LP
C       **************
C
 C******* CONTAINS INFORMATION FOR THE CARS SYSTEM
      CHARACTER DPANMB(10)*1, CNVPRJ*8, UAC*4, SYSDAT*8
      CHARACTER DCTARY*2
      LOGICAL RESPOND
      INTEGER*2 REQCOD, DBLEV
      INTEGER*4 SYSIN,  SYSOT,  SYSER,  SYSTP,  ALL9S,  IQUIT
      INTEGER*4 ENDFL,  IDEX,   NGIDX,  IBNDX,  INNUM
      INTEGER*4 ERRTOT, DPFSEC, SRFRES
      REAL*8 DLAMB,  PRJID,  STRTIM, TXFORM
      REAL*8 PRJFAC, PRJFSQ, PRJSCL
C
      COMMON /SYSTM/  PRJID,  PRJFAC, PRJFSQ, DLAMB,  PRJSCL, SYSDAT,
     +                STRTIM, CNVPRJ, TXFORM, SYSIN,  SYSOT,  SYSER,
     +                SYSTP,  ALL9S,  IQUIT,  ENDFL,  IDEX,   NGIDX,
     +                IBNDX,  INNUM,  ERRTOT, UAC,    DPFSEC, SRFRES,
     +                REQCOD, DCTARY, DBLEV,  DPANMB, RESPOND
C  */
       struct LDA62
       {
         double PRJID,  PRJFAC, PRJFSQ, DLAMB,  PRJSCL;
         char   SYSDAT[9];
         double STRTIM;
         char   CNVPRJ[9];
         double TXFORM;
         long   SYSIN,  SYSOT,  SYSER,  SYSTP,  ALL9S,  IQUIT,  ENDFL, 
                   IDEX,   NGIDX,  IBNDX,  INNUM,  ERRTOT;
         char   UAC[5];
         long   DPFSEC, SRFRES;
         short  REQCOD;
         char   DCTARY[3];
         short  DBLEV;
         char   DPANMB[11];
         BOOL   RESPOND ;
     } SYSTM;