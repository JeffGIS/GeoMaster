#include "graphint.h"   
#include "gmextern.h"    
#include "resource.h"
#include "CRAPI.h"
#include "CurbRamps.h"

static char  Winver[32];
static BOOL _didConnect;
static BOOL _autoSync;
static int  _selectedRow;
static char _minManageVersion[64];
static char _minPrelimVersion[64];
static char _minDetailVersion[64];
static char _minAlleyVersion[64];
static UINT iTimer = 0;
static int  _downFiles;
static int  _downPix;
static int  _totFilesToProcess;
static int  _uploadedFiles;
static int  _origUpFiles;
static int  _processedFiles;
static int  _lastFileNumToUpload;
static int  _nextFileNumToUpload;
static int  _upFiles;
static int  _lastPictNumToUpload;
static int  _nextPictNumToUpload;
static int  _upPix;
static int  _myiPad;
static int  _numiPadsInMsg;
static LPSTR _uploadStatusMsg = 0;
static int	_databaseID = DATABASEID_CURBRAMPS;
static BOOL _useMasterID = FALSE;
static int  _process;
static BOOL _uploadPix = FALSE;
static BOOL _downloadData = TRUE;
static BOOL _downloadPix = FALSE;
static int _downloadiPad;
static int _dataType;
static int _lastUploadFileSize;
static LPSTRD _filePath = 0;
static HWND _popoverView = 0;
static int _nextFileNumToDownload;
static int _lastFileNumToDownload;
static BOOL _first = TRUE;
static double _sendDataStartTime;
static int currentMessage = GMSMessageDefault;
static char _errorFileName[MAX_PATH];
static int _numAttemps = 0;
static BOOL _justDownloadedNewDatabase = FALSE;

#define DOWNLOAD_DATABASE_FUNCTION 3
#define FTP_CATCHUP_DELAY 0.5
#define UPLOAD_STATUS_VERSION 1
#define MAX_ATTEMPS 3

#define UPLOAD_DATA_PROCESS 1
#define UPLOAD_PICT_PROCESS 2
#define DOWNLOAD_DATA_PROCESS 3
#define DOWNLOAD_PICT_PROCESS 4
#define UPLOAD_ERROR_FILE 5
#define CONNECT_PROCESS 6
#define DOWNLOAD_DATABASE_PROCESS 7


CRAPi *CRAPI=NULL;

void setDatabaseIDToDB(_databaseID);
void startSync(HWND hWndDlg);
void updateLabels(HWND hWndDlg);
void updateOverallProgress(HWND hWndDlg,double pct);
void uploadNextFile(HWND hWndDlg);
void doUploadNextFile(HWND hWndDlg);
int nextiPadToDownload(int currentiPad);
void downloadNextFile(HWND hWndDlg);
void doDownloadNextFile(HWND hWndDlg);
void setDownloadFilesFor(int iPad);
void PopoverHide(HWND hWnd);
void PopoverShow(HWND hWnd, double after,LPSTR msg, LPSTR OKButtonText);
void FTPPushDidComplete(HWND hWndDlg, FTPDataType dataType, FTPStatus status, LPSTR statusString, LPSTR path);
void FTPPullDidComplete(HWND hWndDlg, FTPDataType dataType, FTPStatus status, LPSTR statusString, LPSTR file, LPSTR dir);

double currentTime(void)
{
	__time32_t now;

	now = _time32(&now);
	double dnow = now;
	return dnow;
}
BOOL getUseMasterID(void)
{
	return _useMasterID;
}

BOOL isGSSiDevice(void)
{
	return FALSE;
}

NSArray NSArray_Init(NSArray existingArray)
{
	NSArray rtn = calloc(1, sizeof(NSARRAY));
	NSArray_Destroy(&existingArray);
	return rtn;
}
void NSArray_Destroy(NSArray *parray)
{
	if (*parray)
	{
		for (int i = 0; i < (*parray)->count; i++)
		{
			free((*parray)->item[i]);
		}
		free(*parray);
		*parray = 0;
	}
}
void NSArray_addObject(NSArray array, LPVOID pItem)
{
	if (array)
	{
		array->item[array->count++] = pItem;
	}
}
BOOL NSArray_removeObjectAtIndex(NSArray array, int index)
{
	BOOL rtn = FALSE;

	if (array && array->count > index)
	{
		free(array->item[index]);
		for (int j = index + 1; j < array->count; j++)
			array->item[index++] = array->item[j];
		array->count--;
	}
	return rtn;
}

CRAPi* CRAPI_Init(void)
{
	if (!CRAPI)
	{
		CRAPI = calloc(1, sizeof(CRAPi));
		CRAPI->haveInit = TRUE;
		CRAPI->sharedInstance.serverToUse = 3;
		CRAPI->sharedInstance.currentModule = GEOMASTER_MODULE;
		CRAPI->sharedInstance.currentSubModule = 0;
		CRAPI->sharedInstance.placesArray = NSArray_Init(0);
		strcpy(CRAPI->sharedInstance.sharedFilePath, "[%DL][NVCITY]\\sharedFilePath");
		ExpandText(CRAPI->sharedInstance.sharedFilePath);
		strcpy(CRAPI->sharedInstance.appVersionBuild, appAndVersion());
		CRAPI->sharedInstance.GSSiPadNumber = -1;
		CRAPI_sharedInstance_processPlaces();
		CRAPI_sharedInstance_setGeoIDForCurrentApp();
	}
	CRAPI->sharedInstance.hideConnectButton = 1;
	return CRAPI;
}
void CRAPI_Destroy(void)
{
	if (CRAPI && CRAPI->haveInit)
	{
		NSArray_Destroy(&CRAPI->sharedInstance.placesArray);
	}
	free(CRAPI);
	CRAPI = 0;
}

LPSTRD string_Copy(LPSTR str)
{
	int l = strlen(str);
	LPSTR rtn = malloc(l + 1);
	strcpy(rtn, str);
	return rtn;
}
CityOrOrganizationData CityOrOrganizationData_init(void)
{
	CityOrOrganizationData c = calloc(1, sizeof(CITYORORGANIZATIONDATA));
	return c;
}

LPSTRD textAfterFirstChar(LPSTR string, char c)
{
	if (!string)
		return 0;

	else if (!strlen(string))
		return string_Copy(string);

	LPSTR str = strchr(string, c);
	LPSTR rtn;
	if (str)
		rtn = string_Copy(++str);
	else
		rtn = string_Copy(string);

	return rtn;
}

LPSTRD textAfterLastChar(LPSTR string, char c)
{
	if (!string)
		return 0;

	else if (!strlen(string))
		return string_Copy(string);

	LPSTR str = strrchr(string, c);
	LPSTR rtn;
	if (str)
		rtn = string_Copy(++str);
	else
		rtn = string_Copy(string);

	return rtn;
}

LPSTRD textBeforeLastChar(LPSTR string, char c)
{
	if (!string)
		return 0;

	else if (!strlen(string))
		return string_Copy(string);

	LPSTR str = strrchr(string, c);
	LPSTR rtn;
	if (str)
	{
		*str = 0;
		rtn = string_Copy(string);
		*str = c;
	}
	else
		rtn = string_Copy(string);

	return rtn;
}
LPSTR textBeforeFirstChar(LPSTR string, char c)
{
	if (!string)
		return 0;

	else if (!strlen(string))
		return string_Copy(string);

	LPSTR str = strchr(string, c);
	LPSTR rtn;
	if (str)
	{
		*str = 0;
		rtn = string_Copy(string);
		*str = c;
	}
	else
		rtn = string_Copy(string);

	return rtn;
}

