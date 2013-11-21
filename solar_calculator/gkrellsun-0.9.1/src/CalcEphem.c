/* Copyright (C) 2001, Norman Walsh. Derived from gkrellmoon by
 * Dale P. Smith and wmSun by Mike Henderson
 *
 * $Id: CalcEphem.c,v 1.1 2002/10/11 13:04:14 nwalsh Exp $
 *
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <X11/X.h>

#include "CalcEphem.h"
#include "Moon.h"

/* Set this time before calling any of these functions! */
/* This arrangement allows the caller to change the effective date. */
long CurrentGMTTime = 0;

double cosEPS = 0.91748;
double sinEPS = 0.39778;
double P2  = 6.283185307;

static gdouble kepler(gdouble M, gdouble e)
{
	gint n = 0;
	gdouble E, Eold, eps = 1.0e-8;

	E = M + e * sin(M);
	do {
		Eold = E;
		E = Eold + (M - Eold + e * sin(Eold))
		    / (1.0 - e * cos(Eold));
		++n;
	} while ((fabs(E - Eold) > eps) && (n < 100));
	return (E);
}

static gint DayofYear(gint year, gint month, gint day)
{
	gdouble jd();
	return ((gint) (jd(year, month, day, 0.0) - jd(year, 1, 0, 0.0)));
}


static gint DayofWeek(gint year, gint month, gint day, gchar dowstr[])
{
	gdouble JD, A, Afrac, jd();
	gint n, iA;

	JD = jd(year, month, day, 0.0);
	A = (JD + 1.5) / 7.0;
	iA = (gint) A;
	Afrac = A - (gdouble) iA;
	n = (gint) (Afrac * 7.0 + 0.5);
	switch (n) {
	case 0:
		strcpy(dowstr, "Sunday");
		break;
	case 1:
		strcpy(dowstr, "Monday");
		break;
	case 2:
		strcpy(dowstr, "Tuesday");
		break;
	case 3:
		strcpy(dowstr, "Wednesday");
		break;
	case 4:
		strcpy(dowstr, "Thursday");
		break;
	case 5:
		strcpy(dowstr, "Friday");
		break;
	case 6:
		strcpy(dowstr, "Saturday");
		break;
	}
	return (n);
}

/*
 *  Compute the Julian Day number for the given date.
 *  Julian Date is the number of days since noon of Jan 1 4713 B.C.
 */
gdouble jd(gint ny, gint nm, gint nd, gdouble UT)
{
	gdouble A, B, C, D, JD, day;

	day = nd + UT / 24.0;


	if ((nm == 1) || (nm == 2)) {
		ny = ny - 1;
		nm = nm + 12;
	}

	if (((gdouble) ny + nm / 12.0 + day / 365.25) >=
	    (1582.0 + 10.0 / 12.0 + 15.0 / 365.25)) {
		A = ((gint) (ny / 100.0));
		B = 2.0 - A + (gint) (A / 4.0);
	} else {
		B = 0.0;
	}

	if (ny < 0.0) {
		C = (gint) ((365.25 * (gdouble) ny) - 0.75);
	} else {
		C = (gint) (365.25 * (gdouble) ny);
	}

	D = (gint) (30.6001 * (gdouble) (nm + 1));


	JD = B + C + D + day + 1720994.5;
	return (JD);
}

gdouble hour24(gdouble hour)
{
	gint n;

	if (hour < 0.0) {
		n = (gint) (hour / 24.0) - 1;
		return (hour - n * 24.0);
	} else if (hour > 24.0) {
		n = (gint) (hour / 24.0);
		return (hour - n * 24.0);
	} else {
		return (hour);
	}
}

gdouble angle2pi(gdouble angle)
{
	gint n;
	gdouble a;
	a = 2.0 * M_PI;

	if (angle < 0.0) {
		n = (gint) (angle / a) - 1;
		return (angle - n * a);
	} else if (angle > a) {
		n = (gint) (angle / a);
		return (angle - n * a);
	} else {
		return (angle);
	}
}

static gdouble angle360(gdouble angle)
{
	gint n;

	if (angle < 0.0) {
		n = (gint) (angle / 360.0) - 1;
		return (angle - n * 360.0);
	} else if (angle > 360.0) {
		n = (gint) (angle / 360.0);
		return (angle - n * 360.0);
	} else {
		return (angle);
	}
}

