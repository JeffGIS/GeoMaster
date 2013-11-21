
/*	SCCS version: @(#)   ebdefs.h   1.1   9/18/90	*/

/***    ebdefs.h - various defs for thermal & energy xfer processes     ***/
/**     Most have been written by D. Marks, CSL, UCSB   **/

#include        "physdefs.h"

/*      Joules per cal  */
#define J_CAL   4.186798188

/*      cals per Joule (by def.)        */
#define CAL_J   0.238846

/*      liters per m**3 */
#define L_M3    1000.0

/*      m**3 per liter  */
#define M3_L    0.001

/*      grams per kg    */
#define G_KG    1000.0

/*      kg per gram     */
#define KG_G    0.001

/*      specific heat of water at 0C (J / (kg K))       */
#define CP_W0   ( 4217.7 )

/*      density of water at 0C (kg/m**3)        */
/*      (from CRC handbook pg F-11)     */
#define RHO_W0  999.87

/*      density of ice - no air (kg/m**3)       */
/*      (from CRC handbook pg F-1)      */
#define RHO_ICE 917.0

/*      thermal conductivity of wet sand (J/(m sec K))  */
/*      (from Oke, 1978, pg. 38;)       */
#define KT_WETSAND      2.2

/*      cm per m        */
#define CM_M    100.0

/*      m per cm        */
#define M_CM    0.01

/*      secs per minute */
#define SEC_MIN 60.0

/*      secs per hour   */
#define SEC_HR  3600.0

/*      secs per day    */
#define SEC_DAY 86400.0

/*      min per hr      */
#define MIN_HR  60.0

/*      hours per day   */
#define HR_DAY  24.0

/*      thermal emissivity of snow      */
#define SNO_EMISS       0.98

/*      file "write" status     */
#define WRITE   "w"

/*      file "read" status      */
#define READ    "r"

/*  std sea level air temp (K)  */
#define STD_AIRTMP      2.88e2

/*      std lapse rate K/km     */
#define STD_LAPSE       -6.5

/*      degrees to radians      */
#define DEG_RD  PI/1.8e2

/*      radians to degrees      */
#define RD_DEG  1.8e2/PI
