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
 *                                       than 9 degrees from the Central Meridian
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


/***************************************************************************/
/*
 *                               INCLUDES
 */

#include <math.h>
#include "utm.h"
#include "ClAssert.h"

/*
 *    math.h      - Standard C math library
 *    utm.h  - Is for prototype error checking
 */


/***************************************************************************/
/*                               DEFINES 
 *
 */

#define PI_OVER_2         (PI64/2)            /* PI over 2 */
//#define MAX_LAT         ((PI * 89.99)/180.0)    /* 89.99 degrees in radians */
#define MAX_DELTA_LONG  ( (PI64 * 90)/180 ) /* 90 degrees in radians */
#define MIN_SCALE_FACTOR  dtofx64(0.3)
#define MAX_SCALE_FACTOR  3

#define MIN_LAT      ( dtofx64( -80.5 ) ) /* -80.5 degrees in radians    */
#define MAX_LAT      ( dtofx64( 84.5 )  )  /* 84.5 degrees in radians     */
#define MIN_EASTING   100000
#define MAX_EASTING   900000
#define MIN_NORTHING  0
#define MAX_NORTHING  10000000


/**************************************************************************/
/*                               GLOBAL DECLARATIONS
 *
 */

static long UTM_a = ((long)6378137.0);         /* Semi-major axis of ellipsoid in meters  */
static fixed64 UTM_f = dtofx64(1 / 298.257223563); /* Flattening of ellipsoid                 */
static fixed64 UTM_Override = 0;          /* Zone override flag                      */

/* Ellipsoid Parameters, default to WGS 84  */
static long TranMerc_a = 6378137;     /* Semi-major axis of ellipsoid in meters */
static fixed64 TranMerc_f = dtofx64(1 / 298.257223563); /* Flattening of ellipsoid  */
static fixed64 TranMerc_es = dtofx64(0.0066943799901413800); /* Eccentricity (0.08181919084262188000) squared */
static fixed64 TranMerc_ebs = dtofx64(0.0067394967565869);   /* Second Eccentricity squared */

/* Transverse_Mercator projection Parameters */
static long TranMerc_Origin_Lat = 0;           /* Latitude of origin in radians */
static fixed64 TranMerc_Origin_Long = 0;          /* Longitude of origin in radians */
static long TranMerc_False_Northing = 0;       /* False northing in meters */
static long TranMerc_False_Easting = 500000L;        /* False easting in meters */
static fixed64 TranMerc_Scale_Factor =  dtofx64(0.9996);         /* Scale factor  */

/* Isometeric to geodetic latitude parameters, default to WGS 84 */
static long TranMerc_ap = (long)6367449.1458008;
static long TranMerc_bp = (long)16038.508696861;
static long TranMerc_cp = (long)16.832613334334;
static fixed64 TranMerc_dp = dtofx64(0.021984404273757);
static fixed64 TranMerc_ep = dtofx64(3.1148371319283e-005);

/* Maximum variance for easting and northing values for WGS 84. */
static long TranMerc_Delta_Easting = 40000000L;
static long TranMerc_Delta_Northing = 40000000L;

/* These state variables are for optimization purposes. The only function
 * that should modify them is Set_Tranverse_Mercator_Parameters.         */

static long sphtmd( fixed64 Latitude );

static long Set_Transverse_Mercator_Parameters(fixed64 Central_Meridian,
                                        long False_Northing);

static long Convert_Geodetic_To_Transverse_Mercator (fixed64 Latitude,
                                              fixed64 Longitude,
                                              fixed64 *Easting,
                                              fixed64 *Northing);

static long Convert_Geodetic_To_Transverse_Mercator_const (fixed64 Longitude,
                                              long *Easting,
                                              long *Northing);

/************************************************************************/
/*                              FUNCTIONS     
 *
 */