void Interp(double ym, double y0, double yp,
	    double *xe, double *ye, double *z1, double *z2, int *nz){

    double	a, b, c, d, dx;

    *nz = 0;
    a = 0.5*(ym+yp)-y0;
    b = 0.5*(yp-ym);
    c = y0;
    *xe = -b/(2.0*a);
    *ye = (a*(*xe) + b) * (*xe) + c;
    d = b*b - 4.0*a*c;

    if (d >= 0){
	dx = 0.5*sqrt(d)/fabs(a);
	*z1 = *xe - dx;
	*z2 = *xe+dx;
	if (fabs(*z1) <= 1.0) *nz += 1;
	if (fabs(*z2) <= 1.0) *nz += 1;
	if (*z1 < -1.0) *z1 = *z2;
    }

    return;
}

void UTTohhmm(double UT, int *h, int *m) {
  if (UT < 0.0) {
    *h = -1.0;
    *m = -1.0;
  } else {
    *h = (int)UT;
    *m = (int)((UT-(double)(*h))*60.0+0.5);
  }
}

double SinH(int year, int month, int day, double UT, CTrans *c){
  double TU0, TU, TU2, TU3;
  double RA_Sun, DEC_Sun, gmst, lmst, Tau;
  double M, DL, L, SL, X, Y, Z, RHO;

  TU0 = (jd(year, month, day, 0.0) - 2451545.0)/36525.0;

  TU = (jd(year, month, day, UT+62.0/3600.0) - 2451545.0)/36525.0;
  TU2 = TU*TU;
  TU3 = TU2*TU;

  M = P2*frac(0.993133 + 99.997361*TU);
  DL = 6893.0*sin(M) + 72.0*sin(2.0*M);
  L = P2*frac(0.7859453 + M/P2 + (6191.2*TU+DL)/1296e3);
  SL = sin(L);
  X = cos(L); Y = cosEPS*SL; Z = sinEPS*SL; RHO = sqrt(1.0-Z*Z);
  DEC_Sun = atan2(Z, RHO);
  RA_Sun = (48.0/P2)*atan(Y/(X+RHO));
  if (RA_Sun < 0) RA_Sun += 24.0;

  RA_Sun = RA_Sun*15.0*RadPerDeg;

  /*
   *  Compute Greenwich Mean Sidereal Time (gmst)
   */
  UT = 24.0*frac( UT/24.0 );
  gmst = 6.697374558 + 1.0*UT + (8640184.812866+(0.093104-6.2e-6*TU)*TU)*TU/3600.0;
  lmst = 24.0*frac( (gmst-c->Glon/15.0) / 24.0 );

  Tau = 15.0*lmst*RadPerDeg - RA_Sun;
  return(c->SinGlat*sin(DEC_Sun) + c->CosGlat*cos(DEC_Sun)*cos(Tau) );
}

void SunRise(int year, int month, int day, gdouble UT, CTrans *c) {
    double ym, y0, yp, SinH0;
    double xe, ye, z1, z2;
    int    Rise, Set, nz;
    double UTRise, UTSet;
    double LTRise, LTSet;
    struct tm *LocalTime;
    double LocalHour, TimeZone;

    SinH0 = sin( -50.0/60.0 * RadPerDeg );

    /* N.B. CurrentGMTTime must already be set! */
    LocalTime = localtime(&CurrentGMTTime);

    LocalHour = LocalTime->tm_hour + LocalTime->tm_min/60.0 + LocalTime->tm_sec/3600.0;
    TimeZone = UT - LocalHour;

    UT = 1.0+TimeZone;
    UTRise = -999.0;
    UTSet = -999.0;
    Rise = Set = 0;
    ym = SinH(year, month, day, UT-1.0, c) - SinH0;

    while ( (UT <= 24.0+TimeZone) ) {

	y0 = SinH(year, month, day, UT, c) - SinH0;
	yp = SinH(year, month, day, UT+1.0, c) - SinH0;

	Interp(ym, y0, yp, &xe, &ye, &z1, &z2, &nz);

	switch(nz){

		case 0:
			break;
		case 1:
			if (ym < 0.0){
			    UTRise = UT + z1;
			    Rise = 1;
			} else {
			    UTSet = UT + z1;
			    Set = 1;
			}
			break;
		case 2:
			if (ye < 0.0){
			    UTRise = UT + z2;
			    UTSet = UT + z1;
			} else {
			    UTRise = UT + z1;
			    UTSet = UT + z2;
			}
			Rise = 1;
			Set = 1;
			break;
	}
	ym = yp;
	UT += 2.0;

    }

    if (Rise){
        LTRise = UTRise - TimeZone;
        LTRise = hour24(LTRise);
    } else {
        LTRise = -999.0;
    }

    if (Set){
        LTSet = UTSet - TimeZone;
        LTSet = hour24(LTSet);
    } else {
        LTSet = -999.0;
    }

    c->Rise = Rise;
    c->Set = Set;
    c->LTRise = LTRise;
    c->LTSet = LTSet;
}

