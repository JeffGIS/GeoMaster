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
char *rampComplianceCode(RampStruct *ramp, char **detailCode, ToleranceValues *tolerances,int codeSystem)
{
    char *basic = (char *)calloc(16, sizeof(char));
    char *detail = (char *)calloc(64, sizeof(char));
    
    //signals
    if (ramp->PEDButtonHeight > 0)
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
			basic = strcat(basic, selectCode(DetectableWarning, codeSystem));
			detail = strcat(detail, selectCode(DetectableWarning, codeSystem));
        }
    }
    
    int flagged = 0;
    
    //dimensions
    if (ramp->rampWidth > 0)
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
    
    if (ramp->rampDepth > 0)
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
    
    value = fabsf(ramp->swkLeftSlopeSide);
    
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
    
    value = fabsf(ramp->swkRightSlopeSide);
    
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