long Convert_Geodetic_To_UTM (double Lat_Degrees,
                              double Long_Degrees,
                              long   *Zone,
                              char   *Hemisphere,
                              double *Easting,
                              double *Northing)
{ 
/*
 * The function Convert_Geodetic_To_UTM converts geodetic (latitude and
 * longitude) coordinates to UTM projection (zone, hemisphere, easting and
 * northing) coordinates according to the current ellipsoid and UTM zone
 * override parameters.  If any errors occur, the error code(s) are returned
 * by the function, otherwise UTM_NO_ERROR is returned.
 *
 *    Latitude          : Latitude in radians                 (input)
 *    Longitude         : Longitude in radians                (input)
 *    Zone              : UTM zone                            (output)
 *    Hemisphere        : North or South hemisphere           (output)
 *    Easting           : Easting (X) in meters               (output)
 *    Northing          : Northing (Y) in meters              (output)
 */

  long temp_zone;
  long Error_Code = UTM_NO_ERROR;
  fixed64 Origin_Latitude = 0;
  fixed64 Central_Meridian = 0;
  long False_Easting = 500000;
  long False_Northing = 0;
  fixed64 Scale = dtofx64(0.9996);
  fixed64 Latitude64_Radians = 0;
  fixed64 Longitude64_Radians = 0;
  fixed64 Easting64;
  fixed64 Northing64;

  fixed64 Latitude64 = dtofx64( Lat_Degrees );
  fixed64 Longitude64 = dtofx64( Long_Degrees );

  if ((Latitude64 < MIN_LAT) || (Latitude64 > MAX_LAT))
  { /* Latitude out of range */
    Error_Code |= UTM_LAT_ERROR;
  }
  if ((Longitude64 < itofx64(-180)) || (Longitude64 > itofx64(360)))
  { /* Longitude out of range */
    Error_Code |= UTM_LON_ERROR;
  }
  if (!Error_Code)
  { 

    /* no errors */
    if((Latitude64 > dtofx64(-1.0e-9) ) && (Latitude64 < 0))
      Latitude64 = 0;
    if (Longitude64 < 0)
      Longitude64 += itofx64(360) + dtofx64(1.0e-10);

{   
    long z;

    z = fx64toi( Longitude64 / 6 );

    if (Longitude64 < itofx64(180))
    {
      temp_zone = (31 + z );
    }
    else
    {
      temp_zone = ( z  - 29 );
    }
}

    if (temp_zone > 60)
      temp_zone = 1;
    /* UTM special cases */
    if ( (Latitude64 > itofx64(55)) && (Latitude64 < itofx64(64)) && (Longitude64 > itofx64(-1))
        && (Longitude64 < itofx64(3)))
      temp_zone = 31;
    if ((Latitude64 > itofx64(55)) && (Latitude64 < itofx64(64)) && (Longitude64 > itofx64(2))
        && (Longitude64 < itofx64(12)))
      temp_zone = 32;
    if ((Latitude64 > itofx64(71)) && (Longitude64 > itofx64(-1)) && (Longitude64 < itofx64(9)))
      temp_zone = 31;
    if ((Latitude64 > itofx64(71)) && (Longitude64 > itofx64(8)) && (Longitude64 < itofx64(21)))
      temp_zone = 33;
    if ((Latitude64 > itofx64(71)) && (Longitude64 > itofx64(20)) && (Longitude64 < itofx64(33)))
      temp_zone = 35;
    if ((Latitude64 > itofx64(71)) && (Longitude64 > itofx64(32)) && (Longitude64 < itofx64(42)))
      temp_zone = 37;

    if (temp_zone >= 31)
      Central_Meridian = ( (6 * temp_zone - 183) * PI64 ) / 180;
    else
      Central_Meridian = ( (6 * temp_zone + 177) * PI64 ) / 180;
    *Zone = temp_zone;
    if (Latitude64 < 0)
    {
      False_Northing = 10000000L;
      *Hemisphere = 'S';
    }
    else
      *Hemisphere = 'N';

    Latitude64_Radians = (( fx64toi64( Latitude64 << 24 ) / 180) * PI64 )>>24;
    Longitude64_Radians = (( fx64toi64( Longitude64 << 24 ) / 180) * PI64 )>>24;

    Set_Transverse_Mercator_Parameters(Central_Meridian, False_Northing);


    Convert_Geodetic_To_Transverse_Mercator(Latitude64_Radians, Longitude64_Radians, &Easting64,
                                            &Northing64);
    *Easting = fx64tod( Easting64 );
    *Northing = fx64tod( Northing64 );

    if ((*Easting < MIN_EASTING) || (*Easting > MAX_EASTING))
      Error_Code = UTM_EASTING_ERROR;
    if ((*Northing < MIN_NORTHING) || (*Northing > MAX_NORTHING))
      Error_Code |= UTM_NORTHING_ERROR;

  } /* END OF if (!Error_Code) */
  return (Error_Code);
} /* END OF Convert_Geodetic_To_UTM */


