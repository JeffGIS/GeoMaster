void  LOL8(const double *X1,   const double *Y1,
           const double *X2,   const double *Y2,
           const double *CLEN, const double *DIST,
           double *X3, double *Y3, int ITYPE);


 short RCURVE(double *PCX,double *PCY,double *POCX,double *POCY,
             double *PTX,double *PTY,double *RPX,double *RPY,double *CLEN);
 short PCURVE(double *PCX,double *PCY,double *POCX,double *POCY,
             double *PTX,double *PTY,double *RPX,double *RPY,double *CLEN);
 short PRCURV(double *PCX,double *PCY,double *POCX,double *POCY,
              double *PTX,double *PTY,double *RPX,double *RPY,double *CLEN,
              const short HOW);
BOOL CurveMNMX (DPOINT PC,DPOINT POC,DPOINT PT,LPMNMXCORD pMinMax);
void CRVMNMX (double X1,double Y1,double XR,double YR,double LNG,double TOL,
              LPDOUBLE XMIN,LPDOUBLE XMAX,LPDOUBLE YMIN,LPDOUBLE YMAX);
             


 void CNGRNT(const double *BX1,const double *BY1,const double *EX1,const double *EY1,
                    const double *BX2,const double *BY2,const double *EX2,const double *EY2,
                    short *N1,short *N2, short *IRC);
                    

void SECLIN8(const double *X1,const double *Y1,const double *A1,const double *X2,
                    const double *Y2,const double *A2,double *X3,double *Y3,short *K);
                    
double DSPTLN(const double *X1,const double *Y1,const double *X2,const double *Y2,
                     const double *XP,const double *YP, short *N, short m1);
                      
 double DSTMIN(const double *X1,const double *Y1,const double *X2,const double *Y2,
                      const double *X3,const double *Y3,const double *XP,const double *YP,
                      short *N);
                      
short INLNCK(const double *BX,const double *BY,const double *EX,const double *EY,
                  const double *PX8,const double *PY8); 
                    
 double DMNMX(const double *X1,const double *Y1,const double *X2,const double *Y2,
                     const double *X3,const double *Y3,const double *X4,const double *Y4,
                     short *N1,short *N2,short *IRC);
                                                                                  
double DMNMX_TOL(const double *X1,const double *Y1,const double *X2,const double *Y2,
                        const double *X3,const double *Y3,const double *X4,const double *Y4,
                        short *N1,short *N2, const double TOL,short *IRC);
                        
void XLL(const double *BX1, const double *BY1, const double *EX1, const double *EY1,
                const double *BX2, const double *BY2, const double *EX2, const double *EY2,
                double *X1, double *Y1, double *X2, double *Y2, double *X3, double *Y3, 
                short *N1, short *N2, double *DMN, short *ICD);
         
void XLC(const double *ldax1,  const double *lday1,
                const double *ldax2,  const double *lday2, const double *ldalngth,
                const double *ldax21, const double *lday21, 
                const double *ldax22, const double *lday22,
                double far *x1, double far *y1, double far *x2,double far *y2,
                double far *x3, double far *y3, short *n1, short *n2,
                double *gap, short *i2);  

void XCC(const double *c1x,const double *c1y,const double *r1x,const double *r1y,
                const double *cl1,const double *c2x,const double *c2y,const double *r2x,
                const double *r2y,const double *cl2,double *x1, double *y1, 
                double *x2, double *y2, double *x3, double *y3,
                short *n1, short *n2, double *dmn, short *icd);                        
                
 short     INCRVE(const double *CX,
                const double *CY,
                const double *RX,
                const double *RY,
                const double *CL,
                const double *PtX,
                const double *PtY);
 