 /*
  * structure to hold quadrature time and weight, along with solar
  * cosine and azimuth
  */


struct qt {
	struct tm      *q_tp;		/* -> time structure	 */
	double          q_x;		/* abscissa		 */
	double          q_wt;		/* weight		 */
	double          q_cos;		/* cosine solar angle	 */
	double          q_azm;		/* solar azimuth	 */
};

 /*
  * functions used
  */
extern struct qt  ** EXFUN( krontime, (int nkpts, struct tm *t1, struct tm *t2,
                                       double lat, double lon));
extern void          EXFUN( main, (int argc, char **argv));
#ifdef GETARGS_H
extern void          EXFUN( sunlight_opts, (OPTION_T *d, OPTION_T *b,
                                            OPTION_T *l, OPTION_T *r,
                                            OPTION_T *z, OPTION_T *y,
                                            int *zone, bool_t *isdst,
                                            double *lat, double *lon,
                                            int *year, int *month, int *day));
#endif
extern struct tm   * EXFUN( sunrise, (double lat, double lon, int year,
                                      int month, int day));
extern struct tm   * EXFUN( sunset, (double lat, double lon, int year,
                                     int month, int day));
extern struct tm   * EXFUN( sunpos, (double lat, double lon, int year,
                                     int month, int day, int flag));
extern double        EXFUN( sunhorz, (double tsec));
extern void          EXFUN( vqk15, (double tlen, double a, double b,
                                    double *x, double *w));
extern void          EXFUN( vqk21, (double tlen, double a, double b,
                                    double *x, double *w));
extern void          EXFUN( vqkagn, (int nkpt, int ngpt, double tlen, double a,
                                     double b, double *x, double *w,
                                     double *xgk, double *wgk));
extern double        EXFUN( zerobr, (double a, double b, double t,
                                     double (*f)()));


 /*
  * constants
  */
#define	RISE	0
#define	SET	1
#define MIN_DEG	60
#define SEC_DAY	86400
#define SEC_DEG 3600
#define SEC_HR	3600
#define SEC_MIN	60
#define TOL	1.e-5