static long sphtmd( fixed64 Latitude )
{
  return fx64toi( TranMerc_ap * Latitude
          - TranMerc_bp * Fx64Sin(2 * Latitude)
          + TranMerc_cp * Fx64Sin(4 * Latitude)
          - Mulfx64( TranMerc_dp, Fx64Sin(6 * Latitude) )
          + Mulfx64( TranMerc_ep, Fx64Sin(8 * Latitude) )
         );
}

static long Set_Transverse_Mercator_Parameters(fixed64 Central_Meridian,
                                        long False_Northing)

{ /* BEGIN Set_Tranverse_Mercator_Parameters */
  /*
   * The function Set_Tranverse_Mercator_Parameters receives the ellipsoid
   * parameters and Tranverse Mercator projection parameters as inputs, and
   * sets the corresponding state variables. If any errors occur, the error
   * code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
   * returned.
   *
   *    a                 : Semi-major axis of ellipsoid, in meters    (input)
   *    f                 : Flattening of ellipsoid						         (input)
   *    Origin_Latitude   : Latitude in radians at the origin of the   (input)
   *                         projection
   *    Central_Meridian  : Longitude in radians at the center of the  (input)
   *                         projection
   *    False_Easting     : Easting/X at the center of the projection  (input)
   *    False_Northing    : Northing/Y at the center of the projection (input)
   *    Scale_Factor      : Projection scale factor                    (input) 
   */

  long Error_Code = TRANMERC_NO_ERROR;

  if (Central_Meridian > PI64)
    Central_Meridian -= (2*PI64);
  TranMerc_Origin_Long = Central_Meridian;
  TranMerc_False_Northing = False_Northing;

  Convert_Geodetic_To_Transverse_Mercator_const(MAX_DELTA_LONG + Central_Meridian,
                                          &TranMerc_Delta_Easting,
                                          &TranMerc_Delta_Northing);

  TranMerc_Delta_Northing++;
  TranMerc_Delta_Easting++;


  return (Error_Code);
}  /* END of Set_Transverse_Mercator_Parameters  */



static long Convert_Geodetic_To_Transverse_Mercator (fixed64 Latitude,
                                              fixed64 Longitude,
                                              fixed64 *Easting,
                                              fixed64 *Northing)

