#include	<stdlib.h>
#if !defined(HUMMINBIRD)
#include	<malloc.h>
#include	<fcntl.h>
#endif
#include  "text.h"
#include "chart.h"
#include "ClMalloc.h"
#include "fs.h"
#include "ClAssert.h"
#include "ChartLib.h"
#include "lkmhbird.h"

#if defined(HUMMINBIRD)
#include "fs_api.h"
#if !defined(HBIRDDEMO)
#define HBIRDDEMO 1
#endif
#endif

int ProcessiPilotCommand (LPIPILOTSTRUCT piPilot,double Scale)
{
	int	rtn=0;
	CL_POINT	saveHBImageCenter = HBImageCenter, saveHBImageCenterWorld = HBImageCenterWorld,saveHBImageCenterWorld_100 = HBImageCenterWorld_100;
	int		saveImageWidth = ImageWidth, saveImageHeight = ImageHeight;
	MNMXCORL saveHBImageBounds = HBImageBounds;
	HANDLE	enumHandle;
	double	RangeInMeters;
	int	iErr, i;
	int	minStepsBetweenTrackPoints;
	int stepsSinceLastTrackPoint=0;
	int	origMaxTrackPoints = piPilot->maxTrackPoints;
	CL_POINT pickPoint;
	DPOINT	offsetPoint;
	double	depthLeft, depthRight, az, azdir;
	int		pickedDepth;

	if (piPilot == NULL)
		return 0;

	pickPoint = DPOINTtoCL_POINT (&piPilot->pickPointD);
	piPilot->pickPointUser = piPilot->pickPointD;
	switch (piPilot->iOpt)
	{
	case 1:
	{

		RangeInMeters = Scale * piPilot->pickAperature + 1;
		Scale = AdjustToClosestScale (0.5);
		piPilot->iError = 1;
		piPilot->breakPoint.x = 0;
		LKMBeginEnumObjectsText (pickPoint.x,pickPoint.y,RangeInMeters, Scale,&enumHandle );
		while(LKMEnumNextObjectText( enumHandle, piPilot->pickText , sizeof(piPilot->pickText)  ))
		{
			LPENUMSTRUCT	pEnum=(LPENUMSTRUCT)enumHandle;

			if (pEnum->pickedType == 1)
			{
				piPilot->pickedCellID = pEnum->pickedCellID;
				piPilot->pickedCellX = pEnum->pickedCellX;
				piPilot->pickedCellY = pEnum->pickedCellY;
				piPilot->pickColor = pEnum->pickedColor;
				piPilot->iScale = pEnum->iScale;
				rtn = 1;
			}
		}
		LKMEndEnumObjectsText( enumHandle );
		HBImageBounds = saveHBImageBounds;
	}
	goto StartTracking;

	case 3:
	piPilot->trackPointSpacing /= TRACKPOINTMULTIPLIER; //greater accuracy in offset track can be
														// achieved with multiplier of 2 or 4 but requires more
														// memory and slows system
	piPilot->maxTrackPoints *= TRACKPOINTMULTIPLIER *3;
	piPilot->pTrackArray = malloc (piPilot->maxTrackPoints*TRACKPOINTMULTIPLIER*sizeof(DPOINT));
	SetGridVals (piPilot->iScale);
	Scale = GridDefs[piPilot->iScale].MetersPerPixel;
	RangeInMeters = 10000 * Scale;
	// no point in calculating distance until we have moved a minimun number of steps
	minStepsBetweenTrackPoints = (int)((piPilot->trackPointSpacing / Scale)/1.4);
	piPilot->lenTrackArray = 1;
	piPilot->pTrackArray[0] = piPilot->pickPointD;
	piPilot->switchPoint = 0;

StartTracking:
	LKMBeginEnumObjectsText (pickPoint.x,pickPoint.y,RangeInMeters, Scale,&enumHandle );
	LKMEnumNextObjectText( enumHandle, piPilot->pickText , 0);
	HBImageBounds = saveHBImageBounds;
	if (piPilot->iOpt == 1)
	{
		HBImageBounds.xmn -= GridDefs[piPilot->iScale].GridWidth;
		HBImageBounds.xmx += GridDefs[piPilot->iScale].GridWidth;
		HBImageBounds.ymn -= GridDefs[piPilot->iScale].GridHeight;
		HBImageBounds.ymx += GridDefs[piPilot->iScale].GridHeight;
	}
	if (piPilot->iOpt == 3)
	{
		HBImageBounds.xmn -= 200000;
		HBImageBounds.xmx += 200000;
		HBImageBounds.ymn -= 200000;
		HBImageBounds.ymx += 200000;
	}
	//Clear the buffered tiles
	GetLKMImageTileFromScaleAndID (-1,0,0,0,0);

	//load the first tile and start tracking the desired contour. If iOpt==1 track the first direction until
	// off screen or until loop back to start point
	{
		PLKMTILE pLKMTile = GetLKMImageTileFromScaleAndID (piPilot->iScale,piPilot->pickedCellID,piPilot,&iErr,TRUE);
		if (pLKMTile)
		{
			CL_POINT	nextPoint, pickedpt, sideRing[2]={0};
			BYTE	wantPixelValue;
			int		iPixel;
			int		nsteps;
			int		breakRing=0;
			int		icolor=0;
			COLORREF showcolor[5]={RGB(255,255,0),RGB(255,128,0),RGB(255,0,255),RGB(0,255,255),RGB(255,0,255)};

			nextPoint.x = piPilot->pickedCellX;
			nextPoint.y = piPilot->pickedCellY;
			piPilot->pickPointD = TilePointToWorldPoint2 (nextPoint.x,nextPoint.y,pLKMTile);

SplitRing:
			nsteps = 0;
			iPixel = nextPoint.y * pLKMTile->width + nextPoint.x;

			piPilot->pTileCrossingPointArray[piPilot->lenTileCrossingPointArray++] = TilePointToWorldPoint64 (nextPoint.x,nextPoint.y,pLKMTile);
			wantPixelValue = GetPaletteIndexForColor (piPilot->pickColor,pLKMTile);
			do
			{
				iPixel = nextPoint.y * pLKMTile->width + nextPoint.x;
				pLKMTile->Image[iPixel] = pLKMTile->TrackColorPaletteIndex[0];
				SetSurroundingPixels (nextPoint,pLKMTile,piPilot,1,0,wantPixelValue);
				if (nsteps < 8 && piPilot->iOpt == 1)
				{
					// store the first 8 steps in the first direction to get us off in the right direction
					// when iOpt == 3
					piPilot->first8Steps[0][nsteps] = nextPoint;
					piPilot->first8CellID[0][nsteps] = pLKMTile->CellID;
					piPilot->first8PixelVals[0][nsteps] = wantPixelValue;
					piPilot->nFirstSteps[0] = nsteps+1;
					if (nsteps == 7)
						piPilot->directionPoint[0] = TilePointToWorldPoint2 (nextPoint.x,nextPoint.y,pLKMTile);
				}
				if (breakRing)
				{
					if (nsteps == breakRing) // split closed loop into 2 halves when iOpt == 1
					{
						if (piPilot->iOpt == 1)
							piPilot->breakPoint = TilePointToWorldPoint64 (nextPoint.x,nextPoint.y,pLKMTile);
						break;
					}
					if (nsteps == breakRing/2)
						sideRing[0] = TilePointToWorldPoint64 (nextPoint.x,nextPoint.y,pLKMTile);
				}
				if (piPilot->iOpt == 3 && stepsSinceLastTrackPoint++ > minStepsBetweenTrackPoints)
				{
					DPOINT wpoint = TilePointToWorldPoint2 (nextPoint.x,nextPoint.y,pLKMTile);

					if (distpd (&wpoint,&piPilot->pTrackArray[piPilot->lenTrackArray-1]) >= piPilot->trackPointSpacing)
					{
						piPilot->pTrackArray[piPilot->lenTrackArray++] = wpoint;
						stepsSinceLastTrackPoint = 0;
						if (piPilot->lenTrackArray >= piPilot->maxTrackPoints)
							break;
					}
				}

				//showpoint (nextPoint,showcolor[icolor],saveImageWidth,saveImageHeight,saveHBImageCenter,saveHBImageCenterWorld,saveHBImageCenterWorld_100,pLKMTile,piPilot);
			} while (GetNextDepthPixel (&nextPoint,&pLKMTile,piPilot,nsteps++,TRUE,&wantPixelValue));
			
			icolor++;
			if (piPilot->iOpt == 3)
			{
				if (piPilot->lenTrackArray > 0)
					piPilot->endPoint = piPilot->pTrackArray[piPilot->lenTrackArray-1];
				
				// Create offset track
				OffsetTrackPoints (piPilot);

				piPilot->trackPointSpacing *= TRACKPOINTMULTIPLIER;
				piPilot->maxTrackPoints = origMaxTrackPoints;

				// resample track back to original spacing
				if (piPilot->ReSample)
					piPilot->lenTrackArray = ReSampleToOriginalSpacing (piPilot->lenTrackArray,&piPilot->pTrackArray,piPilot->maxTrackPoints,piPilot->trackPointSpacing);
				//put track array into output track array
				piPilot->lenTrackArray = min (piPilot->lenTrackArray,piPilot->maxTrackPoints);
				for (i=0;i<piPilot->lenTrackArray;i++)
					piPilot->pTrackArrayOut[i] = DPOINTtoCL_POINT (&piPilot->pTrackArray[i]);
				//compute depths at each track point
				for (i=0;i<piPilot->lenTrackArray;i++)
					piPilot->pTrackArrayDepth[i] = GetDepthAtPoint (&piPilot->pTrackArray[i])*10;
				cl_free (piPilot->pTrackArray);
				goto Exit;
			}

			pickedpt = WorldPointToTilePoint (piPilot->pickPointD.x,piPilot->pickPointD.y,pLKMTile);
			if (nsteps > 1 &&
				(abs(nextPoint.x-pickedpt.x)+abs(nextPoint.y-pickedpt.y) < 10))
			{
				// We have looped back to the start point. Split the contour into 2 pieces and restart
				breakRing = nsteps/2;
				piPilot->lenTileCrossingPointArray = 0;
				GetLKMImageTileFromScaleAndID (-1,0,0,0,0);
				pLKMTile = GetLKMImageTileFromScaleAndID (piPilot->iScale,piPilot->pickedCellID,piPilot,&iErr,TRUE);
				nextPoint.x = piPilot->pickedCellX;
				nextPoint.y = piPilot->pickedCellY;
				goto SplitRing;
			}
			if (sideRing[0].x && piPilot->lenTileCrossingPointArray < piPilot->maxTileCrossingPointArray)
				piPilot->pTileCrossingPointArray[piPilot->lenTileCrossingPointArray++] = sideRing[0];
			piPilot->switchPoint = piPilot->lenTileCrossingPointArray;
			nextPoint.x = piPilot->pickedCellX;
			nextPoint.y = piPilot->pickedCellY;
			pLKMTile = pLKMTileBuffer[0];
			if (piPilot->lenTileCrossingPointArray < piPilot->maxTileCrossingPointArray)
				piPilot->pTileCrossingPointArray[piPilot->lenTileCrossingPointArray++] = TilePointToWorldPoint64 (nextPoint.x,nextPoint.y,pLKMTile);
			nsteps = 0;

// Track the second direction until off screen			
			wantPixelValue = GetPaletteIndexForColor (piPilot->pickColor,pLKMTile);
			do
			{
				iPixel = nextPoint.y * pLKMTile->width + nextPoint.x;
				pLKMTile->Image[iPixel] = pLKMTile->TrackColorPaletteIndex[1];
				SetSurroundingPixels (nextPoint,pLKMTile,piPilot,1,1,wantPixelValue);
				if (nsteps < 8 && piPilot->iOpt == 1)
				{
					piPilot->first8Steps[1][nsteps] = nextPoint;
					piPilot->first8CellID[1][nsteps] = pLKMTile->CellID;
					piPilot->first8PixelVals[1][nsteps] = wantPixelValue;
					piPilot->nFirstSteps[1] = nsteps+1;
					if (nsteps == 7)
						piPilot->directionPoint[1] = TilePointToWorldPoint2 (nextPoint.x,nextPoint.y,pLKMTile);
				}
				if (breakRing)
				{
					if (nsteps == breakRing/2)
						sideRing[1] = TilePointToWorldPoint64 (nextPoint.x,nextPoint.y,pLKMTile);
				}
				//showpoint (nextPoint,showcolor[icolor],saveImageWidth,saveImageHeight,saveHBImageCenter,saveHBImageCenterWorld,saveHBImageCenterWorld_100,pLKMTile,piPilot);
			} while (GetNextDepthPixel (&nextPoint,&pLKMTile,piPilot,nsteps++,TRUE,&wantPixelValue));
			if (sideRing[1].x && piPilot->lenTileCrossingPointArray < piPilot->maxTileCrossingPointArray)
				piPilot->pTileCrossingPointArray[piPilot->lenTileCrossingPointArray++] = sideRing[1];

			piPilot->iError = 0;
		}

		//compute the headings and depth options for both direction choices on both sides of the contour
		piPilot->AzimuthFromStart[0] = GetAZMInDegrees (&piPilot->pickPointD,&piPilot->directionPoint[0]);
		piPilot->AzimuthFromStart[1] = GetAZMInDegrees (&piPilot->pickPointD,&piPilot->directionPoint[1]);
		az = getazmd (&piPilot->pickPointD,&piPilot->pickPointUser);
		azdir = getazmd (&piPilot->pickPointD,&piPilot->directionPoint[0]);
		if (DeltaAZ (&azdir,&az) >= 0)
			piPilot->userPicked[0] = -1;
		else
			piPilot->userPicked[0] = 1;
		azdir = getazmd (&piPilot->pickPointD,&piPilot->directionPoint[1]);
		if (DeltaAZ (&azdir,&az) >= 0)
			piPilot->userPicked[1] = -1;
		else
			piPilot->userPicked[1] = 1;
		az = getazmd (&piPilot->pickPointD,&piPilot->directionPoint[0]);
		offsetPoint = dnewpt (&piPilot->directionPoint[0],az-HALFPI,2*MAX_CONTOURTEXT_SCALE);
		depthRight = GetDepthAtPoint (&offsetPoint);
		offsetPoint = dnewpt (&piPilot->directionPoint[0],az-HALFPI,-2*MAX_CONTOURTEXT_SCALE);
		depthLeft = GetDepthAtPoint (&offsetPoint);
		pickedDepth = atoi (piPilot->pickText);
		if (depthLeft > depthRight)
		{
			if (pickedDepth <= HazardDepth)
				piPilot->userPicked[0] = -1;
			piPilot->deeperFlag[0] = -1;
		}
		else
		{
			if (pickedDepth <= HazardDepth)
				piPilot->userPicked[0] = 1;
			piPilot->deeperFlag[0] = 1;
		}
		az = getazmd (&piPilot->pickPointD,&piPilot->directionPoint[1]);
		offsetPoint = dnewpt (&piPilot->directionPoint[1],az-HALFPI,2*MAX_CONTOURTEXT_SCALE);
		depthRight = GetDepthAtPoint (&offsetPoint);
		offsetPoint = dnewpt (&piPilot->directionPoint[1],az-HALFPI,-2*MAX_CONTOURTEXT_SCALE);
		depthLeft = GetDepthAtPoint (&offsetPoint);
		if (depthLeft > depthRight)
		{
			if (pickedDepth <= HazardDepth)
				piPilot->userPicked[1] = -1;
			piPilot->deeperFlag[1] = -1;
		}
		else
		{
			if (pickedDepth <= HazardDepth)
				piPilot->userPicked[1] = 1;
			piPilot->deeperFlag[1] = 1;
		}

Exit:
		piPilot->iOpt++;
		rtn = 1;
	}
	LKMEndEnumObjectsText( enumHandle );
	break;
	case 2:
		break;
	case 4:
		break;
	}
	//Clear the buffered tiles
	GetLKMImageTileFromScaleAndID (-1,0,0,0,0);
	return rtn;
}
