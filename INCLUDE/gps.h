void GetGPSModels (HWND hWndDlg,UINT Control);
void GetGPSPorts (HWND hWndDlg,UINT Control);
void GetGPSBaud (HWND hWndDlg,UINT Control);
BOOL GPSImportWP (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL);
short GPSExportWP (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL);
short GPSExportRoute (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL);
BOOL GPSImportTrack (HWND hWndDlg,UINT ListControl,UINT StatusControl);
BOOL GPSImportTrackGarmin (HWND hWndDlg,UINT ListControl,UINT StatusControl);
BOOL GPSImportTrackLSI100 (HWND hWndDlg,UINT ListControl,UINT StatusControl);
BOOL GPSImportTrackMagellan (HWND hWndDlg,UINT ListControl,UINT StatusControl);
BOOL GPSOpen(HWND hWnd,LPSTR Identity,int Format,LPSTR ForcePort,LPSTR ForceBaud,BOOL ShowError);
void GPSClose (LPHANDLE pStream);
short GetGPSFormat (LPSTR Model);
BOOL SendPacketSerial (HANDLE Stream,LPBYTE Packet);
BOOL SendPacket (HANDLE Stream,LPBYTE Packet);
DWORD SendPacketUSB (LPBYTE Packet);
short GetPacketSerial (HANDLE Stream,LPBYTE Packet);
DWORD GetPacketUSB (LPBYTE Packet);
short GetPacket (HANDLE Stream,LPBYTE Packet);




