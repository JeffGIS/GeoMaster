 /*
 * look-up tables passed externally between radtbl and elevrad
 */

extern fpixel_t       *beam;		/* -> beam radiation	 */
extern fpixel_t       *diffuse;		/* -> diffuse radiation	 */

 /*
  * Various macros for atmospheric calculations
  */

 /*
  * gas constant (J / kmole / deg)
  */
#define	RGAS		8.31432e3

 /*
  * integral of hydrostatic equation over layer with linear temperature
  * variation
  * 
  * pb = base level pressure, tb = base level temp (K), L = lapse rate
  * (deg/km), h = layer thickness (km), g = grav accel (m/s^2), m =
  * molec wt (kg/kmole),
  * 
  * (the factors 1.e-3 and 1.e3 are for units conversion)
  */
#define	HYSTAT(pb,tb,L,h,g,m)		((pb) * (((L)==0.) ?\
		exp(-(g)*(m)*(h)*1.e3/(RGAS*(tb))) :\
		pow((tb)/((tb)+(L)*(h)),(g)*(m)/(RGAS*(L)*1.e-3))))

 /*
  * standard sea level pressure (Pa)
  */
#define	SEA_LEVEL	1.013246e5

 /*
  * standard lapse rate (K/km)
  */
#define LAPSE		(-6.5)

 /*
  * gravitational acceleration at reference latitude 45d 32m 33s
  */
#define	GRAV		9.80665

 /*
  * molecular weight of air (kg / kmole)
  */
#define	MOL_AIR		28.9644

 /*
 * standard sea level temp
 */
#define STD_TEMP	2.88e2

