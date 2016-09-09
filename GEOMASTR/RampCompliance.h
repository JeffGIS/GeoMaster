//  RampCompliance.h
//  CCodeLibrary

#ifndef RampCompliance_h
#define RampCompliance_h

#include "graphint.h"   
#include "gmextern.h"
#include "RampStruct.h"
#include "MPIntersection.h"

//definitions for critical values
#define CVButtonHeight 42.0
#define CVDimensions 48.0
#define CV2 100 / 48.0
#define CV5 5.0
#define CV8 100.0 / 12.0
#define CV10 10.0
#define CVNone -9999.0

//code definitions for Basic Code
#define Compliant "A"
#define Signal "B"//button height not within 40-44 inches (target is 42 inches)
#define DetectableWarning "C"//no detectable warning
#define DimensionsMinor "d"//under critical value (36 inches) but over tolerance
#define DimensionsMajor "D"//under tolerance
#define SlopeMinor "e"//over critical value but within tolerance
#define SlopeMajor "E"//over tolerance
#define CracksMinor "f"//1/2 inch wide and under
#define CracksMajor "F"//over 1/2 inch wide
#define LevelChangeMinor "g"//1 inch and under
#define LevelChangeMajor "G"//over 1 inch
#define Obstructions "H"//obstruction(s) present

//code definitions for Detail Code
#define RampWidthMinor "d01"
#define RampWidthMajor "D01"
#define RampDepthMinor "d02"
#define RampDepthMajor "D02"
#define RampFrontMinor "e01"
#define RampFrontMajor "E01"
#define RampSideMinor "e02"
#define RampSideMajor "E02"
#define UpperLandingFrontMinor "e03"
#define UpperLandingFrontMajor "E03"
#define UpperLandingSideMinor "e04"
#define UpperLandingSideMajor "E04"
#define StreetLandingFrontMinor "e05"
#define StreetLandingFrontMajor "E05"
#define StreetLandingSideMinor "e06"
#define StreetLandingSideMajor "E06"
#define FlareLeftFrontMinor "e07"
#define FlareLeftFrontMajor "E07"
#define FlareRightFrontMinor "e08"
#define FlareRightFrontMajor "E08"
#define SwkLeftFrontMinor "e09"
#define SwkLeftFrontMajor "E09"
#define SwkLeftSideMinor "e10"
#define SwkLeftSideMajor "E10"
#define SwkRightFrontMinor "e11"
#define SwkRightFrontMajor "E11"
#define SwkRightSideMinor "e12"
#define SwkRightSideMajor "E12"
#define RampCrackMinor "f01"
#define RampCrackMajor "F01"
#define UpperLandingCrackMinor "f02"
#define UpperLandingCrackMajor "F02"
#define StreetLandingCrackMinor "f03"
#define StreetLandingCrackMajor "F03"
#define SwkLeftCrackMinor "f04"
#define SwkLeftCrackMajor "F04"
#define SwkRightCrackMinor "f05"
#define SwkRightCrackMajor "F05"
#define STOCMinor "g01"
#define STOCMajor "G01"
#define LipMinor "g02"
#define LipMajor "G02"
#define RampObstruction "H01"
#define UpperLandingObstruction "H02"
#define StreetLandingObstruction "H03"

char *rampComplianceCode(RampStruct *ramp, char **detailCode, ToleranceValues *tolerances);
void setStandardToleranceValues(ToleranceValues *tolerances);
char *rampToText(int intNum,RampStruct *ramp);
const char *rampToTextHeader(void);
int fixRampNum(int rampNum);

#endif /* RampCompliance_h */