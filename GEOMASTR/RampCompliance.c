//  RampCompliance.c
//  CCodeLibrary
#if WIN32
#define strcasecmp	stricmp
#endif

#include "RampCompliance.h"

int fixRampNum(int rampNum)
{
    int rtn = rampNum;
    
    switch (rampNum)
    {
        case 23:
            rtn = 9;
            break;
        case 45:
            rtn = 10;
            break;
        case 67:
            rtn = 11;
            break;
        case 81:
            rtn = 12;
            break;
        default:break;
    }
    
    return rtn;
}

static LPSTR selectCode(LPSTR codes, int whichCode)
{
	static char code[64];
	strcpy(code, codes);

	LPSTR pBar = strchr(code, '|');

	if (!pBar)
		return codes;
	*pBar++ = 0;
	if (whichCode)
		return pBar;
	return code;
}

double roundToTenth(double value)
{
    value *= 10;
    value += 0.5;
    value = floor(value);
    value /= 10;
    return value;
}

char *rampComplianceCode(RampStruct *ramp, char **detailCode, ToleranceValues *tolerances,int codeSystem)
{
    char *basic = (char *)calloc(16, sizeof(char));
    char *detail = (char *)calloc(64, sizeof(char));
    
    //signals
    if (ramp->PEDButtonType > 0)
    {
        if (ramp->PEDButtonHeight < (CVButtonHeight - tolerances->buttonHeight) ||
            ramp->PEDButtonHeight > (CVButtonHeight + tolerances->buttonHeight))
        {
			basic = strcat(basic, selectCode(Signal, codeSystem));
			detail = strcat(detail, selectCode(Signal, codeSystem));
        }
    }
    
    //detectable warnings
    if (ramp->texture > 0)
    {
        if (ramp->texture < truncatedStoneDomes || ramp->texture > castironTruncatedDomes)
        {
            if (ramp->texture != castInPlacePanels)
            {
				basic = strcat(basic, selectCode(DetectableWarning, codeSystem));
				detail = strcat(detail, selectCode(DetectableWarning, codeSystem));
            }
        }
    }
    
    int flagged = 0;
    
    //dimensions
    if (ramp->rampWidth > -1)
    {
        if (ramp->rampWidth < (CVDimensions - tolerances->dimensions))
        {
			basic = strcat(basic, selectCode(DimensionsMajor, codeSystem));
			detail = strcat(detail, selectCode(RampWidthMajor, codeSystem));
            flagged = 2;
        }
        
        else if (ramp->rampWidth < CVDimensions)
        {
			basic = strcat(basic, selectCode(DimensionsMinor, codeSystem));
			detail = strcat(detail, selectCode(RampWidthMinor, codeSystem));
            flagged = 1;
        }
    }
    
    if (ramp->rampDepth > -1)
    {
        if (ramp->rampDepth < (CVDimensions - tolerances->dimensions))
        {
            if (flagged < 2)
            {
                if (flagged == 1)
                    basic[strlen(basic) - 1] = '\0';
                
				basic = strcat(basic, selectCode(DimensionsMajor, codeSystem));
            }
            
			detail = strcat(detail, selectCode(RampDepthMajor, codeSystem));
        }
        
        else if (ramp->rampDepth < 36)
        {
            if (flagged < 1)
				basic = strcat(basic, selectCode(DimensionsMinor, codeSystem));
            
			detail = strcat(detail, selectCode(RampDepthMinor, codeSystem));
        }
    }
    
    flagged = 0;
    
    //ramp slopes (8,2)
    float value = fabsf(ramp->rampSlopeFront);
    value = roundToTenth(value);
    
    if (value > (CV8 + tolerances->cv8) && value < 9990.0)
    {
		basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
		detail = strcat(detail, selectCode(RampFrontMajor, codeSystem));
        flagged = 2;
    }
    
    else if (value > CV8 && value < 9990.0)
    {
		basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
		detail = strcat(detail, selectCode(RampFrontMinor, codeSystem));
        flagged = 1;
    }
    
    value = fabsf(ramp->rampSlopeSide);
    value = roundToTenth(value);
    
    if (value > (CV2 + tolerances->cv2) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
			basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
            flagged = 2;
        }
        
		detail = strcat(detail, selectCode(RampSideMajor, codeSystem));
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		detail = strcat(detail, selectCode(RampSideMinor, codeSystem));
    }
    
    //upper landing slopes (2,2)
    value = fabsf(ramp->upperLandingSlopeFront);
    value = roundToTenth(value);
    
    if (value > (CV2 + tolerances->cv2) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
			basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
            flagged = 2;
        }
        
		detail = strcat(detail, selectCode(UpperLandingFrontMajor, codeSystem));
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		detail = strcat(detail, selectCode(UpperLandingFrontMinor, codeSystem));
    }
    
    value = fabsf(ramp->upperLandingSlopeSide);
    value = roundToTenth(value);
    
    if (value > (CV2 + tolerances->cv2) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
			basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
            flagged = 2;
        }
        
		detail = strcat(detail, selectCode(UpperLandingSideMajor, codeSystem));
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		detail = strcat(detail, selectCode(UpperLandingSideMinor, codeSystem));
    }
    
    //street landing slopes (5,5)
    value = fabsf(ramp->streetLandingSlopeFront);
    value = roundToTenth(value);
    
    if (value > (CV5 + tolerances->cv5) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
			basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
            flagged = 2;
        }
        
		detail = strcat(detail, selectCode(StreetLandingFrontMajor, codeSystem));
    }
    
    else if (value > CV5 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		detail = strcat(detail, selectCode(StreetLandingFrontMinor, codeSystem));
    }
    
    value = fabsf(ramp->streetLandingSlopeSide);
    value = roundToTenth(value);
    
    if (value > (CV5 + tolerances->cv5) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
			basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
            flagged = 2;
        }
        
		detail = strcat(detail, selectCode(StreetLandingSideMajor, codeSystem));
    }
    
    else if (value > CV5 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		detail = strcat(detail, selectCode(StreetLandingSideMinor, codeSystem));
    }

    //flare left slopes (10)
    value = fabsf(ramp->flareLeftSlopeFront);
    value = roundToTenth(value);
    
    if (value > (CV10 + tolerances->cv10) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
			basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
            flagged = 2;
        }
        
		detail = strcat(detail, selectCode(FlareLeftFrontMajor, codeSystem));
    }
    
    else if (value > CV10 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		detail = strcat(detail, selectCode(FlareLeftFrontMinor, codeSystem));
    }

    //flare right slopes (10)
    value = fabsf(ramp->flareRightSlopeFront);
    value = roundToTenth(value);
    
    if (value > (CV10 + tolerances->cv10) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
			basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
            flagged = 2;
        }
        
		detail = strcat(detail, selectCode(FlareRightFrontMajor, codeSystem));
    }
    
    else if (value > CV10 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		detail = strcat(detail, selectCode(FlareRightFrontMinor, codeSystem));
    }
    
    //sidewalk left slopes (5,2)
    value = fabsf(ramp->swkLeftSlopeFront);
    value = roundToTenth(value);
    
    if (ramp->rampType == RampTypeParallel)
    {
        if (value > (CV8 + tolerances->cv8) && value < 9990.0)
        {
            if (flagged < 2)
            {
                if (flagged == 1)
                    basic[strlen(basic) - 1] = '\0';
                
                basic = strcat(basic, SlopeMajor);
                flagged = 2;
            }
            
            detail = strcat(detail, SwkLeftFrontMajor);
        }
        
        else if (value > CV8 && value < 9990.0)
        {
            if (flagged < 1)
            {
                basic = strcat(basic, SlopeMinor);
                flagged = 1;
            }
            
            detail = strcat(detail, SwkLeftFrontMinor);
        }
    }
    
    else
    {
	    if (value > (CV5 + tolerances->cv5) && value < 9990.0)
	    {
	        if (flagged < 2)
	        {
	            if (flagged == 1)
	                basic[strlen(basic) - 1] = '\0';
            
				basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
	            flagged = 2;
	        }
        
			detail = strcat(detail, selectCode(SwkLeftFrontMajor, codeSystem));
	    }
    
        else if (value > CV5 && value < 9990.0)
    	{
	        if (flagged < 1)
        	{
				basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
	            flagged = 1;
        	}
        
			detail = strcat(detail, selectCode(SwkLeftFrontMinor, codeSystem));
		}
    }
    
    value = fabsf(ramp->swkLeftSlopeSide);
    value = roundToTenth(value);
    
    if (value > (CV2 + tolerances->cv2) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
			basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
            flagged = 2;
        }
        
		detail = strcat(detail, selectCode(SwkLeftSideMajor, codeSystem));
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		detail = strcat(detail, selectCode(SwkLeftSideMinor, codeSystem));
    }
    
    //sidewalk right slopes (5,2)
    value = fabsf(ramp->swkRightSlopeFront);
    value = roundToTenth(value);
    
    if (ramp->rampType == RampTypeParallel)
    {
        if (value > (CV5 + tolerances->cv5) && value < 9990.0)
        {
            if (flagged < 2)
            {
               if (flagged == 1)
                   basic[strlen(basic) - 1] = '\0';
            
			    basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
                flagged = 2;
            }
        
		detail = strcat(detail, selectCode(SwkRightFrontMajor, codeSystem));
    }
    
    else if (value > CV5 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		detail = strcat(detail, selectCode(SwkRightFrontMinor, codeSystem));
        }
    }
    
    else
    {
        if (value > (CV8 + tolerances->cv8) && value < 9990.0)
        {
            if (flagged < 2)
            {
                if (flagged == 1)
                    basic[strlen(basic) - 1] = '\0';
                
                basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
                flagged = 2;
            }
            
            detail = strcat(detail, selectCode(SwkRightFrontMajor, codeSystem));
        }
        
        else if (value > CV5 && value < 9990.0)
        {
            if (flagged < 1)
            {
                basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
                flagged = 1;
            }
            
            detail = strcat(detail, selectCode(SwkRightFrontMinor, codeSystem));
        }
    }    
    
    value = fabsf(ramp->swkRightSlopeSide);
    value = roundToTenth(value);
    
    if (value > (CV2 + tolerances->cv2) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
			basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
        }
        
		detail = strcat(detail, selectCode(SwkRightSideMajor, codeSystem));
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
        
		detail = strcat(detail, selectCode(SwkRightSideMinor, codeSystem));
    }
    
    flagged = 0;
    
    //ramp cracks
    if (ramp->hasRampCracks == 1)
    {
        if (ramp->crackWidth.rampCrackWidth > tolerances->cracks)
        {
			basic = strcat(basic, selectCode(CracksMajor, codeSystem));
			detail = strcat(detail, selectCode(RampCrackMajor, codeSystem));
            flagged = 2;
        }
        
        else
        {
			basic = strcat(basic, selectCode(CracksMinor, codeSystem));
			detail = strcat(detail, selectCode(RampCrackMinor, codeSystem));
            flagged = 1;
        }
    }
    
    //upper landing cracks
    if (ramp->hasUpperLandingCracks == 1)
    {
        if (ramp->crackWidth.upperLandingCrackWidth > tolerances->cracks)
        {
            if (flagged < 2)
            {
                if (flagged == 1)
                    basic[strlen(basic) - 1] = '\0';
                
				basic = strcat(basic, selectCode(CracksMajor, codeSystem));
                flagged = 2;
            }
            
			detail = strcat(detail, selectCode(UpperLandingCrackMajor, codeSystem));
        }
        
        else
        {
            if (flagged < 1)
            {
				basic = strcat(basic, selectCode(CracksMinor, codeSystem));
                flagged = 1;
            }
            
			detail = strcat(detail, selectCode(UpperLandingCrackMinor, codeSystem));
        }
    }
    
    //street landing cracks
    if (ramp->hasStreetLandingCracks == 1)
    {
        if (ramp->crackWidth.streetLandingCrackWidth > tolerances->cracks)
        {
            if (flagged < 2)
            {
                if (flagged == 1)
                    basic[strlen(basic) - 1] = '\0';
                
				basic = strcat(basic, selectCode(CracksMajor, codeSystem));
                flagged = 2;
            }
            
			detail = strcat(detail, selectCode(StreetLandingCrackMajor, codeSystem));
        }
        
        else
        {
            if (flagged < 1)
            {
				basic = strcat(basic, selectCode(CracksMinor, codeSystem));
                flagged = 1;
            }
            
			detail = strcat(detail, selectCode(StreetLandingCrackMinor, codeSystem));
        }
    }
    
    //left sidewalk cracks
    if (ramp->hasLeftSidewalkCracks == 1)
    {
        if (ramp->crackWidth.leftSidewalkCrackWidth > tolerances->cracks)
        {
            if (flagged < 2)
            {
                if (flagged == 1)
                    basic[strlen(basic) - 1] = '\0';
                
				basic = strcat(basic, selectCode(CracksMajor, codeSystem));
                flagged = 2;
            }
            
			detail = strcat(detail, selectCode(SwkLeftCrackMajor, codeSystem));
        }
        
        else
        {
            if (flagged < 1)
            {
				basic = strcat(basic, selectCode(CracksMinor, codeSystem));
                flagged = 1;
            }
            
			detail = strcat(detail, selectCode(SwkLeftCrackMinor, codeSystem));
        }
    }
    
    //right sidewalk cracks
    if (ramp->hasRightSidewalkCracks == 1)
    {
        if (ramp->crackWidth.rightSidewalkCrackWidth > tolerances->cracks)
        {
            if (flagged < 2)
            {
                if (flagged == 1)
                    basic[strlen(basic) - 1] = '\0';
                
				basic = strcat(basic, selectCode(CracksMajor, codeSystem));
                flagged = 2;
            }
            
			detail = strcat(detail, selectCode(SwkRightCrackMajor, codeSystem));
        }
        
        else
        {
            if (flagged < 1)
            {
				basic = strcat(basic, selectCode(CracksMinor, codeSystem));
                flagged = 1;
            }
            
			detail = strcat(detail, selectCode(SwkRightCrackMinor, codeSystem));
        }
    }
    
    flagged = 0;
    
    //steep top of curb
    if (ramp->SteepTopOfCurb > 0.0)
    {
        if (ramp->SteepTopOfCurb > tolerances->levelChange)
        {
			basic = strcat(basic, selectCode(LevelChangeMajor, codeSystem));
			detail = strcat(detail, selectCode(STOCMajor, codeSystem));
            flagged = 2;
        }
        
        else
        {
			basic = strcat(basic, selectCode(LevelChangeMinor, codeSystem));
			detail = strcat(detail, selectCode(STOCMinor, codeSystem));
            flagged = 1;
        }
    }
    
    //lip
    if (ramp->PedRampLip > 0.0)
    {
        if (ramp->PedRampLip > tolerances->levelChange)
        {
            if (flagged < 2)
            {
                if (flagged == 1)
                    basic[strlen(basic) - 1] = '\0';
                
				basic = strcat(basic, selectCode(LevelChangeMajor, codeSystem));
                flagged = 2;
            }
            
			detail = strcat(detail, selectCode(LipMajor, codeSystem));
        }
        
        else
        {
            if (flagged < 1)
            {
				basic = strcat(basic, selectCode(LevelChangeMinor, codeSystem));
                flagged = 1;
            }
            
			detail = strcat(detail, selectCode(LipMinor, codeSystem));
        }
    }
    
    flagged = 0;
    
    //obstructions
    if (ramp->rampObstruction > 0)
    {
        if (ramp->rampObstruction < none)
        {
			basic = strcat(basic, selectCode(Obstructions, codeSystem));
			detail = strcat(detail, selectCode(RampObstruction, codeSystem));
            flagged = 1;
        }
    }
    
    if (ramp->upperLandingObstruction > 0)
    {
        if (ramp->upperLandingObstruction < none)
        {
            if (flagged < 1)
				basic = strcat(basic, selectCode(Obstructions, codeSystem));
            
			detail = strcat(detail, selectCode(UpperLandingObstruction, codeSystem));
        }
    }
    
    if (ramp->lowerLandingObstruction > 0)
    {
        if (ramp->lowerLandingObstruction < none)
        {
            if (flagged < 1)
				basic = strcat(basic, selectCode(Obstructions, codeSystem));
            
			detail = strcat(detail, selectCode(StreetLandingObstruction, codeSystem));
        }
    }
    
    //mark as compliant if no flags have been added
    if (strlen(basic) == 0)
    {
		basic = strcat(basic, selectCode(Compliant, codeSystem));
		detail = strcat(detail, selectCode(Compliant, codeSystem));
    }
    
    *detailCode = detail;
    
    return basic;
}

