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
	int currentGeoidValue;
	int isRentaliPad;
	int currentModule;
	int currentApp;
	int serverToUse;
	int deviceNumber;
	int controlledInterface;
	int controllControlledInterface;
	BOOL allowSidewalk;
	BOOL showDeveloperFunctions;
    int  databaseToDownload;

	NSArray placesArray;
}CRAPIDATA;
typedef struct {
	BOOL haveInit;
	CRAPIDATA sharedInstance;
}CRAPi;


LPSTR string_Copy(LPSTR str);

NSArray NSArray_Init(NSArray existingArray);
void NSArray_Destroy(NSArray *parray);

void NSArray_addObject(NSArray array, LPVOID pItem);
NSArray componentsSeparatedByString(LPSTR str, LPSTR sepstr);
BOOL NSArray_removeObjectAtIndex(NSArray placearray, int index);
LPSTR textAfterFirstChar(LPSTR string, char c);
LPSTR textBeforeLastChar(LPSTR string, char c);
LPSTR textAfterLastChar(LPSTR string, char c);

BOOL CRAPI_sharedInstance_processPlacesArray(int row);

BOOL UserDefaults_defaults_saveString(LPSTR string, LPSTR key);
LPSTR UserDefaults_defaults_stringForKey(LPSTR key);
int UserDefaults_defaults_integerForKey(LPSTR key);
BOOL UserDefaults_defaults_saveInteger(int value, LPSTR key);

CRAPi * CRAPI_Init(void);
void CRAPI_Destroy(void);

BOOL CRAPI_sharedInstance_setGeoIDForCurrentApp(void);