LPSTR textAfterString(LPSTR string, LPSTR str)
{
	LPSTR rtn;
	if (!string)
		return 0;

	else if (!strlen(string))
		return string_Copy(string);

	LPSTR pLoc = strstr(string, str);
	if (pLoc)
	{
		pLoc += strlen(str);
		rtn = string_Copy(pLoc);
	}
	else
		rtn = string_Copy(string);
	return rtn;
}
LPSTR addSpaceAfterComma(LPSTR string)
{
	LPSTR rtn = 0;
	if (!string)
		return NULL;

	else if (!strlen(string))
		return string_Copy(string);

	LPSTR pComma = strchr(string, ',');
	if (pComma)
	{
		rtn = malloc(strlen(string) + 2);
		*pComma = 0;
		strcpy(rtn, string);
		strcat(rtn, " ");
		strcat(rtn, pComma + 1);
		*pComma = ',';
	}
	if (!rtn)
		rtn = string_Copy(string);
	return rtn;
}


/*
NSString * ConvertSingleQuote(NSString * str)
{
	int ln = (int)str.length;
	LPSTR newstr = calloc(ln + 4, 1);
	LPSTR oldstr = calloc(ln * 3 + 4, 1);
	NSString * convertedStr = @"";
		int i = 0;
	int newln = 0;

	strcpy(oldstr, str.UTF8String);
	ln = (int)strlen(oldstr);
	while (i < ln)
	{
		int ic = (int)*(oldstr + i);
		int ic2 = (int)*(oldstr + i + 1);
		int ic3 = (int)*(oldstr + i + 2);
		if (ic == -30 && ic2 == -128 && ic3 == -103)
		{
			newstr[newln++] = '\'';
			i += 3;
		}
		else
		{
			newstr[newln++] = *(oldstr + i);
			i++;
		}
	}
	convertedStr = [NSString stringWithUTF8String : newstr];
	free(newstr);
	free(oldstr);
	return convertedStr;
}

{
	UIDevice *device = UIDevice.currentDevice;
	NSString *carrier = deviceCarrier();

#ifdef DEBUG
	floatSize = -floatSize;
#endif
	NSString * dname = [device.name stringByReplacingOccurrencesOfString : @"," withString:@""];
		dname = ConvertSingleQuote(dname);
	return[NSString stringWithFormat : @"%@,%@,%@|%@|%llu | %llu | %@|%i",
		vendorUUIDString(), dname, device.systemVersion, deviceSystemInfo(),
		UIDevice.availableStorageSpace, UIDevice.totalStorageSpace, carrier, floatSize];

		$M(GetCitiesForiPad,CB88073E-238B-4168-9E9E-F9EABF7C58AC,GSSiRental006,9.3.5|iPad2|2|12109832192|13846052864|AT&T|NONE|-4,,3.0-1.4.8 (22))

}
*/
BOOL UserDefaults_defaults_saveString(LPSTR string, LPSTR key)
{
	BOOL rtn = WritePrivateProfileString("CRAPI", key, string, GMIni);
	return rtn;
}

LPSTRD UserDefaults_defaults_stringForKey(LPSTR key)
{
	LPSTR str = calloc(1, 1024);
	int nchr = GetPrivateProfileString("CRAPI", key, "", str, 1024, GMIni);

	return str;
}
int UserDefaults_defaults_integerForKey(LPSTR key)
{
	int rtn =0;

	return rtn;
}
BOOL UserDefaults_defaults_saveInteger(int value, LPSTR key)
{
	BOOL rtn = TRUE;

	return rtn;
}
void setGSSiPadNumber (int num)
{
	CRAPI->sharedInstance.GSSiPadNumber = num;

	if (num > 0)
		UserDefaults_defaults_saveInteger(num, "GSSiPadNumber");
}

int ControlChar(LPSTR controlStr, int whichChar)
{
	int rtn = 0;
	char chr[2];
	if (strlen(controlStr) < whichChar)
		return 0;
	chr[0] = controlStr[whichChar - 1];
	chr[1] = 0;
	rtn = atoi(chr);
	return rtn;
}

int integerValue(LPSTR str)
{
	int rtn = 0;
	if (str)
	{
		rtn = atoi(str);
		free(str);
	}
	return rtn;
}

LPSTR textInsideParentheses(LPSTR string)
{
	LPSTR rtn = 0;
	if (!string)
		return NULL;
	int l = strlen(string);
	if (l)
	{
		LPSTR pStart = strchr(string, '(');
		if (pStart)
		{
			LPSTR pEnd = MatchLev(++pStart, ')');

			if (pEnd)
			{
				l = pEnd - pStart;
				rtn = malloc(l + 1);
				strncpy0(rtn, pStart, l);
			}
		}
	}
	if (!rtn)
		rtn = string_Copy(string);
	return rtn;
}
void saveiPadIDs (void)
{
	char key[128];
	sprintf (key,"ManagerForGeoid:%i", CRAPI->sharedInstance.currentGeoid);
	UserDefaults_defaults_saveInteger(CRAPI->sharedInstance.currentManageriPad, key);

	sprintf (key,"iPadWithinManagerForGeoid:%i", CRAPI->sharedInstance.currentGeoid);

	UserDefaults_defaults_saveInteger(CRAPI->sharedInstance.currentiPadWithinManager, key);

	sprintf (key,"totiPadWithinManagerForGeoid:%i", CRAPI->sharedInstance.currentGeoid);

	UserDefaults_defaults_saveInteger(CRAPI->sharedInstance.totiPadWithinManager, key);

}

void loadiPadParametersForGeoid (int geoid)
{
	char key[128];
	CRAPI->sharedInstance.GSSiPadNumber = (int)UserDefaults_defaults_integerForKey("GSSiPadNumber");

	sprintf (key,"ManagerForGeoid:%i",geoid);
	CRAPI->sharedInstance.currentManageriPad = (int)UserDefaults_defaults_integerForKey(key);

	sprintf (key,"iPadWithinManagerForGeoid:%i",geoid);
	CRAPI->sharedInstance.currentiPadWithinManager = (int)UserDefaults_defaults_integerForKey(key);

	sprintf (key,"totiPadWithinManagerForGeoid:%i",geoid);
	CRAPI->sharedInstance.totiPadWithinManager = (int)UserDefaults_defaults_integerForKey(key);
}


