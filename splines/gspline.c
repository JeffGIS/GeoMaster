/* FILE gspline.c       REVISION DATE: April 7, 1997 */
/*-----------------------------------------------------
 This file contains the routine:
  (export)  cinterpolate(double *hp, double *vp, 
                         double f, int16 k, double *mp, 
                         int16 tansw, int16 idsw)
            which may be used to compute an 
            interpolation curve or its integral or 
            derivative in 3-space or in the plane, 
            given a set of data points and, possibly, 
            associated tangent vectors,
  and the ancillary routines:
  (private)  allocharray(int32 nr,int32 nc)
  (private)  dist(int16 a, double *hp)
  (private)  curve(double *hp, int16 j, double p, 
                   double *dest, double f, int16 idsw)
-----------------------------------------------------*/
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
/**********SYSTEM GLOBALS ****************************/
#include "shr.h" 
#include "bigmemln.h"
#include "bigmem.h"
extern	DWORD	App;     
int sign(double x);
BOOL RemoveDupPolyPoints (LPLONG pnPnts,HPDPOINT pPoints, double Tol);
POINT BasePtToWinPt (LPDPOINT WPoint);
double GetPolyLengthD (HPDPOINT lpPoints,long nPnts);  
double *smoothspline(double *hp, double *L, 
                            double r, double *vp,
                            int16 idsw);
//#include <Clib.h> 
/*Clib.h defines the C-library fcts, e.g. sqrt, exp */
/*---------------------------------------------------*/
/********* FILE GLOBALS ******************************
 **(usable by all the functions in this file) ********/

/* macros for accessing the bounds of header-augmented 
   arrays (harrays)*/
#define harrayrows(m)  ( *((int32 *)(m)) )
#define harraycols(m)  ( *((int32 *)(m)+1) )

/* macros to access linear arrays as 2-D matrices */
#define DNC(i,j) (((i)-1)*(nc)+(j))
#define DQ(i,j)  (((i)-1)*(q)+(j))

/* macros for computation */
#define Z(x) ((x)?x:1.0)
#define S(i,j) ((hp[DNC((i)+1,(j))] \
               -hp[DNC((i),(j))])/Z(dist((i),hp)))

/* forward declarations. */
forward private void 
         curve(double *hp, int16 j, double p, 
               double *dest, double f, int16 derivsw);

forward private double dist(int16 a, double *hp);

private double  *m;
   /* matrix of tangent vectors allocated in 
      cinterpolate() if no matrix of tangent vectors 
      is supplied */

private double  *pa;
   /* estimated arc-length or user-given parameter 
      values for the curve */

private int16
   nc,      /* number of columns in hp */
   q,       /* number of dimensions of data points */
   n,       /* used in curve() to index dest[] */
   prevj;   /* used in curve() to see if the cubic 
               coefficients need to be recalculated */

void clearharray (HPDOUBLE dest)
{
	return;
}

/*===================================================*/
private HPDOUBLE allocharray(int32 nr,int32 nc)
/*-----------------------------------------------------
 allocharray() gets space for a matrix of doubles of
size nr by nc, and fills-in the 0th double in the
array with the row and column sizes and returns a
pointer to this space.
-----------------------------------------------------*/
{double *d;
 d = (double *)calloc((size_t)(nr*nc+1),sizeof(double));
 /* install the row and column sizes in the zero 
    element of the matrix */
 *((int32 *)d)=nr; *(((int32 *)d)+1)=nc;
 return(d);
}

/*===================================================*/
export double *cinterpolate(double *hp, double *vp, 
                        double f, int16 k, double *mp, 
                        int16 tansw, int16 idsw)