{      /* BEGIN Convert_Geodetic_To_Transverse_Mercator */

  /*
   * The function Convert_Geodetic_To_Transverse_Mercator converts geodetic
   * (latitude and longitude) coordinates to Transverse Mercator projection
   * (easting and northing) coordinates, according to the current ellipsoid
   * and Transverse Mercator projection coordinates.  If any errors occur, the
   * error code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
   * returned.
   *
   *    Latitude      : Latitude in radians                         (input)
   *    Longitude     : Longitude in radians                        (input)
   *    Easting       : Easting/X in meters                         (output)
   *    Northing      : Northing/Y in meters                        (output)
   */

  fixed64 c;       /* Cosine of latitude                          */
  fixed64 c2;
  fixed64 c3;
  fixed64 c5;
  fixed64 c7;
  fixed64 dlam;    /* Delta longitude - Difference in Longitude       */
  fixed64 dlam2;
  fixed64 dlam3;
  fixed64 dlam4;
  fixed64 dlam5;
  fixed64 dlam6;
  int dlam6Shift = 7;
  fixed64 dlam7;
  fixed64 dlam8;
  fixed64 eta;     /* constant - TranMerc_ebs *c *c                   */
  fixed64 eta2;
  fixed64 eta3;
  fixed64 eta4;
  fixed64 s;       /* Sine of latitude                        */
  fixed64 s2;      /* Sine * Sine */
  long sn;      /* Radius of curvature in the prime vertical       */
  fixed64 t;       /* Tangent of latitude                             */
  fixed64 tan2;
  fixed64 tan3;
  fixed64 tan4;
  fixed64 tan5;
  fixed64 tan6;
  long t1;      /* Term in coordinate conversion formula - GP to Y */
  long t2;      /* Term in coordinate conversion formula - GP to Y */
  long t3;      /* Term in coordinate conversion formula - GP to Y */
  long t4;      /* Term in coordinate conversion formula - GP to Y */
  long t5;      /* Term in coordinate conversion formula - GP to Y */
  long t6;      /* Term in coordinate conversion formula - GP to Y */
  long t7;      /* Term in coordinate conversion formula - GP to Y */
  long t8;      /* Term in coordinate conversion formula - GP to Y */
  long t9;      /* Term in coordinate conversion formula - GP to Y */
  long tmd;     /* True Meridional distance                        */
  long tmdo;    /* True Meridional distance for latitude of origin */
  long    Error_Code = TRANMERC_NO_ERROR;
  fixed64 temp_Origin;
  fixed64 temp_Long;

  if (Longitude > PI64 )
    Longitude -= (2 * PI64 );
  if ((Longitude < (TranMerc_Origin_Long - MAX_DELTA_LONG))
      || (Longitude > (TranMerc_Origin_Long + MAX_DELTA_LONG)))
  {
    if (Longitude < 0)
      temp_Long = Longitude + 2 * PI64;
    else
      temp_Long = Longitude;
    if (TranMerc_Origin_Long < 0)
      temp_Origin = TranMerc_Origin_Long + 2 * PI64;
    else
      temp_Origin = TranMerc_Origin_Long;

    if ((temp_Long < (temp_Origin - MAX_DELTA_LONG))
        || (temp_Long > (temp_Origin + MAX_DELTA_LONG)))
      Error_Code|= TRANMERC_LON_ERROR;
  }
  if (!Error_Code)
  { /* no errors */

    /* 
     *  Delta Longitude
     */
    dlam = Longitude - TranMerc_Origin_Long;

    if (Fabs64(dlam) > (9 * PI64 / 180))
    { /* Distortion will result if Longitude is more than 9 degrees from the Central Meridian */
      Error_Code |= TRANMERC_LON_WARNING;
    }

    if (dlam > PI64)
      dlam -= (2 * PI64);
    if (dlam < -PI64)
      dlam += (2 * PI64);
    if (Fabs64(dlam) < dtofx64(2.e-10) )
      dlam = 0;

    dlam2 = Mulfx64( dlam, dlam );
    dlam3 = Mulfx64( dlam, dlam2 );
    dlam4 = Mulfx64( dlam, dlam3 );
    dlam5 = Mulfx64( dlam, dlam4 );

    /*
      dlam * dlam5 gives an error up to 9%.
      To reduce the error to .279% shift dlam6 to the left dlam6Shift bits.
      Shift dlams 6,7, and 8 to the right by dlam6Shift bits when they are 
      used in an equation.
    */
    dlam6 = Mulfx64( dlam << dlam6Shift, dlam5 );
    dlam7 = Mulfx64( dlam, dlam6 );
    dlam8 = Mulfx64( dlam, dlam7 );

    s = Fx64Sin(Latitude);
    s2 = Mulfx64( s, s );
    c = Fx64Cos(Latitude);
    c2 = Mulfx64( c, c  );
    c3 = Mulfx64( c2, c );
    c5 = Mulfx64( c3, c2);
    c7 = Mulfx64( c5, c2);
    t = Fx64Tan(Latitude);
    tan2 = MulLargefx64( t, 5, t, 0 );
    tan3 = MulLargefx64( tan2, 7, t, 0 );
    tan4 = MulLargefx64( tan3, 8, t, 0 );
    tan5 = MulLargefx64( tan4, 15, t, 0 );
    tan6 = MulLargefx64( tan5, 20, t, 0 );
    eta = Mulfx64( TranMerc_ebs, c2 );
    eta2 = Mulfx64( eta, eta );
    eta3 = Mulfx64( eta2, eta );
    eta4 = Mulfx64( eta3, eta );

    /*      
      Original equation for sn:
      sn = itofx64(TranMerc_a) / FxSqrt64( f64_1 - Mulfx64( TranMerc_es, s2 ) );

      The easting compution had an error that was as high as 16 meters in some cases.
      The error was caused by an imprecise square root operation when computing sn.
      sn is made more presise by adding 0's to the end of the radicand.  The radicand
      is between -1 and 1 so the added zero's are after the decimal point.
    */
    {
      fixed64 radicand;
      fixed64 sqareRoot;

      radicand = f64_1 - Mulfx64( TranMerc_es, s2 );
      /*
        Add four zeroes after the decimal point so that the square root will 
        be more accurate.

        Multiply be 2**4.
      */
      radicand <<= 4;
      sqareRoot = FxSqrt64( radicand );
      /*
        sqrt( 2**4 * radicand ) = 2**2 * sqrt(radicand)
        Divide sqareRoot by sqrt(2**4)=2**2 to get sqrt(radicand).
      */
      sqareRoot >>= 2;
      sn = (long)( itofx64(TranMerc_a) / sqareRoot );
    }


    /* True Meridianal Distances */
    tmd = sphtmd(Latitude);

    /*  Origin  */
    tmdo = sphtmd ( itofx64(TranMerc_Origin_Lat) );

    /* northing */
    t1 = fx64toi( (tmd - tmdo) * TranMerc_Scale_Factor );

    t2 = fx64toi( sn * s );
    t2 = fx64toi( t2 * c);
    t2 = fx64toi( t2 * TranMerc_Scale_Factor/ 2 );

    t3 = fx64toi( sn * s );
    t3 = fx64toi( t3 * c3 );
    t3 = fx64toi( t3 * TranMerc_Scale_Factor );
    t3 = fx64toi( t3 * ( itofx64(5) - tan2 + 9 * eta + 4 * eta2 ) / 24 ); 

    t4 = fx64toi( sn * s );
    t4 = fx64toi( t4 * c5 );
    t4 = fx64toi( t4 * TranMerc_Scale_Factor );
    t4 = fx64toi( t4 * ( itofx64(61) - 58 * tan2
                      + tan4 + 270 * eta - 330 * Mulfx64( tan2, eta ) + 445 * eta2
                      + 324 * eta3 - 680 * Mulfx64( tan2, eta2 ) + 88 * eta4 
                      - 600 * Mulfx64( tan2, eta3 ) - 192 * Mulfx64( tan2, eta4 ) )
                ) / 720;

    t5 = fx64toi( sn * s );
    t5 = fx64toi( t5 * c7 );
    t5 = fx64toi( t5 * TranMerc_Scale_Factor );
    t5 = fx64toi( t5 * ( itofx64(1385) - 3111 * 
                      tan2 + 543 * tan4 - tan6 ) / 40320 ) ;


    *Northing = itofx64(TranMerc_False_Northing + t1) + dlam2 * t2
                + dlam4 * t3 + ( ( dlam6 * t4 ) >> dlam6Shift )
                + ( ( dlam8 * t5 ) >> dlam6Shift ); 

    /* Easting */
    t6 = fx64toi( sn * c );
    t6 = fx64toi( t6 * TranMerc_Scale_Factor );

    t7 = fx64toi( sn * c3 );
    t7 = fx64toi( t7 * TranMerc_Scale_Factor );
    t7 = fx64toi( t7 * ( (itofx64( 1 ) - tan2 + eta ) / 6 ) );

    t8 = fx64toi( sn * c5 );
    t8 = fx64toi( t8 * TranMerc_Scale_Factor );
    t8 = fx64toi( t8 * ( (itofx64(5) - 18 * tan2 + tan4
                  + 14 * eta - 58 * Mulfx64( tan2, eta ) + 13 * eta2 + 4 * eta3 
                  - 64 * Mulfx64( tan2, eta2 ) - 24 * Mulfx64( tan2, eta3 ) ) / 120 )
                );

    t9 = fx64toi( sn * c7 );
    t9 = fx64toi( t9 * TranMerc_Scale_Factor );
    t9 = fx64toi( t9 * ( itofx64(61) - 479 * tan2
                  + 179 * tan4 - tan6 ) / 5040 );

    *Easting = itofx64(TranMerc_False_Easting) + dlam * t6 + dlam3 * t7
               + dlam5 * t8 + (( dlam7 * t9 ) >> dlam6Shift);
  }
  return (Error_Code);
} /* END OF Convert_Geodetic_To_Transverse_Mercator */