void setCurrentGeoid(int geoid)
{
	if (geoid != CRAPI->sharedInstance.currentGeoid)
	{
		CRAPI->sharedInstance.currentGeoid = geoid;
		loadiPadParametersForGeoid(geoid);
		if (CRAPI->sharedInstance.currentManageriPad == 0)
		{
			CRAPI->sharedInstance.currentManageriPad = CRAPI->sharedInstance.GSSiPadNumber;
			CRAPI->sharedInstance.currentiPadWithinManager = 1;
			CRAPI->sharedInstance.totiPadWithinManager = 1;
			saveiPadIDs();
		}
		/*
		_outputDirectory = [NSString stringWithFormat : @"%@/%i / ADAFiles", _documentsDirectory, _currentGeoid];
			_imageDirectory = [NSString stringWithFormat : @"%@/%i / IntersectionImages", UserDefaults.defaults.sharedFilePath, _currentGeoid];
			_deviceListsKey = [NSString stringWithFormat : @"iPads:%i", CRAPI->sharedInstance.currentGeoid];

			[NSFileManager.defaultManager createDirectoryAtPath : _outputDirectory
			withIntermediateDirectories : YES
			attributes : nil
			error : nil];

		[NSFileManager.defaultManager createDirectoryAtPath : _imageDirectory
			withIntermediateDirectories : YES
			attributes : nil
			error : nil];

		_uploadDirectory = [NSString stringWithFormat : @"PedRampSurvey/%i / %i", _currentManageriPad, geoid];
		*/
	}
}
LPSTR CRAPI_sharedInstance_sharedOutputDirectory(LPSTR subDir)
{
	static char outdir[MAX_PATH];

	sprintf (outdir,"%s\\OutputFiles\\%s\\%i\\%i", CRAPI->sharedInstance.sharedFilePath, subDir, CRAPI->sharedInstance.currentManageriPad, CRAPI->sharedInstance.currentGeoid);

	if (!makedirectories(outdir, TRUE, FALSE))
	{
		GSSiMessageBox(0, outdir, "Unable to create output directory", MB_ICONEXCLAMATION, 0);
	}

	return outdir;
}

BOOL  CRAPI_sharedInstance_processPlaces (void)
{
	LPSTRD message = UserDefaults_defaults_stringForKey("PlacesString");

	CRAPI->sharedInstance.placesArray = NSArray_Init(CRAPI->sharedInstance.placesArray);
	if (!message)
		return NO;

	if (strncmp(message, "PlacesFor", 9))
		return NO;

	setGSSiPadNumber (atoi(&message[9]));

	NSArray placearray = componentsSeparatedByString(message, ":");

	NSArray_removeObjectAtIndex(placearray, 0);

	for (int i=0;i<placearray->count;i++)
	{
		LPSTR instr = placearray->item[i];
		LPSTRD str = textBeforeLastChar(instr, '*');
		LPSTRD controlStr = textAfterLastChar(instr, '*');
		CityOrOrganizationData c = CityOrOrganizationData_init();
		c->controlledInterface = ControlChar(controlStr, 1);
		c->allowSidewalk = ControlChar(controlStr, 2);
		strcpy(c->fullDescription,str);
		c->name = textInsideParentheses(str);
		c->number = integerValue(textBeforeFirstChar(str,'('));
		LPSTRD manager = textAfterLastChar(str,')');
		c->managerNumber = integerValue(textBeforeFirstChar(manager, '-'));
		c->withinManager = integerValue(textAfterFirstChar(manager, '-'));
		c->totWithinManager = integerValue(textAfterString(manager, "of"));
		free(manager);
		free(str);
		free(controlStr);
		c->GSSiPadNumber = CRAPI->sharedInstance.GSSiPadNumber;
		NSArray_addObject (CRAPI->sharedInstance.placesArray, c);
	}

	//[_placesArray sortUsingSelector : @selector(compareName : )];
	NSArray_Destroy(&placearray);
	return YES;
}
BOOL CRAPI_sharedInstance_processPlacesArray (int row)
{
	if (!CRAPI->sharedInstance.placesArray || CRAPI->sharedInstance.placesArray->count < 1)
		return FALSE;

	if (row >= (int)CRAPI->sharedInstance.placesArray->count)
	{
		row = CRAPI->sharedInstance.placesArray->count - 1;

		UserDefaults_defaults_saveInteger (row,"SelectedRow");
	}

	if (row == -1) // AlleyWalls
	{
		BOOL haveCity = FALSE;
		row = 0;

		for (int i=0;i< CRAPI->sharedInstance.placesArray->count;i++)
		{
			CityOrOrganizationData c = CRAPI->sharedInstance.placesArray->item[i];
			if (c->number >= 100000001)
			{
				haveCity = TRUE;
				break;
			}

			row++;
		}
		if (!haveCity)
			return FALSE;
	}

	else if (row == -2) // CurrentGeoid
	{
		BOOL haveCity = FALSE;
		row = 0;

		for (int i = 0; i < CRAPI->sharedInstance.placesArray->count; i++)
		{
			CityOrOrganizationData c = CRAPI->sharedInstance.placesArray->item[i];
			if (c->number == CRAPI->sharedInstance.currentGeoid)
			{
				haveCity = TRUE;
				break;
			}

			row++;
		}
		if (!haveCity)
		{
			if (CRAPI->sharedInstance.placesArray->count == 1)
				row = 0;
			else
				return FALSE;
		}
	}

	else if (CRAPI->sharedInstance.placesArray->count == 1 || row >= CRAPI->sharedInstance.placesArray->count)
		row = 0;

	CityOrOrganizationData c = CRAPI->sharedInstance.placesArray->item[row];
	CRAPI->sharedInstance.GSSiPadNumber = (int)c->GSSiPadNumber;
	CRAPI->sharedInstance.currentCityNameAndState = addSpaceAfterComma(c->name);
	setCurrentGeoid (c->number);

	UserDefaults_defaults_saveInteger(CRAPI->sharedInstance.currentGeoid,"CurrentGeoid");

	CRAPI->sharedInstance.currentManageriPad = (int)c->managerNumber;
	CRAPI->sharedInstance.currentiPadWithinManager = (int)c->withinManager;
	CRAPI->sharedInstance.deviceNumber = CRAPI->sharedInstance.currentiPadWithinManager;
	CRAPI->sharedInstance.totiPadWithinManager = (int)c->totWithinManager;
	CRAPI->sharedInstance.controlledInterface = c->controlledInterface;
	CRAPI->sharedInstance.controllControlledInterface = !c->controlledInterface;
	CRAPI->sharedInstance.allowSidewalk = c->allowSidewalk;
	saveiPadIDs();
	return TRUE;
}

void setDatabaseIDToDB (int databaseID)
{
/*	int geoid = CRAPI->sharedInstance.currentGeoid;

	if (geoid > 100000000)
	{
		_awd = AlleyWallData.new;

		_databaseID = _awd;
		[self tempFix : CRAPI->sharedInstance.GSSiPadNumber
			geoid : geoid];
		[_databaseID incrementLastDataFileNum];
		[_databaseID incrementLastPictFileNum];
	}

	else
	{
		_crd = CurbRampData.new;
		_databaseID = _crd;
		[self tempFix : CRAPI->sharedInstance.GSSiPadNumber
			geoid : geoid];
		[_databaseID incrementLastDataFileNum];
	}

	[self updateConnectLabel];*/

	incrementLastDataFileNum(databaseID);
}


BOOL CRAPI_sharedInstance_setGeoIDForCurrentApp(void)
{
	BOOL rtn = YES;
	int selectedRow = UserDefaults_defaults_integerForKey("SelectedRow");
	if (CRAPI->sharedInstance.currentModule == MANAGEMENT_MODULE)
		selectedRow = -2; //current geoid
	else if (CRAPI->sharedInstance.currentModule == GMMOBILE_MODULE && (CRAPI->sharedInstance.currentApp == HCConfigAlleyWalls || CRAPI->sharedInstance.currentApp == HCConfigAlleyWallStatus))
		selectedRow = -1;
	rtn = CRAPI_sharedInstance_processPlacesArray(selectedRow);
	if (!rtn && selectedRow < 0)
	{
		MessageBox(0, "NVMobile-Alleywalls does not set place and manager", 0, MB_ICONEXCLAMATION);
	}
	else if (!rtn)
	{
		MessageBox(0, "NVMobile-CurbRamps does not set place and manager", 0, MB_ICONEXCLAMATION);
	}
	return rtn;
}


