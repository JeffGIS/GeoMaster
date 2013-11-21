/*
	SCCS version: @(#)   physdefs.h   3.1   4/5/90 
*/

/*  QDIPS       physdefs.h      83/09/08  */

/*
 *  various physical constants, mainly for atmosphere
 */

/*
 *  molecular weight of air (kg / kmole)
 */
#define MOL_AIR         28.9644

/*
 *  molecular weight of water vapor (kg / kmole)
 */
#define MOL_H2O         18.0153

/*
 *  gas constant (J / kmole / deg)
 */
#define RGAS            8.31432e3

/*
 *  gravitational acceleration at reference latitude 45d 32m 33s
 */
#define GRAV            9.80665

/*
 *  triple point of water at standard pressure (deg K)
 */
#define FREEZE          2.7316e2
#define BOIL            3.7315e2

/*
 *  Stefan-Boltzmann constant (W / m^2 / deg^4)
 */
#define STEFBO          5.67032e-8

/*
 *  Planck radiation constants
 */
#define hPLANCK         6.626176e-34    /*  J sec                       */
#define PLANCK1st       3.741832e-16
#define PLANCK2nd       1.438786e-2
#define kBOLTZ          1.380662e-23    /*  J / deg                     */

/*
 *  specific heat of air at constant pressure (J / kg / deg)
 */
#define CP_AIR          1.005e3

/*
 *  Von Karman constant
 */
#define VON_KARMAN      3.5e-1

/*
 *  dry adiabatic lapse rate (deg / m)
 */
#define DALR            ( GRAV / CP_AIR )

/*
 *  standard sea level pressure (Pa)
 */
#define SEA_LEVEL       1.013246e5

/*
 *  Earth equivalent spherical radius (km)
 */
#define EARTH_RAD       6.37122e3

/*
 *  velocity of light, m/s (Marx, Nature, 296, 11, 1982)
 */
#define LIGHT_SPEED     2.99722458e8
