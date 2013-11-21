BOOL GetStateID (short StateNum, LPSTR ID);  
short GetStateNum (LPSTR ID);
BOOL GetCountyName (short State,short County,LPSTR CountyName);
long GetNearCity (DPOINT DPoint,short Type,LPSTR InCity,LPDOUBLE Dist, LPDOUBLE az);
BOOL IsInCity (double Dist, long Pop);
long GetPop (LPSTR UDI);
BOOL GetCityCoord (LPSTR Name,LPDPOINT pDPoint,LPMNMXCORD pMinMax);
void FixCaps (LPSTR str);  
void DisplayPNData (long Refno,HANDLE hIntData,HANDLE hPNAddData);
BOOL GetPNData (HANDLE hDB,long iref);
HANDLE DecodePNAddData (HANDLE hPNAddData);
void AZtoDirection2(double az,LPSTR str);
double GetLocalCorrection (LPMNMXCORD pBounds);
long CanZipCharToNum (LPSTR CZip);
LPSTR CanZipNumToChar (long nlong,LPSTR CZip);
HANDLE PNOpen (void);
void PNClose (HANDLE DBHandle);