NSArray componentsSeparatedByString(LPSTR str, LPSTR sepstr)
{
	NSArray rtn;
	LPSTR s = str;
	int n = 1;

	LPSTR ploc = strstr(s, sepstr);
	while (ploc)
	{
		n++;
		s = ploc;
		s += strlen(sepstr);
		ploc = strstr(s, sepstr);
	}
	rtn = malloc(sizeof(NSArray) + n * sizeof(LPVOID));
	rtn->count = n;
	s = str;
	n = 0;

	ploc = strstr(s, sepstr);
	for (int i=0;i<rtn->count;i++)
	{
		if (ploc)
			*ploc = 0;
		rtn->item[i] = malloc(strlen(s) + 1);
		strcpy((LPSTR)rtn->item[i], s);
		if (ploc)
		{
			s = ploc;
			*ploc = *sepstr;
			s += strlen(sepstr);
			ploc = strstr(s, sepstr);
		}
	}

	return rtn;
}
BOOL processServerString(LPSTR instr)
{
	BOOL rtn = TRUE;
	NSArray arr = componentsSeparatedByString(instr, " ");
	LPSTR  str = arr->item[0];
	int server = atoi(str);
	if (server != CRAPI->sharedInstance.serverToUse)
		rtn = FALSE;
	CRAPI->sharedInstance.serverToUse = server;
	CRAPI->sharedInstance.hideConnectButton = 1;
	if (arr->count > 1)
	{
		str = arr->item[1];
		CRAPI->sharedInstance.hideConnectButton = atoi(str);
	}
	if (arr->count > 2)
	{
		str = arr->item[2];
		CRAPI->sharedInstance.showDeveloperFunctions = atoi(str);
		if (CRAPI->sharedInstance.showDeveloperFunctions >= 100)
		{
			char process[3];
			strncpy(process, str, 2);
			if (atoi(process) == 2)
				CRAPI->sharedInstance.showDeveloperFunctions = DOWNLOAD_DATABASE_FUNCTION;
			char database[3];
			strncpy(database, &str[1], 2);
			CRAPI->sharedInstance.databaseToDownload = atoi(database);
		}
		if (isGSSiDevice())
			CRAPI->sharedInstance.showDeveloperFunctions = max(1, CRAPI->sharedInstance.showDeveloperFunctions);
	}
	if (arr->count > 3)
	{
		LPSTR str = arr->item[3];
		CRAPI->sharedInstance.isRentaliPad = atoi(str);
	}
	NSArray_Destroy(&arr);
	return rtn;
}


BOOL CRAPI_sharedInstance_savePlacesString (LPSTR placesStringIN) //returns FALSE if need to switch server
{
	BOOL rtn = TRUE;
	NSArray arr = componentsSeparatedByString (placesStringIN,"|");
	if (arr->count > 3)
	{
		LPSTR messNum = arr->item[0];
		int currentMessageNum = atoi(messNum);
		messNum = arr->item[1];
		rtn = processServerString (messNum);
		LPSTR version = arr->item[2];
		NSArray versions = componentsSeparatedByString (version,":");
		strncpy (_minManageVersion,versions->item[0],63);
		if (versions->count > 1)
			strncpy(_minPrelimVersion, versions->item[1], 63);
		if (versions->count > 2)
			strncpy(_minDetailVersion, versions->item[2], 63);
		if (versions->count > 3)
			strncpy(_minAlleyVersion, versions->item[3], 63);
		NSArray_Destroy(&versions);
	}
	LPSTR placesStart = strstr (placesStringIN,"PlacesFor");
	LPSTR placesString = placesStart;

	UserDefaults_defaults_saveString( placesString,"PlacesString");
	NSArray_Destroy(&arr);

	return rtn;
}

LPSTR vendorUUIDString(void)
{
	static char UUIDString[40];
	LPSTR rtn = UUIDString;

	DWORD Serno = GM32GetNodeInfo(NodeName, UserName, Winver);
	
	sprintf(UUIDString, "PCIc-%31.31ld", Serno);
	return rtn;
}

LPSTR deviceInfo(void)
{
	static char dinfo[256];
	int floatSize = sizeof(CGFloat);
	DWORD Serno = GM32GetNodeInfo(NodeName, UserName, Winver);
	int bitMachine = 32;
	strcpy(dinfo, "[%DL]");
	ExpandText(dinfo);
	double freeSpaceDL = GetDriveFreeSpace(dinfo);
	double freeSpaceC = GetDriveFreeSpace("C:\\");
	if (Is64BitMachine())
		bitMachine = 64;
	sprintf(dinfo, "%s,%s,%s|PC|%i|%.0f|%.0f||-%i", vendorUUIDString(), NodeName, Winver, bitMachine, freeSpaceDL, freeSpaceC, floatSize);
	return dinfo;
}
LPSTR appAndVersion(void)
{
	static char aandv[64];

	strcpy(aandv, "[%VERSION]");
	ExpandText(aandv);

	return aandv;
}

void UpdateConnectLabel (HWND hWndDlg)

{
	char text[256];

	sprintf (text,"%i - %i:%i of %i",
		CRAPI->sharedInstance.GSSiPadNumber, CRAPI->sharedInstance.currentManageriPad,
		CRAPI->sharedInstance.currentiPadWithinManager, CRAPI->sharedInstance.totiPadWithinManager);
	SetWindowText(GetDlgItem(hWndDlg, IDC_CONNECT_LABEL), text);
}

BOOL SendMessageToServer(HWND hWnd, LPSTR message)
{
	BOOL rtn = FALSE;
	HANDLE hCmd = GSSiGlobAlloc(0, GMEM_MOVEABLE, strlen(message) + 256);
	LPSTR pCmd = GlobalLock(hCmd);

	sprintf(pCmd, "$M(SendMessageToServer,%ld,%s)", (long)hWnd, message);
	ExpandText(pCmd);
	rtn = atoi(pCmd);
	GSSiGlobUlFree(&hCmd);

	return rtn;
}
void GMServerSendMessage (HWND hWndDlg,int messageID)
{
	char mes[1024] = { 0 };
	//_serverOpen = YES;

		switch (messageID)
		{
		case GMSMessageOpenConnect:
			//SetUseDemoID(NO);
			sprintf(mes, "@$M(GetCitiesForiPad,%s@,,%s)", deviceInfo(), appAndVersion());
			break;
		case GMSMessageCancelConnect:
			sprintf(mes, "@$M(RemoveNearbyiPad,%s,0 0)", vendorUUIDString());
			break;
		case GMSMessageSetNearby:
			sprintf(mes, "@$M(SetNearbyiPad,%s,%s,0 0)", vendorUUIDString(), NodeName);
			break;
		case GMSMessageCheckNearby:
			sprintf(mes, "@$M(CheckNearbyiPad,%s)", vendorUUIDString());
			break;
		case GMSMessageGetUploadStatus:
			sprintf(mes, "@$M(GetUploadStatus,%i,%i,%i,%i)",
				CRAPI->sharedInstance.currentManageriPad, CRAPI->sharedInstance.currentGeoid,
				CRAPI->sharedInstance.totiPadWithinManager, UPLOAD_STATUS_VERSION);
			break;

		case GMSMessageFileUploaded:
			if (_process == UPLOAD_ERROR_FILE)
			{
				sprintf(mes, "@$M(UploadErrors,%s)", _errorFileName);
			}
			else if (_process == UPLOAD_DATA_PROCESS)
			{
#define AVERAGE_UPLOAD_SPEED 25000.0
				__time32_t t;
				double now = _time32(&t);
				double timePassed = fmax(0.01, (now - _sendDataStartTime) - 1);
				double uploadSpeed = _lastUploadFileSize / timePassed;
				double speedFactor = min(5, max(1.0, AVERAGE_UPLOAD_SPEED / uploadSpeed));
				int interval = speedFactor * (FTP_CATCHUP_DELAY * (1 + _lastUploadFileSize / 500000.0));
				Sleep( interval);
				sprintf (mes,"@$M(UploadCompleteData,%i,%i,%i,%i,%i)",CRAPI->sharedInstance.currentManageriPad, CRAPI->sharedInstance.currentGeoid,
					CRAPI->sharedInstance.currentiPadWithinManager, _nextFileNumToUpload, _lastUploadFileSize);
			}
			else
			{
				Sleep (FTP_CATCHUP_DELAY * 2);
				sprintf (mes,"@$M(UploadCompletePict,%i,%i,%i,%i,%i)",	CRAPI->sharedInstance.currentManageriPad, CRAPI->sharedInstance.currentGeoid,
					CRAPI->sharedInstance.currentiPadWithinManager, _nextFileNumToUpload, _lastUploadFileSize);
			}

			break;
		}
		currentMessage = messageID;
		if (*mes)
			SendMessageToServer(hWndDlg, mes);
}

