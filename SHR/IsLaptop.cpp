#define _WIN32_DCOM 
 
#include < iostream > 
using namespace std; 
 
#include < comdef.h > 
#include < Wbemidl.h > 
#include <tchar.h>
 
#pragma comment(lib, "wbemuuid.lib") 
 
static int numMon;
static HMONITOR hMonIn = 0, hOtherMon;

extern "C" RECT MonitorRectangle[4];

class WMIQuery 
{ 
   IWbemLocator* m_pLocator; 
   IWbemServices* m_pServices; 
 
public: 
   WMIQuery(): 
      m_pLocator(NULL), 
      m_pServices(NULL) 
   { 
   } 
 
   bool Initialize() 
   { 
      // Obtain the initial locator to WMI 
      HRESULT hr = ::CoCreateInstance( 
         CLSID_WbemLocator, 
         0, 
         CLSCTX_INPROC_SERVER, 
         IID_IWbemLocator, (LPVOID *) &m_pLocator); 
 
      if (FAILED(hr)) 
      { 
         cerr << "Failed to create IWbemLocator object. Err code = 0x" << hex << hr << endl; 
         return false; 
      } 
 
      // Connect to WMI through the IWbemLocator::ConnectServer method 
      // Connect to the root\cimv2 namespace with the current user 
      hr = m_pLocator->ConnectServer( 
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace 
         NULL,                    // User name. NULL = current user 
         NULL,                    // User password. NULL = current 
         0,                       // Locale. NULL indicates current 
         NULL,                    // Security flags. 
         0,                       // Authority (e.g. Kerberos) 
         0,                       // Context object 
         &m_pServices             // pointer to IWbemServices proxy 
         ); 
 
      if (FAILED(hr)) 
      { 
         cerr << "Could not connect. Error code = 0x" << hex << hr << endl; 
         m_pLocator->Release(); 
         m_pLocator = NULL; 
         return false; 
      } 
 
      // Set security levels on the proxy 
      hr = ::CoSetProxyBlanket( 
         m_pServices,                 // Indicates the proxy to set 
         RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx 
         RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx 
         NULL,                        // Server principal name 
         RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
         RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx 
         NULL,                        // client identity 
         EOAC_NONE                    // proxy capabilities 
         ); 
 
      if (FAILED(hr)) 
      { 
         cerr << "Could not set proxy blanket. Error code = 0x" << hex << hr << endl; 
         m_pServices->Release(); 
         m_pServices = NULL; 
         m_pLocator->Release(); 
         m_pLocator = NULL; 
         return false; 
      } 
 
      return true; 
   } 
 
   IEnumWbemClassObject* Query(LPCTSTR strquery) 
   { 
      IEnumWbemClassObject* pEnumerator = NULL; 
      HRESULT hr = m_pServices->ExecQuery( 
         bstr_t("WQL"), 
         bstr_t(strquery), 
         WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
         NULL, 
         &pEnumerator); 
 
      if (FAILED(hr)) 
      { 
         cerr << "Query for operating system name failed. Error code = 0x" << hex << hr << endl; 
         return NULL; 
      } 
 
      return pEnumerator; 
   } 
 
   ~WMIQuery() 
   { 
      if(m_pServices != NULL) 
      { 
         m_pServices->Release(); 
         m_pServices = NULL; 
      } 
 
      if(m_pLocator != NULL) 
      { 
         m_pLocator->Release(); 
         m_pLocator = NULL; 
      } 
   } 
}; 
 
