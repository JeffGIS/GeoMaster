#ifndef MPIntersection_h
#define MPIntersection_h
//  MPIntersection.h
//  CCodeLibrary

#define RAMP 1
#define STREET 2
#define UPPER 3
#define STREETLANDING 2
#define UPPERLANDING 3
#define FLARELEFT 4
#define FLARERIGHT 5
#define SWKLEFT 6
#define SWKRIGHT 7
#define HEADINGERRORDIF 75

typedef enum CrackType
{
    CrackTypeRamp,
    CrackTypeUpperLanding,
    CrackTypeStreetLanding,
    CrackTypeLeftSidewalk,
    CrackTypeRightSidewalk
};

typedef enum MiddleMeasurement
{
    MiddleMeasurementNone,
    MiddleMeasurementCurbCutDistance,
    MiddleMeasurementBumpWidth,
    MiddleMeasurementBumpHeight
};
typedef enum IntersectionStatus
{
	InitialStatus,
		ComplexStatus,
		PreliminarySurveyComplete,
		OriginalSurveyComplete,
		ScheduledForRebuild,
		IntersectionRebuilt,
		ResurveyedAfterRebuild,
		AddedInDetail
};

#include "RampStruct.h"

/*@interface PSRamp : NSObject
//Preliminary Survey Ramps
(nonatomic) int lev21x, lev21y;
(nonatomic) int imageX, imageY;
(nonatomic) int refno, rampID;
(nonatomic) float trueAz;
(nonatomic) int adjustedRot;

@end

@interface MPRamp : NSObject

int rampNum;
(nonatomic, copy) NSString *rampID;
int timeComplete;
int yearRebuilt;
BOOL rampExists;
BOOL isComplete;
(nonatomic) float approximateHeading;
int adjustedRot;
BOOL rampInXWalk;
BOOL xWalkisComplete, xWalkWidth;
BOOL signalisComplete;
(nonatomic) enum textures texture;
(nonatomic) enum obstructions upperLandingObstruction;
(nonatomic) enum obstructions lowerLandingObstruction;
(nonatomic) enum obstructions rampObstruction;
BOOL hasRampCracks;
BOOL hasUpperLandingCracks;
BOOL hasStreetLandingCracks;
BOOL hasLeftSidewalkCracks, hasRightSidewalkCracks;
BOOL gradeBreaksArePerpendicular, detectableWarningIsFullWidth;
CrackWidth crackWidth;
CrackType currentCrackMeasurement;//this need not be in copy or comparision methods (internal value - not in database)
MiddleMeasurement currentMiddleMeasurement;//this need not be in copy or comparision methods (internal value - not in database)
int detectableWarningDepth;
float domeSpacing, spaceBetweenDomes, domeBaseWidth, domeCapWidth;//these are for detectable warnings
int streetLandingDepth, leftXWalkSideWidth, rightXWalkSideWidth;//these are for diagonal ramp type
(nonatomic) int rampWidth, rampDepth;
(nonatomic) float rampSlopeFront;
(nonatomic) float rampSlopeSide;
(nonatomic) float rampSlopeHeading;
(nonatomic) float upperLandingSlopeFront;
(nonatomic) float upperLandingSlopeSide;
(nonatomic) float upperLandingSlopeHeading;
(nonatomic) float streetLandingSlopeFront;
(nonatomic) float streetLandingSlopeSide;
(nonatomic) float streetLandingSlopeHeading;
(nonatomic) float flareLeftSlopeFront;
(nonatomic) float flareLeftSlopeSide;
(nonatomic) float flareLeftSlopeHeading;
(nonatomic) float flareRightSlopeFront;
(nonatomic) float flareRightSlopeSide;
(nonatomic) float flareRightSlopeHeading;
(nonatomic) float swkLeftSlopeFront;
(nonatomic) float swkLeftSlopeSide;
(nonatomic) float swkLeftSlopeHeading;
(nonatomic) float swkRightSlopeFront;
(nonatomic) float swkRightSlopeSide;
(nonatomic) float swkRightSlopeHeading;
NVSignalType PEDSignalType;
NVButtonType PEDButtonType;
AudibleWalkIndication awi;
BOOL hasLocatorTone, hasInfoSign, hasBraille, hasTactileArrow;
(nonatomic) float locatorToneVolume, audibleWalkIndicationVolume;//dB measured from 3 feet
int PEDButtonHeight, PEDButtonDist;
(nonatomic) float SteepTopOfCurb;
(nonatomic) float PedRampLip;
int lev21x, lev21y;
NSString *rampComment;
RampType rampType;
float curbCutDistance;
float bumpWidth;
float bumpHeight;

- (BOOL) isEqualTo:(MPRamp*)oldRamp;

@end
*/
typedef struct {
	 int intID;
	 int intRotation;
	int zoomLevel;
	BOOL doLater;
	 int timeComplete;
	int status;
	int lastUpdate;
	int assignedPrelim;
	int assignedDetail;
	 int currentCorner;
	 int currentSignal;
	 int currentObstruction;
	 float currentCornerHeading;
	int currentRamp;
	char name[1024];
	BOOL signal23Exists;
	BOOL signal45Exists;
	BOOL signal67Exists;
	BOOL signal81Exists;
	 int signal23lev21x;
	 int signal23lev21y;
	 int signal45lev21x;
	 int signal45lev21y;
	 int signal67lev21x;
	 int signal67lev21y;
	 int signal81lev21x;
	 int signal81lev21y;
	BOOL xWalkAExists;
	BOOL xWalkBExists;
	BOOL xWalkCExists;
	BOOL xWalkDExists;
	 int xWalkAlev21x;
	 int xWalkAlev21y;
	 int xWalkBlev21x;
	 int xWalkBlev21y;
	 int xWalkClev21x;
	 int xWalkClev21y;
	 int xWalkDlev21x;
	 int xWalkDlev21y;
	 int corner23heading;
	 int corner45heading;
	 int corner67heading;
	 int corner81heading;
	BOOL corner23isComplete;
	BOOL corner45isComplete;
	BOOL corner67isComplete;
	BOOL corner81isComplete;
	 int quadDev0, quadDev90, quadDev180, quadDev270;
	 int lev21x, lev21y;
	 double lat, lon;
	char intersectionComment[256];
	 int pairedMode;
	 RampStruct ramps[13];
} MPINTERSECTION;

#endif