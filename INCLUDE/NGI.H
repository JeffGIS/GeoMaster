#include <windows.h>


 extern long  NRSSN (double SPX, double SPY);
 extern void  NNXSIA (long GEOSEG ,short SUBCEL ,long *IRC);
 extern void  SET_NN_PRINT_ON(void);
 extern void  TRANS2  (double SPX, double SPY,double ROW,double COL,long *TRNRTN);    
 extern void TRAN_PROJECTION (const long u1,const long u3, double SPX, double SPY);
 extern void  UMDB_GET_PATH (short *NGI_DBID,short POS,short COND,short LEVEL,long *UMDB_ST);
 extern void NSETBI (double  AZM, long NOFLIN, long *BINTA,long  IBEG, long IEND,
                      void *FINDXY(),void *IXVAL(), long *JRC);
 extern void   far   NBASSC ( long GSEG, long SUBCL,BOOL *INAREA);
 extern void *SVBPOL(long LINENO,double *X,double *Y);
 extern void *SVXCOR(double *X, double *Y);

            
void  GEOSEG_ID_PACK (char *UGEOS , short *USUBCL, char *MAX_SUBCELL);
void  GEOSEG_ID_UNPACK (char *GEOSEG, short *SUBCEL, char *NGI_SUBCELL_ID);
void  NSCCOR (long GSEG,long u1 , long  *GCORX,  long *GCORY);
 long  NTINELV (double SPX,double SPY);      
 
long  NLOAD (double *SPX,double *SPY); 
void  NLOADS (short *IPNT);
void  NSPFSN (long GEOSEG,short SUBCEL, long I, double *SPX,double *SPY);
void  NEXPND (long LSURNM);
void  GSANE (long LSPXDM,long LSPYDM,long *SNGNUM,long *SNSCNM,long *LLELNM);
void  NIOGNC (long *IPOS,BOOL *INDT,long *NEXKEY,long *IRC) ;
void  NIOGN   (long *LSURNM, BOOL *INDT,long *IRC);
void  NIOGNQ (long *LSURNM, BOOL *INDT, int step, long *IRC);
long  N3PNT (long MODELN[5],float X,float Y,long MISING);
long  N4PNT (long MODELN[5],float X,float Y);
void  NGSANE (long LSPXDM,long LSPYDM,long *SNGNUM,long *SNSCNM,long *LLELNM);
long  NGIELS (short    *SUBCBFA);
long  NGIELV (double SPX,double SPY,long ISURNM); 




/*c
C       **************
C        NGI COMMON   - JS    ]
C       **************
C
C******* CONTAINS THE SURFACE ELEVATION ARRAYS AND RELATED VARIABLES
C        FOR THE NGI SURFACES.        
      short INPUT1, INPUT2, OUTPUT, PATCHA, SUBCEL, USUBCL, SETVAL ;
      long  SCBIAS, GEOSEG, ILNGTH, UGEOS, PRLSPY, SETQUN  ;
      float   MULFAC ;
      BOOL   ARIBYA, NSNINA, SETPRO, NSNIN1, NSNIN2;     */
 
 struct lda56
  {
    long SCBIAS[4], GEOSEG[3], ILNGTH[4];
    float MULFAC;
    long LGEOS,  UGEOS, LSUBCL,  IUSUBC, MNGROW, MXGROW, MNGRWX,
                       MNGRWY, MXGRWX, MXGRWY;
    short           USUBCL, SUBCEL[3], INPUT1[1024] ,   INPUT2[1024] ;
    BOOL          NSNINA[1024], ARIBYA, SETPRO,
                       NSNIN1[1024], NSNIN2[1024];  
   } NGIC1;
   
 struct lda57
   {
      long PRLSPY, SETQUN;
      short  SETVAL,PATCHA[1024],OUTPUT[1024];
   } NGIC2;