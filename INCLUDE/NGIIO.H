/*C
C       ***************
C        NGI-IO COMMON  - JS
C       ***************
C
C******* CONTAINS VARIABLES NEEDED TO EQUATE SURFACE NUMBERS WITH NAMES
C        AND THE SSA'S FOR THE NGI IO ROUTINES
      CHARACTER*8  CALSYS, NGI_SURF, NGI_PROJ*8, LAST_INSERTED*4
      INTEGER * 4 GNORGU
      INTEGER * 2 DAORR, WORLEV, NGI_DATE, NGI_ALT, NGI_REV, NGI_DBID,
     +            SAME_SURF
      LOGICAL     PATCH, DUMMY, PANDS, NGI_FIRST_CALL, UPPER_LEVEL_DATA
      COMMON /NGIIOC/ NGI_SURF(10), CALSYS, GNORGU, WORLEV,
     +                NGI_DATE(10), NGI_ALT(10), NGI_REV(10),
     +                NGI_FIRST_CALL(10),
     +                PATCH, NGI_DBID(10),
     +                NGI_PROJ(10), UPPER_LEVEL_DATA, LAST_INSERTED,
     +                SAME_SURF                */
      BOOL DUMMY, PAND;
      
      struct lda61
      {
         char NGI_SURF[9][11], CALSYS[9];
         long GNORGU;
         short WORLEV, NGI_DATE[11], NGI_ALT[11], NGI_REV[11];
         BOOL  NGI_FIRST_CALL[11], PATCH;
         short NGI_DBID[11];
         char  NGI_PROJ[9][11];
         BOOL  UPPER_LEVEL_DATA;
         char  LAST_INSERTED[9][5];
         BOOL  SAME_SURF ;
     } NGIIOC;