void setStandardToleranceValues(ToleranceValues *tolerances)
{
    tolerances->buttonHeight = 4.0;
    tolerances->dimensions = 4.0;
    tolerances->cv10 = (CV10 / 2.0);
    tolerances->cv8 = (CV8 / 2.0);
    tolerances->cv5 = (CV5 / 2.0);
    tolerances->cv2 = (CV2 / 2.0);
    tolerances->cracks = 0.5;
    tolerances->levelChange = 1.0;
    
    return;
}

static char *textures[] = {"None", "SmoothedConcrete", "BrushedConcrete", "TintedConcrete", "TruncatedStoneDomes", "TruncatedStampedConcrete", "CastironTruncatedDomes", "ExposedAggregate", "CutStone", "None", "Other","CastInPlacePanels","LinearGroove"};
static char *obstructions[] = {"None", "Hydrant", "Manhole", "Polebox", "Pole", "StreetManhole", "Other", "None", "None", "None", "None", "MasterNone","","","","","","","","","","SDWKClearWidth","SDWKProtudingObject","SDWKVerticalClearance","SDWKUndergroundUtility"};
static char *rampTypes[] = {"Perp", "PerpNonWalk", "CombPerpWalk", "CombPerpNonWalk", "OneWayPerp", "OneWayDirBlendTrans", "Parallel", "DepressedCorner", "Fan", "BuiltUp", "Diagonal", "OneWayDirCurbGutter", "CombLeftWalk", "CombRightWalk", "CombLeftNonWalk", "CombRightNonWalk"};
static char *AWITypes[] = {"None", "Tones", "SpeechMessage"};
static char *buttonTypes[] = {"None", "SmallPush", "LargePush", "Touch", "APS"};
static char *signalTypes[] = {"None", "Text", "Symbol", "SideTimer", "BelowTimer"};

