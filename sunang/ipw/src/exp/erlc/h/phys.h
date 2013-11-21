#ifndef	PHYS_H
#define PHYS_H

/*
 *  various physical constants, mainly for atmosphere
 */

#define MOL_AIR		28.9644		/* mol wt air (kg/mole)		*/
#define MOL_H2O		18.0153		/* mol wt water vapor		*/
#define RGAS		8.31432e3	/* gas const (J/kmole/deg)	*/
#define GRAV		9.80665		/* g (m/sec^2) at lat 45-32-33	*/
#define FREEZE		2.7315e2	/* triple pt of water (deg K)	*/
#define BOIL		3.7315e2	/* boil pt at STP		*/
#define CP_AIR		1.005e3		/* spec ht air (J/kg/deg)	*/
#define VON_KARMAN	3.5e-1
#define DALR		( GRAV/CP_AIR )	/* dry adiabatic (deg/m)	*/
#define SEA_LEVEL	1.013246e5	/* sea level pressure (Pa)	*/
#define EARTH_RAD	6.37122e3	/* earth equiv sphere rad (km)	*/

/*
 *  radiation constants
 */
#define STEFBO		5.67032e-8	/* Stefan-Boltzmann (W/m^2/deg^4) */
#define hPLANCK		6.626176e-34	/*  J sec */
#define PLANCK1st	3.741832e-16
#define PLANCK2nd	1.438786e-2
#define kBOLTZ		1.380662e-23	/*  J / deg */
#define LIGHT_SPEED	2.99722458e8 /* m/s (Marx, Nature, 296, 11, 1982) */

/*
 *  Various macros for atmospheric calculations
 */

/*
 *  integral of hydrostatic equation over layer with linear temperature
 *  variation
 *
 *	pb = base level pressure, tb = base level temp (K),
 *	L = lapse rate (deg/km), h = layer thickness (km),
 *	g = grav accel (m/s^2), m = molec wt (kg/kmole),
 *
 *	(the factors 1.e-3 and 1.e3 are for units conversion)
 */
#define HYSTAT(pb,tb,L,h,g,m)	((pb) * (((L)==0.) ?\
		exp(-(g)*(m)*(h)*1.e3/(RGAS*(tb))) :\
		pow((tb)/((tb)+(L)*(h)),(g)*(m)/(RGAS*(L)*1.e-3))))

/*
 *  inverse of integral of hydrostatic equation over layer with linear
 *  temperature variation
 *
 *	pb = base level pressure, tb = base level temp (K),
 *	hb = base level geopotential altitude (km),
 *	p = level pressure, t = level temperature (K),
 *	g = grav accel (m/s^2), m = molec wt (kg/kmole),
 *
 *	(the factor 1.e-3 is for units conversion)
 */
#define INVHE(pb,tb,hb,p,t,g,m)	((hb)+\
		1.e-3*log((p)/(pb))*(RGAS/((g)*(m)))*\
		(((tb)==(t)) ? (-(t)) :\
		((t)-(tb))/log((tb)/(t))))

/*
 *  virtual temperature, i.e. the fictitious temperature that air must have at
 *  pressure p to have the same density, as a water vapor-air mixture at
 *  pressure P, temperature t, and vapor pressure e
 *
 *	t = temperature (K),
 *	e = vapor pressure, P = pressure (e and P in same units),
 */
#define VIRT(t,e,P)	((t)/(1.-(1.-MOL_H2O/MOL_AIR)*((e)/(P))))

/*
 *  inverse of VIRT
 */
#define INVIRT(tv,e,P)	((tv)*(1.-(1.-MOL_H2O/MOL_AIR)*((e)/(P))))

/*
 *  potential temperature:
 *	t = temperature (K), p = pressure (Pa)
 */
#define POT_TEMP(t,p)	((t)*pow(1.e5/(p),RGAS/(MOL_AIR*CP_AIR)))

/*
 *  equation of state, to give density of a gas (kg/m^3) as a function of mol.
 *  wt, pressure, and temperature; or, inversely, to give pressure as a
 *  function of density, mol. wt, and temperature
 *
 *	p = pressure (Pa), m = molecular weight (kg/kmole),
 *	t = temperature (K), rho = density (kg/m^3)
 */
#define GAS_DEN(p,m,t)		((p)*(m)/(RGAS*(t)))
#define EQ_STATE(rho,m,t)	((rho)*(RGAS)*(t)/(m))

/*
 *  specific humidity, as function of e & P
 *	e = vapor pressure, P = pressure (same units)
 */
#define SPEC_HUM(e,P)	((e)*MOL_H2O/(MOL_AIR*(P)+(e)*(MOL_H2O-MOL_AIR)))

/*
 *  mixing ratio, as function of e & P
 *	e = vapor pressure, P = pressure (same units)
 */
#define MIX_RATIO(e,P)	((MOL_H2O/MOL_AIR)*(e)/((P)-(e)))

/*
 *  vapor pressure, as function of P & mixing ratio (w)
 */
#define INV_MIX(w,P)	((w)*(P)/((w)+(MOL_H2O/MOL_AIR)))

/*
 *  vapor pressure, as function of P & specific humidity (q)
 */
#define INV_SPH(q,P)	(-MOL_AIR*(P)*(q)/((MOL_H2O-MOL_AIR)*(q)-MOL_H2O))

/*
 *  latent heat of vaporization at temperature t (deg K)
 */
#define LH_VAP(t)	(2.5e6 - 2.95573e3 *((t) - FREEZE))

/*
 *  latent heat of fusion at temperature t (deg K)
 */
#define LH_FUS(t)	(3.336e5 + 1.6667e2 * (FREEZE - (t)))

/*
 *  convert wavelength (um) to wave number (1/cm)
 */
#define waveno(x)	( (int)(10000. / (x) + .5) )
#define WAVENO(x)	( (int)(10000. / (x) + .5) )

/*
 *  convert wave number (1/cm) to wavelength (um)
 */
#define wavelen(nu)	( 10000. / (nu) )
#define WAVELEN(nu)	( 10000. / (nu) )

/* end of phys.h */
#endif