BOOL FAR PASCAL SynchronizeMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	char mes[1024];
	int	BRtn;
	BOOL rtn;
	if ((BRtn = DIALOGSTYLEMsgProc(hWndDlg, Message, wParam, lParam)))
		return (BRtn);
	switch (Message)
	{
	case WM_INITDIALOG:
		currentMessage = GMSMessageDefault;

		CRAPI = CRAPI_Init();
		cwCenter(hWndDlg, 0);
		GMServerSendMessage(hWndDlg,GMSMessageOpenConnect);
		break; /* End of WM_INITDIALOG                                 */

	case WM_CLOSE:
		/* Closing the Dialog behaves the same as Cancel               */
		PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		break; /* End of WM_CLOSE                                      */

	case  GSSI_WINDOW_MESSAGE:
	{
		HANDLE hMessage = (HANDLE)wParam;
		LPSTR message = GlobalLock(hMessage);
		switch (currentMessage)
		{
		case GMSMessageFileUploaded:
			if (_process == UPLOAD_ERROR_FILE)
			{
				clearErrorFile();
				startSync(hWndDlg);
			}
			else if (!stricmp (message,"OK"))
			{
				_nextFileNumToUpload++;
				_uploadedFiles++;
				_processedFiles++;
				if (_totFilesToProcess)
					updateOverallProgress (hWndDlg, (float)_processedFiles / _totFilesToProcess);

				if (_process == UPLOAD_DATA_PROCESS)
					_upFiles--;
				else
					_upPix--;
				updateLabels(hWndDlg);
				uploadNextFile(hWndDlg);
			}
			else if (!strnicmp(message, "ERR1",4) &&
				_numAttemps++ < MAX_ATTEMPS)
			{
				Sleep( 2.0*_numAttemps);
				GMServerSendMessage(hWndDlg, GMSMessageFileUploaded);
			}
			else
			{
				LPSTRD title = malloc(strlen(message) + 64);
				sprintf (title,"Upload Size / Rename Error : %s",message);
				PopoverShow (_popoverView,0.1,title,"OK");
				logToErrorFile (title);
				free(title);
			}
			break;
		case GMSMessageCancelConnect:
			currentMessage = GMSMessageDefault;
			PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
			break;
		case GMSMessageSetNearby:
			iTimer = SetTimer(hWndDlg, 1, 5000, 0);
			break;
		case GMSMessageCheckNearby:
			if (strstr(message, "NO"))
			{
				if (iTimer)
					KillTimer(hWndDlg, iTimer);
				iTimer = 0;
				PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
			}
			break;
		case GMSMessageOpenConnect:
			if (strstr(message, "InvalidVersion"))
			{
				MessageBox(hWndDlg, "Invalid Version Message", 0, MB_ICONEXCLAMATION);
				break;
			}
			CRAPI_sharedInstance_savePlacesString(message);
			CRAPI_sharedInstance_processPlaces();
	

			if (CRAPI->sharedInstance.placesArray->count == 0)
			{
				SetWindowText(hWndDlg, "Synchronize Data - NO CITIES IN LIST");
				EnableWindow(GetDlgItem(hWndDlg, IDC_SYNCHRONIZE), FALSE);
				_didConnect = NO;

				sprintf (message,"%i - NO MANAGER",CRAPI->sharedInstance.GSSiPadNumber);
				SetWindowText(GetDlgItem(hWndDlg, IDC_CONNECT_LABEL),message);

				ShowWindow (GetDlgItem(hWndDlg,IDC_CONNECT),CRAPI->sharedInstance.hideConnectButton ? SW_HIDE : SW_SHOW);

				break;
			}

			_didConnect = YES;

			_selectedRow = 0;
			if (_selectedRow < 0)
				_selectedRow = UserDefaults_defaults_integerForKey("SelectedRow");
			else
			{
				UserDefaults_defaults_saveInteger(_selectedRow,"SelectedRow");
				_autoSync = TRUE;
			}
			if (CRAPI->sharedInstance.placesArray->count < 2 ||
				(int)CRAPI->sharedInstance.placesArray->count <= _selectedRow ||
				CRAPI->sharedInstance.currentGeoid == 0)
			{
				_selectedRow = 0;
				UserDefaults_defaults_saveInteger(_selectedRow, "SelectedRow");
			}

			CRAPI_sharedInstance_setGeoIDForCurrentApp ();
			rtn = CRAPI_sharedInstance_processPlacesArray (_selectedRow);
			setDatabaseIDToDB(_databaseID);

			sprintf(message, "Synchronize Data for\n%s",
				CRAPI->sharedInstance.currentCityNameAndState);
			
			SetWindowText(GetDlgItem(hWndDlg, IDC_TITLE), message);
			UpdateConnectLabel(hWndDlg);
			ShowWindow(GetDlgItem(hWndDlg, IDC_CONNECT), CRAPI->sharedInstance.hideConnectButton ? SW_HIDE : SW_SHOW);
			GMServerSendMessage(hWndDlg, GMSMessageGetUploadStatus);

			break;
		case GMSMessageGetUploadStatus:
		{
			_uploadStatusMsg = string_Copy(message);
			// NSString *edit = [_uploadStatusMsg stringByReplacingOccurrencesOfString:@"3-5" withString:@"1-5"];
			// _uploadStatusMsg = edit;
			_numiPadsInMsg = atoi(_uploadStatusMsg) + 1;
			_myiPad = CRAPI->sharedInstance.currentiPadWithinManager;

			/*if (_justDownloadedNewDatabase)
			{
				NSArray * ar = [_uploadStatusMsg componentsSeparatedByString : @"|"];
					for (int i = 0; i < _numiPadsInMsg; i++)
					{
						NSString * vals = ar[i + 1];
						NSArray<NSString *> * ivals = [vals componentsSeparatedByString : @"-"];
							int dataNum = (int)ivals[0].integerValue;
						int pictNum = (int)ivals[1].integerValue;
						if (i == _myiPad)
						{
							dataNum = 1;
							pictNum = 0;
						}
						[_databaseID setLastDataUpdateNum : dataNum
							pictNum : pictNum
							foriPad : i];
					}
				_justDownloadedNewDatabase = FALSE;
			}*/
			int pictNum = 0;
			int lastNum = getLastDataUpdateNumber(_databaseID, _myiPad, &pictNum);
			if (_myiPad > 0 || getUseMasterID())
			{
				EnableWindow(GetDlgItem(hWndDlg, IDC_SYNCHRONIZE), TRUE);
				SetWindowText(GetDlgItem(hWndDlg, IDC_SYNCHRONIZE), "Synchronize");
			}


			_lastFileNumToUpload = lastNum - 1; //last num will always be non-existent file since incrementLastDataFileNum called at start of sync

			_nextFileNumToUpload = (getDataNumFromServerMsg(_myiPad,NO)+ 1);
			if (_nextFileNumToUpload > 0)
				_upFiles = (_lastFileNumToUpload - _nextFileNumToUpload + 1);

			lastNum = pictNum;
			_lastPictNumToUpload = lastNum;

			_nextPictNumToUpload = (getDataNumFromServerMsg(_myiPad, YES) + 1);

			if (_nextPictNumToUpload > 0)
				_upPix = (_lastPictNumToUpload - _nextPictNumToUpload + 1);

			getDownloadCounts(NO);
			updateLabels(hWndDlg);
			updateOverallProgress(hWndDlg,0);
			//if (_autoSync)
			//	startSync(hWndDlg);
		}
		break;
		GSSiGlobUlFree(&hMessage);
		}
	}

	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			if (iTimer)
				KillTimer(hWndDlg, iTimer);
			iTimer = 0;
			if (currentMessage == GMSMessageSetNearby || currentMessage == GMSMessageCheckNearby)
			{
				GMServerSendMessage(hWndDlg, GMSMessageCancelConnect);
			}
			else
				EndDialog(hWndDlg, FALSE);
			break;
		case IDC_CONNECT:

		{
			GMServerSendMessage(hWndDlg, GMSMessageSetNearby);
		}
		break;
		case IDC_SYNCHRONIZE:
			startSync(hWndDlg);
		break;

		}
		break;    /* End of WM_COMMAND                                 */
