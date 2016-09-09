//  RampCompliance.c
//  CCodeLibrary

#include "RampCompliance.h"

int fixRampNum (int rampNum)
{
	if (rampNum == 23)
		return 9;
	if (rampNum == 45)
		return 10;
	if (rampNum == 67)
		return 11;
	if (rampNum == 81)
		return 12;
	return rampNum;
}

char *rampComplianceCode(RampStruct *ramp, char **detailCode, ToleranceValues *tolerances)
{
    char *basic = (char *)calloc(16, sizeof(char));
    char *detail = (char *)calloc(64, sizeof(char));
    
    //signals
    if (ramp->PEDButtonHeight > 0)
    {
        if (ramp->PEDButtonHeight < (CVButtonHeight - tolerances->buttonHeight) ||
            ramp->PEDButtonHeight > (CVButtonHeight + tolerances->buttonHeight))
        {
            basic = strcat(basic, Signal);
            detail = strcat(detail, Signal);
        }
    }
    
    //detectable warnings
    if (ramp->texture > 0)
    {
        if (ramp->texture < truncatedStoneDomes || ramp->texture > castironTruncatedDomes)
        {
            basic = strcat(basic, DetectableWarning);
            detail = strcat(detail, DetectableWarning);
        }
    }
    
    int flagged = 0;
    
    //dimensions
    if (ramp->rampWidth > 0)
    {
        if (ramp->rampWidth < (CVDimensions - tolerances->dimensions))
        {
            basic = strcat(basic, DimensionsMajor);
            detail = strcat(detail, RampWidthMajor);
            flagged = 2;
        }
        
        else if (ramp->rampWidth < CVDimensions)
        {
            basic = strcat(basic, DimensionsMinor);
            detail = strcat(detail, RampWidthMinor);
            flagged = 1;
        }
    }
    
    if (ramp->rampDepth > 0)
    {
        if (ramp->rampDepth < (CVDimensions - tolerances->dimensions))
        {
            if (flagged < 2)
            {
                if (flagged == 1)
                    basic[strlen(basic) - 1] = '\0';
                
                basic = strcat(basic, DimensionsMajor);
            }
            
            detail = strcat(detail, RampDepthMajor);
        }
        
        else if (ramp->rampDepth < 36)
        {
            if (flagged < 1)
                basic = strcat(basic, DimensionsMinor);
            
            detail = strcat(detail, RampDepthMinor);
        }
    }
    
    flagged = 0;
    
    //ramp slopes (8,2)
    float value = fabsf(ramp->rampSlopeFront);
    
    if (value > (CV8 + tolerances->cv8) && value < 9990.0)
    {
        basic = strcat(basic, SlopeMajor);
        detail = strcat(detail, RampFrontMajor);
        flagged = 2;
    }
    
    else if (value > CV8 && value < 9990.0)
    {
        basic = strcat(basic, SlopeMinor);
        detail = strcat(detail, RampFrontMinor);
        flagged = 1;
    }
    
    value = fabsf(ramp->rampSlopeSide);
    
    if (value > (CV2 + tolerances->cv2) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
            basic = strcat(basic, SlopeMajor);
            flagged = 2;
        }
        
        detail = strcat(detail, RampSideMajor);
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
            basic = strcat(basic, SlopeMinor);
            flagged = 1;
        }
        
        detail = strcat(detail, RampSideMinor);
    }
    
    //upper landing slopes (2,2)
    value = fabsf(ramp->upperLandingSlopeFront);
    
    if (value > (CV2 + tolerances->cv2) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
            basic = strcat(basic, SlopeMajor);
            flagged = 2;
        }
        
        detail = strcat(detail, UpperLandingFrontMajor);
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
            basic = strcat(basic, SlopeMinor);
            flagged = 1;
        }
        
        detail = strcat(detail, UpperLandingFrontMinor);
    }
    
    value = fabsf(ramp->upperLandingSlopeSide);
    
    if (value > (CV2 + tolerances->cv2) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
            basic = strcat(basic, SlopeMajor);
            flagged = 2;
        }
        
        detail = strcat(detail, UpperLandingSideMajor);
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
            basic = strcat(basic, SlopeMinor);
            flagged = 1;
        }
        
        detail = strcat(detail, UpperLandingSideMinor);
    }
    
    //street landing slopes (5,5)
    value = fabsf(ramp->streetLandingSlopeFront);
    
    if (value > (CV5 + tolerances->cv5) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
            basic = strcat(basic, SlopeMajor);
            flagged = 2;
        }
        
        detail = strcat(detail, StreetLandingFrontMajor);
    }
    
    else if (value > CV5 && value < 9990.0)
    {
        if (flagged < 1)
        {
            basic = strcat(basic, SlopeMinor);
            flagged = 1;
        }
        
        detail = strcat(detail, StreetLandingFrontMinor);
    }
    
    value = fabsf(ramp->streetLandingSlopeSide);
    
    if (value > (CV5 + tolerances->cv5) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
            basic = strcat(basic, SlopeMajor);
            flagged = 2;
        }
        
        detail = strcat(detail, StreetLandingSideMajor);
    }
    
    else if (value > CV5 && value < 9990.0)
    {
        if (flagged < 1)
        {
            basic = strcat(basic, SlopeMinor);
            flagged = 1;
        }
        
        detail = strcat(detail, StreetLandingSideMinor);
    }

    //flare left slopes (10)
    value = fabsf(ramp->flareLeftSlopeFront);
    
    if (value > (CV10 + tolerances->cv10) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
            basic = strcat(basic, SlopeMajor);
            flagged = 2;
        }
        
        detail = strcat(detail, FlareLeftFrontMajor);
    }
    
    else if (value > CV10 && value < 9990.0)
    {
        if (flagged < 1)
        {
            basic = strcat(basic, SlopeMinor);
            flagged = 1;
        }
        
        detail = strcat(detail, FlareLeftFrontMinor);
    }

    //flare right slopes (10)
    value = fabsf(ramp->flareRightSlopeFront);
    
    if (value > (CV10 + tolerances->cv10) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
            basic = strcat(basic, SlopeMajor);
            flagged = 2;
        }
        
        detail = strcat(detail, FlareRightFrontMajor);
    }
    
    else if (value > CV10 && value < 9990.0)
    {
        if (flagged < 1)
        {
            basic = strcat(basic, SlopeMinor);
            flagged = 1;
        }
        
        detail = strcat(detail, FlareRightFrontMinor);
    }
    
    //sidewalk left slopes (5,2)
    value = fabsf(ramp->swkLeftSlopeFront);
    
    if (value > (CV5 + tolerances->cv5) && value < 9990.0)
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
    
    else if (value > CV5 && value < 9990.0)
    {
        if (flagged < 1)
        {
            basic = strcat(basic, SlopeMinor);
            flagged = 1;
        }
        
        detail = strcat(detail, SwkLeftFrontMinor);
    }
    
    value = fabsf(ramp->swkLeftSlopeSide);
    
    if (value > (CV2 + tolerances->cv2) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
            basic = strcat(basic, SlopeMajor);
            flagged = 2;
        }
        
        detail = strcat(detail, SwkLeftSideMajor);
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
            basic = strcat(basic, SlopeMinor);
            flagged = 1;
        }
        
        detail = strcat(detail, SwkLeftSideMinor);
    }
    
    //sidewalk right slopes (5,2)
    value = fabsf(ramp->swkRightSlopeFront);
    
    if (value > (CV5 + tolerances->cv5) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
            basic = strcat(basic, SlopeMajor);
            flagged = 2;
        }
        
        detail = strcat(detail, SwkRightFrontMajor);
    }
    
    else if (value > CV5 && value < 9990.0)
    {
        if (flagged < 1)
        {
            basic = strcat(basic, SlopeMinor);
            flagged = 1;
        }
        
        detail = strcat(detail, SwkRightFrontMinor);
    }
    
    value = fabsf(ramp->swkRightSlopeSide);
    
    if (value > (CV2 + tolerances->cv2) && value < 9990.0)
    {
        if (flagged < 2)
        {
            if (flagged == 1)
                basic[strlen(basic) - 1] = '\0';
            
            basic = strcat(basic, SlopeMajor);
        }
        
        detail = strcat(detail, SwkRightSideMajor);
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
            basic = strcat(basic, SlopeMinor);
        
        detail = strcat(detail, SwkRightSideMinor);
    }
    
    flagged = 0;
    
    //ramp cracks
    if (ramp->hasRampCracks == 1)
    {
        if (ramp->crackWidth.rampCrackWidth > tolerances->cracks)
        {
            basic = strcat(basic, CracksMajor);
            detail = strcat(detail, RampCrackMajor);
            flagged = 2;
        }
        
        else
        {
            basic = strcat(basic, CracksMinor);
            detail = strcat(detail, RampCrackMinor);
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
                
                basic = strcat(basic, CracksMajor);
                flagged = 2;
            }
            
            detail = strcat(detail, UpperLandingCrackMajor);
        }
        
        else
        {
            if (flagged < 1)
            {
                basic = strcat(basic, CracksMinor);
                flagged = 1;
            }
            
            detail = strcat(detail, UpperLandingCrackMinor);
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
                
                basic = strcat(basic, CracksMajor);
                flagged = 2;
            }
            
            detail = strcat(detail, StreetLandingCrackMajor);
        }
        
        else
        {
            if (flagged < 1)
            {
                basic = strcat(basic, CracksMinor);
                flagged = 1;
            }
            
            detail = strcat(detail, StreetLandingCrackMinor);
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
                
                basic = strcat(basic, CracksMajor);
                flagged = 2;
            }
            
            detail = strcat(detail, SwkLeftCrackMajor);
        }
        
        else
        {
            if (flagged < 1)
            {
                basic = strcat(basic, CracksMinor);
                flagged = 1;
            }
            
            detail = strcat(detail, SwkLeftCrackMinor);
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
                
                basic = strcat(basic, CracksMajor);
                flagged = 2;
            }
            
            detail = strcat(detail, SwkRightCrackMajor);
        }
        
        else
        {
            if (flagged < 1)
            {
                basic = strcat(basic, CracksMinor);
                flagged = 1;
            }
            
            detail = strcat(detail, SwkRightCrackMinor);
        }
    }
    
    flagged = 0;
    
    //steep top of curb
    if (ramp->SteepTopOfCurb > 0.0)
    {
        if (ramp->SteepTopOfCurb > tolerances->levelChange)
        {
            basic = strcat(basic, LevelChangeMajor);
            detail = strcat(detail, STOCMajor);
            flagged = 2;
        }
        
        else
        {
            basic = strcat(basic, LevelChangeMinor);
            detail = strcat(detail, STOCMinor);
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
                
                basic = strcat(basic, LevelChangeMajor);
                flagged = 2;
            }
            
            detail = strcat(detail, LipMajor);
        }
        
        else
        {
            if (flagged < 1)
            {
                basic = strcat(basic, LevelChangeMinor);
                flagged = 1;
            }
            
            detail = strcat(detail, LipMinor);
        }
    }
    
    flagged = 0;
    
    //obstructions
    if (ramp->rampObstruction > 0)
    {
        if (ramp->rampObstruction < none)
        {
            basic = strcat(basic, Obstructions);
            detail = strcat(detail, RampObstruction);
            flagged = 1;
        }
    }
    
    if (ramp->upperLandingObstruction > 0)
    {
        if (ramp->upperLandingObstruction < none)
        {
            if (flagged < 1)
                basic = strcat(basic, Obstructions);
            
            detail = strcat(detail, UpperLandingObstruction);
        }
    }
    
    if (ramp->lowerLandingObstruction > 0)
    {
        if (ramp->lowerLandingObstruction < none)
        {
            if (flagged < 1)
                basic = strcat(basic, Obstructions);
            
            detail = strcat(detail, StreetLandingObstruction);
        }
    }
    
    //mark as compliant if no flags have been added
    if (strlen(basic) == 0)
    {
        basic = strcat(basic, Compliant);
        detail = strcat(detail, Compliant);
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

static char *textures[] = {"None", "SmoothedConcrete", "BrushedConcrete", "TintedConcrete", "TruncatedStoneDomes", "TruncatedStampedConrete", "CastironTruncatedDomes", "ExposedAggregate", "CutStone", "None", "Other"};

static char *obstructions[] = {"None", "Hydrant", "Manhole", "Polebox", "Pole", "StreetManhole", "Other", "None", "None", "None", "None", "MasterNone"};

static char *rampTypes[] = {"Perp", "PerpNonWalk", "CombPerpWalk", "CombPerpNonWalk", "OneWayDirCurbGutter", "OneWayDirBlendTrans", "Parallel", "DepressedCorner", "Fan", "BuiltUp", "Diagonal", "CombLeftWalk", "CombRightWalk", "CombLeftNonWalk", "CombRightNonWalk"};


char *rampToText(int intNum,RampStruct *ramp)
{
    char *rampText = (char *)calloc(4480, sizeof(char));
    
    sprintf(rampText, "%i\t%i\t%i\t'%s'\t'%s'\t%i\t%.8f\t%.8f\t%i\t'%s'\t'%s'\t'%s'\t'%s'\t%i\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%i\t%i\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%i\t%i\t%.2f\t%.2f\t'%s'\t%.2f\t%.2f\t%.2f",
            ramp->uniqueID,
			intNum,
            ramp->rampNum,
            ramp->rampID,
            rampTypes[ramp->rampType],
            ramp->timeComplete,
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
            ramp->PEDButtonHeight,
            ramp->PEDButtonDist,
            ramp->SteepTopOfCurb,
            ramp->PedRampLip,
            ramp->rampComment,
            ramp->curbCutDistance,
            ramp->bumpWidth,
            ramp->bumpHeight
            );
    
    return rampText;
}

const char *rampToTextHeader(void)
{
    return "UniqueRampID\tIntersectionNum\tRampNum\tRampID\tRampType\tTimeComplete\tLatitude\tLongitude\tRampInXWalk\tTexture\tUpperLandingObstruction\tStreetLandingObstruction\tRampObstruction\tHasRampCracks\tHasUpperLandingCracks\tHasStreetLandingCracks\tHasLeftSidewalkCracks\tHasRightSidewalkCracks\tRampCrackWidth\tUpperLandingCrackWidth\tStreetLandingCrackWidth\tLeftSidewalkCrackWidth\tRightSidewalkCrackWidth\tRampWidth\tRampDepth\tRampSlopeFront\tRampSlopeSide\tUpperLandingSlopeFront\tUpperLandingSlopeSide\tStreetLandingSlopeFront\tStreetLandingSlopeSide\tFlareLeftSlopeFront\tFlareRightSlopeFront\tSidewalkLeftSlopeFront\tSidewalkLeftSlopeSide\tSidewalkRightSlopeFront\tSidewalkRightSlopeSide\tPEDButtonHeight\tPEDButtonDistance\tSteepTopOfCurb\tLipAtFlowLine\tRampComment\tCurbCutDistance\tBumpWidth\tBumpHeight\tComplianceCodeDetail\tComplianceCodeSummary";
}