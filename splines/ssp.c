/* FILE ssp.c            REVISION DATE: May 7, 1997 */
/*----------------------------------------------------
This file contains the routine:
 (export) smoothspline(double *hp,double *L,
                       double r,double *vp,int16 idsw)
          which may be used to compute an optimal
          2D-smoothing spline or its integral or 
          derivative, with the smoothing parameter
          specified or estimated, 
 and the ancillary routines:
 (private)  allocharray(int32 nr, int32 nc)
 (private)  ox(int16 i,int16 j,int16 h, int16 w)
 (private)  bandsolve(double *A, double *b)
 (private)  dnu(double r)
----------------------------------------------------*/
/* macros used to improve readability */
#define int16 short int
#define int32 long int
#define EQ ==
#define NE  !=
#define AND &&
#define OR  ||
#define NOT !
#define XOR ^
#define private static 
#define forward 
#define import extern
#define export 
#define MAX(x,y) ((x) > (y) ? (x) : (y) )
#define MIN(x,y) ((x) < (y) ? (x) : (y) )
#define ABS(x) (((x)<0)?-(x):(x))    



/*************SYSTEM GLOBALS ************************/
//#include <Clib.h> /*Clib.h defines the C-library functions, e.g. sqrt, exp */
#include "shr.h"
static	char	str[128];  
static	short	ii;
extern	HWND	hWndMain;
import double *cinterpolate(double *hp, double *vp, 
                            double f, int16 k,
                            double *mp, int16 tansw,
                            int16 idsw);

import double *mmean(double *M, int32 x, int32 y, 
                     int32 z, double *f);

import double *mmedian(double *M, int32 x, int32 y, 
                       int32 z, double *f);
LPDOUBLE copyharray(LPDOUBLE A);
/*--------------------------------------------------*/
/**************FILE GLOBALS **************************
 ** (usable by all the functions in this file) ******/

/* macros for harrays */
#define harrayrows(m)  ( *((int32 *)(m)) )
#define harraycols(m)  ( *((int32 *)(m)+1) )

/* macros to access 1-dimensional matrices as 
   multi-dimensional matrices */
#define DNC(i,j) (((i)-1)*(nc)+(j))
#define DQ(i,j)  (((i)-1)*(q)+(j)) 
      
//#define DX(i,j,n) (((i)-1)*(n)+(j))   //js
#define DX(i,j,c) (((i)-1)*(c) +(j))


/* band-matrix access macros */
#define GbarM(i,j) Gbar[ox(i,j,1,n2)]
#define HM(i,j) H[ox(i,j,2,n)]
#define lhtM(i,j) lht[ox(i,j,2,n)]
#define qfM(i,j) qf[ox(i,j,2,n2)]
#define AM(i,j) A[ox(i,j,2,n2)]
#define ZM(i,j) Z[ox(i,j,2,n)]

/* Gbar, qf, fd, lht, hf are all file globals computed
   in smoothspline() and used in dnu(). */
private double *Gbar;
private double *qf;
private double *fd;
private double *lht;
private double *hf;
private double *A;

/*forward*/ private int16   ox(int16 i,int16 j,int16 h, 
                           int16 w);
/*forward*/ private double *bandsolve(double *A, double *b);
/*forward*/ private double  dnu(double r);

/*===================================================*/
private double *allocharray(int32 nr,int32 nc)
/*----------------------------------------------------
allocharray() gets space for a matrix of doubles
of size nr by nc, fills in 0th double in the array
with the row and column sizes and returns a
pointer to this space.
----------------------------------------------------*/
{double *d;
 d = (double *)calloc((size_t)(nr*nc+1),sizeof(double));
 /* install the row and column sizes in the 0th 
    element of the matrix */
 *((int32 *)d)=nr; *(((int32 *)d)+1)=nc;
 return(d);
}

/*==================================================*/
export double *smoothspline(double *hp, double *L, 
                            double r, double *vp,
                            int16 idsw)
