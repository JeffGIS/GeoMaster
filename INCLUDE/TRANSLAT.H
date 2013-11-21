LPSTR GetTIGERFileExtension(char FileType);  
BOOL LoadCFCCTable (void);
//BOOL LoadCitiesTable (void);
//void ConvertClass (LPSTR str);
//void MIFToTIGER (short nPoly,LPSHORT lpnPntsIn,LPHANDLE phDPoints,LPLONG pTLID); 
BOOL GetMIFCharacteristics (HFILE FidMIF,LPSTR str,LPLONG lineno);
//DPOINT  OrthoToGround(DPOINT Ortho,LPTRN lptr);
void GetTLIDShapePoints (long TLID,LPSHORT NumShapes,HPDPOINT lpShapes,HFILE FidTIGER2);
BOOL WantThisTLID (long TLID,LPTIGER1 Tiger1,LPBOOL WantAreas, short Pass,HANDLE hCFCC);
BOOL GetMIDData(HFILE FidMID,LPSTR lpMIDstr,HANDLE hDLT); 
short StreetNameType (LPSTR STName);
short GetIntersectStuff (long TLID, HANDLE hIntersect,LPTIGER1 Tiger1,LPSHORT Stuff);
BOOL GetNextStreet (long TLID, long Snum, HANDLE hIntersect,LPLONG Long, LPLONG Lat, LPLONG NextSNum);
//void CreateStateLev (short State,HWND hWndDlg,int StatusCntl,int MessCntl);
DWORD MIFColorToWinColor (COLORREF MIFColor);
BOOL GetDOQCoord (LPSTR File,LPMNMXCORD pBounds, LPDOUBLE pResolution,LPRECT32 ClipRect,HFILE *Fid, LPHANDLE phDibInfo, LPLONG ImageOffset);
short AddPNExtraData (LPTIGER1 Tiger1,LPSHORT pStuff); 

                                   
                                   