static long Convert_Geodetic_To_Transverse_Mercator_const (fixed64 Longitude,
                                              long *Easting,
                                              long *Northing)

{      /* BEGIN Convert_Geodetic_To_Transverse_Mercator */

  /*
   * The function Convert_Geodetic_To_Transverse_Mercator converts geodetic
   * (latitude and longitude) coordinates to Transverse Mercator projection
   * (easting and northing) coordinates, according to the current ellipsoid
   * and Transverse Mercator projection coordinates.  If any errors occur, the
   * error code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
   * returned.
   *
   *    Latitude      : Latitude in radians                         (input)
   *    Longitude     : Longitude in radians                        (input)
   *    Easting       : Easting/X in meters                         (output)
   *    Northing      : Northing/Y in meters                        (output)
   */

  long t6;
  long t7;
  long t8;
  long t9;

  fixed64 dlam;    /* Delta longitude - Difference in Longitude       */
  fixed64 dlam2;
  fixed64 dlam3;
  fixed64 dlam4;
  fixed64 dlam5;
  fixed64 dlam6;
  fixed64 dlam7;
  fixed64 dlam8;
  long    Error_Code = TRANMERC_NO_ERROR;
  fixed64 temp_Origin;
  fixed64 temp_Long;

  if (Longitude > PI64 )
    Longitude -= (2 * PI64);
  if ((Longitude < (TranMerc_Origin_Long - MAX_DELTA_LONG))
      || (Longitude > (TranMerc_Origin_Long + MAX_DELTA_LONG)))
  {
    if (Longitude < 0)
      temp_Long = Longitude + 2 * PI64;
    else
      temp_Long = Longitude;
    if (TranMerc_Origin_Long < 0)
      temp_Origin = TranMerc_Origin_Long + 2 * PI64;
    else
      temp_Origin = TranMerc_Origin_Long;
    if ((temp_Long < (temp_Origin - MAX_DELTA_LONG))
        || (temp_Long > (temp_Origin + MAX_DELTA_LONG)))
      Error_Code|= TRANMERC_LON_ERROR;
  }
  if (!Error_Code)
  { /* no errors */

    /* 
     *  Delta Longitude
     */
    dlam = Longitude - TranMerc_Origin_Long;

    if (Fabs64(dlam) > ( 9 * PI64 / 180))
    { /* Distortion will result if Longitude is more than 9 degrees from the Central Meridian */
      Error_Code |= TRANMERC_LON_WARNING;
    }

    if (dlam > PI64)
      dlam -= (2 * PI64);
    if (dlam < -PI64)
      dlam += (2 * PI64);
    if (Fabs64(dlam) < dtofx64(2.e-10) )
      dlam = 0;

    dlam2 = Mulfx64( dlam, dlam );
    dlam3 = Mulfx64( dlam, dlam2);
    dlam4 = Mulfx64( dlam, dlam3);
    dlam5 = Mulfx64( dlam, dlam4);
    dlam6 = Mulfx64( dlam, dlam5);
    dlam7 = Mulfx64( dlam, dlam6);
    dlam8 = Mulfx64( dlam, dlam7);

    /* Easting */
    t6 = (long)6375585.7452;
    t7 = (long)1069758.9974266468;
    t8 = (long)270693.80399723159;
    t9 = (long)77164.827471666678;

    *Easting = TranMerc_False_Easting + fx64toi(dlam * t6) + fx64toi(dlam3 * t7 )
               + fx64toi(dlam5 * t8) + fx64toi(dlam7 * t9);
 
    /* northing */
    *Northing = TranMerc_False_Northing + 
                (long)9996848.4500014372 +
                fx64toi( dlam2 * (long)558.24649844331350 ) +
                fx64toi( dlam4 * (long)-46.520533034362479 ) +
                fx64toi( dlam6 * (long)1.5506818837048251 ) +
                fx64toi( dlam8 * (long)-0.027690337977568889 );

  }
  return (Error_Code);
} /* END OF Convert_Geodetic_To_Transverse_Mercator */

