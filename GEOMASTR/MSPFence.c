#include "mspData.h"

#define LITTLEENDIAN 0

static int ByteSwapNeeded(void)
{
   long one= 1;
   return !(*((char *)(&one)));
}

int intflip (int byte1,int byte2)
{
//	if (LITTLEENDIAN)
	if (ByteSwapNeeded())
		return (byte1 * 256 + byte2);
	else
		return (byte2 * 256 + byte1);
}

void ConvertLatLonToGrid (double *lat,double *lon,int *gridx,int *gridy)
{
#include "MSPTran.h"

	double XI = *lon - BASX; 
	double YI = *lat - BASY;
	double XOUT = A1 + B1 * XI + C1 * YI;
	double YOUT = A2 + B2 * YI + C2 * XI;

	if (XOUT < 0)
		*gridx = XOUT - 0.5;
	else
		*gridx = XOUT + 0.5;
	if (YOUT < 0)
		*gridy = YOUT - 0.5;
	else
		*gridy = YOUT + 0.5;
	return;
}

int AmInGeofence (double lat, double lon)
{
	int gridx, gridy, gridval;
	int	rowpos, nextRowPos=0, colpos;
	int	lenrow;
	int	nRepeat = 0;

	ConvertLatLonToGrid (&lat,&lon,&gridx,&gridy);

	if (gridx < minGridX || gridx > maxGridX ||
		gridy < minGridY || gridy > maxGridY)
		return 0;

	//find row
	while (gridy--)
	{
		if (nRepeat)
			nRepeat--;
		else
		{
			rowpos = nextRowPos;
			lenrow = intflip (mspFence[rowpos],mspFence[rowpos+1]);
			nRepeat = intflip (mspFence[rowpos+lenrow+2],mspFence[rowpos+lenrow+3]);
			nextRowPos = rowpos + lenrow + 4;
		}
	}
	//find column
	colpos = rowpos + 2;
	gridval = mspFence[colpos++];
	nRepeat = intflip (mspFence[colpos],mspFence[colpos+1]);
	colpos += 2;
	while (gridx--)
	{
		if (nRepeat)
			nRepeat--;
		else
		{
			gridval = mspFence[colpos++];
			nRepeat = intflip (mspFence[colpos],mspFence[colpos+1]);
			colpos += 2;
		}
	}
	return gridval;
}


