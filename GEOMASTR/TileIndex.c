#include <math.h>
#include <string.h>
#include "TileGraphics.h"
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))

static	const double EarthRadius = 6378137;
static	const double MinLatitude = -85.05112878;
static	const double MaxLatitude = 85.05112878;
static	const double MinLongitude = -180;
static	const double MaxLongitude = 180;
static	const double PI = 3.14159265358979323846;


        /// <summary>
        /// Clips a number to the specified minimum and maximum values.
        /// </summary>
        /// <param name="n">The number to clip.</param>
        /// <param name="minValue">Minimum allowable value.</param>
        /// <param name="maxValue">Maximum allowable value.</param>
        /// <returns>The clipped value.</returns>
static double Clip(double n, double minValue, double maxValue)
{
    return min(max(n, minValue), maxValue);
}
        
        

        /// <summary>
        /// Determines the map width and height (in pixels) at a specified level
        /// of detail.
        /// </summary>
        /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
        /// to 23 (highest detail).</param>
        /// <returns>The map width and height in pixels.</returns>
unsigned int MapSize(int levelOfDetail)
{
    return (unsigned int) 256 << levelOfDetail;
}



        /// <summary>
        /// Determines the ground resolution (in meters per pixel) at a specified
        /// latitude and level of detail.
        /// </summary>
        /// <param name="latitude">Latitude (in degrees) at which to measure the
        /// ground resolution.</param>
        /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
        /// to 23 (highest detail).</param>
        /// <returns>The ground resolution, in meters per pixel.</returns>
double GroundResolution(double latitude, int levelOfDetail)
{
    latitude = Clip(latitude, MinLatitude, MaxLatitude);
    return cos(latitude * PI / 180) * 2 * PI * EarthRadius / MapSize(levelOfDetail);
}



        /// <summary>
        /// Determines the map scale at a specified latitude, level of detail,
        /// and screen resolution.
        /// </summary>
        /// <param name="latitude">Latitude (in degrees) at which to measure the
        /// map scale.</param>
        /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
        /// to 23 (highest detail).</param>
        /// <param name="screenDpi">Resolution of the screen, in dots per inch.</param>
        /// <returns>The map scale, expressed as the denominator N of the ratio 1 : N.</returns>
static double MapScale(double latitude, int levelOfDetail, int screenDpi)
{
    return GroundResolution(latitude, levelOfDetail) * screenDpi / 0.0254;
}



        /// <summary>
        /// Converts a point from latitude/longitude WGS-84 coordinates (in degrees)
        /// into pixel XY coordinates at a specified level of detail.
        /// </summary>
        /// <param name="latitude">Latitude of the point, in degrees.</param>
        /// <param name="longitude">Longitude of the point, in degrees.</param>
        /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
        /// to 23 (highest detail).</param>
        /// <param name="pixelX">Output parameter receiving the X coordinate in pixels.</param>
        /// <param name="pixelY">Output parameter receiving the Y coordinate in pixels.</param>
void LatLongToPixelXY(double latitudein, double longitudein, int levelOfDetail, int *pixelX, int *pixelY)
{
    double latitude = Clip(latitudein, MinLatitude, MaxLatitude);
    double longitude = Clip(longitudein, MinLongitude, MaxLongitude);

    double x = (longitude + 180) / 360; 
    double sinLatitude = sin(latitude * PI / 180);
    double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * PI);

    unsigned int mapSize = MapSize(levelOfDetail);
    *pixelX = (int)(Clip(x * mapSize + 0.5, 0, mapSize - 1));
    *pixelY = (int)(Clip(y * mapSize + 0.5, 0, mapSize - 1));
	*pixelY = (mapSize - 1) - *pixelY;
}
void LatLongToPixelXYd(double latitudein, double longitudein, int levelOfDetail, double *pixelX, double *pixelY)
{
    double latitude = Clip(latitudein, MinLatitude, MaxLatitude);
    double longitude = Clip(longitudein, MinLongitude, MaxLongitude);

    double x = (longitude + 180) / 360; 
    double sinLatitude = sin(latitude * PI / 180);
    double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * PI);

    unsigned int mapSize = MapSize(levelOfDetail);
    *pixelX = Clip(x * mapSize, 0, mapSize - 1);
    *pixelY = Clip(y * mapSize, 0, mapSize - 1);
	*pixelY = (mapSize - 1) - *pixelY;

}



        /// <summary>
        /// Converts a pixel from pixel XY coordinates at a specified level of detail
        /// into latitude/longitude WGS-84 coordinates (in degrees).
        /// </summary>
        /// <param name="pixelX">X coordinate of the point, in pixels.</param>
        /// <param name="pixelY">Y coordinates of the point, in pixels.</param>
        /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
        /// to 23 (highest detail).</param>
        /// <param name="latitude">Output parameter receiving the latitude in degrees.</param>
        /// <param name="longitude">Output parameter receiving the longitude in degrees.</param>