void CalcEphem(glong date, gdouble UT, CTrans * c)
{
	gint year, month, day;
	gdouble TU, TU2, TU3, T0, gmst;
	gdouble varep, varpi;
	gdouble eccen, epsilon;
	gdouble days, M, E, nu, lambnew;
	gdouble r0, earth_sun_distance;
	gdouble RA, DEC, RA_Moon, DEC_Moon;
	gdouble TDT;
	gdouble AGE;
	gdouble LambdaMoon, BetaMoon, R;
	gdouble Ta, Tb, Tc, frac();
	gdouble SinGlat, CosGlat, SinGlon, CosGlon, Tau, lmst, x, y, z;
	gdouble SinTau, CosTau, SinDec, CosDec;

	c->UT = UT;

	year = (gint) (date / 10000);
	month = (gint) ((date - year * 10000) / 100);
	day = (gint) (date - year * 10000 - month * 100);
	c->year = year;
	c->month = month;
	c->day = day;

	c->doy = DayofYear(year, month, day);
	c->dow = DayofWeek(year, month, day, c->dowstr);

	/*  
	 *  Compute Greenwich Mean Sidereal Time (gmst)
	 *  The TU here is number of Julian centuries
	 *  since 2000 January 1.5
	 *  From the 1996 astronomical almanac
	 */
	TU = (jd(year, month, day, 0.0) - 2451545.0) / 36525.0;
	TU2 = TU * TU;
	TU3 = TU2 * TU;
	T0 =
	    (6.0 + 41.0 / 60.0 + 50.54841 / 3600.0) +
	    8640184.812866 / 3600.0 * TU + 0.093104 / 3600.0 * TU2 -
	    6.2e-6 / 3600.0 * TU3;
	T0 = hour24(T0);

	c->gmst = hour24(T0 + UT * 1.002737909);

	/* convert to radians for ease later on */
	gmst = c->gmst * 15.0 * M_PI / 180.0;

	lmst = 24.0 * frac((c->gmst - c->Glon / 15.0) / 24.0);

	/*

	 *   Construct Transformation Matrix from GEI to GSE  systems
	 *
	 * 
	 *   First compute:
	 *          mean ecliptic longitude of sun at epoch TU (varep)
	 *          elciptic longitude of perigee at epoch TU (varpi)
	 *          eccentricity of orbit at epoch TU (eccen)
	 *
	 *   The TU here is the number of Julian centuries since
	 *   1900 January 0.0 (= 2415020.0)
	 */
	TDT = UT + 59.0 / 3600.0;
	TU = (jd(year, month, day, TDT) - 2415020.0) / 36525.0;
	varep =
	    (279.6966778 + 36000.76892 * TU +
	     0.0003025 * TU * TU) * RadPerDeg;
	varpi =
	    (281.2208444 + 1.719175 * TU +
	     0.000452778 * TU * TU) * RadPerDeg;
	eccen = 0.01675104 - 0.0000418 * TU - 0.000000126 * TU * TU;
	c->eccentricity = eccen;

	/*
	 * Compute the Obliquity of the Ecliptic at epoch TU
	 * The TU in this formula is the number of Julian
	 * centuries since epoch 2000 January 1.5
	 */
	TU = (jd(year, month, day, TDT) - jd(2000, 1, 1, 12.0)) / 36525.0;
	epsilon = (23.43929167 - 0.013004166 * TU - 1.6666667e-7 * TU * TU
		   - 5.0277777778e-7 * TU * TU * TU) * RadPerDeg;
	c->epsilon = epsilon;

	/*
	 * Compute:
	 *          Number of Days since epoch 1990.0 (days)
	 *          The Mean Anomaly (M)
	 *          The True Anomaly (nu)
	 *      The Eccentric Anomaly via Keplers equation (E)
	 *          
	 *          
	 */
	days = jd(year, month, day, TDT) - jd(year, month, day, TDT);
	M = angle2pi(2.0 * M_PI / 365.242191 * days + varep - varpi);
	E = kepler(M, eccen);
	nu =
	    2.0 * atan(sqrt((1.0 + eccen) / (1.0 - eccen)) * tan(E / 2.0));
	lambnew = angle2pi(nu + varpi);
	c->lambda_sun = lambnew;

	/*
	 *  Compute distance from earth to the sun
	 */
	r0 = 1.495985e8;	/* in km */
	earth_sun_distance =
	    r0 * (1 - eccen * eccen) / (1.0 + eccen * cos(nu)) / 6371.2;
	c->earth_sun_dist = earth_sun_distance;

	/*
	 * Compute Right Ascension and Declination of the Sun
	 */
	RA =
	    angle360(atan2(sin(lambnew) * cos(epsilon), cos(lambnew)) *
		     180.0 / M_PI);
	DEC = asin(sin(epsilon) * sin(lambnew)) * 180.0 / M_PI;
	c->RA_sun = RA;
	c->DEC_sun = DEC;

	/*
	 * Compute Moon Phase and AGE Stuff. The AGE that comes out of Moon()
	 * is actually the Phase converted to days. Since AGE is actually defined
	 * to be time since last NewMoon, we need to figure out what the JD of the
	 * last new moon was. Thats done below....
	 */
	TU = (jd(year, month, day, TDT) - 2451545.0) / 36525.0;
	c->MoonPhase = Moon(TU, &LambdaMoon, &BetaMoon, &R, &AGE);
	LambdaMoon *= RadPerDeg;
	BetaMoon *= RadPerDeg;

	RA_Moon =
	    angle360(atan2
		     (sin(LambdaMoon) * cos(epsilon) -
		      tan(BetaMoon) * sin(epsilon),
		      cos(LambdaMoon)) * DegPerRad);
	DEC_Moon =
	    asin(sin(BetaMoon) * cos(epsilon) +
		 cos(BetaMoon) * sin(epsilon) * sin(LambdaMoon)) *
	    DegPerRad;
	c->RA_moon = RA_Moon;
	c->DEC_moon = DEC_Moon;

	/*
	 *  Compute Alt/Az coords
	 */
	Tau = (15.0 * lmst - RA_Moon) * RadPerDeg;
	CosGlat = cos(c->Glat * RadPerDeg);
	SinGlat = sin(c->Glat * RadPerDeg);
	CosGlon = cos(c->Glon * RadPerDeg);
	SinGlon = sin(c->Glon * RadPerDeg);
	CosTau = cos(Tau);
	SinTau = sin(Tau);
	SinDec = sin(DEC_Moon * RadPerDeg);
	CosDec = cos(DEC_Moon * RadPerDeg);
	x = CosDec * CosTau * SinGlat - SinDec * CosGlat;
	y = CosDec * SinTau;
	z = CosDec * CosTau * CosGlat + SinDec * SinGlat;
	c->A_moon = DegPerRad * atan2(y, x) + 180.0;
	c->h_moon = DegPerRad * asin(z);
	c->Visible = (c->h_moon < 0.0) ? 0 : 1;

	/*
	 * Compute accurate AGE of the Moon
	 */
	Tb = TU - AGE / 36525.0;	/* should be very close to minimum */
	Ta = Tb - 0.4 / 36525.0;
	Tc = Tb + 0.4 / 36525.0;
	c->MoonAge = (TU - NewMoon(Ta, Tb, Tc)) * 36525.0;

	/*
	 * Set Earth-Moon distance (calc above w/ func Moon)
	 */
	c->EarthMoonDistance = R;

	c->SinGlat = SinGlat;
	c->CosGlat = CosGlat;

	SunRise(year, month, day, UT, c);
}
