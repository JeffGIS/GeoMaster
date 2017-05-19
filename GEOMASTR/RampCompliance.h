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
#define DetectableWarning "C|W"//no detectable warning
#define DimensionsMinor "d"//under critical value (36 inches) but over tolerance
#define DimensionsMajor "D"//under tolerance
#define SlopeMinor "e|s"//over critical value but within tolerance
#define SlopeMajor "E|S"//over tolerance
#define CracksMinor "f|c"//1/2 inch wide and under
#define CracksMajor "F|C"//over 1/2 inch wide
#define LevelChangeMinor "g|l"//1 inch and under
#define LevelChangeMajor "G|L"//over 1 inch
#define Obstructions "H|O"//obstruction(s) present

//code definitions for Detail Code
#define RampWidthMinor "d01"
#define RampWidthMajor "D01"
#define RampDepthMinor "d02"
#define RampDepthMajor "D02"
#define RampFrontMinor "e01|s01"
#define RampFrontMajor "E01|S01"
#define RampSideMinor "e02|s02"
#define RampSideMajor "E02|S02"
#define UpperLandingFrontMinor "e03|s03"
#define UpperLandingFrontMajor "E03|S03"
#define UpperLandingSideMinor "e04|s04"
#define UpperLandingSideMajor "E04|S04"
#define StreetLandingFrontMinor "e05|s05"
#define StreetLandingFrontMajor "E05|S05"
#define StreetLandingSideMinor "e06|s06"
#define StreetLandingSideMajor "E06|S06"
#define FlareLeftFrontMinor "e07|s07"
#define FlareLeftFrontMajor "E07|S07"
#define FlareRightFrontMinor "e08|s08"
#define FlareRightFrontMajor "E08|S08"
#define SwkLeftFrontMinor "e09|s09"
#define SwkLeftFrontMajor "E09|S09"
#define SwkLeftSideMinor "e10|s10"
#define SwkLeftSideMajor "E10|S10"
#define SwkRightFrontMinor "e11|s11"
#define SwkRightFrontMajor "E11|S11"
#define SwkRightSideMinor "e12|s12"
#define SwkRightSideMajor "E12|S12"
#define RampCrackMinor "f01|c01"
#define RampCrackMajor "F01|C01"
#define UpperLandingCrackMinor "f02|c02"
#define UpperLandingCrackMajor "F02|C02"
#define StreetLandingCrackMinor "f03|c03"
#define StreetLandingCrackMajor "F03|C03"
#define SwkLeftCrackMinor "f04|c04"
#define SwkLeftCrackMajor "F04|C04"
#define SwkRightCrackMinor "f05|c05"
#define SwkRightCrackMajor "F05|C05"
#define STOCMinor "g01|l01"
#define STOCMajor "G01|L01"
#define LipMinor "g02|l02"
#define LipMajor "G02|L02"
#define RampObstruction "H01|O01"
#define UpperLandingObstruction "H02|O02"
#define StreetLandingObstruction "H03|O03"

char *rampComplianceCode(RampStruct *ramp, char **detailCode, ToleranceValues *tolerances, int codeSystem);
void setStandardToleranceValues(ToleranceValues *tolerances);
char *rampToText(int intNum,RampStruct *ramp);
const char *rampToTextHeader(int type);
int fixRampNum(int rampNum);
int NVCTextureToCode(LPSTR texture);
int NVCObstructionToCode(LPSTR obstruction);


#endif /* RampCompliance_h */