/*----------------------------------------------------
double *hp, 
hp[1:n,1:2] is a 2-column matrix.  Each row of hp
represents a data point in the xy-plane, and the
two columns hold 	 the x- and y-coordinates
of the data points for which we want to 	
compute the interpolation points, p, for the
optimal smoothing 	 spline curve.  The rows
of the matrix hp[] are assumed to be sorted in
increasing order on column 1.

double *L,  
L[1:n,1:1] is a 1-column harray of weight values
for the data points in hp.  These weights are
taken as the reciprocal variances for the data
points in hp[].  If L=NULL, then L[] is taken to
be an n-vector of 1's.

double r;  
r is the smoothness factor. If -1<=r<0, then r is
to be computed by minimizing the difference
between a moving mean curve for the data and the
r-parameterized smoothing spline. If r<-1, then a
moving quantile is used instead of a moving mean.
A non-negative r value is used as-is. r=0 implies
a perfect-fitting natural global cubic spline.

double *vp, 
vp[1:m,1:1] is a 1-column matrix containing the
x-values whereat we want values of our smoothing
spline computed and returned.  If vp=NULL then vp
is taken as hp col 1.

int16 idsw, 
idsw is the integral/derivative control switch.
If idsw<0, then we want points of the derivative
of the smoothing spline to be returned.  If
idsw>0, then we want points of the integral of the
smoothing spline to be returned.  If idsw=0, we
want points of the smoothing spline itself to be
returned.

------------------------------------------------------
This procedure computes the optimal smoothing
spline for the 2D functional data points given in
the rows of hp[1:n,1:2] with the associated
weights L[1:n], and the smoothing parameter r.
This is done by computing the points p[1:n,1:2]
for which the natural global cubic spline c that
interpolates the points in p[1:n,1:2] is the
minimizer of
sum(i,1,n,L[i]*(hp[i,2]-c(hp[i,1]))^2) +
r*integral(x,hp[1,1],hp[n,1],|c''(x)|^2).

Given p[1:n,1:2], the optimal smoothing spline c
is determined.  We return the points on the graph
of c(x) (if idsw=0), or the derivative c'(x) (if
idsw<0), or the integral
integral(s,hp[1,1],x,c(s)) (if idsw>0) for
x=vp[1],vp[2],...,vp[m], as the rows of a matrix
dest[1:m,1:2].

Let x[1:n] = hp col 1, let f[1:n] = hp col 2, and
define the (n-1)-vector t[1:(n-1)] so that t[i] =
x[i+1]-x[i] for i = 1,...,(n-1).

To compute the actual interpolation points p[1:n,1:2]:

1) compute v[1:(n-2)] in (Gbar+r*H*L^(-1)*H')v = Hf, 
   where f = hp col 2.
2) compute p[1:n] as f-rL^(-1)H'v.
3) return the matrix x&'p, where x = hp col 1.

Let a1 = t row 1:(n-1), let a2 = t row 1:(n-2),
let b1 = t row 2:(n-1), let b2 = t row 2:(n-2).

Gbar[1:(n-2),1:(n-2)] is the symmetric tridiagonal
matrix whose lower diagonal is (b2/6), whose main
diagonal is (a2+b1)/3, and whose upper diagonal is
(b2/6).

H[1:(n-2),1:n] is a matrix with a main diagonal
and two upper diagonals.  The main diagonal is 1/a2, 
the first upper diagonal is -((1/a2)+(1/b1)), and
the second upper diagonal is 1/b1.

L[1:n,1:n] is a diagonal matrix whose diagonal
element L[i,i] is the weight (reciprocal variance)
for the i-th data point hp row i.
----------------------------------------------------*/ 
{int16 i,j,k,n,m; 
 int16 vpsw=0, Lsw=0; 
 int16 kl,kh,jl,jh,n2,winsize; 
 double s,c,d; 
 double *H;
 double *v; 
 double *t; 
 double *b; 
 double *p; 
 double *f; 
 double *dest;   
 long	ncall=0, nloops;

 //SetWindowText (hWndMain,"Enter Main");

 /* Get the number of data points, n, in hp[1:n,1:2], 
    remember hp is assumed to be sorted on col 1. */
 n = harrayrows(hp); n2 = n-2;

 /* Get the vector of interpolation x-values, vp, 
    whereat we want points. */
 if (vp EQ NULL) /* create vp = hp col 1 */
    {vpsw=1; vp=allocharray(n,1); 
     for (j=1; j<=n; j++) vp[j] = hp[DX(j,1,2)];
    }
 m = harrayrows(vp);

 /* If n<=2, we have a simple straight-line 
    interpolating spline which will be handled 
    by cinterpolate(). */
 if (n < 3) goto c2spline;

 /* Get the weight vector L[1:n] */
 if (L EQ NULL) /* create L = 1^^n */
    {Lsw=1; L = allocharray(n,1); 
     for (j=1; j<=n; j++) L[j] = 1.0;
    }

 /* Now begin to compute the harray of desired actual 
    data points p[1:n,1:2] for the global natural 
    cubic spline which is our final desired optimal 
    smoothing spline. */

 /* Construct the local harray vector t[1:(n-1)]. We 
    assume that no t[i]-value will be zero! */
 t = allocharray(n-1,1);
 for (i=1; i<=n-1; i++) 
     t[i] = hp[DX(i+1,1,2)]-hp[DX(i,1,2)];

 /* Construct the file global harray 
    Gbar[1:(n-2),1:(n-2)].
    Let a1 = t row 1:(n-1), let a2 = t row 1:(n-2),
    let b1 = t row 2:(n-1), let b2 = t row 2:(n-2).
    Gbar[1:(n-2),1:(n-2)] is the symmetric tridiagonal
    matrix whose lower diagonal is (b2/6), whose main
    diagonal is (a2+b1)/3, and whose upper diagonal 
    is (b2/6).*/
 Gbar = allocharray(3,n2); /*size [1:n2,1:n2], 
                             symmetric with 3 bands */
 for (i=1; i<=n2; i++)
    {GbarM(i,i) = (t[i]+t[i+1])/3;
     if (i != n2) 
        GbarM((int16)(i+1),i) = GbarM(i,(int16)(i+1)) = t[i+1]/6;
    }

 /* Construct the local harray H[1:(n-2),1:n].
    H is a matrix with a main diagonal and two upper 
    diagonals.  The main diagonal is 1/a2, the first 
    upper diagonal is -((1/a2)+(1/b1)), and the second
    upper diagonal is 1/b1.*/
 H = allocharray(3,n); /*size [1:n2,1:n], with 1 
                     main-diagonal and 2 upper-bands*/
 for (i=1; i<=n2; i++)
    {HM(i,i) = 1/t[i];
     HM(i,(int16)(i+1)) = -((1/t[i])+(1/t[i+1]));
     HM(i,(int16)(i+2)) = 1/t[i+1];
    }

 /* Construct the file global harray (n-2)x1 vector 
    hf = H*f, where f = hp col 2 */
 hf = allocharray(n2,1);
 for (i=1; i<=n2; i++)
    {kh = MIN(n,i+2);
     for (s=0.,k=i; k<=kh; k++) 
         s += HM(i,k)*hp[DX(k,2,2)];
     hf[i] = s;
    }

 /* We need the file global harray A[1:n2,1:n2] in 
    dnu(), and also later, so we allocate it now. */
 A = allocharray(5,n2); /*diagonals -2,-1,0,1,2 
                          of A[1:n-2,1:n-2] */

 /* If the smoothing factor r is given, use it.*/
 if (r >= 0.) goto computespline;

 /*-------------------------------------------------*/
 /* If r < 0, compute r by minimizing the difference
    between the moving mean smoother( when -1<=r<0) or
    the moving median smoother (when r<-1) and the
    optimal spline smoother.

    To do this, we must:
    1) compute fbar = the moving mean of the vector 
       of values f or moving median of f.
    2) compute the matrices H*L^(-1)*H', Gbar, fbar-f,
       L^(-1)*H', and H*f for reuse in the subroutine
       that computes -.5 times the derivative of our 
       objective function denoted by dnu(r), where
       dnu(r) = (fbar-f+r*L^(-1)*H'*
                (Gbar+r*H*L^(-1)*H')^(-1)*H*f)'*
                [L^(-1)*H'*(Gbar+r*H*L^(-1)*H')^(-1)*
                Gbar*(Gbar+r*H*L^(-1)*H')^(-1)*H*f]

    To compute dnu(r):
    1) solve for x in (Gbar+r*H*L^(-1)*H')*x = H*f.
    2) compute y = Gbar*x.
    3) solve for z in (Gbar+r*H*L^(-1)*H')*z = y.
    4) compute dnu(r) = (fbar-f+r*L^(-1)*H'*x)'*L^(-1)*
                        H'*z.

    We wish to solve for r in dnu(r) = 0.
    The function dnu() given above depends upon the 
    file global arrays Gbar, qf, fd, lht, hf, which 
    are computed below.
 */

 /* Compute the file global harray n x (n-2) matrix 
    lht = L^(-1)*H'.  lht is stored in a 5-row, 
    n-column diagonal table. */
 lht = allocharray(5,n);
 for (i=1; i<=n; i++)
   {kl = MAX(1,i-2); kh = MIN(i,n2);
    for (k=kl; k<=kh; k++) 
        lhtM(i,k) = HM(k,i)/L[i];
   }

 /* Compute the file global harray (n-2) x (n-2) 
    matrix qf = H*L^(-1)*H'.  Note H*L^(-1)*H' is a 
    bandwidth 5 symmetric (n-2)x(n-2) matrix.  qf is
    stored in a 5-row n2-column diagonal table. */
 qf = allocharray(5,n2); /* qf is stored as 5 
                          diagonals (initially zero)*/
 for (i=1; i<=n2; i++)
   {jl = MAX(1,i-2); jh = MIN(n2,i+2);
    for (j=jl; j<=jh; j++)
      {kl = MAX(i,j); 
       kh =kl+2-ABS(i-j); 
       kh = MIN(kh,n);
       for (s=0.,k=kl; k<=kh; k++)
           s += (HM(i,k)/L[k])*HM(j,k);
       qfM(i,j) = s;
      }
    }

 /* Compute the file global harray (n x 1) vectors f 
    and fd = fbar-f, where fbar is computed via a 
    moving mean computation.*/
 f = allocharray(n,1); /* construct f = hp col 2 */
 for (i=1; i<=n; i++) f[i] = hp[DX(i,2,2)];
//hp[
 winsize = n/10; 
 if (winsize < 5) winsize = 5; 
 if (winsize > n) winsize = n-1;
 winsize= winsize | 1; /* OR to make winsize odd */

 p = allocharray(winsize,1); /* set-up triangular 
                                weight vector */
 j = (winsize+1)/2;
 for (i=1; i<j; i++) p[i] = p[winsize+1-i] = i;
 p[j] = j-1;

 /* If -1<=r<0, then compute the moving-mean curve
    via the externally-supplied subroutine mmean(),
    otherwise r<-1, and we want to compute the
    moving-median curve via the externally-supplied
    subroutine mmedian().  In either case, we use the
    window size winsize and the weights
    p[1:winsize,1:1]. */
 if (r<-2.) fd = mmedian(f,winsize,0,-1,p);
 else fd = mmean(f,winsize,0,-1,p);
 free(p);

 for (i=1; i<=n; i++) fd[i] -= f[i];

 /* Compute r as the solution to dnu(r) = 0 via a
    binary search bracketing.  We assume (safely) that
    dnu(0)<0 and we search for a value d such that
    dnu(d)>=0.  Our starting interval is thus [0,d].*/ 
 //SetWindowText (hWndMain,"In Loop 1");
 for (d=10.,nloops=0; dnu(d)<0.; d+=10.)
 {
 	nloops++;
 	if (nloops > 10000)
 		ii=1;
 }
 //SetWindowText (hWndMain,"In Loop 2");
 for (c=0.; (d-c)>.0001;)
   {r=(c+d)/2.;
    s=dnu(r);
    if (s<=0.)
    	c=r;
    else
    	d=r;
    sprintf (str,"%ld %f %f",ncall++,c,d);
    //SetWindowText (hWndMain,str);
    }

 /* Free the no-longer-needed global harrays. */
 free(qf); free(lht); free(f); free(fd);

/*-------------------------------------------------*/
computespline:
 //SetWindowText (hWndMain,"Compute Spline");

 /* construct the file global harray (n-2)x(n-2) 
    matrix A = Gbar+r*H*L^(-1)*H'.  Note H*L^(-1)*H' 
    is a bandwidth 5 symmetric (n-2) x (n-2) matrix.*/
 A[1] = 0.; /* A is reinitialized to zero */
 for (i=1; i<=n2; i++)
   {jl = MAX(1,i-2); jh = MIN(n2,i+2);
    for (j=jl; j<=jh; j++)
      {kl = MAX(i,j); kh =kl+2-ABS(i-j); kh= MIN(kh,n);
       for (s=0.,k=kl; k<=kh; k++)
           s += (HM(i,k)/L[k])*HM(j,k);
       AM(i,j) = GbarM(i,j)+r*s;
      }
   }

 /* compute the local harray vector v[1:(n-2)] in 
    (Gbar+r*H*L^(-1)*H')v = Hf, where f = hp col 2. 
    Do this by calling bandsolve(A,hf) to return
    the vector v such that A*v = hf.*/
v = bandsolve(A,hf);

 /* compute the local harray p[1:n,1:2], where 
    p col 1 = hp col 1, and p col 2 = f-r*L^(-1)H'v.*/
 p = allocharray(n,2);
 for (i=1; i<=n; i++)
    {p[DX(i,1,2)] = hp[DX(i,1,2)];
     kl = MAX(1,i-2); kh=MIN(i,n2);
     for (s=0.,k=kl; k<=kh; k++)
         s += HM(k,i)*v[k];
     p[DX(i,2,2)] = hp[DX(i,2,2)]-(r/L[i])*s;
    }

 /* free all unneeded allocated temporary arrays */
 free(Gbar); free(H);
 free(hf); free(A);
 free(t); free(v);

 c2spline:
 //SetWindowText (hWndMain,"c2spline");
 /* Compute the desired points (specified by vp[]) of 
    the global natural C^2 spline (or its derivative 
    or integral) for  the nx2 matrix p */

 /* Compute dest[1:m,1:2] = the desired output. */ 
// dest = cinterpolate(p,vp,1.,0,NULL,4,idsw);  //orig
 dest = cinterpolate(p,vp,1.,1,NULL,5,idsw);  //js
/*import double *cinterpolate(double *hp, double *vp, 
                            double f, int16 k,
                            double *mp, int16 tansw,
                            int16 idsw);*/
//vp[10]   dest[1] p[2]
 /* If idsw !=0, copy vp into dest col 1. (it's 
    already done for idsw=0) */
 //SetWindowText (hWndMain,"Compute Dest");
 if (idsw) for (j=1; j<=m; j++) 
                dest[DX(j,1,2)] = vp[j];

 /* Free p if needed, and free L and/or vp if we 
    created either. */
 if (n > 2) free(p);
 if (Lsw) free(L);
 if (vpsw) free(vp);
 //SetWindowText (hWndMain,"Exit Main");

 return(dest);
}

