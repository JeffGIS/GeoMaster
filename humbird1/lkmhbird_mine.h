#include "ChartLib.h"

#define	MAX_PATH	128

typedef void	*HANDLE;
typedef char	*LPSTR;
typedef int   BOOL;

int LKMToHBInit (char *PathToLKMData,LPSTR CID);
CL_BOOL LakeMasterToHBirdImage (HANDLE HBImageHandle,int HBImageWidth,int HBImageHeight,int CenterX,int CenterY,double *pScale,
								int DepthOff, int HighlightLMLakes, int SeamLess,int Rotation,
							int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,
							BOOL ShowHazardAreas, int HazDepth,
							BOOL AdjustScale,
              ClChartRenderCb pfCallBack, void *pCallBackData );
BOOL LKMBeginEnumObjectsText (int WorldPtX,int WorldPtY,double RangeInMeters,double Scale,HANDLE *enumHandle );
BOOL LKMEnumNextObjectText(HANDLE enumHandle, LPSTR text , int maxtextlen );
void LKMEndEnumObjectsText(HANDLE enumHandle );
BOOL LKMBeginEnumObjects (int CenterX, int CenterY, double RangeInMeters,double Scale,int WantType,HANDLE *penumHandle);
BOOL LKMEnumNextName (HANDLE enumHandle, HANDLE *ObjectHandle  );	
void LKMEndEnumObjects (HANDLE enumHandle );
BOOL LKMEnumNextObject(HANDLE enumHandle, HANDLE *ObjectHandle  );	
BOOL LKMGetNavaidData (HANDLE ObjectHandle,int *x,int *y,int *BitmapID);
BOOL LKMGetChartText (HANDLE ObjectHandle,int WantType,int *x,int *y, char *text , int maxtextlen );
BOOL LKMGetHighwayShieldData (HANDLE ObjectHandle,int *x,int *y,int *ShieldID, char *text); 
BOOL LKMGetObjectNameFromID (int ID,char *Name);
BOOL LKMPostRotationProcessing(HANDLE HBImageHandle,int HBImageWidth,int HBImageHeight,int CenterX,int CenterY,double Scale,
								int DepthOff, int HighlightLMLakes,
								int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,
								BOOL DisplayHazardAreas, int HazDepth,
								ClChartRenderCb pfCallBack, void *pCallBackData );
BOOL HBDisplayBitmap (HANDLE HBImageHandle,char * BitmapPathName,int PCTSize,int WorldX,int WorldY);