/*-----------------------------------------------------
  double  *hp;    hp[12]
hp is a matrix pointer.  The matrix hp[1:nh,1:nc]
contains data to be interpolated in sequential order.
The matrix hp is an harray (i.e.  the first 32 bits of
*hp contains the number of rows nh, and the second 32
bits contains the number of columns nc); The element
h[i,j] is accessed by writing h[DNC(i,j)], where
DNC(i,j) = (i-1)*nc + j. There are five different
forms the data can take:

(1) nc = 2 and k = 1: the data hp[] is a set of 
    points in 2-space
(2) nc = 3 and k = 1: the data hp[] is a set of 
    points in 3-space
(3) nc = 3 and k = 0: the data hp[] is a set of 
    points in 2-space with a 3rd column containing 
    parameter values for each point.
(4) nc = 4 and k = 0: the data hp[] is a set of 
    points in 3-space with a 4th column containing 
    parameter values for each point.
(5) nc = 2 and k = 0; the data hp[] is a set of 
    points in 2-space from the graph of a 
    2D-function with column 1 used as the
    associated parameter values.

For all cases it is assumed that the data is sorted by
parameter value, but only cases (3), (4) and (5)
contain the parameter values.  Cases (1) and (2) use
implicit estimated arc-length parameters.

  double  *vp;
vp is a matrix pointer to an harray vp[1:nv,1:1].  If
k=0, vp contains the parameter values for which a
point is desired, or if k=1, vp contains arc-length
values relative to the length of the curve for which a
point is desired. That is, let the curve have total
length 1, with the relative arclength values being in
between.

  double f; 
f is a flatness parameter used to scale all tangent
vector magnitudes. Generally f should be 1 when case
(5) above holds.

  int16  k;
k determines what vp and hp represent. k = 0 means a
range of explicit parameter values are given in hp[],
and vp[] contains such parameter values in the same
units.  k = 1 means relative polygonal(straight-line) 
arc-length from 0 to 1 over the span of the data in
hp[] is the parameterization, and vp[] contains 
relative polygonal arc-length values.

  double  *mp;
mp is a matrix pointer to an harray mp[1:nh,1:q]. mp[]
contains tangent vectors for each data point.  This
input is optional; if mp = NULL, the procedure will
estimate the tangent vectors, according to tansw.

  int16 tansw; 
If mp = NULL, tansw determines the method used to
estimate the tangent vectors at each point.
Otherwise, tansw is ignored.
-------------------------------------
If tansw=0, the tangent vectors (m[i,1),m[i,2),m[i,3))
at each point i, with 1<i<nh, are estimated using the
following formula, when point(i-1) != point(i) !=
point(i+1).

m[i,j] = .5*[(dist(i)*s(i-1,j) + dist(i-1)*s(i,j))]

for 1<i<nh, where dist(i) is the distance between
point i and the adjacent point i+1 of hp[] and s(i,j)
is the jth component of the unit vector from point i
to point i+1.  The tangent vectors at the end points,
m[1,*) and m[nh,*), are set to be the vectors 
dist(1)*s(1,*) and dist(nh-1)*s(nh-1,*), respectively. 
This is a form of the 'distance-weighted adjacent 
tangents method' of tangent-estimation.
-------------------------------------
If tansw=1, the tangent vectors (m[i,1),m[i,2),m[i,3))
at each point i, with 1<i<nh, are estimated using the
following formulas.

m[1,j] = (hp[2,j]-hp[1,j]),
m[nh,j] = (hp[nh,j]-hp[nh-1,j]), and
m[i,j] = (hp[i+1,j]-hp[i-1,j])/2  for i = 2,...,nh-1.

This is a form of the 'chordal method' for tangent-
estimation.
-------------------------------------
If tansw=2, the tangent vectors (m[i,1),m[i,2),m[i,3))
at each point i, with 1<i<nh, are estimated using the
following formula.  Let g(i)=pa(i)-pa(i-1), let
h(i)=pa(i+1)-pa(i-1), and let d(i)=pa(i+1)-pa(i) for
1<i<nh where pa(i) is the given or computed parameter
value associated with the point i.  (Usually pa(1)=0.)

m[1,j] = -g(2)hp[3,j]/(h(2)d(2))+h(2)hp[2,j]/(g(2)d(2))
          -(h(2)+g(2))hp[1,j]/(h(2)g(2)),

m[nh,j] =-g(nh-1)hp[nh,j]/(h(nh-1)d(nh-1))
          +h(nh-1)hp[nh-1,j]/(g(nh-1)d(nh-1))
          -(h(nh-1)+g(-nh-1))hp[nh-2,j]/
             (h(nh-1)g(nh-1))
          +h(nh-1)hp[nh,j]/(h(nh-1)d(nh-1))
          -h(nh-1)hp[nh-1,j]/(g(nh-1)d(nh-1))
          +h(nh-1)hp[nh-2,j]/(h(nh-1)g(nh-1)), and

m[i,j] =  -g(i)hp[i+1,j]/(h(i)d(i))
           +(h(i)-2g(i))hp[i,j]/(g(i)d(i))
           -d(i)hp[i-1,j],       for i = 2,...,nh-1.

This is the 'Bessel-tangents method' for tangent
estimation.
-------------------------------------
If tansw=3, then for i=2,...,nh-1, if hp[i,j] is
greater than both hp[i-1,j] and hp[i+1,j] or hp[i,j]
is less than both hp[i-1,j] and hp[i+1,j], then m[i,j]
is set to 0. Otherwise, hp[i-1,j], hp[i,j], and
hp[i+1,j] are a monotonic triplet, and then the
tangent vector component m[i,j] is computed as:

m[i,j] = [(hp[i,j]-hp[i-1,j])/(pa(i)-pa(i-1))]*
         [(hp[i+1,j]-hp[i,j])/(pa(i+1)-pa(i))]*
         [(pa(i+1)-pa(i-1))/(hp[i+1,j]-hp[i-1,j])]
 
where pa(i) is the given or computed parameter value
associated with the point i.

The tangent vectors at the endpoints, m[1,*) and
m[nh,*), are set to the following:

m[1,j] = [(hp[2,j]-hp[1,j])/(pa(2)-pa(1))]^2 *
                ((hp[3,j]-hp[1,j])/(pa(3)-pa(1))

m[nh,j] = [(hp[nh,j]-hp[nh-1,j])/(pa(nh)-pa(nh-1))]^2
             *(hp[nh,j]-hp[n-2,j])/(pa(nh)-pa(n-2))

This is the 'Davis-Dowden method' for tangent
estimation.  It is particularly suitable for monotonic
data.
-----------------------------------
If tansw = 4, then the C^2 global natural cubic spline
tangent vectors are computed and used.
-----------------------------------
If tansw = 5, then the C^2 global clamped cubic spline
tangent vectors are computed and used, where:

m[1,j] = [(hp[2,j]-hp[1,j])/dist(1)

m[nh,j] = [(hp[nh,j]-hp[nh-1,j])/dist(nh-1).

Here dist(i) denotes the euclidean distance between
the points hp row i and hp row (i+1).
-----------------------------------

  int16 idsw; 
idsw<0 when we want to compute the tangent vector
(i.e.  the derivative) of the spline curve. idsw>0
when we want to compute the integral of the spline
curve. idsw = 0 when we want to compute the spline
curve itself.  
-------------------------------------------------------
This procedure takes as input an harray matrix hp[]
that represents a set of points in 2-space or 3-space,
sometimes accompanied by a set of associated parameter
values and also accompanied by a set of associated
tangent vectors mp[] or by a switch tansw that
specifies a tangent estimation method.  It also takes
an array vp[] of explicitly-given parameter values (if
k=0) or of relative arc-length values (if k=1).
Relative arc-length is 0 at the first point of the
curve and 1 at the last point of the curve.  Below is
a table of the types of inputs accepted by this
routine.

(1) Points on a space curve with parameters. The
matrix hp[] has 4 columns; x-, y- and z-coordinates in
the first three columns, and parameter values for each
point in the fourth column.  In this case, k must be 0
and the array vp[] must contain parameter values for
which coordinates on the curve are desired.  The
output is a three-column matrix dest[] that contains
nv points corresponding to the parameter values in
vp[].

(2) Points on an xy-planar curve with parameters.  The
matrix hp[] has 3 columns; x- and y-coordinates in the
first two columns, and parameter values for each point
in the third column.  As in case 1, k must be 0 and
the array vp[] must contain parameter values.  The
output is a 2-column matrix dest[] that contains nv
points corresponding to the parameter values in vp[].

(3) Points on a space curve only.  The matrix hp[] has
3 columns containing x-, y- and z- coordinates.  The
points must be in the parameterized order for the
routine to work.  In this case, k must be 1 and the
array vp[] must contain relative arc-length values
(the arc-length value is 0 at the first point of the
curve and 1 at the last point).  The output is a
3-column matrix dest[] that contains nv points
corresponding to the arc-length values in vp[]

(4) Points on an xy-plane curve.  The matrix hp[] has
2 columns containing x- and y-coordinates.  As in case
(3), the points must be in correct order for the
routine to work.  k must also be 1 and the array vp[]
must contain relative arc-length values.  The output
is a 2-column matrix dest[] that contains nv points
corresponding to the arc-length values in vp[].

(5) Points on a two-dimensional function y=f(x). This
is similar to case (2), except that the parameter
values are identical to the x-values of the input
points.  As in case (2), k must be 0 and the array
vp[] must contain x-values for which y-values are
desired.  The output is a 2-column matrix dest[] that
contains nv points having the x-values from vp[] and
y-values corresponding to these x-values.

The input also includes a flatness parameter, f, that
determines how much the interpolation curve follows
the straight lines between the points.  If f = 0, the
interpolation curve is merely a set of straight line
segments connecting the points. As f increases the
curve becomes "looser".

If no points are input, all output coordinates are 0.
If the input is one point with no tangent vector, the
curve that the interpolated points will be on is the
line defined by the vector consisting of the single
input point.  If the input is one point with a tangent
vector or 2 points without tangent vectors, that is
enough to define a line to interpolate points on.  All
other cases, including the case of two points and no
tangent vectors, are handled by the general algorithm.

The result is a pointer to an allocated harray
dest[1:nv,1:q].  dest[] is a matrix in which
coordinates of points matching parameter values or
arc-length values given in vp[1:nv] are returned.
These will be spline function values if idsw=0,
derivative values if idsw<0, and integral values if
idsw>0.  The global value q=nc if we have a function,
and q=nc-1+k if we have a curve.  Thus q is the number
of physical dimensions of data points.
----------------------------------------------------*/
{double temp,ti,ti1,ti2,
        vpval,
        *dest,   /* the result harray pointer */
        d1,d2,   /* used to calculate m */
        a;       /* the estimated arc-length of the 
                    entire curve */

 double *ra,    /* temporary working arrays for   */    
        *ta,    /* back-substitution, parameter-  */
        *da;    /* limits and diagonal values     */

 int16  nh,      /* number of rows in hp */
        nv,      /* number of values in vp */
        funcsw,  /* 1 if we have 2-column functional  
                    data, else 0 */
        v,       /* indexing offset */
        i,j;     /* loop control variables */

 nh=harrayrows(hp); /* nh = number of rows in hp    */
 nc=harraycols(hp); /* nc = number of columns in hp */
 nv=harrayrows(vp); /* nv = number of rows in vp    */

 /*funcsw=1 means: hp col 1=the explicit parm. vals. 
   in place of hp col 3 */
 funcsw=(k EQ 0 AND nc EQ 2);
 q=(funcsw)?nc:(nc-1+k); /* q = physical dimension  
                            of the data points */
  
 /* allocate the result harray, dest. */
 dest=allocharray(nv,q);
 /* If no points are given as input, set all 
    coordinates to 0 */
 if (nh EQ 0) {clearharray(dest); return(dest);}

 /* If the input is one point without a tangent
    vector, use the point itself as the tangent vector
    and interpolate on the line thus determined.  If
    the input is one point with a tangent vector,
    interpolate on the defined line */
 if (nh EQ 1)
    {if (funcsw) a=hp[1]; else a=(k EQ 0)?hp[nc]:0.0;
     for (i=1; i<=nv; i++) for (j=1; j<=q; j++)
        {if (mp EQ NULL) temp=hp[j]; else temp=mp[j];
         if (idsw <0) d1=temp;
         else {d1=hp[j]+(vp[i]-a)*f*temp;
               if(idsw>0) d1=(vp[i]-a)*(d1+hp[j])/2.;}
         dest[DQ(i,j)]=d1;
        }
     return(dest);
    }

/* Allocate and load the parameter vector pa[1:nh] */
 pa=allocharray(nh,1);
/* If k=1, use estimated arc-length as the 
   parameter-value */
 if (k EQ 1) 
    {/* calculate polygonal 'arc-length' values */
     pa[1]=0.0;
     for (a=0.0,i=2; i<=nh; i++) 
         pa[i]=(a+=dist((int16)(i-1),hp));
     for (a=Z(a),i=2; i<=nh; i++) pa[i]/=a;
    }
 else 
/* The explicit parm. vals. are in hp col j in 
   increasing order. */
    {j=(funcsw)?1:(q+1); 
     for (i=1; i<=nh; i++) pa[i]=hp[DNC(i,j)];}

/* If mp is not given, compute the estimated tangent 
   vectors; otherwise take m[] as the given input 
   matrix mp[]. */
 if (mp != NULL) {m=mp; goto fixslopes;}

 m=allocharray(nh,q); /* allocate m[1:nh,1:q] */

 switch (tansw) /* compute tangent-vector estimates */
 {case 4:/* tansw=4: natural C^2 spline tangents */
  /* Compute m[1:nh,1:q] = global C^2 tangent vectors
     for a natural global cubic spline, using the nh
     data points in hp[1:nh,1:q] and the associated
     parameter values in pa[1:nh] to determine the
     component slopes m[1:nh,1:q]. Here 'natural' 
     means we specify the second-derivative vectors  
     at the endpoints, hp[1] and hp[n], to be 0. 
     (hp[j,k] is accessed as hp[DNC(j,k)].)  */

  ra = allocharray(nh,1); /* allocate space for the 
                             working space array ra.*/
  da = allocharray(nh,1); /* allocate space for the 
                             working space array da.*/
  ta = allocharray(nh,1); /* allocate space for the 
                             working space array ta.*/

  da[1]=2; ti=1; ta[nh]=1;

  for (i=1; i<nh; i++)   
    {ra[i]=ti/da[i];  
     ti=pa[i+1]-pa[i]; ta[i]=ti;        
     if ((i+1) EQ nh) ti1=1;
     else ti1=pa[i+2]-pa[i+1];
     da[i+1]=2*(ti1+ti)-ti1*ra[i];
    } 

  for (j=1; j<=q;j++) 
    {m[DQ(1,j)]=3*(hp[DNC(2,j)]-hp[DNC(1,j)])/ta[1];

     for (i=1; i<nh; i++)   
       {/* elimination-loop:
           convert to upper triangular form */
        m[DQ(i,j)]/=da[i];
        if ((i+1) EQ nh) {v=1; ti1=1;} 
        else {v=2; ti1=ta[i+1];}
        temp=ti1/ta[i];
        m[DQ(i,j)]=3*(hp[DNC(i+v,j)]/temp
                     +(temp-1/temp)*hp[DNC(i+1,j)]
                     -temp*hp[DNC(i,j)])
                     -ti1*m[DNC(i,j)];
       }

     m[DQ(nh,j)]/=2-ra[nh-1];

     for (i=nh-1; i>=1; i--) /* back-substitution-loop*/
     m[DQ(i,j)] -= m[DQ(i+1,j)]*ra[i];
    }
 
  free(ra); free(da); free(ta);
  break;
  

  case 5:/* tansw=5: clamped C^2 spline tangents */
  /* Compute m[1:nh,1:q] = global C^2 tangent vectors
     for a clamped global cubic spline, using the nh
     data points in hp[1:nh,1:q] and the associated
     parameter values in pa[1:nh] to determine the
     component slopes m[1:nh,1:q]. Here 'clamped' 
     means we specify the tangent vectors at the
     endpoints, hp[1] and hp[n], explicitly as the 
     unit vectors in the directions (hp row 2)-
     (hp row 1) and (hp row nh)-(hp row (nh-1)).  
     (mp is ignored.)  Thus there are n-2 unknown
      vectors to find. (hp[j,k] is accessed as
     hp[DNC(j,k)].)  */

  ra = allocharray(nh,1); /* allocate space for the 
                             working space array ra.*/
  for (j=1; j<=q;j++) 
      {m[DQ(1,j)]=S(1,j); m[DQ(nh,j)]=S((int16)(nh-1),j);}

  ra[1]=1.0;
  ti1=pa[2]; ti2=0;
  for (i=2; i<nh; i++)   
    {/* elimination-loop:
        convert to upper triangular form */
     ti=pa[i+1]-pa[i]; temp=ti/ti1;
     for (j=1; j<=q; j++)
        m[DQ(i,j)]=3*(hp[DNC(i+1,j)]/temp
                     +(temp-1/temp)*hp[DNC(i,j)]
                     -temp*hp[DNC(i-1,j)])
                     -(ti/ra[i-1])*m[DNC(i-1,j)];

     ra[i]=2*(ti+ti1)-(ti/ra[i-1])*ti2;
     ti2=ti1; ti1=ti;
    }

  for (i=nh-1; i>1; i--)  /* back-substitution-loop */
     {ti1=pa[i]-pa[i-1];
      for (j=1; j<=q; j++) 
        m[DQ(i,j)]=(m[DQ(i,j)]-m[DQ(i+1,j)]*ti1)/ra[i];
     }

  free(ra);
  break;

  /*tansw=0: compute distance-weighted ave. tangents */
  case 0:  
  for (i=2; i<nh; i++) /*compute tan. vectors 2:nh-1 */
      {/* note: if point[i-1] != point[i] = point[i+1] 
          then tangent[i] is in the direction S[i-1]. 
          If point[i-1] = point[i] != point[i+1], then 
          tangent[i] is in the direction S[i].  
          If point[i-1] = point[i] = point[i+1] then 
          tangent[i]=S[i](= 0). */
       d1=dist((int16)(i-1),hp); d2=dist(i,hp);
       for (j=1; j<=q; j++) 
           m[DQ(i,j)] =.5*(d2*S((int16)(i-1),j)+d1*S(i,j));
      }

  /* Go assign the first and last tangent vectors 
     m[1] and m[nh]. */
  goto setends;

  case 1: /* tansw=1: compute chordal tangents */
  for (i=2; i<nh; i++)
  /* compute m[1:nh,1:q]=chordal tangent vectors */
  for (j=1; j<=q; j++) 
      m[DQ(i,j)]=.5*(hp[DNC(i+1,j)]-hp[DNC(i-1,j)]);

 setends: 
  /* Assign the first and last tangent vectors 
     m[1] and m[nh] */
  for (j=1; j<=q; j++)
      {/* compute m[1,j] */  
       m[j]=(hp[DNC(2,j)]-hp[j]); 
       /* compute m[nh,j] */
       m[DQ(nh,j)]=(hp[DNC(nh,j)]-hp[DNC(nh-1,j)]); 
      }
  break;

  case 2: /* tansw=2: compute Bessel tangents */
  for (i=2; i<nh; i++)
      {/* compute m[1:nh,1:q]=Bessel tangent vectors */
       ti=pa[i]-pa[i-1]; 
       ti1=pa[i+1]-pa[i]; 
       ti2=pa[i+1]-pa[i-1];
       for (j=1; j<=q; j++)
          m[DQ(i,j)]=(ti2-2*ti)*hp[DNC(i,j)]/
                      (ti*ti1)-ti1*hp[DNC(i-1,j)]
                      -ti*hp[DNC(i+1,j)]/(ti1*ti2);
      }

  /* Assign the first and last tangent vectors 
     m[1] and m[nh]. */
  ti=pa[2]-pa[1]; ti1=pa[3]-pa[2]; ti2=pa[3]-pa[1];
  for (j=1; j<=q; j++)
      m[j] = -ti*hp[DNC(3,j)]/(ti2*ti1)
              +ti2*hp[DNC(2,j)]/(ti*ti1)
              -(ti2+ti)*hp[DNC(1,j)]/(ti2*ti);

  ti=pa[nh-1]-pa[nh-2]; 
  ti1=pa[nh]-pa[nh-1]; 
  ti2=pa[nh]-pa[nh-2];
  for (j=1; j<=q; j++)
      m[nh,j] = -ti*hp[DNC(nh,j)]/(ti2*ti1)
                 +ti2*hp[DNC(nh-1,j)]/(ti*ti1)
                 -(ti2+ti)*hp[DNC(nh-2,j)]/(ti2*ti)
                 +ti2*(hp[DNC(nh,j)]/(ti1*ti2)
                 -hp[DNC(nh-1,j)]/(ti*ti1)
                 +hp[DNC(nh-2,j)]/(ti*ti2));
  break;

  case 3: /* tansw=3: compute Davis-Dowden tangents */
  /* set tangents at relative maxima and relative 
     minima to 0 */
  for (i=2; i<nh; i++) for (j=1; j<=q; j++)
      {d1=hp[DNC(i,j)]-hp[DNC(i-1,j)];
       d2=hp[DNC(i+1,j)]-hp[DNC(i,j)];
       if (sign(d1) != sign(d2)) m[DQ(i,j)]=0.0;
       else m[DQ(i,j)]=(d1/(pa[i]-pa[i-1]))*
                       (d2/(pa[i+1]-pa[i]))*
                       ((pa[i+1]-pa[i-1])/(d1+d2));
      }

  /* Compute the tangent vectors at the endpoints 
     m[1,*] and m[nh,*]. */
  for (j=1; j<=q; j++)
      {temp=(hp[DNC(2,j)]-hp[DNC(1,j)])/(pa[2]-pa[1]);
       m[DQ(1,j)]=temp*temp*
                  ((hp[DNC(3,j)]-hp[DNC(1,j)])/
                  (pa[3]-pa[1]));
       temp=(hp[DNC(nh,j)]-hp[DNC(nh-1,j)])/
            (pa[nh]-pa[nh-1]);
       m[nh,j]=temp*temp*
               (hp[DNC(nh,j)]-hp[DNC(nh-2,j)])/
               (pa[nh]-pa[nh-2]);
      }
 } /* end switch(tansw) */


fixslopes:
 /* If we are interpolating a function, scale each
    tangent vector m[i] to be of the form 
    (1,slopeval), except if any m[i,1] = 0, we will 
    produce (1,0) as a result.*/
 if (funcsw)
    for (i=1; i<=nh; i++)
       {if (m[DQ(i,1)] != 0.0) m[DQ(i,2)]/=m[DQ(i,1)];
        else m[DQ(i,2)]=0.0;
        /* Note we divide out the flatness factor so 
           that this will be restored to 1 when we 
           multiply by f in curve(). */
        m[DQ(i,1)]=1.0/f;
       }

 /* Now compute the output array dest[1:nv,1:q]. */

 n=1; /* start by setting n to row 1 of dest[] */
 
/* Setting prevj=-1 will cause us to calculate cubic 
   coefficients in curve() the first time through */
 prevj=-1;

/* vp[] contains a list of parameter values (k=0), or
   a list of relative arc-length values(k=1). Take
   vp[] and use curve() to find the coordinates
   associated with each vp[]-value and place them in
   the output array dest[].  That is, for each
   parameter in vp[], find which two points it is
   between, or if it is before the 1st or beyond the
   last point, and then call curve(). */
 for (j=i=1; i<=nv; i++) 
    {/* loop for each parameter-value in vp[] */
     vpval=vp[i];
     while ((j>1) AND (vpval<pa[j])) j--;
     while ((j<nh-1) AND (vpval>=pa[j+1])) j++;
     /* Now pa[j] <= vpval < pa[j+1], or
        j=1 and vpval < pa[1], or j=nh-1 and
        vpval>=pa[nh-1]. */
     curve(hp,j,vpval-pa[j],dest,f,idsw); 
    }

/* If we have functional data, place the parameter 
   values vp[1:nv] in dest col 1 */
 if (funcsw EQ 1) 
    for (j=1; j<=nv; j++) dest[2*j-1]=vp[j];
// dest[60*2+1]
 free(pa); if (mp EQ NULL) free(m);
 return(dest);
}

