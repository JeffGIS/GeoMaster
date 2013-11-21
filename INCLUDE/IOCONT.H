/*C
C       ***************
C        IOCONT COMMON - LP
C       ***************
C
C******* CONTAINS DATA USED IN I/O SUBROUTINES
      LOGICAL       COPOK$, STONLY
      INTEGER * 2   ILEV, IALIN, CALT$(2), CREV$(2), CDATE$(2), STODAT
      INTEGER * 4   CSURF, HLTOU$
      CHARACTER * 8 CPROJ$(2)
      CHARACTER * 1 PAGEJT
C
      COMMON /IOCONT/ CPROJ$, CSURF , HLTOU$, CALT$ , CREV$ ,
     +                CDATE$, ILEV  , IALIN , STODAT, COPOK$,
     +                STONLY, PAGEJT
C */  

    struct lda60
    {
        char  CPROJ$[3][9];
        long CSURF , HLTOU$;
        short CALT$[3] , CREV$[3] , CDATE$[3]
             , ILEV  , IALIN , STODAT;
        BOOL  COPOK$, STONLY;
        char PAGEJT;
     } IOCONT;