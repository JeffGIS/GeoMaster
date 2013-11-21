
#ifndef ERLC_H
#define ERLC_H

/*
 *  These include files define types referened by the function 'solwav' below.
 */
#include "txtio.h"
#include "solar.h"

/* function declarations */

extern int	EXFUN(days_in_month, (int year, int month));
extern void	EXFUN(waterday, (int year, int month, int day,
                               int *wyear, int *wday));

/* library routines from QDIPS */

extern void	EXFUN(bdfunc, 	  (double z, double z0, double t, double t0,
				   double q, double q0, double u, double u0,
				   double *ustar, double *tstar,
				   double *qstar));
extern double	EXFUN(bevap,	  (double netad, double advec, double bowen,
				   double storage, double ts));
extern double	EXFUN(bowen,	  (double p, double ta, double ts,
				   double ea, double es)); 
extern double	EXFUN(brutsaert,  (double ta, double lambda, double ea,
				   double z, double pa));
extern int	EXFUN(budit2,	  (double zu, double zt, double z0, double t,
				   double t0, double e, double e0, double u,
				   double p, double *h, double *le));
extern int	EXFUN(budyer,	  (double z, double z0, double t, double t0,
				   double e, double e0, double u, double u0,
				   double p, double *h, double *le));
extern double	EXFUN(dew_point,  (double e));
extern double	EXFUN(efcon,	  (double k, double t, double p));
extern double	EXFUN(evap,	  (double le, double ts));
extern double	EXFUN(g_snow,	  (double rho1, double rho2, double ts1,
				   double ts2, double ds1, double ds2,
				   double pa));
extern double	EXFUN(g_soil,	  (double rho, double tsno, double tg,
				   double ds, double dg, double pa));
extern double	EXFUN(getf,	  (char *prompt, double minval, double maxval));
extern int	EXFUN(getok,	  (char *prompt));
extern double	EXFUN(heat_stor,  (double cp, double spm, double tdif));
extern int	EXFUN(hle1,	  (double press, double ta, double ts,
				   double za, double ea, double es, double zq,
				   double u, double zu, double z0,
                                   double *h, double *le, double *e));
extern double	EXFUN(kasten,	  (double z, double cz, double *ma,
				   double *mw, double *mo)); 
extern double	EXFUN(net_therm,  (double ea, double lapse, double pa,
				   double skyfac, double surfemiss, double ta,
				   double ts, double z));
extern double	EXFUN(new_tsno,	  (double spm, double t0, double ccon));
extern double	EXFUN(psychrom,   (double tdry, double twet, double press));
extern double   EXFUN(ri_no,	  (double z2, double z1, double t2,
				   double t1, double u2, double u1));
extern double	EXFUN(sati,       (double tk));
extern double	EXFUN(satw,       (double tk));
extern double	EXFUN(satm,       (double t));
extern double	EXFUN(solwav,     (int iparm, double a, double b,
				   struct atten *attenc, struct astro *astrov,
				   struct topog *topov, struct atmos *atmosv,
	   double EXFUN( (*albs), (double lambda, float cosZ, int flag) ),
	   double EXFUN( (*albd), (double lambda, int flag) ),
				   TEXT_FD_T fd));
extern double	EXFUN(ssxfr,	  (double k1, double k2, double t1, double t2,
				   double d1, double d2));
extern int	EXFUN(vmesh,	  (int na, double *a, int nb, double *b,
				   int *jstart, int *jfin, int *nj));
extern double	EXFUN(zerobr,     (double a, double b, double t,
				   double EXFUN( (*f), (double a) ) ));

#endif
