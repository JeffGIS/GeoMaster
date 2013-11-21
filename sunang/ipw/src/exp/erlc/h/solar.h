#define	SL_PRESS	1.013246e5
#define	NUMWAVE		257
#define	LAPSE		-6.5e-3
#define NATT		5

/* astronomical variables */
struct astro {
	double
		rv,		/* earth-sun radius vector	*/
		zenith,		/* solar zenith angle (radians) */
		cosz,		/* cos (zenith)			*/
		sinz,		/* sin (zenith)			*/
		azm;		/* solar azimuth (from south)	*/
};
/* atmospheric variables */
struct atmos {
	double
		airmass,	/* optical path length, air molecues	*/
		omass,		/* ozone optical path length		*/
		watmass,	/* water vapor optical path length	*/
		press;		/* air pressure, Pa			*/
};
/* attenuation variables */
struct atten {
	double
		ozone,		/* atmospheric ozone in mm */
		watvap,		/* precipitable water in mm */
		beta,		/* Angstrom turbidity coefficient, for nm */
		alpha,		/* Angstrom turbidity exponent */
		aore;		/* absorption/reflection ratio for aerosols */
};
/* grain size and depth of snow cover */
struct state {
	int	finite;	/* 0 for semi-infinite, 1 for finite */
	double
		pt_size,	/* in mu m, grain size at pt */
		reg_size,
		pt_swe,		/* snow water equiv (mm) at pt */
		reg_swe,
		ss_alb;		/* albedo of underlying surface */
};
/* topographic variables */
struct topog {
	double
		zprime,		/* Z angle on slope		*/
		coszp,		/* cos (zprime)			*/
		alt,		/* elevation (meters)		*/
		vter,		/* terrain view factor		*/
		vdiff,		/* vegetation view factor (diffuse)	*/
		vbeam;		/* vegetation view factor (beam)	*/

	float
		zdp[8],		/* Z angles on surrounding slope segments  */
		czdp[8],	/* cos (zdp)				   */
		spf[8];		/* coefficients for anisotropic refl.	   */
};