case WM_TIMER:
{
	GMServerSendMessage(hWndDlg, GMSMessageCheckNearby);

}
	default:
		return FALSE;
	}
	return TRUE;
}

void CallSynchronizeMsgProc(void)
{
	int nRc = DialogBox(hInst, (LPSTR)"SYNCHRONIZE_DATA", hWndMain, (DLGPROC)SynchronizeMsgProc);
}
int getDataNumFromServerMsg (int iPad,BOOL wantPict)
{
	int dataNum = -4;

	if (strlen (_uploadStatusMsg) < 5)
		return -3;

	if (iPad > _numiPadsInMsg)
		return -2;

	LPSTR pMsg = malloc(strlen(_uploadStatusMsg) + 4);
	strcpy(pMsg, _uploadStatusMsg);
	LPSTR pPipe = strchr(pMsg, '|');
	int i = 0;

	while (pPipe)
	{
		pPipe++;

		if (i++ == iPad)
		{
			if (wantPict)
			{
				pPipe = strchr(pPipe, '-');

				if (!pPipe)
					break;

				pPipe++;
			}

			dataNum = atoi(pPipe);
			break;
		}

		pPipe = strchr(pPipe, '|');
	}

	free(pMsg);
	return dataNum;
}

void getDownloadCounts(BOOL loading)
{
	_downFiles = _downPix = 0;
	_totFilesToProcess = _origUpFiles;
	_processedFiles = 0;
	for (int iPad = 0; iPad <= _numiPadsInMsg; iPad++)
	{
		if (iPad != _myiPad)
		{
			int lastPictIHave;

			int lastDataIHave = getLastDataUpdateNumber(_databaseID, iPad, &lastPictIHave);


			int lastDataFileOnServer = getDataNumFromServerMsg(iPad,NO);

			int lastPictFileOnServer = getDataNumFromServerMsg(iPad, YES);

			_downFiles += max(0, lastDataFileOnServer - lastDataIHave);
			if (!loading || CRAPI->sharedInstance.downloadData)
				_totFilesToProcess += max(0, lastDataFileOnServer - lastDataIHave);
			if (!loading || CRAPI->sharedInstance.downloadPix)
			{
				_downPix += max(0, lastPictFileOnServer - lastPictIHave);
				_totFilesToProcess += max(0, lastPictFileOnServer - lastPictIHave);
			}
		}
	}
}

void updateLabels(HWND hWndDlg)
{
	if (_first)
	{
		_first = NO;
		if (_upFiles < 0 || _upPix < 0 || _downFiles < 0 || _downPix < 0)
		{
			LPSTRD msg = malloc(1024);
			sprintf (msg,"Negative numbers in sync : %i %i %i %i",_upFiles,_upPix,_downFiles,_downPix);
			if (getUseMasterID())
			{
				_upFiles = 0;
			}
			else
				logToErrorFile(msg);
		}
	}
	char label[256];
	sprintf(label, "Data Files to be Uploaded : %i", _upFiles);
	SetWindowText(GetDlgItem(hWndDlg, IDC_DATAFILESTOUPLOAD), label);
	sprintf(label, "Data Files to be Downloaded : %i", _downFiles);
	SetWindowText(GetDlgItem(hWndDlg, IDC_DATAFILESTODOWNLOAD), label);
	sprintf(label, "Pictures to be Downloaded : %i", _downPix);
	SetWindowText(GetDlgItem(hWndDlg, IDC_PICTURESTODOWNLOAD), label);

}
void updateOverallProgress(HWND hWndDlg, double pct)
{

}
void startSync (HWND hWndDlg)
{
	_autoSync = FALSE;
	_process = UPLOAD_DATA_PROCESS;
	_uploadedFiles = 0;
	_origUpFiles = _upFiles + _upPix;
	getDownloadCounts(YES);
	updateLabels(hWndDlg);
	uploadNextFile(hWndDlg);
}
void syncButtonTapped (HWND hWndDlg)
{
	EnableWindow(GetDlgItem(hWndDlg, IDC_SYNCHRONIZE), FALSE);
	CRAPI->sharedInstance.refreshMap = YES;

	if (CRAPI_sharedInstance_haveErrorLog())
	{
		_process = UPLOAD_ERROR_FILE;
		LPSTRD errorFile = CRAPI_sharedInstance_errorLogPath();
		HWND popoverView = 0;
		int dataType = FTPErrorFile;
		__time32_t time;
		int iTime = _time32(&time);
		double showAfter = 0.5;
		BOOL deleteWhenDone = NO;
		LPSTRD fromDir = stringByDeletingLastPathComponent(errorFile);
		LPSTRD fromFile = lastPathComponent(errorFile);
		char toDir[] = "NVCRISData/ErrorFiles";
		char title[] = "Upload Error File";
		sprintf (_errorFileName,"%i_%i.txt",CRAPI->sharedInstance.GSSiPadNumber,iTime);
		BOOL appendTempExtension = NO;
		BOOL st = PushFileToServer(fromFile, _errorFileName, fromDir, toDir, deleteWhenDone, appendTempExtension, &popoverView,title, showAfter,"FTPPushError");
		free(errorFile);
		free(fromDir);
		free(fromFile);
		if (!st)
		{
			LPSTRD mess = malloc(4096);
			strcpy(mess, "[FTPPushError]");
			ExpandText(mess);
			MessageBox(hWndDlg, mess, "Unable to send file to server", MB_ICONEXCLAMATION);
			free(mess);
		}
	}
	else
		startSync(hWndDlg);
}

