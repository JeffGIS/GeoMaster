/*C*****************************************************
C*       /umsc/include/projections.ftn
C*
C*       Data related to map projections.
C*
C*****************************************************
C   */
#include <proj_api.h>
#define MAXGZOOMS	21
#define MAXPROJ4PROJ	8
#define LATLONPROJECTION	 0
#define GOOGLEMAPSPROJECTION 1
#define SphericalMercatorPROJECTION	-1

#ifdef  PROJECTION_INCLUDE



/*C
C     The  following 3 dimensional arrays contain:
C                            Element 1 - Base Projection
C                                    2 - Input Projection
C                                    3 - Output Projection
C  */

      BOOL    	 IS_BASE[4];             // {.TRUE. when projection = base projection
      double     PRJ_GRND_TO_GRID[4];    //{Ground to grid conversion factor (Must be non-zero)
      double     PRJ_X_BIAS[4];          //{Bias applied to latitude
      double     PRJ_Y_BIAS[4];          //{Bias applied to longitude

      long  PRJ_SPHEROID[4];        //{Coordinate spheroid code number
/*C                                          0 = CLARKE 1866
C                                            1 = CLARKE 1880               
C                                            2 = BESSEL
C                                            3 = NEW INTERNATIONAL 1967    
C                                            4 = INTERNATIONAL 1909
C                                            5 = WGS 72                    
C                                            6 = EVEREST
C                                            7 = WGS 66                    
C                                            8 = GRS 1980
C                                            9 = AIRY                      
C                                           10 = MODIFIED EVEREST
C                                           11 = MODIFIED AIRY             
C                                           12 = WALBECK
C                                           13 = SOUTHEAST ASIA            
C                                           14 = AUSTRALIAN NATIONAL
C                                           15 = KRASSOVSKY                
C                                           16 = HOUGH
C                                           17 = MERCURY 1960              
C                                           18 = MODIFIED MERC 1968
C                                           19 = SPHERE OF RADIUS 6370997 M */
      long  PRJ_TYPE[4];            //{Coordinate system code number
/*                                           0 = Latitude & Longitude
C                                            1 = U T M                                  
C                                            2 = STATE PLANE                            
C                                            3 = ALBERS CONICAL EQUAL-AREA              
C                                            4 = LAMBERT CONFORMAL CONIC                
C                                            5 = MERCATOR                               
C                                            6 = POLAR STEREOGRAPHIC                    
C                                            7 = POLYCONIC                              
C                                            8 = EQUIDISTANT CONIC                      
C                                            9 = TRANSVERSE MERCATOR                    
C                                           10 = STEREOGRAPHIC                          
C                                           11 = LAMBERT AZIMUTHAL EQUAL-AREA           
C                                           12 = AZIMUTHAL EQUIDISTANT                  
C                                           13 = GNOMONIC                               
C                                           14 = ORTHOGRAPHIC                           
C                                           15 = GENERAL VERTICAL NEAR-SIDE PERSPECTIVE 
C                                           16 = SINUSOIDAL                             
C                                           17 = EQUIRECTANGULAR                        
C                                           18 = MILLER CYLINDRICAL                     
C                                           19 = VAN DER GRINTEN I                      
C                                           20 = OBLIQUE MERCATOR (HOTINE)                    
C                                          100 = Used for plot files    */
      long  PRJ_UNITS[4];           //{Coordinate unit of measure code
/*C                                           0 =  Inches
C                                           1 =  Feet            
C                                           2 =  Meters          
C                                           3 =  Seconds of arc  
C                                           4 =  Degrees of arc  
C                                           5 =  Packed dms */ 
	  long	PRJ_BASEUNITS[4];    
      long  PRJ_ZONE[4];            //{Coordinate zone code number
      char  PROJECTION_ID[4][128];    //{Coord. system English description
      char  PROJECTION_UNITS[4][8];  //{Unit of measure English description
      HANDLE  PRJ_TRAN[4][3]={0,0,0,0,0,0,0,0,0,0,0,0} ;
	  
	  projPJ	projdef[MAXPROJ4PROJ];
	  char		projid[MAXPROJ4PROJ][32]={0};


#else

extern      double     INPUT_UNITS_CONV;      // {Input coordinates unit of measure
extern      double     OUTPUT_UNITS_CONV;     // {Output coordinates unit of measure
extern      BOOL       HAVE_BASE_PROJ;        // {.TRUE. when coordinates from base projection
extern      BOOL       IS_BASE[4];          
extern      double     PRJ_GRND_TO_GRID[4];    //{Ground to grid conversion factor (Must be non-zero)
extern      double     PRJ_X_BIAS[4];          //{Bias applied to latitude
extern      double     PRJ_Y_BIAS[4];          //{Bias applied to longitude
extern      long       PRJ_SPHEROID[4];        //{Coordinate spheroid code number
extern      long       PRJ_TYPE[4];            //{Coordinate system code number
extern		long	   PRJ_BASEUNITS[4];	   //{Units coord def file uses for offsets and transformations
extern      long       PRJ_UNITS[4];           //{Coordinate unit of measure code 0=inches, 1=feet, 2=meters, 4=degrees 5=degrees*1,000,000
extern      long       PRJ_ZONE[4];            //{Coordinate zone code number
extern      char       PROJECTION_ID[4][128];    //{Coord. system English description
extern      char       PROJECTION_UNITS[4][8];  //{Unit of measure English description
extern      HANDLE     PRJ_TRAN[4][3] ; 
extern		projPJ	   projdef[MAXPROJ4PROJ];
extern		char	   projid[MAXPROJ4PROJ][32];

#endif      