/*===================================================*/
private double dist(int16 a, double *hp)
/*-----------------------------------------------------
 dist(a,hp) returns the distance between points 
 hp row a and hp row (a+1).
-----------------------------------------------------*/
{int16 i;
 double t,temp = 0.0;

 for (i=1; i<=q; i++) 
     {t=hp[DNC(a,i)]-hp[DNC(a+1,i)]; temp+=t*t;}
 return(sqrt(temp));
}

/*===================================================*/
private void curve(double *hp, int16 j, double p, 
                   double *dest, double f, int16 idsw)
/*----------------------------------------------------
   double *hp; matrix of points on curve.
   int16 j;    index of the data point defining the 
               curve segment to be used for p.
   double *dest;    matrix to place coordinates 
                    of parameter values and corres-
                    ponding interpolated values in.
   double p;   parameter value at which to 
               interpolate coordinates for.
   double f;   flatness parameter that modifies 
               all tangent vector magnitudes.
   int16 idsw; idsw<0 when we want to compute the 
               tangent vector (i.e. the derivative) 
               of the spline curve. idsw>0 when we 
               want to compute the integral of the 
               spline curve. idsw = 0 when we want 
               to compute the spline curve itself.
------------------------------------------------------
This procedure takes a scalar parameter value p
and returns in dest[] the coordinates of the
corresponding point on the spline curve (if
idsw=0) or on the derivative (if idsw<0) or on the
integral (if idsw>0).  We use cubic polynomial
space splines. Given points hp[1,*],...,hp[nh,*]
and corresponding tangent vectors
m[1,*],...,m[nh,*], the curve between the two
points i and i+1 is interpolated with the Hermite
cubic polynomial space curve with vector coefficients
ai, bi, ci, and di, parametrically defined by

	xi(p) = ai + bi*p + ci*p*p + di*p*p*p
where     ai = hp[i,*)
          bi = m[i,*)
          ci = 3 * (hp[i+1,*]-hp[i,*])/
               (t*t) - (2*m[i,*]+m[i-1,*])/t
	  di = 2 * (hp[i,*]-hp[i+1,*])/(t*t*t) 
                + (m[i+1)+m[i,*])/(t*t),
    and t is (parameter value of point (i+1))  
              - (parameter value of point t).

 The flatness parameter, f, is also used herein.  
 Each tangent vector m[i,*] is multiplied by f.
-----------------------------------------------------*/
{int16  i,k;
 double t,sv,r;
 static double ival[4];  /* previous integral values */
 static double a[4],b[4],c[4],d[4];  /* coefficients 
                                 of cubic polynomial */

 /* Note for 1<j<nh-1, the modified parameter value 
    p+pa[j] lies in [pa[j],pa[j+1]], and if j=1,
    p+pa[j]<pa[2], and if j=nh-1, p+pa[j]>=pa[nh-1].  
    
    When we are to compute an integral value, we need 
    to accumulate the integral values for every spline 
    segment 1,2,...,j-1 correponding to the intervals
    [pa[1],pa[2]],...,[pa[j-1],pa[j]]. If prevj != -1,
    the values ival[1:3] holds the component integral
    values over the interval pa[1] to pa[prevj].
 */
 k = j; /* p+pa[j] ``belongs'' to the cubic for
           [pa[j],pa[j+1]]. if idsw<=0,
           we keep k=j.  otherwise, we will reset k 
           appropriately so that we add the integral 
           from pa[k] to pa[j] to ival[1:3]. */
 if (idsw > 0)
    /* If this is the first entry, or if we are given 
       a "back" parameter value, then we will recompute
       the prior integral starting from pa[1].*/
    {if (j < prevj) prevj=-1;
     if (prevj EQ -1) 
        {ival[1]=ival[2]=ival[3]=0.; k=1;} 
     else k=prevj;
    }

 if (prevj EQ j) goto calc; /*don't compute 
                          coefficients unless needed */
 prevj = j;
 /* compute currently-needed spline coefficients, 
    and if k<j, compute any additional integral parts 
    needed first.*/
 for (; k<=j; k++)
     {/* set t to the parameter or relative 
         arc-length difference value
         covering the curve part to be used.  
         The parameter value p to be
         interpolated at is in [0,t]. */
      t = pa[k+1]-pa[k]; r=t;
      if (t EQ 0.0) t = 1.0; /* avoid zero divide */
      for (i=1; i<=q; i++) /*calculate coefficients */
          {a[i] = hp[DNC(k,i)];
           b[i] = f*m[DQ(k,i)];
           c[i] = (3.*(hp[DNC(k+1,i)]-a[i])/t 
                  -(2.*b[i]+f*m[DQ(k+1,i)]))/t;
           d[i] = (2.*(a[i]-hp[DNC(k+1,i)])/t 
                  + b[i]+f*m[DQ(k+1,i)])/(t*t);

           if (k < j) /* If k<j, then idsw > 0! */
              ival[i] += (a[i]+(b[i]/2 
                         +(c[i]/3+(d[i]/4)*r)*r)*r)*r;
          }
     }

 calc:/* calculate coordinates of spline pt. at p */
 for (i=1; i<=q; i++)  
    {if (idsw > 0)
        sv = ival[i] +(a[i] +(b[i]/2 
             +(c[i]/3 +(d[i]/4)*p)*p)*p)*p;
     else if (idsw<0) sv = (b[i]+(2*c[i] +3*d[i]*p)*p);
     else sv = (a[i]+(b[i]+(c[i]+d[i]*p)*p)*p);
     dest[DQ(n,i)] = sv;
    }

 /* For functions, the x-parametric part is a straight
    line, with all its estimated slope entries (in
    dest col 1) equal to 1/f; often we want to replace
    these 1/f-values with the given x-values found in
    vp when we are computing the derivative of a
    spline model for functional data. [This
    replacement is done in cinterpolate()!]

    When f EQ 1 then dest[n,1] = hp[j,1]+p.  Note f 
    should be 1 for the x-curve when a function is 
    being interpolated, and placing v[1:nv] into
    dest col 1 and scaling mp col 2 by 1/f prior
    to entering this routine enforces this. */

 n++;    /* advance to next output row of dest[] */
}