int NVCTextureToCode(LPSTR texture)
{
	if (!strlen(texture))
		return 0;
	for (int i = 0; i < sizeof (textures) / 4; i++)
	{
		if (!strcasecmp(texture, textures[i]))
			return i;
	}
	if (!strcasecmp(texture, "SmoothConcrete"))
		return 1;
	return 0;
}
int NVCObstructionToCode(LPSTR obstruction)
{
	if (!strlen(obstruction))
		return 0;
	for (int i = 0; i < sizeof (obstructions)/4; i++)
	{
		if (!strcasecmp(obstruction, obstructions[i]))
			return i;
	}
	return 0;
}

char *rampToText(int intNum, RampStruct *ramp)
{
	int minTime = 1338526800;
    char *rampText = (char *)calloc(4480*2, sizeof(char));
	char timeCompleteC[32];
	int rtype = ramp->rampType;
	if (rtype > 100)
		rtype = 11 + (rtype - 100);
	if (ramp->timeComplete < minTime)
		ramp->timeComplete = minTime;
	sprintf(timeCompleteC, "$CAL(%i,3)", ramp->timeComplete);
	ExpandText(timeCompleteC);
	if (rtype < 0 || rtype > 15)
		rtype = 0;
	if (ramp->upperLandingObstruction < 0 || ramp->upperLandingObstruction >= obstructions_max)
		ramp->upperLandingObstruction = 0;
	if (ramp->lowerLandingObstruction < 0 || ramp->lowerLandingObstruction >= obstructions_max)
		ramp->lowerLandingObstruction = 0;
	if (ramp->rampObstruction < 0 || ramp->rampObstruction >= obstructions_max)
		ramp->rampObstruction = 0;
	if (ramp->PEDSignalType < 0 || ramp->PEDSignalType >= NVSignalTypeCount)
		ramp->PEDSignalType = 0;
	if (ramp->PEDButtonType < 0 || ramp->PEDButtonType >= NVButtonTypeCount)
		ramp->PEDButtonType = 0;
	if (ramp->awi <= 0 || ramp->awi >= AudibleWalkIndicationCount)
	{
		ramp->awi = 0;
		ramp->locatorToneVolume = 0;
		ramp->audibleWalkIndicationVolume = 0;
	}
	ramp->locatorToneVolume = max(ramp->locatorToneVolume,0);
	ramp->audibleWalkIndicationVolume = max(ramp->audibleWalkIndicationVolume, 0);
	sprintf(rampText, "%i\t%i\t%i\t'%s'\t'%s'\t%i\t'%s'\t%.10f\t%.10f\t%i\t'%s'\t'%s'\t'%s'\t'%s'\t%i\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%i\t%i\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%i\t%i\t'%s'\t'%s'\t%i\t%i\t'%s'\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t'%s'\t'%s'",
		ramp->uniqueID,
		intNum,
		ramp->rampNum,
		ramp->rampID,
		rampTypes[rtype],
		ramp->yearRebuilt,
		timeCompleteC,
		ramp->latitude,
		ramp->longitude,
		ramp->rampInXWalk,
		textures[ramp->texture],
		obstructions[ramp->upperLandingObstruction],
		obstructions[ramp->lowerLandingObstruction],
		obstructions[ramp->rampObstruction],
		ramp->hasRampCracks,
		ramp->hasUpperLandingCracks,
		ramp->hasStreetLandingCracks,
		ramp->hasLeftSidewalkCracks,
		ramp->hasRightSidewalkCracks,
		ramp->crackWidth.rampCrackWidth,
		ramp->crackWidth.upperLandingCrackWidth,
		ramp->crackWidth.streetLandingCrackWidth,
		ramp->crackWidth.leftSidewalkCrackWidth,
		ramp->crackWidth.rightSidewalkCrackWidth,
		ramp->rampWidth,
		ramp->rampDepth,
		ramp->rampSlopeFront,
		ramp->rampSlopeSide,
		ramp->upperLandingSlopeFront,
		ramp->upperLandingSlopeSide,
		ramp->streetLandingSlopeFront,
		ramp->streetLandingSlopeSide,
		ramp->flareLeftSlopeFront,
		ramp->flareRightSlopeFront,
		ramp->swkLeftSlopeFront,
		ramp->swkLeftSlopeSide,
		ramp->swkRightSlopeFront,
		ramp->swkRightSlopeSide,
		ramp->SteepTopOfCurb,
		ramp->PedRampLip,
		ramp->curbCutDistance,
		ramp->bumpWidth,
		ramp->bumpHeight,
		ramp->dwWidth,
		ramp->dwDepth,
		signalTypes[ramp->PEDSignalType],
		buttonTypes[ramp->PEDButtonType],
		ramp->PEDButtonHeight,
		ramp->PEDButtonDist,
		AWITypes[ramp->awi],
		ramp->hasLocatorTone,
		ramp->hasInfoSign,
		ramp->hasBraille,
		ramp->hasTactileArrow,
		ramp->locatorToneVolume,
		ramp->audibleWalkIndicationVolume,
		ramp->rampComment,
		ramp->fileID
		);
    
    return rampText;
}

