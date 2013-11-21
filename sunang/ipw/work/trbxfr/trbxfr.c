/*
**  name
**	trbxfr - calculates H & LE using Brutsaert's method
**
**  synopsis
**	trbxfr	elev= [ K ] [ mm ] [ dif ] [ <infile ] [ >outfile ]
**
**  description
**	Calculates turbulent transfer using Brutsaert's description
**	of the Businger-Dyer approach, using the Obukhov length for
**	stability determination: (Refs in hle1)
**	Air pressure is set from site elev.
**	If temp/vapor pressure are measured at a different height 
**	from wind speed, "dif" is set, and z0 is assumed to be the 
**	roughness length.  Reads input file from stdin:
**
**		z, z0, t, t0, e, e0, u, u0
**
**	or, if "dif" is set:
**
**		zu, zt, z0, t, t0, e, e0, u
**
**		z  = upper height (m)
**		z0 = lower height (m)
**		zu = wind speed height (m)
**		zt = temp/humidity height (m)
**		t  = upper temperature (C)
**		t0 = lower temperature (C)
**		e  = upper vapro press. (Pa)
**		e0 = lower vapor press. (Pa)
**		u  = upper wind speed (m/sec)
**		u0 = lower wind speed (m/sec)
**
**	(if z0 = roughness length, t0 = surface temp., e0 = surface
**	 vapor press., and u0 = 0.0)
**
**	Outputs water gain/loss (+/-mm m^-2 s^-1), if mm is set.
**	Output is to stdout.
**
**  diagnostics
**	terminates with error message;
**
**  history
**	July, 1984:  written by D. Marks, (GSFC) CSL, UCSB;
**	June, 1987:  updated to use Brutsaert's method by
**		     J. Dozier, CRSEO, UCSB;
**
**  bugs
**	currently only allows 2 measurement heights, though could
**	support three (for wind speed, air temp, and humidity);
**
*/

#define		MAIN

#include	<math.h>
#include 	"qdips.h"
#include 	"physdefs.h"
#include 	"physmacs.h"

#define		TB	288.0
#define		STDLR	-0.0065

HELP =	{
	"Calculates H & LE from Brutsaert's interpretation of",
	"Businger-Dyer model using Obukhov stability length;",
	"Expects input from stdin as:",
  	"	z, z0, t, t0, e, e0, u, u0",
  	"or, if dif is set:",
  	"	zu, zt, z0, t, t0, e, e0, u",
  	"where:	z  = upper height (m)",
  	"	z0 = lower height (m) (or roughness length)",
  	"	zu = wind speed height (m)",
  	"	zt = temp/humidity height (m)",
  	"	t  = upper air temp. (C (default) or K)",
  	"	t0 = lower air temp. (or surface temp.) (C (default) or K)",
  	"	e  = upper vapor press. (Pa)",
  	"	e0 = lower vapor press. (Pa) (or surface vapor press.)",
  	"	u  = upper wind speed (m/sec)",
  	"	u0 = lower wind speed (m/sec) (can be 0.0)",
	"Outputs water gain/loss (+/-mm m^-2 s^-1) if mm set;",
	"Output to stdout;",
	0
};

USAGE =	{
	{"elev", A_OPT, "measurement site elevation (m);"},
	{"K", A_BOOL, "flag for input temperatures in K (C default);"},
	{"mm", A_BOOL, "flag for output of water gain/loss (mm/m^2 s^-1);"},
	{"dif", A_BOOL, "flag for zu != zt;"},
	{ 0 }
};

Q_MAIN()
{

	int	dif;
	int	crt;
	int	K;
	int	mm;
	int	n;

	double	elev;
	double	z;
	double	z0;
	double	zt;
	double	zu;
	double	t;
	double	t0;
	double	e;
	double	e0;
	double	u;
	double	u0;
	double	pa;
	double	h;
	double	le;
	double	ev_air;

	int	hle1();

	Q_INIT();

/*	process optional args	*/

	switch	(q_arg("elev", A_DOUBLE, 1, &elev)) {
		case 0:
			elev = 0.0;
			break;
		case 1:
			break;
		default:
			q_usage();
	}

/*	check K flag	*/

	if ((K = q_arg("K", A_BOOL)) < 0)
		q_usage();

/*	check mm flag	*/

	if ((mm = q_arg("mm", A_BOOL)) < 0)
		q_usage();

/*	check dif flag	*/

	if ((dif = q_arg("dif", A_BOOL)) < 0)
		q_usage();

	q_chkargs();

/*	check stdin for re-direct	*/

	if (isatty(fileno(stdin))) {
		crt = YES;
		if (dif)
			fprintf(stderr,
			   "input zu, zt, z0, t, t0, e, e0, u;\n");
		else
			fprintf(stderr,
			   "input z, z0, t, t0, e, e0, u, u0;\n");
	}
	else
		crt = NO;

/*	set pa from site elev	*/

	pa = HYSTAT (SEA_LEVEL, TB, (STDLR * 1000.0), (elev / 1000.0),
			GRAV, MOL_AIR);

/*	read input data and do calculations	*/

	n = 0;

	if (dif) {
		while(scanf("%lf %lf %lf %lf %lf %lf %lf %lf",
		      &zu, &zt, &z0, &t, &t0, &e, &e0, &u) == 8) {

			n++; 

			if (!K) {
				t  += FREEZE;
				t0 += FREEZE;
			}
			else
				if ((t < 0.0) || (t0 < 0.0))
					q_error("bad Kelvin temp. input",
						NULLSTR);

			if (hle1(pa, t, t0, zt, e, e0, zt, u, zu, z0,
				&h, &le, &ev_air) != 0) {
				q_error("bad return code %d from hle1", 
					NULLSTR);
			}

			if (mm) {
				if (crt)
					printf("H=%.2f; LE=%.2f; EVAP=%e\n",
						h, le, ev_air);
				else
					printf("%.2f\t%.2f\t%e\n",
						h, le, ev_air);
			}
			else if (crt)
				printf("H=%.2f; LE=%.2f;\n",
					h, le);
			else
				printf("%.2f\t%.2f\n", h, le);
		}
	}

	else {
		while(scanf("%lf %lf %lf %lf %lf %lf %lf %lf",
		      &z, &z0, &t, &t0, &e, &e0, &u, &u0) == 8) {

			n++; 

			if (!K) {
				t  += FREEZE;
				t0 += FREEZE;
			}
			else
				if ((t < 0.0) || (t0 < 0.0))
					q_error("bad Kelvin temp. input",
						NULLSTR);

			if (hle1(pa, t, t0, z, e, e0, z, u, z, z0,
				&h, &le, &ev_air) != 0) {
				q_error("bad return code %d from hle1", 
					NULLSTR);
			}
			
			if (mm) {
				if (crt)
					printf("H=%.2f; LE=%.2f; EVAP=%e\n",
						h, le, ev_air);
				else
					printf("%.2f\t%.2f\t%e\n",
						h, le, ev_air);
			}
			else if (crt)
				printf("H=%.2f; LE=%.2f;\n",
					h, le);
			else
				printf("%.2f\t%.2f\n", h, le);
		}
	}

	if (n <= 0)
		q_error("bad or empty input file", NULL);

	exit(EX_OK);
}
