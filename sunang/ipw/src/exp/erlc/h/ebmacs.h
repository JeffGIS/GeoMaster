
/*	SCCS version: @(#)   ebmacs.h   1.1   9/18/90	*/

/***    ebmacs.h - various macs for thermal properties  ***/
/**     Most have been written by D. Marks, CSL, UCSB   **/

#include "physmacs.h"

/*      specific heat of ice (J/(kg K)),
*       as a function of temperature
*       (from CRC table D-159; most accurate from 0 to -10 C)
*       argument:
*               t       temperature (K)
*/
#define CP_ICE(t)       ( (0.024928+(0.00176*(t))) * (J_CAL/KG_G) )

/*      specific heat of water (J/(kg K)).
*       as a function of temperature
*       from CRC table D-158; most accurate from 0 to +10 C)
*       (incorrect at temperatures above 25 C)
*       argument:
*               t       temperature (K)
*/
#define CP_WATER(t)     ( CP_W0-2.55*(t-FREEZE) )

/*      thermal conductivity of snow (J/(m sec K))
*       as a function of density;
*       after Yen, 1965,
*       (see Anderson, 1976, pg. 31)
*       argument:
*               rho     snow density (kg/m**3)
*/
#define KTS(rho)        ( (0.0077*((rho)/1000.0)*((rho)/1000.0)) * (J_CAL) )

/*      latent heat of sublimination (J/kg)
*       from the sum of latent heats of vaporization and fusion,
*       as a function of temperature;
*       argument:
*               t       temperature     (K)
*/
#define LH_SUB(t)       (LH_VAP(t)+LH_FUS(t))

/*      effectuve diffusion coef. (m**2/sec) for saturated porous layer
*       (like snow...) as a function of air pressure and layer temp.;
*       See Anderson, 1976, pg. 32, eq. 3.13;
*       arguments:
*               pa      air pressure (Pa)
*               ts      layer temperature (K)
*/
#define DIFFUS(pa,ts)   ( (0.65*(SEA_LEVEL/(pa)) * \
                        pow(((ts)/FREEZE),14.0)) * (M_CM*M_CM) )

/*      melt (kg/m**2) as a function of available energy;
*       argument:
*               Q       energy (J/m**2)
*/
#define MELT(Q)         ( (Q) / LH_FUS(FREEZE) )

/*      water vapor flux (kg/(m**2 sec)) between two layers;
*       arguments:
*               air_d   air density (kg/m**3)
*               k       diffusion coef. (m**2/sec)
*               q_dif   specific hum. diff between layers (kg/kg)
*               z_dif   absolute distance between layers (m)
*
*       note:   q_dif controls the sign of the computed flux:
*/
#define EVAP(air_d,k,q_dif,z_dif)       ( air_d * k * (q_dif/z_dif) )

/*      snow - liquid water mass fraction;
*       arguments:
*               mw      mass of liquid water (kg)
*               ms      total mass of snow (kg)
*
*       from:   Davis, et. al. (1985)
*/
#define SNOH2O_MF(mw,ms)        ( mw / ms )

/*      snow - liquid water volume fraction;
*       arguments:
*               mw      mass of liquid water (kg)
*               ms      total mass of snow (kg)
*               rhos    density of snow (kg/m**3)
*
*       from:   Davis, et. al (1985)
*/
#define SNOH2O_VF(mw,ms,rhos)   ( SNOH2O_MF(mw,ms) * (rhos/RHO_W0) )

/*      snow porosity;
*       arguments:
*               mw      mass of liquid water (kg)
*               ms      total mass of snow (kg)
*               rhos    density of snow (kg/m**3)
*
*       from:  Davis, et. al (1985)
*/
#define SNO_PORO(mw,ms,rhos)    \
        ( (RHO_ICE - (rhos * (1.0 - SNOH2O_MF(mw,ms)))) / RHO_ICE )

/*      snow water saturation;
*       arguments:
*               mw      mass of liquid water (kg)
*               ms      total mass of snow (kg)
*               rhos    density of snow (kg/m**3)
*
*       from:   Davis, et. al (1985)
*/
#define SNO_SAT(mw,ms,rhos)     \
( (RHO_ICE * rhos * SNOH2O_MF(mw,ms)) / ((RHO_ICE - rhos) * RHO_W0) )

/*      water retained by snow at given saturation (see SNO_SAT)
*       arguments:
*               ms      total mass of snow (kg)
*               rhos    density of snow (kg/m**3)
*               sat     % saturation
*/
#define H2O_LEFT(d,rhos,sat)    \
        ( (sat * d * RHO_W0 * (RHO_ICE - rhos)) / RHO_ICE )