/*=================================================*/
private int16 ox(int16 i, int16 j, int16 h, int16 w)
/*--------------------------------------------------
Given a banded matrix M[1:nr,1:nc] with 2h+1
bands, we can save space by storing these bands in
an array D whose rows correspond to the diagonals
of M which may contain non-zero elements.

ox(i,j,h,w) returns the 1-origin index of the
element in the Diagonal Storage array D that
corresponds to the banded matrix M.  The values i
and j are assumed to be in the correct ranges:
1<=i<=nr and 1<=j<=nc.  This computation does not
require accessing the actual storage area, so the
address D is not provided as an input.

The matrix M is assumed to have diagonals
h,h-1,...,1,0,-1,-2,...,-h, where the main
diagonal is diagonal 0, and the numbers of lower
diagonals are negative, and the numbers of upper
diagonals are positive.  Thus M has bandwidth
2h+1.  (Actually, the list of stored diagonals may
be truncated if they will not be accessed!  i.e.
if diagonals -h and -(h-1) are not accessed, they
need not be stored.  But the diagonals that are
stored must be contiguous and must include the
main-diagonal 0 and diagonal 1.

The diagonals h,h-1,... of M[1:nr,1:nc] are stored
in the successive rows of an associated diagonal
storage array D[0:2h,1:w].  For k=0,1,...,h, the
diagonals k and -k are stored with k zeros
prefixed.  The argument w is the "width" of D
(i.e. the number of columns), which is that value
needed to completely hold the zero-padded stored
diagonals; w depends upon the values nr and nc.

For example, in the functions below, we access the
matrix element Gbar[i,j] of the matrix
Gbar[1:n-2,1:n-2] via ox(i,j,1,n-2), and we access
the matrix element H[i,j] of the matrix
H[1:n-2,1:n] via ox(i,j,2,n). Macros are given
above for all the matrices which involve the use
of ox().
---------------------------------------------------*/
{if (ABS(j-i)>h) return(1); /* 1 indexes a 0-value!*/ 
 return(w*(h+i-j)+MAX(i,j)); }