void uploadNextFile (HWND hWndDlg)
{
	doUploadNextFile(hWndDlg);
}

void doUploadNextFile (HWND hWndDlg)
{
Top:
	if (_nextFileNumToUpload > _lastFileNumToUpload)
	{
		if (_process == UPLOAD_DATA_PROCESS)
		{
			int pictNum = 0;
			_process = UPLOAD_PICT_PROCESS;

			if (_uploadPix)
			{
				/*_myiPad = CRAPI.sharedInstance.currentiPadWithinManager;

				int lastNum = [_databaseID getLastDataUpdateNumber : _myiPad
					lastPictNum : &pictNum];

				lastNum = pictNum;
				_lastFileNumToUpload = lastNum;

				_nextFileNumToUpload = ([self getDataNumFromServerMsg : _myiPad
					wantPictNum : YES] + 1);

				if (_nextFileNumToUpload > 0)
					_upPix = (_lastFileNumToUpload - _nextFileNumToUpload + 1);

				[self updateLabels];
				*/
				goto Top;
			}
		}

		_process = DOWNLOAD_DATA_PROCESS;
		_downloadiPad = nextiPadToDownload(-1);

		if (_downloadiPad > -1)
		{
			setDownloadFilesFor(_downloadiPad);
			downloadNextFile(hWndDlg);
		}
		SetWindowText(GetDlgItem(hWndDlg, IDC_SYNCHRONIZE), "Complete");
		EnableWindow(GetDlgItem(hWndDlg, IDC_SYNCHRONIZE), FALSE);

		return;
	}

	if (_process == UPLOAD_DATA_PROCESS)
	{
		GSSiFree(&_filePath);
		_filePath = outputDataFile (_databaseID,_nextFileNumToUpload, _myiPad);
	}

	/*else if (_process == UPLOAD_PICT_PROCESS)
	{
		_filePath = [_databaseID outputPictFile : _nextFileNumToUpload
			iPad : _myiPad];

		[_popoverView showAfter : 0.5
			withTitle : @"Upload Picture"];
	}*/
	else
	{
		MessageBox(hWndDlg, "Invalid process", 0, MB_ICONEXCLAMATION);
		//[CRAPI.sharedInstance logToErrorFile : [NSString stringWithFormat : @"Invalid process : %i in upload %i %i",_process,_nextFileNumToUpload,_myiPad]];
			return;
	}
	int fileSize = GSSiLength(_filePath);

	if (!fileSize)
	{
/*		if ([[_filePath pathExtension] isEqualToString:@"jpg"])
		{
			NSError *error = nil;

			NSString *defaultPath = [NSBundle.mainBundle pathForResource : @"deletedImage"
				ofType : @"jpg"];
			[NSFileManager.defaultManager copyItemAtPath : defaultPath toPath : _filePath error : &error];
			if (error)
			{
				NSLog(@"Copy deletedImage.jpg ERROR : %@", error.localizedDescription);
				[CRAPI.sharedInstance logToErrorFile : [NSString stringWithFormat : @"Copy deletedImage to %@:%@",[_filePath lastPathComponent], error.localizedDescription]
					withHeader : @"Copy empty image"];

			}

		}
		else
		{
			NSString * emptyFileMessage = @"/*This file is empty* / ";
				[emptyFileMessage writeToFile : _filePath atomically : YES encoding : NSUTF8StringEncoding error : nil];
		}
		[CRAPI.sharedInstance logToErrorFile : [_filePath lastPathComponent]
			withHeader : @"Writing empty file"];*/
		MessageBox(hWndDlg, _filePath, "Empty File", MB_ICONEXCLAMATION);
	}

	if (_process == UPLOAD_DATA_PROCESS)
	{
		_dataType = FTPData;
		_sendDataStartTime = currentTime();
	}
	else
		_dataType = FTPDataPhoto;

	LPSTRD toDir = malloc(MAX_PATH);
	sprintf (toDir,"NVCRISData/%i/%i",CRAPI->sharedInstance.currentManageriPad, CRAPI->sharedInstance.currentGeoid);
	_lastUploadFileSize = GSSiLength(_filePath);
	LPSTRD fromDir = stringByDeletingLastPathComponent(_filePath);
	LPSTRD fromFile = lastPathComponent(_filePath);
	char title[128];
	sprintf(title,"Upload Data File:%s", fromFile);
	BOOL deleteWhenDone = FALSE;
	BOOL appendTempExtension = YES;
	BOOL st = PushFileToServer(fromFile, fromFile, fromDir, toDir, deleteWhenDone, appendTempExtension, &_popoverView,title, 0.5,"FTPPushError");
	free(fromDir);
	free(toDir);
	if (!st)
	{
		LPSTRD mess = malloc(4096);
		strcpy(mess,"[FTPPushError]");
		ExpandText(mess);
		FTPPushDidComplete(hWndDlg, _dataType, st, mess, fromFile);
		free(mess);
	}
	else
		FTPPushDidComplete(hWndDlg, _dataType, st, "", fromFile);
	free(fromFile);


}
void FTPPushDidComplete(HWND hWndDlg, FTPDataType dataType, FTPStatus status, LPSTR statusString, LPSTR path)
{
	if (status == FTPStatusOK)
	{
		PopoverHide(_popoverView);
		_numAttemps = 0;
		if (dataType != FTPDatabase)
			GMServerSendMessage(hWndDlg, GMSMessageFileUploaded);
	}

	else
	{
		LPSTRD msg = malloc(4096);
		sprintf(msg, "Unable to send file to server\n%s\n%s", statusString, path);
		logToErrorFile(msg);
		PopoverShow(_popoverView, 0, msg, "OK");
		free(msg);
	}

	//NSLog(@"FTP push completed : %@", statusString);
}

void FTPPullDidComplete(HWND hWndDlg, FTPDataType dataType, FTPStatus status, LPSTR statusString, LPSTR file,LPSTR dir)
{
	char path[MAX_PATH];
	sprintf(path, "%s\\%s", dir, file);
	if (status == FTPStatusOK || status == FTPStatusFileExists)
	{
		if (_process == DOWNLOAD_DATA_PROCESS)
		{
			if (loadUpdate(_databaseID, path))
			{
				setLastDataUpdateNum (_databaseID,_nextFileNumToDownload,_downloadiPad);
			}
			else
			{
				LPSTRD msg = malloc(1024);
				LPSTRD name = lastPathComponent(path);
				sprintf (msg,"Data load failed\n%@",name);
				MessageBox(0, msg, 0, MB_ICONEXCLAMATION);
				logToErrorFile(msg);
				free(msg);
				free(name);
				return;
			}
		}

		else if (_process == DOWNLOAD_PICT_PROCESS)
		{
			 setLastPictUpdateNum(_databaseID, _nextFileNumToDownload, _downloadiPad);
		}
		else if (_process == DOWNLOAD_DATABASE_PROCESS)
		{
			_justDownloadedNewDatabase = TRUE;
			PopoverHide(_popoverView);
			GMServerSendMessage(hWndDlg,GMSMessageGetUploadStatus);
			return;
		}
		_nextFileNumToDownload++;

		if (_process == DOWNLOAD_DATA_PROCESS)
			_downFiles--;
		else
			_downPix--;

		_processedFiles++;

		if (_totFilesToProcess)
			updateOverallProgress(hWndDlg, (float)_processedFiles / _totFilesToProcess);

		updateLabels(hWndDlg);
		downloadNextFile(hWndDlg);
	}
	else
	{
		LPSTRD msg = malloc(4096);
		sprintf(msg, "Unable to get file from server\n%s\n%s", statusString, file);
		logToErrorFile(msg);
		PopoverShow(_popoverView, 0, msg, "OK");
		free(msg);
	}

	//NSLog(@"FTP push completed : %@", statusString);
}

