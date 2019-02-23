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

enum FTPDataType
{
	FTPDataPlat,
		FTPDataPDF,
		FTPDataJPG,
		FTPDataCrimes,
		FTPDataOther,
		GetDatabaseSettingProperty,
		GetDatabaseSettingTiles,
		GetDatabaseSettingParcels,
		GetDatabaseSettingAreas,
		GetDatabaseSettingSewer,
		GetDatabaseSettingPlan,
		GetDatabaseSettingCensus,
		GetDatabaseSettingAlleyWall,
		GetDatabaseSettingAlleyWallNew,
		GetDatabaseSettingCurbRamp,
		GetDatabaseSettingCrimes,
		GetDatabaseSettingMapleGrove,
		GetDatabaseSettingHunting,
		GetDatabaseSettingNVMetro,
		FTPDataPhoto,
		FTPData,
		FTPDatabase,
		FTPErrorFile,
};
typedef int FTPDataType;

enum FTPStatus
{
	FTPStatusDefault,
		FTPStatusOK,
		FTPStatusStop,
		FTPStatusCancel,
		FTPStatusFileWriteError,
		FTPStatusConnectionFailed,
		FTPStatusConnectionFailedTryingSecondary,
		FTPStatusFailedToUnzip,
		FTPStatusFileExists,
		FTPStatusUserCancel,
		FTPStatusBusy,
};
typedef int FTPStatus;

typedef struct {
	int count;
	LPVOID item[1];
}NSARRAY;
typedef NSARRAY *NSArray;

typedef struct {
	char	fullDescription[256];
	char	name[256];
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
	BOOL uploadPix;
	BOOL refreshMap;
	char sharedFilePath[MAX_PATH];
	char errFile[MAX_PATH];
	char currentCurbRampDB[MAX_PATH];
}CRAPIDATA;
typedef struct {
	BOOL haveInit;
	CRAPIDATA sharedInstance;
}CRAPi;


NSArray NSArray_Init(NSArray existingArray);
void NSArray_Destroy(NSArray *parray);

void NSArray_addObject(NSArray array, LPVOID pItem);
NSArray componentsSeparatedByString(LPSTR str, LPSTR sepstr);
BOOL NSArray_removeObjectAtIndex(NSArray placearray, int index);

BOOL CRAPI_sharedInstance_processPlacesArray(int row);

void getDownloadCounts(BOOL loading);

CRAPi * CRAPI_Init(void);
void CRAPI_Destroy(void);

BOOL CRAPI_sharedInstance_setGeoIDForCurrentApp(void);
LPSTR CRAPI_sharedInstance_sharedOutputDirectory(LPSTR subDir);
BOOL CRAPI_sharedInstance_haveErrorLog(void);
LPSTRD CRAPI_sharedInstance_errorLogPath(void);

BOOL appendStringToFile(LPSTR str, LPSTR filePath);
void NSLog(LPSTR fmt, LPSTR str);

extern CRAPi *CRAPI;