/*=================================================*/
private double *bandsolve(double *A, double *b)
/*---------------------------------------------------
Solve for and return x[1:n] in A*x=b, where A is a 
bandwidth 5 symmetric n x n matrix.
 --------------------------------------------------*/
{int16 j,i,i1,i2,n,n1,jl,jh;
 double mv,Zii;
 double *x;
 double *Z;

 Z = copyharray(A);
 x = copyharray(b);
 n = harrayrows(b); n1=n-1;

 /* For i=1,...,n: 
        divide [A&'b] row i by A[i,i].
        if i<n, subtract A[i+1,i]*([A&'b] row i) 
                from [A&'b] row i+1.
        if i<n-1, subtract A[i+2,i]*([A&'b] row i) 
                  from [A&'b] row i+2.
    Now A is upper-triangular with ones on its main 
    diagonal and 2 upper diagonals.  Back-solve to 
    compute x such that Ax=b.
 */
 for (i=1; i<=n; i++)
    {i1=i+1; i2=i+2; /*jl = MAX(1,i-2);*/ 
     jh=MIN(n,i2);
     Zii = ZM(i,i); x[i] /= Zii;
     for (j=i1; j<=jh; j++) ZM(i,j) /= Zii;

     if (i < n)
       {mv = ZM(i1,i); x[i1] -= mv*x[i];
        ZM(i1,i1) -= mv*ZM(i,i1);
        if (i<(n1)) ZM(i1,i2) -= mv*ZM(i,i2);
       }

      if (i<(n1))
        {mv = ZM(i2,i); x[i2] -= mv*x[i];
         ZM(i2,i1) -= mv*ZM(i,i1);
         ZM(i2,i2) -= mv*ZM(i,i2);
        }
    }

 for (i=n1; i>=0; i--)     //js >
    {jh = MIN(n,i+2); 
     for (j=i+1; j<=jh; j++) x[i] -= ZM(i,j)*x[j];
    }

 free(Z);
 return(x);
}