const char *rampToTextHeader(int type)
{
	if (!type)
		return "UniqueRampID\tIntersectionNum\tRampNum\tRampID\tRampType\tYearBuilt\tTimeComplete\tLatitude\tLongitude\tRampInXWalk\tTexture\tUpperLandingObstruction\tStreetLandingObstruction\tRampObstruction\tHasRampCracks\tHasUpperLandingCracks\tHasStreetLandingCracks\tHasLeftSidewalkCracks\tHasRightSidewalkCracks\tRampCrackWidth\tUpperLandingCrackWidth\tStreetLandingCrackWidth\tLeftSidewalkCrackWidth\tRightSidewalkCrackWidth\tRampWidth\tRampDepth\tRampSlopeFront\tRampSlopeSide\tUpperLandingSlopeFront\tUpperLandingSlopeSide\tStreetLandingSlopeFront\tStreetLandingSlopeSide\tFlareLeftSlopeFront\tFlareRightSlopeFront\tSidewalkLeftSlopeFront\tSidewalkLeftSlopeSide\tSidewalkRightSlopeFront\tSidewalkRightSlopeSide\tSteepTopOfCurb\tLipAtFlowLine\tCurbCutDistance\tBumpWidth\tBumpHeight\tDetectableWarningWidth\tDetectableWarningDepth\tPEDSignalType\tPEDButtonType\tPEDButtonHeight\tPEDButtonDistance\tPEDButtonAWIType\tPEDButtonHasLocatorTone\tPEDButtonHasInfoSign\tPEDButtonHasBraille\tPEDButtonHasTactileArrow\tPEDButtonLocatorToneVolume\tPEDButtonAWIVolume\tRampComment\tSourceFile\tComplianceCodeDetail\tComplianceCodeSummary";
	return "UniqueRampID(B4)\tIntersectionNum(B4)\tRampNum(B4)\tRampID(C16)\tRampType(C32)\tYearBuild(B2)\tTimeComplete(C16)\tLatitude(R8)\tLongitude(R8)\tRampInXWalk(B2)\tTexture(C40)\tUpperLandingObstruction(C40)\tStreetLandingObstruction(C40)\tRampObstruction(C40)\tHasRampCracks(B2)\tHasUpperLandingCracks(B2)\tHasStreetLandingCracks(B2)\tHasLeftSidewalkCracks(B2)\tHasRightSidewalkCracks(B2)\tRampCrackWidth(R4)\tUpperLandingCrackWidth(R4)\tStreetLandingCrackWidth(R4)\tLeftSidewalkCrackWidth(R4)\tRightSidewalkCrackWidth(R4)\tRampWidth(B2)\tRampDepth(B2)\tRampSlopeFront(R4)\tRampSlopeSide(R4)\tUpperLandingSlopeFront(R4)\tUpperLandingSlopeSide(R4)\tStreetLandingSlopeFront(R4)\tStreetLandingSlopeSide(R4)\tFlareLeftSlopeFront(R4)\tFlareRightSlopeFront(R4)\tSidewalkLeftSlopeFront(R4)\tSidewalkLeftSlopeSide(R4)\tSidewalkRightSlopeFront(R4)\tSidewalkRightSlopeSide(R4)\tSteepTopOfCurb(R4)\tLipAtFlowLine(R4)\tCurbCutDistance(R4)\tBumpWidth(R4)\tBumpHeight(R4)\tDetectableWarningWidth(B4)\tDetectableWarningDepth(B4)\tPEDSignalType(C32)\tPEDButtonType(C32)\tPEDButtonHeight(B2)\tPEDButtonDistance(B2)\tPEDButtonAWIType(C16)\tPEDButtonHasLocatorTone(B2)\tPEDButtonHasInfoSign(B2)\tPEDButtonHasBraille(B2)\tPEDButtonHasTactileArrow(B2)\tPEDButtonLocatorToneVolume(B2)\tPEDButtonAWIVolume(B2)\tRampComment(C255)\tSourceFile(C12)\tComplianceCodeDetail(C100)\tComplianceCodeSummary(C32)";
}