/*
-(void)FTPdidComplete:(FTPDataType)dataType
withStatus : (FTPStatus)status
	andStatusString : (NSString *)statusString
	filePath : (NSString *)path
{

	if (status == FTPStatusOK || status == FTPStatusFileExists)
	{
		{
			if (_process == DOWNLOAD_DATA_PROCESS)
			{
				if ([self loadUpdate : path])
				{
					[_databaseID setLastDataUpdateNum : _nextFileNumToDownload
						foriPad : _downloadiPad];
				}

				else
				{
					NSString *msg = [NSString stringWithFormat : @"Data load failed\n%@",[path lastPathComponent]];
					_popoverView.title = msg;
					[CRAPI.sharedInstance logToErrorFile : msg];
					return;
				}
			}

			else if (_process == DOWNLOAD_PICT_PROCESS)
			{
				[_databaseID setLastPictUpdateNum : _nextFileNumToDownload
					foriPad : _downloadiPad];
			}
			else if (_process == DOWNLOAD_DATABASE_PROCESS)
			{
				_justDownloadedNewDatabase = TRUE;
				[_popoverView hide];
				[self sendGMServerMessage : GMSMessageGetUploadStatus];
				return;
			}
			_nextFileNumToDownload++;

			if (_process == DOWNLOAD_DATA_PROCESS)

				_downFiles--;
			else
				_downPix--;

			_processedFiles++;

			if (_totFilesToProcess)
				[self updateOverallProgress : (float)_processedFiles / _totFilesToProcess];

			[self updateLabels];
			[self downloadNextFile];
		}
	}

	else
	{
		NSString * msg = [NSString stringWithFormat : @"%@\n%@",statusString, [path lastPathComponent]];
		_popoverView.title = msg;
		[CRAPI.sharedInstance logToErrorFile : msg];
		if (_showErrorPopover)
		{
			[_popoverView setCancelButtonTitle : @"OK"];
			[_popoverView show];
		}
	}
}

-(BOOL)loadUpdate:(NSString*)path
{
	return[_databaseID updateFromFile : path];
}
*/
int nextiPadToDownload(int currentiPad)
{
	currentiPad++;

	if (currentiPad == _myiPad && !getUseMasterID())
		currentiPad++;

	if (currentiPad > _numiPadsInMsg)
		return -1;

	return currentiPad;
}
void downloadNextFile (HWND hWndDlg)
{
	if (_process == DOWNLOAD_DATA_PROCESS && !_downloadData)
		_process = DOWNLOAD_PICT_PROCESS;

	if (_process == DOWNLOAD_PICT_PROCESS && !_downloadPix)
	{
		SetWindowText(GetDlgItem(hWndDlg, IDC_SYNCHRONIZE), "Complete");
		EnableWindow(GetDlgItem(hWndDlg, IDC_SYNCHRONIZE), FALSE);
		PopoverHide(_popoverView);
		return;
	}

	doDownloadNextFile(hWndDlg);
}


void doDownloadNextFile(HWND hWndDlg)
{
Top:
	if (_nextFileNumToDownload > _lastFileNumToDownload)
	{
		_downloadiPad = nextiPadToDownload (_downloadiPad);

		if (_downloadiPad < 0)
		{
			if (_process == DOWNLOAD_DATA_PROCESS)
			{
				_process = DOWNLOAD_PICT_PROCESS;
				if (!_downloadPix)
					goto Done;
				_downloadiPad = nextiPadToDownload(0);

				if (_downloadiPad > -1)
				{
					setDownloadFilesFor(_downloadiPad);
					goto Top;
				}
				else
					goto Done;
			}
			else
			{
			Done:
				SetWindowText(GetDlgItem(hWndDlg, IDC_SYNCHRONIZE), "Complete");
				EnableWindow(GetDlgItem(hWndDlg, IDC_SYNCHRONIZE), FALSE);
				PopoverHide(_popoverView);
				return;
			}
		}

		else
		{
			setDownloadFilesFor(_downloadiPad);
			goto Top;
		}
	}

	if (_process == DOWNLOAD_DATA_PROCESS)
	{
		_filePath = outputDataFile(_databaseID, _nextFileNumToDownload, _myiPad);
	}

	else if (_process == DOWNLOAD_PICT_PROCESS)
	{
		_filePath = outputPictFile(_databaseID, _nextFileNumToDownload, _myiPad);
	}
	else
	{
		LPSTRD mess = malloc(1024);
		sprintf (mess, "Invalid process : %i in download %i %i", _process, _nextFileNumToDownload, _downloadiPad);
		logToErrorFile(mess);
		free(mess);
		return;
	}


	LPSTRD fromDir = malloc(MAX_PATH);
	LPSTRD toDir = stringByDeletingLastPathComponent(_filePath);
	LPSTRD fromFile = lastPathComponent(_filePath);
	LPSTRD title = malloc(1024);
	sprintf (fromDir,"NVCRISData/%i/%i",CRAPI->sharedInstance.currentManageriPad, CRAPI->sharedInstance.currentGeoid);
	if (_process == DOWNLOAD_DATA_PROCESS)
	{
		_dataType = FTPData;
		sprintf(title, "Download Data\n%s", fromFile);
	}
	else
	{
		_dataType = FTPDataPhoto;
		sprintf (title,"Download Photo\n%s",fromFile);
	}
	BOOL st = GetFileFromServer(fromFile,fromFile, fromDir, toDir,&_popoverView, title, 0.5,"FTPPullError");
	free(fromDir);
	if (!st)
	{
		LPSTRD mess = malloc(4096);
		strcpy(mess, "[FTPPullError]");
		ExpandText(mess);
		FTPPullDidComplete(hWndDlg, _dataType, st, mess, fromFile,toDir);
		free(mess);
	}
	else
		FTPPullDidComplete(hWndDlg, _dataType, st, "", fromFile,toDir);
	free(toDir);
	free(fromFile);

}
void setDownloadFilesFor(int iPad)
{
	int lastPictIHave = 0, fileSize = 0;

	int lastDataIHave = getLastDataUpdateNumber (_databaseID ,iPad, &lastPictIHave);
	int lastDataFileOnServer = getDataNumFromServerMsg(iPad,NO);

	int lastPictFileOnServer = getDataNumFromServerMsg(iPad, YES);
	if (_process == DOWNLOAD_DATA_PROCESS)
	{
		_nextFileNumToDownload = lastDataIHave + 1;
		_lastFileNumToDownload = lastDataFileOnServer;
	}
	else
	{
		_nextFileNumToDownload = lastPictIHave + 1;
		_lastFileNumToDownload = lastPictFileOnServer;
	}
}

void GSSiFree(LPSTR *str)
{
	if (str)
	{
		if (*str)
		{
			free(*str);
			*str = 0;
		}
	}
}

void PopoverHide(HWND hWnd)
{
	return;
}

void PopoverShow(HWND hWnd, double after, LPSTR msg, LPSTR OKButtonText)
{

}