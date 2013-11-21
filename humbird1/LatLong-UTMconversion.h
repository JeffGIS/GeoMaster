//LatLong- UTM conversion..h
//definitions for lat/long to UTM and UTM to lat/lng conversions
/* Derek Ditch: Taken from http://www.gpsy.com/gpsinfo/geotoutm/ 
 * Original author is Chuck Gantz.
 */

#include <string.h>

#ifndef LATLONGCONV
#define LATLONGCONV

void LLtoUTM(int ReferenceEllipsoid, const double Lat, const double Long, double *pUTMNorthing, double *pUTMEasting, int ZoneNumber, char *cUTMZoneActual);
void UTMtoLL(int ReferenceEllipsoid, const double UTMNorthing, const double UTMEasting, int iUTMZone,double *pLat,  double *pLong );
char UTMLetterDesignator(double Lat);


typedef struct
{
// public:
	// Ellipsoid(){};
	// Ellipsoid(int Id, char* name, double radius, double ecc)
	// {
		// id = Id; ellipsoidName = name; 
		// EquatorialRadius = radius; eccentricitySquared = ecc;
	// }
	int id;
	char* ellipsoidName;
	double EquatorialRadius; 
	double eccentricitySquared;  
}Ellipsoid;



#endif
