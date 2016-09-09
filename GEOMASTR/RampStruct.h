//  RampStruct.h
//  CCodeLibrary

#ifndef RampStruct_h
#define RampStruct_h

#include "stdbool.h"

enum textures {
    smoothConcrete=1,
    brushedConcrete=2,
    tintedConcrete=3,
    truncatedStoneDomes=4,
    truncatedStampedConcrete=5,
    castironTruncatedDomes=6,
    exposedAggregate=7,
    cutStone=8,
    other=10
};

enum obstructions {
    hydrant=1,
    manhole=2,
    polebox=3,
    pole=4,
    streetmanhole=5,
    otherob=6,
    none=10,
    masterNone=11
};

typedef enum {
    RampTypePerp,
    RampTypePerpNonWalk,
    RampTypeCombPerpWalk,
    RampTypeCombPerpNonWalk,
    RampTypeOneWayDirCurbGutter,
    RampTypeOneWayDirBlendTrans,
    RampTypeParallel,
    RampTypeDepressedCorner,
    RampTypeFan,
    RampTypeBuiltUp,
    RampTypeDiagonal,
    RampTypeCombLeftWalk,//left side of RampTypeCombPerpWalk
    RampTypeCombRightWalk,//right side of RampTypeCombPerpWalk
    RampTypeCombLeftNonWalk,//left side of RampTypeCombPerpNonWalk
    RampTypeCombRightNonWalk,//right side of RampTypeCombPerpNonWalk
    rampTypeCount //always last item
} RampType;

typedef enum {
    NVSignalTypeNone,
    NVSignalTypeText,
    NVSignalTypeSymbol,
    NVSignalTypeSideTimer,
    NVSignalTypeBelowTimer
} NVSignalType;

typedef enum {
    AudibleWalkIndicationNone,
    AudibleWalkIndicationTones,
    AudibleWalkIndicationSpeechMessage,
} AudibleWalkIndication;

typedef enum {
    NVButtonTypeNone,
    NVButtonTypeSmallPush,
    NVButtonTypeLargePush,
    NVButtonTypeTouch,
    NVButtonTypeAPS,
} NVButtonType;

typedef struct CrackWidth {
    float rampCrackWidth;
    float upperLandingCrackWidth;
    float streetLandingCrackWidth;
    float leftSidewalkCrackWidth;
    float rightSidewalkCrackWidth;
} CrackWidth;

typedef struct RampStruct {
	int uniqueID;
    int rampNum;
    char rampID[16];
    int timeComplete;
    int yearRebuilt;
    bool rampExists;
    bool isComplete;
    float approximateHeading;
    int adjustedRot;
    bool rampInXWalk;
    bool xWalkisComplete, xWalkWidth;
    bool signalisComplete;
    enum textures texture;
    enum obstructions upperLandingObstruction;
    enum obstructions lowerLandingObstruction;
    enum obstructions rampObstruction;
    bool hasRampCracks;
    bool hasUpperLandingCracks;
    bool hasStreetLandingCracks;
    bool hasLeftSidewalkCracks, hasRightSidewalkCracks;
    bool gradeBreaksArePerpendicular, detectableWarningIsFullWidth;
    CrackWidth crackWidth;
    int detectableWarningDepth;
    float domeSpacing, spaceBetweenDomes, domeBaseWidth, domeCapWidth;//these are for detectable warnings
    int streetLandingDepth, leftXWalkSideWidth, rightXWalkSideWidth;//these are for diagonal ramp type
    int rampWidth, rampDepth;
    float rampSlopeFront;
    float rampSlopeSide;
    float rampSlopeHeading;
    float upperLandingSlopeFront;
    float upperLandingSlopeSide;
    float upperLandingSlopeHeading;
    float streetLandingSlopeFront;
    float streetLandingSlopeSide;
    float streetLandingSlopeHeading;
    float flareLeftSlopeFront;
    float flareLeftSlopeSide;
    float flareLeftSlopeHeading;
    float flareRightSlopeFront;
    float flareRightSlopeSide;
    float flareRightSlopeHeading;
    float swkLeftSlopeFront;
    float swkLeftSlopeSide;
    float swkLeftSlopeHeading;
    float swkRightSlopeFront;
    float swkRightSlopeSide;
    float swkRightSlopeHeading;
    NVSignalType PEDSignalType;
    NVButtonType PEDButtonType;
    AudibleWalkIndication awi;
    bool hasLocatorTone, hasInfoSign, hasBraille, hasTactileArrow;
    float locatorToneVolume, audibleWalkIndicationVolume;//dB measured from 3 feet
    int PEDButtonHeight, PEDButtonDist;
    float SteepTopOfCurb;
    float PedRampLip;
    int lev21x, lev21y;
	double latitude, longitude;
    char rampComment[4096];
    RampType rampType;
    float curbCutDistance;
    float bumpWidth;
    float bumpHeight;
} RampStruct;

typedef struct ToleranceValues {
    float buttonHeight;
    float dimensions;
    float cv10;
    float cv8;
    float cv5;
    float cv2;
    float cracks;
    float levelChange;
} ToleranceValues;

#endif /* RampStruct_h */