/*=================================================*/
private double dnu(double r)
/*---------------------------------------------------
dnu(r) is -.5 times the derivative of our objective 
function.
  dnu(r) = (fbar-f+r*L^(-1)*H'*
           (Gbar+r*H*L^(-1)*H')^(-1)*H*f)'*
           [L^(-1)*H'*(Gbar+r*H*L^(-1)*H')^(-1)*Gbar*
           (Gbar+r*H*L^(-1)*H')^(-1)*H*f].

We are given Gbar, and qf = H*L^(-1)*H', fd =
fbar-f, lht = L^(-1)*H', and hf = H*f.  The
harrays Gbar[1:n-2,1:n-2], qf[1:n-2,1:n-2],
fd[1:n], lht[1:n,1:n-2], and hf[1:n-2] are all
file globals. We are also given the file global
array A[1:n-2,1:n-2] to use to hold Gbar+r*qf
(computed for each call during root-finding.) The
arrays A, qf,Gbar, and lht are all stored by
diagonals and accessed via the gnomin-based index
function computed in ox().

To compute dnu(r):
 1) solve for x in (Gbar+r*qf)*x = hf.
 2) compute y = Gbar*x.
 3) solve for z in (Gbar+r*qf)*z = y.
 4) compute dnu(r) = (fd+r*lht*x)'*lht*z.
----------------------------------------------------*/
{int16 n2,kl,kh;
 int16 i,j,k,n;
 double *x;
 double *y;
 double *z;
 double nuv,s,t;

 n = harrayrows(fd); n2 = n-2;

 /* Compute A=Gbar+r*qf */
 A[1] = 0.; /* To be sure there is zero there. */
 for (i=1; i<=n2; i++)
   {kh = MIN(n2,i+2);
    for (j=i; j<=kh; j++)
      {t = GbarM(i,j)+r*qfM(i,j);
       AM(i,j) = t;
       if (i != j) AM(j,i) = t;
      }
   }

 x = bandsolve(A,hf); /* x is (n-2) x 1 */

 /* Compute y=Gbar*x */
 y = allocharray(n2,1);
 for (i=1; i<=n2; i++)
    {kl = MAX(1,i-1); kh = MIN(n2,i+1);
     for (s=0.,k=kl; k<=kh; k++)
     	s += GbarM(i,k)*x[k];
     y[i] = s;
    }

 z = bandsolve(A,y); /* z is (n-2) x 1 */

 /* compute dnu(r) = (fd+r*lht*x)'*lht*z. */
 for (nuv=0.,i=1; i<=n; i++)
    {kl = MAX(1,i-2); kh = MIN(n2,i);
     for (s=0., k=kl; k<=kh; k++)
     	s += lhtM(i,k)*x[k]; 
     	
     t = fd[i]+r*s;
     for (s=0., k=kl; k<=kh; k++)
     	s += lhtM(i,k)*z[k];
     	
     nuv += t*s;
    }

 /* clean up allocated arrays */
 free(z); free(y); free(x);

 return(nuv);
}