/*==================================================*/
/* end of file gspline.c */

long SplinePointsD (long Startp,long np, HPDPOINT PointsIn, LPLONG nPnts, HPDPOINT Points,double CurveExpansionFactor,double Thin,double EndDist,long MaxPoints) 
{   
	long	npout; 
	short	tansw=GetGlobalLVal2("[%TANSW]",5);
	short	idsw=0;  
	double	f=GetGlobalDVal2 ("[%SPLINEF]",1);
	
	HANDLE	hHP=GSSiGlobAlloc(GAIDNO 1408,GMEM_MOVEABLE,8+(np-Startp+1)*sizeof(DPOINT));
	HANDLE	hVP;
//	HANDLE	hOutPoint=GSSiGlobAlloc(GAIDNO GMEM_MOVEABLE,npout*sizeof(DPOINT));
	HPDOUBLE	HP=(HPDOUBLE)GlobalLock (hHP);
	HPDOUBLE	VP; 
	HPDOUBLE	pOutArray;
	HPDPOINT	PointsD;
	LPLONG		pNumRow=(LPLONG)HP;
	LPLONG		pNumCol=pNumRow+1;
	LPLONG		nVP; 
	LPLONG		nOutP;
	HPDPOINT	pMatrixPoint=(HPDPOINT)&HP[1];
	long		i,n=0,nump;     
	double		TotDist,PCTInc, PCT, EndPCT;
	
	if (App==1)
		return 0;
    *pNumCol = 2;
    *pNumRow = np-Startp+1;
    if (*pNumRow < 3)
    { 
    	(*nPnts) += *pNumRow; 
    	for (n=0;n<*pNumRow;n++)
			*Points++ = PointsIn[n];
        npout = *pNumRow;
	    GSSiGlobUlFree (&hHP);
        return npout;
    }
    nump = np-Startp+1;
    RemoveDupPolyPoints (&nump,&PointsIn[Startp-1], 0);    
//    TotDist = GetPolyLengthD (&PointsIn[Startp-1],np-Startp+1); 
    TotDist = GetPolyLengthD (&PointsIn[Startp-1],nump); 
    EndPCT = EndDist / TotDist;
    npout = TotDist / CurveExpansionFactor; 
    npout = min (npout,(double)USHRT_MAX/(2*sizeof(DPOINT))-1);
    npout = min (npout,MaxPoints);
    hVP = GSSiGlobAlloc(GAIDNO 1409,GMEM_MOVEABLE,(1+npout)*sizeof(double)); 
    VP=(HPDOUBLE)GlobalLock (hVP); 
    nVP=(LPLONG)VP;
//    for (i=Startp-1;i<np;i++)
    for (i=Startp-1;i<Startp+nump-1;i++)
    	pMatrixPoint[n++] = PointsIn[i];
    *nVP = npout;
    PCTInc = (1.0 - EndPCT) / npout;
    PCT = PCTInc/2; 
    for (i=0;i<npout;i++,PCT += PCTInc)
    	VP[i+1]=PCT;
    
	pOutArray = (HPDOUBLE)cinterpolate(HP,VP,f,1,NULL,tansw,idsw);
//	pOutArray = smoothspline(HP,NULL,f,VP,idsw);
   	nOutP = (LPLONG)pOutArray;
	npout = *nOutP;
	PointsD = (HPDPOINT)&pOutArray[1];
	ThinPoly (&npout,PointsD,Thin);
	for (i=0;i<npout;i++)
		*Points++ = *PointsD++;	 
	(*nPnts) += npout;
	free (pOutArray);    
	GSSiGlobUlFree (&hHP);
	GSSiGlobUlFree (&hVP);
	return npout;
}

long SplinePoints (long Startp,long np, HANDLE hPoints, LPLONG nPnts, HPPOINT Points,double CurveExpansionFactor,long MaxPoints) 
{   
	HANDLE		hDPoints=GSSiGlobAlloc(GAIDNO 1410,GMEM_MOVEABLE,MaxPoints*sizeof(DPOINT));
	HPDPOINT	pDPoints=(HPDPOINT)GlobalLock(hDPoints); 
	HPDPOINT	pOrigPoints = (HPDPOINT)GlobalLock (hPoints);
	long		i;
	long npnew = SplinePointsD (Startp,np,pOrigPoints,nPnts, pDPoints,CurveExpansionFactor,0,0,MaxPoints); 
	GlobalUnlock (hPoints);
	for (i=0;i<npnew;i++)
		*Points++ = BasePtToWinPt (pDPoints++);	 
	GSSiGlobUlFree (&hDPoints);
	return npnew;
}
