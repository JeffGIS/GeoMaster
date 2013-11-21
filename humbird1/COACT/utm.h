#ifndef UTM_H
#define UTM_H

/***************************************************************************/
/* RSC IDENTIFIER: TRANSVERSE MERCATOR
 *
 * ABSTRACT
 *
 *    This component provides conversions between Geodetic coordinates 
 *    (latitude and longitude) and Transverse Mercator projection coordinates
 *    (easting and northing).
 *
 * ERROR HANDLING
 *
 *    This component checks parameters for valid values.  If an invalid value
 *    is found the error code is combined with the current error code using 
 *    the bitwise or.  This combining allows multiple error codes to be
 *    returned. The possible error codes are:
 *
 *       TRANMERC_NO_ERROR           : No errors occurred in function
 *       TRANMERC_LAT_ERROR          : Latitude outside of valid range
 *                                      (-90 to 90 degrees)
 *       TRANMERC_LON_ERROR          : Longitude outside of valid range
 *                                      (-180 to 360 degrees, and within
 *                                        +/-90 of Central Meridian)
 *       TRANMERC_EASTING_ERROR      : Easting outside of valid range
 *                                      (depending on ellipsoid and
 *                                       projection parameters)
 *       TRANMERC_NORTHING_ERROR     : Northing outside of valid range
 *                                      (depending on ellipsoid and
 *                                       projection parameters)
 *       TRANMERC_ORIGIN_LAT_ERROR   : Origin latitude outside of valid range
 *                                      (-90 to 90 degrees)
 *       TRANMERC_CENT_MER_ERROR     : Central meridian outside of valid range
 *                                      (-180 to 360 degrees)
 *       TRANMERC_A_ERROR            : Semi-major axis less than or equal to zero
 *       TRANMERC_INV_F_ERROR        : Inverse flattening outside of valid range
 *								  	                  (250 to 350)
 *       TRANMERC_SCALE_FACTOR_ERROR : Scale factor outside of valid
 *                                     range (0.3 to 3.0)
 *		 TM_LON_WARNING              : Distortion will result if longitude is more
 *                                      than 9 degrees from the Central Meridian
 *
 * REUSE NOTES
 *
 *    TRANSVERSE MERCATOR is intended for reuse by any application that 
 *    performs a Transverse Mercator projection or its inverse.
 *    
 * REFERENCES
 *
 *    Further information on TRANSVERSE MERCATOR can be found in the 
 *    Reuse Manual.
 *
 *    TRANSVERSE MERCATOR originated from :  
 *                      U.S. Army Topographic Engineering Center
 *                      Geospatial Information Division
 *                      7701 Telegraph Road
 *                      Alexandria, VA  22310-3864
 *
 * LICENSES
 *
 *    None apply to this component.
 *
 * RESTRICTIONS
 *
 *    TRANSVERSE MERCATOR has no restrictions.
 *
 * ENVIRONMENT
 *
 *    TRANSVERSE MERCATOR was tested and certified in the following 
 *    environments:
 *
 *    1. Solaris 2.5 with GCC, version 2.8.1
 *    2. Windows 95 with MS Visual C++, version 6
 *
 * MODIFICATIONS
 *
 *    Date              Description
 *    ----              -----------
 *    10-02-97          Original Code
 *    03-02-97          Re-engineered Code
 *
 */


#include "FixedPoint64.h"

/***************************************************************************/
/*
 *                              DEFINES
 */

  #define TRANMERC_NO_ERROR           0x0000
  #define TRANMERC_LAT_ERROR          0x0001
  #define TRANMERC_LON_ERROR          0x0002
  #define TRANMERC_EASTING_ERROR      0x0004
  #define TRANMERC_NORTHING_ERROR     0x0008
  #define TRANMERC_ORIGIN_LAT_ERROR   0x0010
  #define TRANMERC_CENT_MER_ERROR     0x0020
  #define TRANMERC_A_ERROR            0x0040
  #define TRANMERC_INV_F_ERROR        0x0080
  #define TRANMERC_SCALE_FACTOR_ERROR 0x0100
  #define TRANMERC_LON_WARNING        0x0200

  #define UTM_NO_ERROR            0x0000
  #define UTM_LAT_ERROR           0x0001
  #define UTM_LON_ERROR           0x0002
  #define UTM_EASTING_ERROR       0x0004
  #define UTM_NORTHING_ERROR      0x0008
  #define UTM_ZONE_ERROR          0x0010
  #define UTM_HEMISPHERE_ERROR    0x0020
  #define UTM_ZONE_OVERRIDE_ERROR 0x0040
  #define UTM_A_ERROR             0x0080
  #define UTM_INV_F_ERROR         0x0100

/***************************************************************************/
/*
 *                              Macros
 *
 */

#define DEG_2_RAD( deg ) \
  ( ( (deg) * PI ) / 180.0 )

/***************************************************************************/
/*
 *                              FUNCTION PROTOTYPES
 *                                for TRANMERC.C
 */

/* ensure proper linkage to c++ programs */
  #ifdef __cplusplus
extern "C" {
  #endif


long Convert_Geodetic_To_UTM (double Latitude,
                              double Longitude,
                              long   *Zone,
                              char   *Hemisphere,
                              double *Easting,
                              double *Northing); 


  #ifdef __cplusplus
}
  #endif

#endif /* UTM_H */
