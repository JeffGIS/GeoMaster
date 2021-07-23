//  RampCompliance.c
//  CCodeLibrary

#include "RampCompliance.h"

static char majorFields[1024] = "|";
static char minorFields[1024] = "|";

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
BOOL CCodeToFile(LPSTR code, LPSTR File)
{
	BOOL rtn = FALSE;
	HFILE fid = GSSiOpenFile(File, 0, OF_CREATE);

	if (fid != HFILE_ERROR)
	{
		rtn = TRUE;
		fputstring("CCODE", fid);
		while (*code)
		{
			if (*code == 'W')
			{
				fputstring("W", fid);
				code++;
			}
			else if (*code == 'B')
			{
				fputstring("B", fid);
				code++;
			}
			else
			{
				char savec = code[3];
				code[3] = 0;
				fputstring(code, fid);
				code[3] = savec;
				code+=3;
			}
		}
		GSSiClose(fid);
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
    
	strcpy (majorFields,"|");
	strcpy (minorFields,"|");
    //signals
    if (ramp->PEDButtonType > 0)
    {
        if (ramp->PEDButtonHeight < (CVButtonHeight - tolerances->buttonHeight) ||
            ramp->PEDButtonHeight > (CVButtonHeight + tolerances->buttonHeight))
        {
			basic = strcat(basic, selectCode(Signal, codeSystem));
			detail = strcat(detail, selectCode(Signal, codeSystem));
			sprintf(strchr(majorFields, 0), "%s|", "PEDButtonHeight");
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
				sprintf(strchr(majorFields, 0), "%s|", "texture");
			}
        }
    }
    
    int flagged = 0;
    
    //dimensions
    if (ramp->rampWidth > -1)
    {
        if (ramp->rampWidth < (CVDimensions - tolerances->dimensions))
        {
			sprintf(strchr(majorFields, 0), "%s|", "rampWidth");
			basic = strcat(basic, selectCode(DimensionsMajor, codeSystem));
			detail = strcat(detail, selectCode(RampWidthMajor, codeSystem));
            flagged = 2;
        }
        
        else if (ramp->rampWidth < CVDimensions)
        {
			sprintf(strchr(minorFields, 0), "%s|", "rampWidth");
			basic = strcat(basic, selectCode(DimensionsMinor, codeSystem));
			detail = strcat(detail, selectCode(RampWidthMinor, codeSystem));
            flagged = 1;
        }
    }
    
    if (ramp->rampDepth > -1)
    {
        if (ramp->rampDepth < (CVDimensions - tolerances->dimensions))
        {
			sprintf(strchr(majorFields, 0), "%s|", "rampDepth");
			if (flagged < 2)
            {
                if (flagged == 1)
                    basic[strlen(basic) - 1] = '\0';
                
				basic = strcat(basic, selectCode(DimensionsMajor, codeSystem));
            }
            
			detail = strcat(detail, selectCode(RampDepthMajor, codeSystem));
        }
        
        else if (ramp->rampDepth < CVDimensions)
        {
			sprintf(strchr(minorFields, 0), "%s|", "rampDepth");
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
		sprintf(strchr(majorFields, 0), "%s|", "rampSlopeFront");
		basic = strcat(basic, selectCode(SlopeMajor, codeSystem));
		detail = strcat(detail, selectCode(RampFrontMajor, codeSystem));
        flagged = 2;
    }
    
    else if (value > CV8 && value < 9990.0)
    {
		sprintf(strchr(minorFields, 0), "%s|", "rampSlopeFront");
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
		sprintf(strchr(majorFields, 0), "%s|", "rampSlopeSide");
	}
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		detail = strcat(detail, selectCode(RampSideMinor, codeSystem));
		sprintf(strchr(minorFields, 0), "%s|", "rampSlopeSide");
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
        
		sprintf(strchr(majorFields, 0), "%s|", "upperLandingSlopeFront");
		detail = strcat(detail, selectCode(UpperLandingFrontMajor, codeSystem));
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		sprintf(strchr(minorFields, 0), "%s|", "upperLandingSlopeFront");
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
        
		sprintf(strchr(majorFields, 0), "%s|", "upperLandingSlopeSide");
		detail = strcat(detail, selectCode(UpperLandingSideMajor, codeSystem));
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		sprintf(strchr(minorFields, 0), "%s|", "upperLandingSlopeSide");
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
        
		sprintf(strchr(majorFields, 0), "%s|", "streetLandingSlopeFront");
		detail = strcat(detail, selectCode(StreetLandingFrontMajor, codeSystem));
    }
    
    else if (value > CV5 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		sprintf(strchr(minorFields, 0), "%s|", "streetLandingSlopeFront");
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
        
		sprintf(strchr(majorFields, 0), "%s|", "streetLandingSlopeSide");
		detail = strcat(detail, selectCode(StreetLandingSideMajor, codeSystem));
    }
    
    else if (value > CV5 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		sprintf(strchr(minorFields, 0), "%s|", "streetLandingSlopeSide");
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
        
		sprintf(strchr(majorFields, 0), "%s|", "flareLeftSlopeFront");
		detail = strcat(detail, selectCode(FlareLeftFrontMajor, codeSystem));
    }
    
    else if (value > CV10 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		sprintf(strchr(minorFields, 0), "%s|", "flareLeftSlopeFront");
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
        
		sprintf(strchr(majorFields, 0), "%s|", "flareRightSlopeFront");
		detail = strcat(detail, selectCode(FlareRightFrontMajor, codeSystem));
    }
    
    else if (value > CV10 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		sprintf(strchr(minorFields, 0), "%s|", "flareRightSlopeFront");
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
            
			sprintf(strchr(majorFields, 0), "%s|", "swkLeftSlopeFront");
			detail = strcat(detail, SwkLeftFrontMajor);
        }
        
        else if (value > CV8 && value < 9990.0)
        {
            if (flagged < 1)
            {
                basic = strcat(basic, SlopeMinor);
                flagged = 1;
            }
            
			sprintf(strchr(minorFields, 0), "%s|", "swkLeftSlopeFront");
			detail = strcat(detail, selectCode(SwkLeftFrontMinor, codeSystem));
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
        
			sprintf(strchr(majorFields, 0), "%s|", "swkLeftSlopeFront");
			detail = strcat(detail, selectCode(SwkLeftFrontMajor, codeSystem));
	    }
    
        else if (value > CV5 && value < 9990.0)
    	{
	        if (flagged < 1)
        	{
				basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
	            flagged = 1;
        	}
        
			sprintf(strchr(minorFields, 0), "%s|", "swkLeftSlopeFront");
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
        
		sprintf(strchr(majorFields, 0), "%s|", "swkLeftSlopeSide");
		detail = strcat(detail, selectCode(SwkLeftSideMajor, codeSystem));
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		sprintf(strchr(minorFields, 0), "%s|", "swkLeftSlopeSide");
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
        
			sprintf(strchr(majorFields, 0), "%s|", "swkRightSlopeFront");
			detail = strcat(detail, selectCode(SwkRightFrontMajor, codeSystem));
    }
    
    else if (value > CV5 && value < 9990.0)
    {
        if (flagged < 1)
        {
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
            flagged = 1;
        }
        
		sprintf(strchr(minorFields, 0), "%s|", "swkRightSlopeFront");
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
            
			sprintf(strchr(majorFields, 0), "%s|", "swkRightSlopeFront");
			detail = strcat(detail, selectCode(SwkRightFrontMajor, codeSystem));
        }
        
        else if (value > CV5 && value < 9990.0)
        {
            if (flagged < 1)
            {
                basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
                flagged = 1;
            }
            
			sprintf(strchr(minorFields, 0), "%s|", "swkRightSlopeFront");
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
        
		sprintf(strchr(majorFields, 0), "%s|", "swkRightSlopeSide");
		detail = strcat(detail, selectCode(SwkRightSideMajor, codeSystem));
    }
    
    else if (value > CV2 && value < 9990.0)
    {
        if (flagged < 1)
			basic = strcat(basic, selectCode(SlopeMinor, codeSystem));
        
		sprintf(strchr(minorFields, 0), "%s|", "swkRightSlopeSide");
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
			sprintf(strchr(majorFields, 0), "%s|", "rampCrackWidth");
		}
        
        else
        {
			basic = strcat(basic, selectCode(CracksMinor, codeSystem));
			detail = strcat(detail, selectCode(RampCrackMinor, codeSystem));
            flagged = 1;
			sprintf(strchr(minorFields, 0), "%s|", "rampCrackWidth");
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
			sprintf(strchr(majorFields, 0), "%s|", "upperLandingCrackWidth");
		}
        
        else
        {
            if (flagged < 1)
            {
				basic = strcat(basic, selectCode(CracksMinor, codeSystem));
                flagged = 1;
            }
            
			detail = strcat(detail, selectCode(UpperLandingCrackMinor, codeSystem));
			sprintf(strchr(minorFields, 0), "%s|", "upperLandingCrackWidth");
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
			sprintf(strchr(majorFields, 0), "%s|", "streetLandingCrackWidth");
		}
        
        else
        {
            if (flagged < 1)
            {
				basic = strcat(basic, selectCode(CracksMinor, codeSystem));
                flagged = 1;
            }
            
			detail = strcat(detail, selectCode(StreetLandingCrackMinor, codeSystem));
			sprintf(strchr(minorFields, 0), "%s|", "streetLandingCrackWidth");
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
			sprintf(strchr(majorFields, 0), "%s|", "leftSidewalkCrackWidth");
		}
        
        else
        {
            if (flagged < 1)
            {
				basic = strcat(basic, selectCode(CracksMinor, codeSystem));
                flagged = 1;
            }
            
			detail = strcat(detail, selectCode(SwkLeftCrackMinor, codeSystem));
			sprintf(strchr(minorFields, 0), "%s|", "leftSidewalkCrackWidth");
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
			sprintf(strchr(majorFields, 0), "%s|", "rightSidewalkCrackWidth");
		}
        
        else
        {
            if (flagged < 1)
            {
				basic = strcat(basic, selectCode(CracksMinor, codeSystem));
                flagged = 1;
            }
            
			detail = strcat(detail, selectCode(SwkRightCrackMinor, codeSystem));
			sprintf(strchr(minorFields, 0), "%s|", "rightSidewalkCrackWidth");
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
			sprintf(strchr(majorFields, 0), "%s|", "SteepTopOfCurb");
		}
        
        else
        {
			basic = strcat(basic, selectCode(LevelChangeMinor, codeSystem));
			detail = strcat(detail, selectCode(STOCMinor, codeSystem));
            flagged = 1;
			sprintf(strchr(minorFields, 0), "%s|", "SteepTopOfCurb");
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
            
			sprintf(strchr(majorFields, 0), "%s|", "PedRampLip");
			detail = strcat(detail, selectCode(LipMajor, codeSystem));
        }
        
        else
        {
            if (flagged < 1)
            {
				basic = strcat(basic, selectCode(LevelChangeMinor, codeSystem));
                flagged = 1;
            }
            
			sprintf(strchr(minorFields, 0), "%s|", "PedRampLip");
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
			sprintf(strchr(majorFields, 0), "%s|", "RampObstruction");
		}
    }
    
    if (ramp->upperLandingObstruction > 0)
    {
        if (ramp->upperLandingObstruction < none)
        {
            if (flagged < 1)
				basic = strcat(basic, selectCode(Obstructions, codeSystem));
            
			detail = strcat(detail, selectCode(UpperLandingObstruction, codeSystem));
			sprintf(strchr(majorFields, 0), "%s|", "UpperLandingObstruction");
		}
    }
    
    if (ramp->lowerLandingObstruction > 0)
    {
        if (ramp->lowerLandingObstruction < none)
        {
            if (flagged < 1)
				basic = strcat(basic, selectCode(Obstructions, codeSystem));
            
			detail = strcat(detail, selectCode(StreetLandingObstruction, codeSystem));
			sprintf(strchr(majorFields, 0), "%s|", "lowerLandingObstruction");
		}
    }
    
    //mark as compliant if no flags have been added
    if (strlen(basic) == 0)
    {
		basic = strcat(basic, selectCode(Compliant, codeSystem));
		detail = strcat(detail, selectCode(Compliant, codeSystem));
    }
    
    *detailCode = detail;
	strlwr(majorFields);
	strlwr(minorFields);
    return basic;
}
void ConvertRampDBFieldToDisplayField(LPSTR FieldName, int maxl)
{
	REPLAC(FieldName, "rampDepth", "UpperLandingDepth", maxl);
	REPLAC(FieldName, "PedRampLip", "LipAtFlowLine", maxl);
	REPLAC(FieldName, "swkLeft", "SidewalkLeft", maxl);
	REPLAC(FieldName, "swkRight", "SidewalkRight", maxl);
	REPLAC(FieldName, "SlopeFront", "RunningSlope", maxl);
	REPLAC(FieldName, "SlopeSide", "CrossSlope", maxl);
	REPLAC(FieldName, "lowerLandingObstruction", "StreetLandingObstruction", maxl);	
}
void ConvertRampDisplayFieldToDBField(LPSTR FieldName, int maxl)
{
	REPLAC(FieldName, "UpperLandingDepth", "rampDepth", maxl);
	REPLAC(FieldName, "LipAtFlowLine", "PedRampLip", maxl);
	REPLAC(FieldName, "SidewalkLeft", "swkLeft", maxl);
	REPLAC(FieldName, "SidewalkRight", "swkRight", maxl);
	REPLAC(FieldName, "RunningSlope", "SlopeFront", maxl);
	REPLAC(FieldName, "CrossSlope", "SlopeSide", maxl);
	REPLAC(FieldName, "StreetLandingObstruction", "lowerLandingObstruction", maxl);
	REPLAC(FieldName, "Condition Rating", "condition", maxl);
	REPLAC(FieldName, "Surface Material", "material", maxl);
	REPLAC(FieldName, "Point Type", "type", maxl);
	REPLAC(FieldName, "Crack Width", "crack_width", maxl);
	REPLAC(FieldName, "Level Change", "level_change", maxl);
	REPLAC(FieldName, "Sidewalk Width", "width", maxl);
	REPLAC(FieldName, "Boulevard Width", "boulevardWidth", maxl);
	REPLAC(FieldName, "Boulevard Material", "boulevardMaterial", maxl);
}
COLORREF GetRampFieldValueColor(LPSTR DisplayName)
{
	COLORREF c = 0;
	COLORREF majorProblem = RGB(255, 0, 0);
	COLORREF minorProblem = RGB(0, 255, 0);
	char searchField[256];

	ConvertRampDisplayFieldToDBField(DisplayName,255);
	strlwr(DisplayName);
	sprintf(searchField, "|%s|", DisplayName);
	if (strstr(majorFields, searchField))
		c = majorProblem;
	else if(strstr(minorFields, searchField))
		c = minorProblem;
	return c;
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
static char *rampTypes[] = {"Perp", "PerpNonWalk", "CombPerpWalk", "CombPerpNonWalk", "OneWayPerp", "OneWayDirBlendTrans", "Parallel", "DepressedCorner", "Fan", "BuiltUp", "Diagonal", "OneWayDirCurbGutter","NoRamp", "CombLeftWalk", "CombRightWalk", "CombLeftNonWalk", "CombRightNonWalk"};
static char *AWITypes[] = {"None", "Tones", "SpeechMessage"};
static char *buttonTypes[] = {"None", "SmallPush", "LargePush", "Touch", "APS"};
static char *signalTypes[] = {"None", "Text", "Symbol", "SideTimer", "BelowTimer"};
static char *conditionCode[] = { "Unknown","New","Good","Fair","Poor","Unacceptable" };
static char* sidewalkMaterial[] = { "Unknown","Concrete", "Asphalt", "Brick Pavers","Interlocking Pavers","Other" };
static char* boulevardMaterial[] = {"Unknown","Concrete", "Grass","Dirt","Other" };
static char* sidewalkPointTypes[] = { "Sidewalk",//"Lower Landing","Ramp","Upper Landing","Crosswalk","Island","Driveway Crossing","Driveway Flare","Driveway Apron","Slope Change"};
"Ramp", "Upper Landing","Slope Change","Displacement","Cracks","Obstruction","Driveway Crossing","Driveway Flare","Driveway Apron","Lower Landing","Crosswalk", "Island"};

void RampTypeFromCode(int code, LPSTR OutLoc)
{
	int maxType = sizeof(rampTypes) / 4;

	*OutLoc = 0;
	if (code > 100)
		code -= 100;
	if (code >= 0 && code < maxType)
		strcpy(OutLoc, rampTypes[code]);
	return;
}

void ObstructionFromCode(int code, LPSTR OutLoc)
{
	int maxType = sizeof(obstructions) / 4;

	*OutLoc = 0;
	if (code >= 0 && code < maxType)
		strcpy(OutLoc, obstructions[code]);
	return;
}
void TextureFromCode(int code, LPSTR OutLoc)
{
	int maxType = sizeof(textures) / 4;

	*OutLoc = 0;
	if (code >= 0 && code < maxType)
		strcpy(OutLoc, textures[code]);
	return;
}
void ConditionFromCode(int code, LPSTR OutLoc)
{
	int maxType = sizeof(conditionCode) / 4;

	*OutLoc = 0;
	if (code >= 0 && code < maxType)
		strcpy(OutLoc, conditionCode[code]);
	return;
}
void MaterialFromCode(int code, LPSTR OutLoc)
{
	int maxType = sizeof(sidewalkMaterial) / 4;

	*OutLoc = 0;
	if (code >= 0 && code < maxType)
		strcpy(OutLoc, sidewalkMaterial[code]);
	return;
}void BoulevardMaterialFromCode(int code, LPSTR OutLoc)
{
	int maxType = sizeof(boulevardMaterial) / 4;

	*OutLoc = 0;
	if (code >= 0 && code < maxType)
		strcpy(OutLoc, boulevardMaterial[code]);
	return;
}
void PointTypeFromCode(int code, LPSTR OutLoc)
{
	int maxType = sizeof(sidewalkPointTypes) / 4;

	*OutLoc = 0;
	if (code >= 0 && code < maxType)
		strcpy(OutLoc, sidewalkPointTypes[code]);
	return;
}

void GetTextureList(LPSTR OutLoc)
{
	char delim[2] = "";
	*OutLoc = 0;
	for (int i = 0; i < sizeof(textures) / 4; i++)
	{
		if (i && (!*textures[i] || !stricmp(textures[i], "None")))
			continue;
		sprintf (strchr(OutLoc,0), "%s%s",delim,textures[i]);
		*delim = ',';
	}
	return;
}
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
void GetObstructionList(LPSTR OutLoc)
{
	char delim[2] = "";
	*OutLoc = 0;
	for (int i = 0; i < sizeof(obstructions) / 4; i++)
	{
		if (i && (!*obstructions[i] || !stricmp(obstructions[i], "None")))
			continue;
		sprintf(strchr(OutLoc, 0), "%s%s", delim, obstructions[i]);
		*delim = ',';
	}
	return;
}
int NVCObstructionToCode(LPSTR obstruction)
{
	if (!strlen(obstruction))
		return 0;
	for (int i = 0; i < sizeof(obstructions) / 4; i++)
	{
		if (!strcasecmp(obstruction, obstructions[i]))
			return i;
	}
	return 0;
}
int NVCMaterialToCode(LPSTR material)
{
	if (!strlen(material))
		return 0;
	for (int i = 0; i < sizeof(sidewalkMaterial) / 4; i++)
	{
		if (!strcasecmp(material, sidewalkMaterial[i]))
			return i;
	}
	return 0;
}
int NVCPointTypeToCode(LPSTR material)
{
	if (!strlen(material))
		return 0;
	for (int i = 0; i < sizeof(sidewalkPointTypes) / 4; i++)
	{
		if (!strcasecmp(material, sidewalkPointTypes[i]))
			return i;
	}
	return 0;
}
int NVCConditionToCode(LPSTR condition)
{
	if (!strlen(condition))
		return 0;
	for (int i = 0; i < sizeof(conditionCode) / 4; i++)
	{
		if (!strcasecmp(condition, conditionCode[i]))
			return i;
	}
	return 0;
}
void GetRampTypesList(LPSTR OutLoc)
{
	char delim[2] = "";
	*OutLoc = 0;
	for (int i = 0; i < sizeof(rampTypes) / 4; i++)
	{
		sprintf(strchr(OutLoc, 0), "%s%s", delim, rampTypes[i]);
		*delim = ',';
	}
	return;
}
void GetPointTypeList(LPSTR OutLoc)
{
	char delim[2] = "";
	*OutLoc = 0;
	for (int i = 0; i < sizeof(sidewalkPointTypes) / 4; i++)
	{
		sprintf(strchr(OutLoc, 0), "%s%s", delim, sidewalkPointTypes[i]);
		*delim = ',';
	}
	return;
}
void GetConditionCodeList(LPSTR OutLoc)
{
	char delim[2] = "";
	*OutLoc = 0;
	for (int i = 0; i < sizeof(conditionCode) / 4; i++)
	{
		sprintf(strchr(OutLoc, 0), "%s%s", delim, conditionCode[i]);
		*delim = ',';
	}
	return;
}
void GetMaterialCodeList(LPSTR OutLoc)
{
	char delim[2] = "";
	*OutLoc = 0;
	for (int i = 0; i < sizeof(sidewalkMaterial) / 4; i++)
	{
		sprintf(strchr(OutLoc, 0), "%s%s", delim, sidewalkMaterial[i]);
		*delim = ',';
	}
	return;
}
void GetBoulevardMaterialCodeList(LPSTR OutLoc)
{
	char delim[2] = "";
	*OutLoc = 0;
	for (int i = 0; i < sizeof(boulevardMaterial) / 4; i++)
	{
		sprintf(strchr(OutLoc, 0), "%s%s", delim, boulevardMaterial[i]);
		*delim = ',';
	}
	return;
}
int NVCRampTypeToCode(LPSTR rampType)
{
	int rtn = 0;
	if (!strlen(rampType))
		return 0;
	for (int i = 0; i < sizeof(rampTypes) / 4; i++)
	{
		if (!strcasecmp(rampType, rampTypes[i]))
			rtn = i;
	}
	if (rtn > 12)
		rtn = 100 + rtn - 12;
	return rtn;
}

static BOOL NameEndsWith(LPSTR Name, LPSTR text)
{
	BOOL rtn = FALSE;
	int lName = strlen(Name);
	int lText = strlen(text);

	if (lName >= lText)
	{
		if (!stricmp(&Name[lName - lText], text))
			rtn = TRUE;
	}

	return rtn;
}
BOOL GetRampCodeForValue(LPSTR VarName, LPSTR VarValue, LPSTR ErrorVarName, LPSTR OutLoc)
{
	int rtn = 0;
	int iCode;
	char mess[256];

	if (!stricmp(VarName, "Condition"))
	{
		iCode = NVCConditionToCode(VarValue);
		itoa(iCode, OutLoc, 10);
	}
	else if (!stricmp(VarName, "Material"))
	{
		iCode = NVCMaterialToCode(VarValue);
		itoa(iCode, OutLoc, 10);
	}
	else if (!stricmp(VarName, "type"))
	{
		iCode = NVCPointTypeToCode(VarValue);
		itoa(iCode, OutLoc, 10);
	}
	else if (!stricmp(VarName, "RampType"))
	{
		iCode = NVCRampTypeToCode(VarValue);
		itoa(iCode, OutLoc, 10);
	}
	else if (!stricmp(VarName, "Texture"))
	{
		iCode = NVCTextureToCode(VarValue);
		itoa(iCode, OutLoc, 10);
	}
	else if (!stricmp(VarName, "RampWidth") || !stricmp(VarName, "RampDepth"))
	{
		if (IsInteger(VarValue))
			strcpy(OutLoc, VarValue);
		else
		{
			sprintf(mess, "Error: Non integer value '%s' for field '%s'", VarValue, VarName);
			MessageBox(0, mess, "Input Error", MB_ICONEXCLAMATION);
			SetGlobalValueLong(ErrorVarName, 1);
		}
	}
	else if (NameEndsWith(VarName, "CrackWidth"))
	{
		if (IsReal(VarValue))
			strcpy(OutLoc, VarValue);
		else
		{
			sprintf(mess, "Error: Non numeric value '%s' for field '%s'", VarValue, VarName);
			MessageBox(0, mess, "Input Error", MB_ICONEXCLAMATION);
			SetGlobalValueLong(ErrorVarName, 1);
		}
	}
	else if (NameEndsWith(VarName, "Obstruction"))
	{
		iCode = NVCObstructionToCode(VarValue);
		itoa(iCode, OutLoc, 10);
	}
	else if (NameEndsWith(VarName, "Cracks"))
	{
		iCode = 0;
		if (atob(VarValue))
			iCode = 1;
		itoa(iCode, OutLoc, 10);
	}
	else
		strcpy(OutLoc, VarValue);
	rtn = strlen(OutLoc);
	return rtn;
}

static char YorN(int i)
{
	if (i)
		return 'Y';
	return 'N';
}

char *rampToText(int intNum, RampStruct *ramp, int codeSystem,LPSTR photos)
{
#define MAX_PHOTOS	4
	int minTime = 1338526800;
	int altIntNum;
	char intStreets[512];
	char *rampText = (char *)calloc(4480 * 2, sizeof(char));
	char timeCompleteC[32];
	int rtype = ramp->rampType;
	if (rtype > 100)
		rtype = 12 + (rtype - 100);
	if (ramp->timeComplete < minTime)
		ramp->timeComplete = minTime;
	sprintf(timeCompleteC, "$CAL(%i,3)", ramp->timeComplete);
	ExpandText(timeCompleteC);
	if (rtype < 0 || rtype > 16)
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
	GetIntersectionStreetNames("", intNum, intStreets,&altIntNum);

	ToleranceValues tolerances;
	setStandardToleranceValues(&tolerances);
	LPSTR detailCode;
	LPSTR ccode = rampComplianceCode(ramp, &detailCode, &tolerances, codeSystem);

	if (photos)
	{
		LPSTR ploc = photos;
		char photoDir[MAX_PATH];
		char photoFiles[MAX_PHOTOS][MAX_PATH] = { 0 };
		int iphoto = 0;

		GetGlobalCVal("[%PHOTODIR]",photoDir, "");
		while (*ploc)
		{
			if (iphoto < MAX_PHOTOS)
				sprintf(photoFiles[iphoto++], "%s%s", photoDir, ploc);
			ploc = strchr(ploc, 0);
			ploc++;
		}
		maxNumPhotos = max(iphoto, maxNumPhotos);
		sprintf(rampText,
			"%i\t%i\t%i\t%s\t%i\t%s\t%s\t%s\t%s\t%s\t%s\t%i\t%s\t%i\t%i\t%i\t%c\t%s\t%s\t%s\t%s\t%s\t%.10f\t%.10f\t%i\t%s\t%s\t%s\t%s\t%c\t%c\t%c\t%c\t%c\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%i\t%i\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%i\t%i\t%s\t%s\t%i\t%i\t%s\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%s\t%s\t%i",
			ramp->uniqueID,
			intNum, altIntNum,
			intStreets,
			ramp->rampNum,
			ramp->rampID,
			rampTypes[rtype],
			photoFiles[0],
			photoFiles[1],
			photoFiles[2],
			photoFiles[3],
			ramp->yearRebuilt,
			ramp->rampStatus,
			ramp->rampCode,
			ramp->proximityValue,
			ramp->proximityScore,
			ramp->retired ? 'Y' : 'N',
			detailCode,
			ccode,
			timeCompleteC,
			ramp->rampNotes,
			ramp->lastUpdate,
			ramp->latitude,
			ramp->longitude,
			ramp->rampInXWalk,
			textures[ramp->texture],
			obstructions[ramp->upperLandingObstruction],
			obstructions[ramp->lowerLandingObstruction],
			obstructions[ramp->rampObstruction],
			YorN(ramp->hasRampCracks),
			YorN(ramp->hasUpperLandingCracks),
			YorN(ramp->hasStreetLandingCracks),
			YorN(ramp->hasLeftSidewalkCracks),
			YorN(ramp->hasRightSidewalkCracks),
			ramp->crackWidth.rampCrackWidth,
			ramp->crackWidth.upperLandingCrackWidth,
			ramp->crackWidth.streetLandingCrackWidth,
			ramp->crackWidth.leftSidewalkCrackWidth,
			ramp->crackWidth.rightSidewalkCrackWidth,
			ramp->rampWidth,
			ramp->rampDepth,
			fabs(ramp->rampSlopeFront),
			fabs(ramp->rampSlopeSide),
			fabs(ramp->upperLandingSlopeFront),
			fabs(ramp->upperLandingSlopeSide),
			fabs(ramp->streetLandingSlopeFront),
			fabs(ramp->streetLandingSlopeSide),
			fabs(ramp->flareLeftSlopeFront),
			fabs(ramp->flareRightSlopeFront),
			fabs(ramp->swkLeftSlopeFront),
			fabs(ramp->swkLeftSlopeSide),
			fabs(ramp->swkRightSlopeFront),
			fabs(ramp->swkRightSlopeSide),
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
			ramp->fileID,
			ramp->cornerID
		);
	}
	else
	{
		sprintf(rampText,
			"%i\t%i\t%i\t%s\t%i\t%s\t%s\t%i\t%s\t%i\t%i\t%i\t%c\t%s\t%s\t%s\t%s\t%s\t%.10f\t%.10f\t%i\t%s\t%s\t%s\t%s\t%c\t%c\t%c\t%c\t%c\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%i\t%i\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%i\t%i\t%s\t%s\t%i\t%i\t%s\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%s\t%s\t%i",
			ramp->uniqueID,
			intNum, altIntNum,
			intStreets,
			ramp->rampNum,
			ramp->rampID,
			rampTypes[rtype],
			ramp->yearRebuilt,
			ramp->rampStatus,
			ramp->rampCode,
			ramp->proximityValue,
			ramp->proximityScore,
			ramp->retired ? 'Y' : 'N',
			detailCode,
			ccode,
			timeCompleteC,
			ramp->rampNotes,
			ramp->lastUpdate,
			ramp->latitude,
			ramp->longitude,
			ramp->rampInXWalk,
			textures[ramp->texture],
			obstructions[ramp->upperLandingObstruction],
			obstructions[ramp->lowerLandingObstruction],
			obstructions[ramp->rampObstruction],
			YorN(ramp->hasRampCracks),
			YorN(ramp->hasUpperLandingCracks),
			YorN(ramp->hasStreetLandingCracks),
			YorN(ramp->hasLeftSidewalkCracks),
			YorN(ramp->hasRightSidewalkCracks),
			ramp->crackWidth.rampCrackWidth,
			ramp->crackWidth.upperLandingCrackWidth,
			ramp->crackWidth.streetLandingCrackWidth,
			ramp->crackWidth.leftSidewalkCrackWidth,
			ramp->crackWidth.rightSidewalkCrackWidth,
			ramp->rampWidth,
			ramp->rampDepth,
			fabs(ramp->rampSlopeFront),
			fabs(ramp->rampSlopeSide),
			fabs(ramp->upperLandingSlopeFront),
			fabs(ramp->upperLandingSlopeSide),
			fabs(ramp->streetLandingSlopeFront),
			fabs(ramp->streetLandingSlopeSide),
			fabs(ramp->flareLeftSlopeFront),
			fabs(ramp->flareRightSlopeFront),
			fabs(ramp->swkLeftSlopeFront),
			fabs(ramp->swkLeftSlopeSide),
			fabs(ramp->swkRightSlopeFront),
			fabs(ramp->swkRightSlopeSide),
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
			ramp->fileID,
			ramp->cornerID
		);
	}
	free(ccode);
	free(detailCode);

    return rampText;
}

const char *rampToTextHeader(int type, BOOL havePhotos)
{
	if (!type)
	{
		if (havePhotos)
			return "UniqueRampID\tIntersectionNum\tAlternateIntersectionNum\tStreet Names\tRampNum\tRampID\tRampType\tYearBuilt\tPhoto1\tPhoto2\tPhoto3\tPhoto4\tRampStatus\tRampCode\tProximityValue\tProximityScore\tRetired\tComplianceCodeDetail\tComplianceCodeSummary\tTimeComplete\tRampNotes\tLastUpdate\tLatitude\tLongitude\tRampInXWalk\tTexture\tUpperLandingObstruction\tStreetLandingObstruction\tRampObstruction\tHasRampCracks\tHasUpperLandingCracks\tHasStreetLandingCracks\tHasLeftSidewalkCracks\tHasRightSidewalkCracks\tRampCrackWidth\tUpperLandingCrackWidth\tStreetLandingCrackWidth\tLeftSidewalkCrackWidth\tRightSidewalkCrackWidth\tRampWidth\tUpperLandingDepth\tRampRunningSlope\tRampCrossSlope\tUpperLandingRunningSlope\tUpperLandingCrossSlope\tStreetLandingRunningSlope\tStreetLandingCrossSlope\tFlareLeftRunningSlope\tFlareRightRunningSlope\tSidewalkLeftRunningSlope\tSidewalkLeftCrossSlope\tSidewalkRightRunningSlope\tSidewalkRightCrossSlope\tSteepTopOfCurb\tLipAtFlowLine\tCurbCutDistance\tBumpWidth\tBumpHeight\tDetectableWarningWidth\tDetectableWarningDepth\tPEDSignalType\tPEDButtonType\tPEDButtonHeight\tPEDButtonDistance\tPEDButtonAWIType\tPEDButtonHasLocatorTone\tPEDButtonHasInfoSign\tPEDButtonHasBraille\tPEDButtonHasTactileArrow\tPEDButtonLocatorToneVolume\tPEDButtonAWIVolume\tRampComment\tSourceFile\tCornerID";
		else
			return "UniqueRampID\tIntersectionNum\tAlternateIntersectionNum\tStreet Names\tRampNum\tRampID\tRampType\tYearBuilt\tRampStatus\tRampCode\tProximityValue\tProximityScore\tRetired\tComplianceCodeDetail\tComplianceCodeSummary\tTimeComplete\tRampNotes\tLastUpdate\tLatitude\tLongitude\tRampInXWalk\tTexture\tUpperLandingObstruction\tStreetLandingObstruction\tRampObstruction\tHasRampCracks\tHasUpperLandingCracks\tHasStreetLandingCracks\tHasLeftSidewalkCracks\tHasRightSidewalkCracks\tRampCrackWidth\tUpperLandingCrackWidth\tStreetLandingCrackWidth\tLeftSidewalkCrackWidth\tRightSidewalkCrackWidth\tRampWidth\tUpperLandingDepth\tRampRunningSlope\tRampCrossSlope\tUpperLandingRunningSlope\tUpperLandingCrossSlope\tStreetLandingRunningSlope\tStreetLandingCrossSlope\tFlareLeftRunningSlope\tFlareRightRunningSlope\tSidewalkLeftRunningSlope\tSidewalkLeftCrossSlope\tSidewalkRightRunningSlope\tSidewalkRightCrossSlope\tSteepTopOfCurb\tLipAtFlowLine\tCurbCutDistance\tBumpWidth\tBumpHeight\tDetectableWarningWidth\tDetectableWarningDepth\tPEDSignalType\tPEDButtonType\tPEDButtonHeight\tPEDButtonDistance\tPEDButtonAWIType\tPEDButtonHasLocatorTone\tPEDButtonHasInfoSign\tPEDButtonHasBraille\tPEDButtonHasTactileArrow\tPEDButtonLocatorToneVolume\tPEDButtonAWIVolume\tRampComment\tSourceFile\tCornerID";
	}
	else
	{
		if (havePhotos)
			return "UniqueRampID(B4)\tIntersectionNum(B4)\tAlternateIntersectionNum(B4)\tStreet Names(C255)\tRampNum(B4)\tRampID(C16)\tRampType(C32)\tYearBuilt(B2)\tPhoto1(C32)\tPhoto2(C32)\tPhoto3(C32)\tPhoto4(C32)\tRampStatus(C256)\tRampCode(B2)\tProximityValue(B4)\tProximityScore(B2)\tRetired(C1)\tComplianceCodeDetail(C100)\tComplianceCodeSummary(C32)\tTimeComplete(C16)\tRampNotes(C255)\tLastUpdate(C64)\tLatitude(R8)\tLongitude(R8)\tRampInXWalk(B2)\tTexture(C40)\tUpperLandingObstruction(C40)\tStreetLandingObstruction(C40)\tRampObstruction(C40)\tHasRampCracks(B2)\tHasUpperLandingCracks(B2)\tHasStreetLandingCracks(B2)\tHasLeftSidewalkCracks(B2)\tHasRightSidewalkCracks(B2)\tRampCrackWidth(R4)\tUpperLandingCrackWidth(R4)\tStreetLandingCrackWidth(R4)\tLeftSidewalkCrackWidth(R4)\tRightSidewalkCrackWidth(R4)\tRampWidth(B2)\tUpperLandingDepth(B2)\tRampRunningSlope(R4)\tRampCrossSlope(R4)\tUpperLandingRunningSlope(R4)\tUpperLandingCrossSlope(R4)\tStreetLandingRunningSlope(R4)\tStreetLandingCrossSlope(R4)\tFlareLeftRunningSlope(R4)\tFlareRightRunningSlope(R4)\tSidewalkLeftRunningSlope(R4)\tSidewalkLeftCrossSlope(R4)\tSidewalkRightRunningSlope(R4)\tSidewalkRightCrossSlope(R4)\tSteepTopOfCurb(R4)\tLipAtFlowLine(R4)\tCurbCutDistance(R4)\tBumpWidth(R4)\tBumpHeight(R4)\tDetectableWarningWidth(B4)\tDetectableWarningDepth(B4)\tPEDSignalType(C32)\tPEDButtonType(C32)\tPEDButtonHeight(B2)\tPEDButtonDistance(B2)\tPEDButtonAWIType(C16)\tPEDButtonHasLocatorTone(B2)\tPEDButtonHasInfoSign(B2)\tPEDButtonHasBraille(B2)\tPEDButtonHasTactileArrow(B2)\tPEDButtonLocatorToneVolume(B2)\tPEDButtonAWIVolume(B2)\tRampComment(C255)\tSourceFile(C12)\tCornerID";
		else
			return "UniqueRampID(B4)\tIntersectionNum(B4)\tAlternateIntersectionNum(B4)\tStreet Names(C255)\tRampNum(B4)\tRampID(C16)\tRampType(C32)\tYearBuilt(B2)\tRampStatus(C256)\tRampCode(B2)\tProximityValue(B4)\tProximityScore(B2)\tRetired(C1)\tComplianceCodeDetail(C100)\tComplianceCodeSummary(C32)\tTimeComplete(C16)\tRampNotes(C255)\tLastUpdate(C64)\tLatitude(R8)\tLongitude(R8)\tRampInXWalk(B2)\tTexture(C40)\tUpperLandingObstruction(C40)\tStreetLandingObstruction(C40)\tRampObstruction(C40)\tHasRampCracks(B2)\tHasUpperLandingCracks(B2)\tHasStreetLandingCracks(B2)\tHasLeftSidewalkCracks(B2)\tHasRightSidewalkCracks(B2)\tRampCrackWidth(R4)\tUpperLandingCrackWidth(R4)\tStreetLandingCrackWidth(R4)\tLeftSidewalkCrackWidth(R4)\tRightSidewalkCrackWidth(R4)\tRampWidth(B2)\tUpperLandingDepth(B2)\tRampRunningSlope(R4)\tRampCrossSlope(R4)\tUpperLandingRunningSlope(R4)\tUpperLandingCrossSlope(R4)\tStreetLandingRunningSlope(R4)\tStreetLandingCrossSlope(R4)\tFlareLeftRunningSlope(R4)\tFlareRightRunningSlope(R4)\tSidewalkLeftRunningSlope(R4)\tSidewalkLeftCrossSlope(R4)\tSidewalkRightRunningSlope(R4)\tSidewalkRightCrossSlope(R4)\tSteepTopOfCurb(R4)\tLipAtFlowLine(R4)\tCurbCutDistance(R4)\tBumpWidth(R4)\tBumpHeight(R4)\tDetectableWarningWidth(B4)\tDetectableWarningDepth(B4)\tPEDSignalType(C32)\tPEDButtonType(C32)\tPEDButtonHeight(B2)\tPEDButtonDistance(B2)\tPEDButtonAWIType(C16)\tPEDButtonHasLocatorTone(B2)\tPEDButtonHasInfoSign(B2)\tPEDButtonHasBraille(B2)\tPEDButtonHasTactileArrow(B2)\tPEDButtonLocatorToneVolume(B2)\tPEDButtonAWIVolume(B2)\tRampComment(C255)\tSourceFile(C12)\tCornerID";
	}
}