extern "C" int isLaptop(int ii)
{
	HRESULT hres;

	// Initialize COM. 
	/*hres =  ::CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
	cout << "Failed to initialize COM library. Error code = 0x" << hex << hres << endl;
	return 1;
	}
	*/
	// Set general COM security levels 
	hres = ::CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication 
		NULL,                        // Authentication services 
		NULL,                        // Reserved 
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation 
		NULL,                        // Authentication info 
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved 
		);

	if (FAILED(hres))
	{
		//     cout << "Failed to initialize security. Error code = 0x" << hex << hres << endl; 
		//      ::CoUninitialize(); 
		return 1;
	}
	else
	{
		WMIQuery query;
		if (query.Initialize())
		{
			IEnumWbemClassObject* pEnumerator = query.Query(_T("SELECT * FROM Win32_SystemEnclosure"));

			if (pEnumerator != NULL)
			{
				// Get the data from the query 
				IWbemClassObject *pclsObj;
				ULONG uReturn = 0;

				while (pEnumerator)
				{
					HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

					if (0 == uReturn)
					{
						break;
					}

					VARIANT vtProp;

					hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
					wcout << "Name:    " << vtProp.bstrVal << endl;

					hr = pclsObj->Get(L"ChassisTypes", 0, &vtProp, 0, 0);
					wcout << "Chassis: ";
					SAFEARRAY* parrValues = NULL;

					if (vtProp.vt & VT_ARRAY)
					{
						if (VT_BYREF & vtProp.vt)
							parrValues = *vtProp.pparray;
						else
							parrValues = vtProp.parray;
					}

					if (parrValues != NULL)
					{
						SAFEARRAYBOUND arrayBounds[1];
						arrayBounds[0].lLbound = 0;
						arrayBounds[0].cElements = 0;

						SafeArrayGetLBound(parrValues, 1, &arrayBounds[0].lLbound);
						SafeArrayGetUBound(parrValues, 1, (long*)&arrayBounds[0].cElements);
						arrayBounds[0].cElements -= arrayBounds[0].lLbound;
						arrayBounds[0].cElements += 1;

						if (arrayBounds[0].cElements > 0)
						{
							for (ULONG i = 0; i < arrayBounds[0].cElements; i++)
							{
								LONG lIndex = (LONG)i;
								INT item;

								HRESULT hr = ::SafeArrayGetElement(parrValues, &lIndex, &item);

								if (SUCCEEDED(hr))
								{
									LPCTSTR szType = NULL;
									switch (item)
									{
									case 1: szType = _T("Other"); break;
									case 2: szType = _T("Unknown"); break;
									case 3: szType = _T("Desktop"); break;
									case 4: szType = _T("Low Profile Desktop"); break;
									case 5: szType = _T("Pizza Box"); break;
									case 6: szType = _T("Mini Tower"); break;
									case 7: szType = _T("Tower"); break;
									case 8: szType = _T("Portable"); break;
									case 9: szType = _T("Laptop"); break;
									case 10:szType = _T("Notebook"); break;
									case 11:szType = _T("Hand Held"); break;
									case 12:szType = _T("Docking Station"); break;
									case 13:szType = _T("All in One"); break;
									case 14:szType = _T("Sub Notebook"); break;
									case 15:szType = _T("Space-Saving"); break;
									case 16:szType = _T("Lunch Box"); break;
									case 17:szType = _T("Main System Chassis"); break;
									case 18:szType = _T("Expansion Chassis"); break;
									case 19:szType = _T("SubChassis"); break;
									case 20:szType = _T("Bus Expansion Chassis"); break;
									case 21:szType = _T("Peripheral Chassis"); break;
									case 22:szType = _T("Storage Chassis"); break;
									case 23:szType = _T("Rack Mount Chassis"); break;
									case 24:szType = _T("Sealed-Case PC"); break;
									}
									wcout << szType;
									if (i + 1 < arrayBounds[0].cElements)
										wcout << ", ";
								}
							}

							wcout << endl;
						}
					}

					VariantClear(&vtProp);

					pclsObj->Release();
				}

				pEnumerator->Release();
			}
		}
	}

	//   ::CoUninitialize(); 

	return 0;
}
	extern "C" int ChassisType(void) // returns -1 if error, 1 for desktop, 2 for laptop, 3 for handheld and 4 for other
	{
		HRESULT hres;
		int rtn = -1;

		// Initialize COM. 
		/*hres =  ::CoInitializeEx(0, COINIT_MULTITHREADED);
		if (FAILED(hres))
		{
		cout << "Failed to initialize COM library. Error code = 0x" << hex << hres << endl;
		return 1;
		}
		*/
		// Set general COM security levels 
		hres = ::CoInitializeSecurity(
			NULL,
			-1,                          // COM authentication 
			NULL,                        // Authentication services 
			NULL,                        // Reserved 
			RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
			RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation 
			NULL,                        // Authentication info 
			EOAC_NONE,                   // Additional capabilities 
			NULL                         // Reserved 
			);

		if (FAILED(hres))
		{
			//     cout << "Failed to initialize security. Error code = 0x" << hex << hres << endl; 
			//      ::CoUninitialize(); 
			return rtn;
		}
		else
		{
			WMIQuery query;
			if (query.Initialize())
			{
				IEnumWbemClassObject* pEnumerator = query.Query(_T("SELECT * FROM Win32_SystemEnclosure"));

				if (pEnumerator != NULL)
				{
					// Get the data from the query 
					IWbemClassObject *pclsObj;
					ULONG uReturn = 0;

					while (pEnumerator)
					{
						HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

						if (0 == uReturn)
						{
							break;
						}

						VARIANT vtProp;

						hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
						//wcout << "Name:    " << vtProp.bstrVal << endl;

						hr = pclsObj->Get(L"ChassisTypes", 0, &vtProp, 0, 0);
						//wcout << "Chassis: ";
						SAFEARRAY* parrValues = NULL;

						if (vtProp.vt & VT_ARRAY)
						{
							if (VT_BYREF & vtProp.vt)
								parrValues = *vtProp.pparray;
							else
								parrValues = vtProp.parray;
						}

						if (parrValues != NULL)
						{
							SAFEARRAYBOUND arrayBounds[1];
							arrayBounds[0].lLbound = 0;
							arrayBounds[0].cElements = 0;

							SafeArrayGetLBound(parrValues, 1, &arrayBounds[0].lLbound);
							SafeArrayGetUBound(parrValues, 1, (long*)&arrayBounds[0].cElements);
							arrayBounds[0].cElements -= arrayBounds[0].lLbound;
							arrayBounds[0].cElements += 1;

							if (arrayBounds[0].cElements > 0)
							{
								for (ULONG i = 0; i < arrayBounds[0].cElements; i++)
								{
									LONG lIndex = (LONG)i;
									INT item;

									HRESULT hr = ::SafeArrayGetElement(parrValues, &lIndex, &item);

									if (SUCCEEDED(hr))
									{
										LPCTSTR szType = NULL;
										switch (item)
										{
										case 1: szType = _T("Other"); break;
										case 2: szType = _T("Unknown"); break;
										case 3: szType = _T("Desktop"); break;
										case 4: szType = _T("Low Profile Desktop"); break;
										case 5: szType = _T("Pizza Box"); break;
										case 6: szType = _T("Mini Tower"); break;
										case 7: szType = _T("Tower"); break;
										case 8: szType = _T("Portable"); break;
										case 9: szType = _T("Laptop"); break;
										case 10:szType = _T("Notebook"); break;
										case 11:szType = _T("Hand Held"); break;
										case 12:szType = _T("Docking Station"); break;
										case 13:szType = _T("All in One"); break;
										case 14:szType = _T("Sub Notebook"); break;
										case 15:szType = _T("Space-Saving"); break;
										case 16:szType = _T("Lunch Box"); break;
										case 17:szType = _T("Main System Chassis"); break;
										case 18:szType = _T("Expansion Chassis"); break;
										case 19:szType = _T("SubChassis"); break;
										case 20:szType = _T("Bus Expansion Chassis"); break;
										case 21:szType = _T("Peripheral Chassis"); break;
										case 22:szType = _T("Storage Chassis"); break;
										case 23:szType = _T("Rack Mount Chassis"); break;
										case 24:szType = _T("Sealed-Case PC"); break;
										}
										switch (item)
										{
										case 3:
										case 4:
										case 5:
										case 6:
										case 7:
											rtn = 1;
											break;
										case 9:
										case 10:
										case 14:
											rtn = 2;
											break;
										case 11:
											rtn = 3;
											break;
										default:
											rtn = 4;
											break;
										}
										//wcout << szType;
										//if (i + 1 < arrayBounds[0].cElements)
										//	wcout << ", ";
									}
								}

								//wcout << endl;
							}
						}

						VariantClear(&vtProp);

						pclsObj->Release();
					}

					pEnumerator->Release();
				}
			}
		}

		//   ::CoUninitialize(); 

		return rtn;
	}

	extern "C" int MonitorType(int whichDisplay, LPSTR monName) // returns -1 if error, 1 for desktop, 2 for laptop, 3 for handheld and 4 for other
	{
		HRESULT hres;
		int rtn = -1;

		// Initialize COM. 
		/*hres =  ::CoInitializeEx(0, COINIT_MULTITHREADED);
		if (FAILED(hres))
		{
		cout << "Failed to initialize COM library. Error code = 0x" << hex << hres << endl;
		return 1;
		}

		// Set general COM security levels
		hres = ::CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities
		NULL                         // Reserved
		);

		if (FAILED(hres))
		{
		//     cout << "Failed to initialize security. Error code = 0x" << hex << hres << endl;
		//      ::CoUninitialize();
		return rtn;
		}
		else*/
		{
			WMIQuery query;
			if (query.Initialize())
			{
				IEnumWbemClassObject* pEnumerator = query.Query(_T("SELECT * FROM Win32_DesktopMonitor"));

				if (pEnumerator != NULL)
				{
					// Get the data from the query 
					IWbemClassObject *pclsObj;
					ULONG uReturn = 0;

					while (pEnumerator)
					{
						HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

						if (0 == uReturn)
						{
							break;
						}

						VARIANT vtProp;
						BSTR	name;

						hr = pclsObj->Get(L"Availability", 0, &vtProp, 0, 0);
						rtn = vtProp.iVal;
						VariantClear(&vtProp);
						//hr = pclsObj->Get(L"Caption", 0, &vtProp, 0, 0);
						//VariantClear(&vtProp);
						//hr = pclsObj->Get(L"CreationClassName", 0, &vtProp, 0, 0);
						//hr = pclsObj->Get(L"Description", 0, &vtProp, 0, 0);
						//hr = pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);
						hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
						_bstr_t b(vtProp.bstrVal);
						const char* c = b;
						strcpy(monName, c);

						//wstrcpy(name,vtProp.bstrVal);
						VariantClear(&vtProp);
						hr = pclsObj->Get(L"Status", 0, &vtProp, 0, 0);
						VariantClear(&vtProp);
						//hr = pclsObj->Get(L"SystemName", 0, &vtProp, 0, 0);
						//hr = pclsObj->Get(L"MonitorManufacturer", 0, &vtProp, 0, 0);
						//hr = pclsObj->Get(L"MonitorType", 0, &vtProp, 0, 0);
						//hr = pclsObj->Get(L"PNPDeviceID", 0, &vtProp, 0, 0);
						//hr = pclsObj->Get(L"DisplayType", 0, &vtProp, 0, 0);

						//VariantClear(&vtProp);

						pclsObj->Release();
					}

					pEnumerator->Release();
				}
			}
		}


		return rtn;
	}
	extern "C" int MouseType(void) // returns -1 if error, 1 for desktop, 2 for laptop, 3 for handheld and 4 for other
	{
		HRESULT hres;
		int rtn = -1;
		int avail, statusinfo;
		char		mouseName[256];
		char		mouseDesc[256];

			WMIQuery query;
			if (query.Initialize())
			{
				IEnumWbemClassObject* pEnumerator = query.Query(_T("SELECT * FROM Win32_PointingDevice"));

				if (pEnumerator != NULL)
				{
					// Get the data from the query 
					IWbemClassObject *pclsObj;
					ULONG uReturn = 0;

					while (pEnumerator)
					{
						HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

						if (0 == uReturn)
						{
							break;
						}

						VARIANT vtProp;
						BSTR	name;

						hr = pclsObj->Get(L"ConfigManagerErrorCode", 0, &vtProp, 0, 0);
						rtn = vtProp.iVal;
						VariantClear(&vtProp);
						hr = pclsObj->Get(L"Availability", 0, &vtProp, 0, 0);
						rtn = vtProp.iVal;
						avail = vtProp.intVal;
						VariantClear(&vtProp);
						hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
						{
							_bstr_t b(vtProp.bstrVal);
							const char* c = b;
							strcpy(mouseName, c);

							//wstrcpy(name,vtProp.bstrVal);
							VariantClear(&vtProp);
						}
						hr = pclsObj->Get(L"Description", 0, &vtProp, 0, 0);
						{
							_bstr_t b(vtProp.bstrVal);
							const char* c = b;
							strcpy(mouseDesc, c);

							//wstrcpy(name,vtProp.bstrVal);
							VariantClear(&vtProp);
						}
						hr = pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);
						{
							_bstr_t b(vtProp.bstrVal);
							const char* c = b;
							//strcpy(monName, c);

							//wstrcpy(name,vtProp.bstrVal);
							VariantClear(&vtProp);
						}
						hr = pclsObj->Get(L"Status", 0, &vtProp, 0, 0);
						VariantClear(&vtProp);
						hr = pclsObj->Get(L"StatusInfo", 0, &vtProp, 0, 0);
						rtn = vtProp.iVal;
						VariantClear(&vtProp);

						pclsObj->Release();
					}

					pEnumerator->Release();
				}
			}


		return rtn;
	}

	extern "C" BOOL IsPointOnTouchScreen(HWND hWnd,POINT pt)
	{
		BOOL rtn = FALSE;
		HMONITOR hMonitor;
		MONITORINFOEX mi;

		ClientToScreen(hWnd, &pt);

		// 
		// get the nearest monitor to the passed rect. 
		// 
		hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

		// 
		// get the work area or entire monitor rect. 
		// 
		if (hMonitor)
		{
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);
		}

		return rtn;
	}

	BOOL CALLBACK MonitorEnumProc(
		_In_  HMONITOR hMonitor,
		_In_  HDC hdcMonitor,
		_In_  LPRECT lprcMonitor,
		_In_  LPARAM dwData
		)
	{
		MONITORINFOEX mi;
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(hMonitor, &mi);
		MonitorRectangle[numMon++] = *lprcMonitor;

		if (hMonIn && hMonIn != hMonitor)
		{
			hOtherMon = hMonitor;
			return FALSE;
		}
		return TRUE;
	}


	extern "C" HMONITOR GetOtherMonitor(POINT pt)
	{
		HMONITOR rtn;
		
		numMon = 0;
		hMonIn = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, NULL);
		hMonIn = 0;
		return hOtherMon;
	}

		extern "C" int GetNumMonitors(void)
		{
			DWORD deviceNum = 0;
			DISPLAY_DEVICE displayDevice;
			DEVMODE devMode;

			memset(&displayDevice, 0, sizeof(displayDevice));
			displayDevice.cb = sizeof(DISPLAY_DEVICE);
			while (EnumDisplayDevices(NULL, deviceNum, &displayDevice, 0))
			{
				EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
				//printf("deviceNum:%d\n", deviceNum);
				//printf("DeviceName:%s\n", displayDevice.DeviceName);
				//printf("DeviceString:%s\n\n", displayDevice.DeviceString);
				++deviceNum;
			}
			numMon = 0;
			EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, NULL);

			numMon = GetSystemMetrics(SM_CMONITORS);

			HWND hWnd = GetDesktopWindow();
			RECT rect;
			GetClientRect(hWnd,&rect);
			return numMon;
		}

