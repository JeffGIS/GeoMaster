//  PolylineCompression.c
//  CCodeLibrary

#include "graphint.h"   

//  PolylineCompression.c
//  CCodeLibrary


char *EncodeString(LPDPOINT llpoints, int numPoints)
{
	int maxSize = (numPoints * 10);
	char *encodedString = (char *)malloc((sizeof(char) * maxSize) + 4);

	int index = 0;
	int dlat = 0, dlng = 0;
	int oldLat = 0, oldLng = 0;

	for (int i = 0; i < numPoints; i++)
	{
		DPOINT point = llpoints[i];

		int newLat = (int)(point.y * 1e+6);
		int newLng = (int)(point.x * 1e+6);

		dlat = (newLat - oldLat);
		dlng = (newLng - oldLng);

		oldLat = newLat;
		oldLng = newLng;

		int sgn_dlat = (dlat << 1);
		int sgn_dlng = (dlng << 1);

		if (dlat < 0)
			sgn_dlat = ~(sgn_dlat);

		if (dlng < 0)
			sgn_dlng = ~(sgn_dlng);

		while (sgn_dlat >= 0x20)
		{
			encodedString[index++] = (char)((0x20 | (sgn_dlat & 0x1f)) + 63);
			sgn_dlat >>= 5;
		}

		encodedString[index++] = (char)(sgn_dlat + 63);

		while (sgn_dlng >= 0x20)
		{
			encodedString[index++] = (char)((0x20 | (sgn_dlng & 0x1f)) + 63);
			sgn_dlng >>= 5;
		}

		encodedString[index++] = (char)(sgn_dlng + 63);
	}

	char *result = (char *)malloc(sizeof(char) * (index + 1));

	for (int i = 0; i < index; i++)
		result[i] = encodedString[i];

	result[index] = 0;

	free(encodedString);

	return result;
}

LPDPOINT DecodeString(char *encodedString, int *numPoints)
{
	int index = 0, nPoints = 0;
	double lat = 0, lng = 0;

	unsigned long len = strlen(encodedString);

	int maxSize = ceil(len / 4) + 1;

	LPDPOINT llpoints = (LPDPOINT)malloc((sizeof(DPOINT) * maxSize) + 4);

	while (index < len)
	{
		char b = 0;
		int shift = 0, result = 0;

		do
		{
			b = (encodedString[index++] - 63);
			result |= (b & 0x1f) << shift;
			shift += 5;
		}

		while (b >= 0x20);

		double dlat = ((result & 1) ? ~(result >> 1) : (result >> 1));
		lat += dlat;

		shift = 0, result = 0;

		do
		{
			b = (encodedString[index++] - 63);
			result |= (b & 0x1f) << shift;
			shift += 5;
		}

		while (b >= 0x20);

		double dlng = ((result & 1) ? ~(result >> 1) : (result >> 1));
		lng += dlng;

		double finalLat = (lat * 1e-6);
		double finalLong = (lng * 1e-6);

		if (nPoints >= maxSize)
			llpoints = realloc(llpoints, ((sizeof(DPOINT) * (nPoints + 1)) + 4));

		llpoints[nPoints].x = finalLong;
		llpoints[nPoints++].y = finalLat;
	}

	*numPoints = nPoints;

	llpoints = (LPDPOINT)realloc(llpoints, (sizeof(DPOINT) * nPoints));

	return llpoints;
}