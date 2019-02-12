#include "graphint.h"   
#include "gmextern.h"    
#include "resource.h"
#include "CRAPI.h"

static char  Winver[32];
static BOOL _didConnect;
static BOOL _autoSync;
static int  _selectedRow;
static char _minManageVersion[64];
static char _minPrelimVersion[64];
static char _minDetailVersion[64];
static char _minAlleyVersion[64];
static UINT iTimer = 0;

void connectButtonTapped(void);


#define DOWNLOAD_DATABASE_FUNCTION 3
#define FTP_CATCHUP_DELAY 0.5
#define UPLOAD_STATUS_VERSION 1
#define MAX_ATTEMPS 3

CRAPi *CRAPI=NULL;

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
		CRAPI->sharedInstance.placesArray = NSArray_Init(0);
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

LPSTR string_Copy(LPSTR str)
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

LPSTR textAfterFirstChar(LPSTR string, char c)
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

LPSTR textAfterLastChar(LPSTR string, char c)
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

LPSTR textBeforeLastChar(LPSTR string, char c)
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

LPSTR UserDefaults_defaults_stringForKey(LPSTR key)
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
				l = pEnd - pStart + 1;
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
	sprintf (key,"ManagerForGeoid:%i", CRAPI->sharedInstance.currentGeoidValue);
	UserDefaults_defaults_saveInteger(CRAPI->sharedInstance.currentManageriPad, key);

	sprintf (key,"iPadWithinManagerForGeoid:%i", CRAPI->sharedInstance.currentGeoidValue);

	UserDefaults_defaults_saveInteger(CRAPI->sharedInstance.currentiPadWithinManager, key);

	sprintf (key,"totiPadWithinManagerForGeoid:%i", CRAPI->sharedInstance.currentGeoidValue);

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
	if (geoid != CRAPI->sharedInstance.currentGeoidValue)
	{
		CRAPI->sharedInstance.currentGeoidValue = geoid;
		loadiPadParametersForGeoid(geoid);
		if (CRAPI->sharedInstance.currentManageriPad == 0)
		{
			CRAPI->sharedInstance.currentManageriPad = CRAPI->sharedInstance.GSSiPadNumber;
			CRAPI->sharedInstance.currentiPadWithinManager = 1;
			CRAPI->sharedInstance.totiPadWithinManager = 1;
			saveiPadIDs();
		}
		/*
		_outputDirectory = [NSString stringWithFormat : @"%@/%i / ADAFiles", _documentsDirectory, _currentGeoidValue];
			_imageDirectory = [NSString stringWithFormat : @"%@/%i / IntersectionImages", UserDefaults.defaults.sharedFilePath, _currentGeoidValue];
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

BOOL  CRAPI_sharedInstance_processPlaces (void)
{
	LPSTR message = UserDefaults_defaults_stringForKey("PlacesString");

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
		LPSTR str = textBeforeLastChar(instr, '*');
		LPSTR controlStr = textAfterLastChar(instr, '*');
		CityOrOrganizationData c = CityOrOrganizationData_init();
		c->controlledInterface = ControlChar(controlStr, 1);
		c->allowSidewalk = ControlChar(controlStr, 2);
		strcpy(c->fullDescription,str);
		c->name = textInsideParentheses(str);
		c->number = integerValue(textBeforeFirstChar(str,'('));
		LPSTR manager = textAfterLastChar(str,')');
		c->managerNumber = integerValue(textBeforeFirstChar(manager, '-'));
		c->withinManager = integerValue(textAfterFirstChar(manager, '-'));
		c->totWithinManager = integerValue(textAfterString(manager, "of"));
		free(manager);
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
			if (c->number == CRAPI->sharedInstance.currentGeoidValue)
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

	UserDefaults_defaults_saveInteger(CRAPI->sharedInstance.currentGeoidValue,"CurrentGeoid");

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

void setDatabaseIDToDB (void)
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
	
	sprintf(UUIDString, "PCIb-%31.31ld", Serno);
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
BOOL FAR PASCAL SynchronizeMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	static int currentMessage = GMSMessageDefault;
	char mes[1024];
	int	BRtn;
	BOOL rtn;
	if ((BRtn = DIALOGSTYLEMsgProc(hWndDlg, Message, wParam, lParam)))
		return (BRtn);
	switch (Message)
	{
	case WM_INITDIALOG:
		CRAPI = CRAPI_Init();
		cwCenter(hWndDlg, 0);
		sprintf(mes, "@$M(GetCitiesForiPad,%s@,,%s)", deviceInfo(), appAndVersion());
		currentMessage = GMSMessageOpenConnect;
		SendMessageToServer(hWndDlg, mes);
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
				CRAPI->sharedInstance.currentGeoidValue == 0)
			{
				_selectedRow = 0;
				UserDefaults_defaults_saveInteger(_selectedRow, "SelectedRow");
			}

			CRAPI_sharedInstance_setGeoIDForCurrentApp ();
			rtn = CRAPI_sharedInstance_processPlacesArray (_selectedRow);
			setDatabaseIDToDB();

			sprintf(message, "Synchronize Data for\n%s",
				CRAPI->sharedInstance.currentCityNameAndState);
			
			SetWindowText(GetDlgItem(hWndDlg, IDC_TITLE), message);
			UpdateConnectLabel(hWndDlg);
			ShowWindow(GetDlgItem(hWndDlg, IDC_CONNECT), CRAPI->sharedInstance.hideConnectButton ? SW_HIDE : SW_SHOW);
			sprintf(mes, "@$M(GetUploadStatus,%i,%i,%i,%i)",
				CRAPI->sharedInstance.currentManageriPad, CRAPI->sharedInstance.currentGeoidValue,
				CRAPI->sharedInstance.totiPadWithinManager, UPLOAD_STATUS_VERSION);
			currentMessage = GMSMessageGetUploadStatus;
			SendMessageToServer(hWndDlg, mes);

			break;
		case GMSMessageGetUploadStatus:
			if (strstr(message, "InvalidVersion"))
				ii = 1;
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
				sprintf(mes, "@$M(RemoveNearbyiPad,%s,0 0)", vendorUUIDString());
				currentMessage = GMSMessageCancelConnect;
				SendMessageToServer(hWndDlg, mes);
			}
			else
				EndDialog(hWndDlg, FALSE);
			break;
		case IDC_CONNECT:

		{
			sprintf(mes, "@$M(SetNearbyiPad,%s,%s,0 0)", vendorUUIDString(), NodeName);
			currentMessage = GMSMessageSetNearby;
			SendMessageToServer(hWndDlg, mes);
		}
		break;

		}
		break;    /* End of WM_COMMAND                                 */
case WM_TIMER:
{
	sprintf(mes, "@$M(CheckNearbyiPad,%s)", vendorUUIDString());
	currentMessage = GMSMessageCheckNearby;
	SendMessageToServer(hWndDlg, mes);

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