void PixelXYToLatLong(double pixelX, double pixelY, int levelOfDetail, double *latitude, double *longitude)
{
    unsigned int mapSize = MapSize(levelOfDetail);
    double x = (Clip(pixelX, 0, mapSize - 1) / mapSize) - 0.5;
    double y = 0.5 - (Clip(mapSize - pixelY, 0, mapSize - 1) / mapSize);

    *latitude = 90 - 360 * atan(exp(-y * 2 * PI)) / PI;
    *longitude = 360 * x;
}

void PixelXYToLatLongd(double pixelX, double pixelY, int levelOfDetail, double *latitude, double *longitude)
{
    unsigned int mapSize = MapSize(levelOfDetail);
    double x = (Clip(pixelX, 0, mapSize - 1) / mapSize) - 0.5;
	double y = 0.5 - (Clip(mapSize - pixelY, 0, mapSize - 1) / mapSize);

    *latitude = 90 - 360 * atan(exp(-y * 2 * PI)) / PI;
    *longitude = 360 * x;
}





        /// <summary>
        /// Converts pixel XY coordinates into tile XY coordinates of the tile containing
        /// the specified pixel.
        /// </summary>
        /// <param name="pixelX">Pixel X coordinate.</param>
        /// <param name="pixelY">Pixel Y coordinate.</param>
        /// <param name="tileX">Output parameter receiving the tile X coordinate.</param>
        /// <param name="tileY">Output parameter receiving the tile Y coordinate.</param>
void PixelXYToTileXY(int pixelX, int pixelY, int *tileX, int *tileY)
{
    *tileX = pixelX / 256;
    *tileY = pixelY / 256;
}

void PixelXYToTileXYd(double pixelX, double pixelY, int *tileX, int *tileY)
{
    *tileX = pixelX / 256.0;
    *tileY = pixelY / 256.0;
}



        /// <summary>
        /// Converts tile XY coordinates into pixel XY coordinates of the upper-left pixel
        /// of the specified tile.
        /// </summary>
        /// <param name="tileX">Tile X coordinate.</param>
        /// <param name="tileY">Tile Y coordinate.</param>
        /// <param name="pixelX">Output parameter receiving the pixel X coordinate.</param>
        /// <param name="pixelY">Output parameter receiving the pixel Y coordinate.</param>
void TileXYToPixelXY(int tileX, int tileY, int *pixelX, int *pixelY)
{
    *pixelX = tileX * 256;
    *pixelY = tileY * 256;
}



        /// <summary>
        /// Converts tile XY coordinates into a QuadKey at a specified level of detail.
        /// </summary>
        /// <param name="tileX">Tile X coordinate.</param>
        /// <param name="tileY">Tile Y coordinate.</param>
        /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
        /// to 23 (highest detail).</param>
        /// <returns>A string containing the QuadKey.</returns>
void TileXYToQuadKey(int tileX, int tileY, int levelOfDetail,char *quadKey,int maxLen)
{
	int	i;

    memset (quadKey,0,maxLen);
    for (i = levelOfDetail; i > 0; i--)
    {
		char digit[2] = {'0',0};
        int mask = 1 << (i - 1);
        if ((tileX & mask) != 0)
        {
            digit[0]++;
        }
        if ((tileY & mask) != 0)
        {
            digit[0]++;
            digit[0]++;
        }
        strcat (quadKey,digit);
    }
	return;
}



        /// <summary>
        /// Converts a QuadKey into tile XY coordinates.
        /// </summary>
        /// <param name="quadKey">QuadKey of the tile.</param>
        /// <param name="tileX">Output parameter receiving the tile X coordinate.</param>
        /// <param name="tileY">Output parameter receiving the tile Y coordinate.</param>
        /// <param name="levelOfDetail">Output parameter receiving the level of detail.</param>
int QuadKeyToTileXY(char *quadKey, int *tileX, int *tileY, int *levelOfDetail)
{
	int	i;

    *tileX = *tileY = 0;
    *levelOfDetail = strlen (quadKey);

    for (i = *levelOfDetail; i > 0; i--)
    {
        int mask = 1 << (i - 1);
        switch (quadKey[*levelOfDetail - i])
        {
            case '0':
                break;

            case '1':
                *tileX |= mask;
                break;

            case '2':
                *tileY |= mask;
                break;

            case '3':
                *tileX |= mask;
                *tileY |= mask;
                break;

            default:
                return 0;
        }
    }
	return 1;
}