/*==================================================*/
/* end of file ssp.c */        

 
//Listing 3: Moving average, C version. 
 

// type definition 
typedef struct{ 
   short N; 
   float *p; 
}Vector; 
 
//  constructor 
void init_vector(short n, Vector *ptr){ 
   int i; 
   ptr->N = n; 
   ptr->p = malloc(ptr->N * 
     sizeof(float)); 
   for(i=0; i<n; i++) 
      ptr->p[i] = 0; 
} 
 
// destructor 
void free_vector(Vector *ptr){ 
   free(ptr->p); 
} 
 
// average function 
float Vector_mean(Vector *ptr){ 
   float sum = 0; 
   int i; 
//   assert(ptr->N != 0); 
   for(i=0; i<ptr->N; i++) 
      sum += ptr->p[i]; 
   return sum/ptr->N; 
} 
 
// shift vector as a shift register 
// (last element = newest) 
void Vector_shift(float input, Vector 
  *ptr){ 
   int i; 
   for(i=0; i< ptr->N-1; i++) 
      ptr->p[i] = ptr->p[i+1]; 
   ptr->p[ptr->N-1] = input; 
} 
 
// update vector & compute average 
float Vector_moving_avg(float input, 
  Vector *ptr){ 
   Vector_shift(input, ptr); 
   return Vector_mean(ptr); 
} 
 
 
   
