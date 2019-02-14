#pragma once
enum ModuleNames
{
	MANAGEMENT_MODULE = 1,
	PRELIM_MODULE = 2,
	DETAIL_MODULE = 3,
	GMMOBILE_MODULE = 4,
	FAC_MANAGEMENT_MODULE = 5,
	FAC_PRELIM_MODULE = 6,
	FAC_DETAIL_MODULE = 7,
	ALLEYWALL_DETAIL_MODULE = 8,
	GEOMASTER_MODULE = 9,
};
enum GMSMessage
{
	GMSMessageDefault,
	GMSMessageOpenConnect,
	GMSMessageSetNearby,
	GMSMessageCheckNearby,
	GMSMessageCancelConnect,
	GMSMessageGetPoliceUpdate,
	GMSMessageGetUploadStatus,
	GMSMessageFileUploaded,
	GMSMessageGetiPadNumber,
};
enum HardCodedConfigs
{
		HCConfigAlleyWalls,
		HCConfigAlleyWallStatus,
		HCConfigCurbRamps,
		HCConfigSidewalk,
		HCConfigSewers,
		HCConfigMetroProp,
		HCConfigCrimes,
		HCConfigMapleGrove,
		HCConfigFromSpreadsheet,
		HCConfigMapleGrovePDF,
		HCConfigFromSlowValues,
		HCConfigMinnesotaGame,
		HCConfigHunting,
		HCConfigFacilities,
		NUMHardCodedConfigs
};
typedef struct {
	int count;
	LPVOID item[1];
}NSARRAY;
typedef NSARRAY *NSArray;

typedef struct {
	char	fullDescription[256];
	LPSTR	name;
	int		number;
	int		managerNumber;
	int		withinManager;
	int		totWithinManager;
	int		GSSiPadNumber;
	int		controlledInterface;
	int		allowSidewalk;
}CITYORORGANIZATIONDATA;
typedef CITYORORGANIZATIONDATA *CityOrOrganizationData;

typedef struct {
	int GSSiPadNumber;
	int currentManageriPad;
	LPSTR currentCityNameAndState;
	int currentiPadWithinManager;
	int totiPadWithinManager;
	BOOL hideConnectButton;
	int currentGeoid;
	int isRentaliPad;
	int currentModule;
	int currentSubModule;
	char appVersionBuild[128];
	int currentApp;
	int serverToUse;
	int deviceNumber;
	int controlledInterface;
	int controllControlledInterface;
	BOOL allowSidewalk;
	BOOL showDeveloperFunctions;
    int  databaseToDownload;
	NSArray placesArray;
	BOOL downloadData;
	BOOL downloadPix;
	BOOL refreshMap;
	char sharedFilePath[MAX_PATH];
	char errFile[MAX_PATH];
	char currentCurbRampDB[MAX_PATH];
}CRAPIDATA;
typedef struct {
	BOOL haveInit;
	CRAPIDATA sharedInstance;
}CRAPi;

LPSTRD string_Copy(LPSTR str);

NSArray NSArray_Init(NSArray existingArray);
void NSArray_Destroy(NSArray *parray);

void NSArray_addObject(NSArray array, LPVOID pItem);
NSArray componentsSeparatedByString(LPSTR str, LPSTR sepstr);
BOOL NSArray_removeObjectAtIndex(NSArray placearray, int index);
LPSTRD textAfterFirstChar(LPSTR string, char c);
LPSTRD textBeforeLastChar(LPSTR string, char c);
LPSTRD textAfterLastChar(LPSTR string, char c);

BOOL CRAPI_sharedInstance_processPlacesArray(int row);

BOOL UserDefaults_defaults_saveString(LPSTR string, LPSTR key);
LPSTRD UserDefaults_defaults_stringForKey(LPSTR key);
int UserDefaults_defaults_integerForKey(LPSTR key);
BOOL UserDefaults_defaults_saveInteger(int value, LPSTR key);
void getDownloadCounts(BOOL loading);

CRAPi * CRAPI_Init(void);
void CRAPI_Destroy(void);

BOOL CRAPI_sharedInstance_setGeoIDForCurrentApp(void);
LPSTR CRAPI_sharedInstance_sharedOutputDirectory(LPSTR subDir);
BOOL CRAPI_sharedInstance_haveErrorLog(void);
LPSTRD CRAPI_sharedInstance_errorLogPath(void);

BOOL appendStringToFile(LPSTR str, LPSTR filePath);
LPSTRD stringByDeletingLastPathComponent(LPSTR path);
LPSTRD lastPathComponent(LPSTR path);
void NSLog(LPSTR fmt, LPSTR str);
void GSSiFree(LPSTR *str);

extern CRAPi *CRAPI;