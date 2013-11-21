/*C
C       **************
C        NLOAD COMMON   - JS
C       **************
C
C******* CONTAINS VARIABLES FOR THE NGI LOAD PROCESS.
      INTEGER*4 TRNRTN
      INTEGER*4 NROWS, NCOLS, LOUNIT, LODBIA
      REAL*4    INDVAL
      CHARACTER LODFOR*200
C
      COMMON / NLOADC / TRNRTN, NROWS, NCOLS, INDVAL, LOUNIT, LODBIA,
     +                  LODFOR
C  */
  struct lda65
    {
      long   TRNRTN, NROWS, NCOLS;
      float   INDVAL;
      long LOUNIT, LODBIA;
      char   LODFOR[201];
    } NLOADC;  