//Listing 7: Average with variable count. 
 
/*
class Average_Buffer{ 
   double x[SIZE]; 
   int index; 
   int count; 
   ... 
}; 
 
Average_Buffer::Average_Buffer(){ 
   index = 0; 
   sum = 0; 
   count = 0; 
} 
 
void Average_Buffer::add_item(double 
input){ 
   if(count == SIZE) 
      sum -= x[index]; 
   else 
      ++count; 
   sum += input; 
   x[index++] = input; 
   index %= SIZE; 
   average = sum/count; 
} 
 
 
*/ 

import double *mmean(double *M, int32 x, int32 y, 
                     int32 z, double *f)
{   
	LPDOUBLE	pmean;
	long	nrows = harrayrows(M);
	long	ncols = harraycols(M);
    int16	i;
    
//    sprintf (str,"Enter %ld %ld",nrows,ncols); 
//    SetWindowText (hWndMain,str);
	pmean = allocharray(nrows,ncols); 
	for (i=1;i<nrows+1;i++)
		pmean[i] = M[i];
//	SetWindowText (hWndMain,"Exit");
	return pmean;
}

import double *mmedian(double *M, int32 x, int32 y, 
                       int32 z, double *f)

{   
	LPDOUBLE	pmean;
	long	nrows = harrayrows(M);
	long	ncols = harraycols(M);
    int16	i;
    
//    sprintf (str,"Enter %ld %ld",nrows,ncols); 
//    SetWindowText (hWndMain,str);
	pmean = allocharray(nrows,ncols); 
	for (i=1;i<nrows+1;i++)
		pmean[i] = M[i];
//	SetWindowText (hWndMain,"Exit");
	return pmean;
}

LPDOUBLE copyharray(LPDOUBLE A)
{
	LPDOUBLE NewA;
	long	nrows = harrayrows(A);
	long	ncols = harraycols(A);
	
	NewA = allocharray(nrows,ncols);
	hmemmove ((HPSTR)NewA,(HPSTR)A,(1+nrows*ncols)*sizeof(double));
	return NewA;
}