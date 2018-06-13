#ifndef address_h
#define address_h
#define IDM_HOUSE                   2010
#define IDM_STREET                  2020
#define IDM_STREET_MENU             2030   

#define MAXSTREETNAMELEN			32    

#define	TRUENAM_INDEX	1
#define	STDNAM_INDEX	2
#define	NMONLY_INDEX	3
#define	SANSCH_INDEX	4	
#define	NANDCH_INDEX	5	
#define	SANSCP_INDEX	6	
#define	SANSCS_INDEX	7
	
typedef	struct	{long	Munic;
				 long	ZIP;
				 } MUNICZIPS;
typedef MUNICZIPS	FAR	*LPMUNICZIPS;   
typedef	struct	{char	FullName[64];
				 char	Abv[8];
				 char	StateCode[2];
				 char	CountyName[64];
				 } MUNICNAME;
typedef MUNICNAME	FAR	*LPMUNICNAME;   
typedef	struct	{char	ALTName[64];
				 long	Munic;
				 } ALTMUNICNAME;
typedef ALTMUNICNAME	FAR	*LPALTMUNICNAME;   

typedef struct  {
				long	StreetNum;
		 		long	ZIPCode;
		 		short	Side;
         		long	MaxHouseNum;
         		long	Segid;
         		}  SEGMAXKEY; 
typedef	SEGMAXKEY	FAR	*LPSEGMAXKEY;       
         
typedef struct{
            long    StreetNum;
            char    TrueNameUC[32],
                    StdName[32],
                    NamePortionOfStdName[32],
                    StdNameWOHeading[32],
                    StdNameWOType[32],
                    NonReorderedNameNoPrefixPortion[32],
                    NonReorderedNameNoSuffixPortion[32],
                    NonReorderedNamePortion[32],
                    NonCompressedNamePortion[32],
                    OriginalNamePortion[32], 
                    FEDIRP[2],
                    FENAME[30],
                    FETYPE[4],
                    FEDIRS[2],
                    TrueName[32];
        } STREETNAMETABLE;  
typedef STREETNAMETABLE FAR *LPSTREETNAMETABLE;
            
typedef struct {
				short	MatchCode, 	  //0=No match,1=Match no change, 2=Match minor change, 4=multiple match
						LocationCode; //1=House-Street, 2=Intersection match,3=Block Center,5=Match to Precinct,6 = User locate questionable, 7=User located, 8=User selected from list, 9=match to zip, 10=unmatchable
				long	StreetNum;
				long	HouseNum;
				long	IntID;
				long	StreetNum1;
				long	StreetNum2; 
				long	Munic;
				long	ZIP;  
				DPOINT	Point;
				BOOL	ChangedMunic,
						ChangedZIP;
				} ADDMATCH_ver1; 
typedef ADDMATCH_ver1	FAR	*LPADDMATCH_ver1;

typedef struct {
				short	MatchCode, 	  //0=No match,1=Match no change, 2=Match minor change, 4=multiple match
						LocationCode; //1=House-Street, 2=Intersection match,3=Block Center,4=Point Based,5=Precinct Centroid,6 = User locate questionable, 7=User located, 8=User selected from list(point), 9=match to zip, 10=unmatchable, 11=OnFromTo match,12=User selected from list(onfromto),13=User Located(onfromto)
				long	StreetNum;
				long	HouseNum;
				long	IntID;
				long	StreetNum1;
				long	StreetNum2; 
				STREETMP	Street1MP;
				STREETMP	Street2MP;
				long	Munic;
				long	ZIP;  
				DPOINT	Point;
				short	ChangedMunic,
						ChangedZIP;
				short	PartsRemoved; 
				char	TAGPrefix[8];
				char	TAGUDI[64];
				long	OnStreetNum;
				long	IntIDTo;
				DPOINT	Point2;
				} ADDMATCH; 
typedef ADDMATCH	FAR	*LPADDMATCH;

typedef struct {
				short		MatchCode;
				long		RecNum;       
				} INTMATCHEDITKEY1; 
typedef INTMATCHEDITKEY1	FAR	*LPINTMATCHEDITKEY1;    

typedef struct {
				long		RecordNum;
				ADDMATCH	IM;       
				double		Dist, Dir, Offset;
				char		StreetA[32], StreetB[32];
				char		OrigKey[256];
				} INTMATCHEDITREC; 
typedef INTMATCHEDITREC	FAR	*LPINTMATCHEDITREC;    

typedef struct {
				short	MatchCodeFrom,
						MatchCodeTo, 	  //0=No match,1=Match no change, 2=Match minor change, 4=multiple match
						LocationCodeFrom,
						LocationCodeTo; //1=House-Street, 2=Intersection match,3=Block Center,6 = User locate questionable, 7=User located, 8=User selected from list, 9=match to zip, 10=unmatchable
				long	IntIDFrom;
				long	IntIDTo; 
				long	StreetNumOn;
				long	StreetNumFrom;
				long	StreetNumTo; 
				STREETMP	FromMP;
				STREETMP	ToMP;
				long	MunicFrom;
				long	MunicTo;
				long	ZIPFrom;  
				long	ZIPTo;
				DPOINT	PointFrom;
				DPOINT	PointTo;
				short	ChangedMunicFrom,
						ChangedMunicTo,
						ChangedZIPFrom,
						ChangedZIPTo;
				} OFTMATCH; 
typedef OFTMATCH	FAR	*LPOFTMATCH;

typedef struct {
				long		RecordNum;
				OFTMATCH	IM;       
				float		Dir;
				double		Offset; 
				long		NumSegsFound;
				char		OnStreet[32],StreetA[32], StreetB[32];
				char		OrigKeyx[128];//variable type based on input
				} STREET_SEGS_BETWEEN_INTSMATCHEDITREC; 
typedef STREET_SEGS_BETWEEN_INTSMATCHEDITREC	FAR	*LPSTREET_SEGS_BETWEEN_INTSMATCHEDITREC;    

typedef struct {
				long		IntID; 
				short		NumStreets;
				long		Accidents;       
				long		ADT;
				double		AccidentRate, 
							AveAccidentRate,
				 			AccidentRate90,
							AccidentRate95,
							AccidentRate995,
							AccidentRate999;
				short		AccidentLevel;
				} INTACCIDENTREC; 
typedef INTACCIDENTREC	FAR	*LPINTACCIDENTREC;    

typedef struct {
				//long		RecordNum;
				ADDMATCH	AM;       
				char		Street[80]; 
				char		House[12];
				char		City[32];
				char		ZIP[12];   
				char		Symbol[64];
				long		FromDate;
				long		ToDate;
				char		Note[100];
				//char		OrigKey[128]; 
				} ADDMATCHEDITREC; 
typedef ADDMATCHEDITREC	FAR	*LPADDMATCHEDITREC;    

typedef struct {
				long		Key; 
				long		TotRecords,
							NumMatched,
							NumNotMatched,
							NumMultiMatched;
				float		PCTMatched;
				} ADDMATCHBYAREA; 
typedef ADDMATCHBYAREA	FAR	*LPADDMATCHBYAREA;    

typedef struct {
				char		Street[64]; 
				long		HouseNum;
				long		Munic;
				} USERLOCATEDADDRESSKEY; 
typedef USERLOCATEDADDRESSKEY	FAR	*LPUSERLOCATEDADDRESSKEY;    

typedef struct {
				char	Name[40];
				long	RecNum;
				} BADNAMEKEY;

typedef struct {long	Path;
				double	MP;
				} NETREFSKEY;
typedef struct {long	Ref;
				short	Dir;
				} NETREFSDATA;    
typedef struct {long	Ref;
				short	NetID;
				long	Path;
				} NETLINKSKEY;
typedef struct {double	MP;
				double	Length; 
				} NETLINKSDATA;
typedef struct {short	MarkerID;
				long	Path; 
				char	Prefix[2], Suffix[2];
				double	Value;
				} NETMARKERSKEY1;
typedef struct {DPOINT	Location;
				double	MP;
				} NETMARKERSDATA1;
typedef struct {short	MarkerID;
				long	Path; 
				double	MP;
				} NETMARKERSKEY2;
typedef struct {
				char	Prefix[2], Suffix[2];
				double	Value;
				} NETMARKERSDATA2;   
typedef struct {
				long	Refno,
						IntID;
				} NETREFINTKEY;   
typedef struct {
				 short	WhichEnd;
				 DPOINT	Coord;
				} NETREFINTDATA;   
typedef struct {
				long	IntID,
						Refno;
				} NETINTREFKEY;   
typedef struct {
				DPOINT	Coord;
				double	AZ,
						Length;
				short	WhichEnd;
				} NETINTREFDATA;   
typedef struct {
				long	Path;
				double	MP;
				} NETINTMPKEY; 
typedef struct {
				long	Path1,
						Path2,
						IntID;
				} NETINTPATHSKEY;   
typedef struct {
				DPOINT	Point;
				long	Munics[5];
				}INTPATHSDATA;
typedef struct {
				long	Path; 
				short	Dir;
				short	View;
				short	MarkerID;
				char	Prefix[2], Suffix[2];
				double	Value;
				short	BegOrEnd;
				} NETVIDEOKEY1;
typedef struct {
				char	DiskID[16];
				long	Frame;
				} NETVIDEODATA1;
typedef struct {
				char	DiskID[16];
				long	Frame;
				} NETVIDEOKEY2;
typedef struct {
				long	Path; 
				short	Dir;
				short	View; 
				short	MarkerID;
				char	Prefix[2], Suffix[2];
				double	Value;
				short	BegOrEnd;
				} NETVIDEODATA2; 

typedef struct {
				long	FromRef;
				short	FromEnd;
				long	ToRef;
				short	ToEnd; 
				long	Time;
				} TURNKEY;

typedef	struct
	{ long	TLID,
			StreetNum[4],
			faddl, faddr, taddl, taddr,
			ZIPL,
			ZIPR;
	  int	
			STATEL,
			STATER,
			COUNTYL,
			COUNTYR;
	  long
			FPLL,
			FPLR;
	  char
			CTBNAL[6],
			CTBNAR[6],
			BLKL[4],
			BLKR[4];
	  long
			FMCDL, FMCDR;
	  long	TrafVol; 
	  long	FromTLID;
	  short Lanes;
	  short	Filler[3];
	  short	ChangedFlag;
	  float	Width;
	  short	Speed,
	  		OneWay;
	  char	CFCC[3];
	} SEGDATAGM_V1; 
typedef	SEGDATAGM_V1	FAR	*LPSEGDATAGM_V1;	

//static char numpad[11] = "1234567890?";
//static char keypad[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ/#";

BOOL FAR PASCAL LOCATION_OFFSETMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL LOC_INTERSECTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL LOC_STREETMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL ADDEDIT_HELPERMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL ABVEDITMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL ADDLOC_FROMINTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL STREET_SEGS_BETWEEN_INTSMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL ADDLOC_FROMADDMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);  
BOOL FAR PASCAL INTACCIDPROFMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);  
BOOL FAR PASCAL TEST_STREETMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL ADDRESS_EDITMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL STREET_NAME_EDITMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL ADDRESS1MsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL ADDRESSPIDMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL LOCATEPIDMsgProc(HWND, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL INTERSECT_MATCH_EDITMsgProc(HWND hWndDlg,int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL ADD_MATCH_EDITMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL SetModeless(BOOL set);

BOOL OpenPointAddressTable (BOOL Update);
void ClosePointAddressTable (void);
short DisplayStreetsINT (HWND hDlg,USHORT iMenu, LPSTR InName,short nchar,UINT EntryControl,short FindOpt,BOOL CheckForSegs);
//void CreateSegData (void);
void CreateIntersectionFile (BOOL GeoMaster,HWND hWndStatus);
short GetNumZIPsInMunic (long Munic,LPHANDLE phZIPsInMunic);
long GetGSStreetNum (LPSTR Name);
BOOL AddressLocation1 (HWND hWnd, HINSTANCE hInst,UINT iopt);
BOOL OpenGSStreetNames (int Mode,int which);
void CloseGSStreetNames(void);
BOOL LocatePID (HWND hWnd, HINSTANCE hInst);
BOOL AddressLocationPID (HWND hWnd, HINSTANCE hInst);
//BOOL NetworkLocation (HWND hWnd, HINSTANCE hInst, LPDPOINT Point);
short DisplayStreets (HWND hDlg,LONG House, int OddEven, LPSTR InName,int nchar,USHORT EntryControl);
short DisplayStreetsPID (HWND hDlg,LONG HouseMin, long HouseMax, short OddEven, LPSTR InName,short nchar,long MunicNum,USHORT EntryControl);
short DisplayStreetsPIDFromStreetNum (HWND hDlg,LONG HouseMin, LONG HouseMax, short OddEven, long InStreet,long WantMunicNum,USHORT EntryControl);
void GetFullAdd (LONG House, LPSTR Street, LPSTR FullAddress);
//void LoadAddressTable(void);
BOOL CreateStreetNameTable (LPSTR Dir);
BOOL OpenStreetNameTable (BOOL Update);
BOOL OpenStreetSegmentTable (BOOL Update,LPBOOL pOpenedSeg);
void CloseStreetSegmentTable (BOOL Opened);
BOOL SetAddressDir (void);
void CloseStreetNameTable (void);
long GetStreetNumFromName (LPSTR Name,short Index, short FirstOrNext, LPSTR TrueName);
long AddStreetName (LPSTR TrueName,long num,LPSTR FEDIRP,LPSTR FENAME,LPSTR FETYPE, LPSTR FEDIRS);
BOOL CreateNetLinkAndRef (LPSTR NetworkDir,short NetworkID);
BOOL SetNetworkDir (void);
BOOL CreateNetRefIntersect (LPSTR NetworkDir,short NetworkID);
BOOL DeleteStreetNetwork (void);
BOOL OpenNetLinkAndRef  (short NetworkID,BOOL Update,LPBOOL Opened);
void CloseNetLinkAndRef (BOOL Opened);
void PutNetRefAndLink (int NetID,long Path,long Ref,int RefDir,double MP,double Length,int NetDir); 
BOOL OpenNetMarkers  (short NetworkID,BOOL Update,LPBOOL Opened);
void CloseNetMarkers (BOOL Opened);
BOOL CreateNetMarkers (LPSTR NetworkDir,short NetworkID);
BOOL CreateNetIntersect (LPSTR NetworkDir,short NetworkID);
BOOL OpenNetIntersect (short NetworkID,BOOL Update,LPBOOL Opened);
void CloseNetIntersect (BOOL Opened);
HANDLE AddMunicsToInts (HANDLE hBTNetIntPaths,LPSTR File); 
BOOL CreateNetVideoIndex (LPSTR NetworkDir,short NetworkID);
BOOL OpenNetVideoIndex  (short NetworkID,BOOL Update,LPBOOL Opened);
void CloseNetVideoIndex (BOOL Opened);
BOOL GetTrueStreetName (long SNum, LPSTR TrueName,long State,int index);
HANDLE CreateAddLocTable (LPSTR Name,short UDIFieldLen, short AdditionalFieldLen,BOOL New);
short INT_MATCH (LPSTR Street1, LPSTR Street2, LPSTR City, long ZIP, short MOPT, LPHANDLE hHandle,
				 LPLONG	pStreetNum1, LPLONG pStreetNum2,LPLONG pMunicNum);
short OFT_MATCH (LPSTR OnStreet,LPSTR FromStreet, LPSTR ToStreet, LPSTR City, long ZIP, short MOPT, LPHANDLE hHandle,
				 LPLONG pOnStreetNum, LPLONG pFromStreetNum, LPLONG pToStreetNum,LPLONG pMunicNum,LPINT pNumMatch1,LPINT pNumMatch2,LPHANDLE phMatch1,LPHANDLE phMatch2);
short INT_MATCH_DLG (HWND hWndDlg,UINT List1, UINT List2, long Munic, long ZIP, short MOPT,LPHANDLE hMatch,
					 LPLONG	pStreetNum1, LPLONG pStreetNum2,HWND hwnddlg1,HWND hwnddlg2);
short ADD_MATCH (LPSTR Street, LPSTR House, LPSTR Munic, LPSTR ZIP, short MOPT,LPHANDLE hMatch,
				 LPLONG	pStreetNum,LPLONG pMunicNum,BOOL UsePointBased,BOOL UseNetBased,HWND hWndDlg,short nControls,LPINT Controls);
BOOL ReloadSTNDTables (void);  
BOOL CreateINT_MATCHTable (LPSTR File, LPSTR BadNames, short OrigKeyLen);
BOOL CreateADD_MATCHTable (LPSTR File,LPSTR BadNames,LPSTR KeyDef,short AddUDILength);
BOOL FindNonNetworkedStreets (void);
BOOL RemoveNonNetworkedStreets (void);
BOOL MatchAddress (int MatchCode, int nList,HANDLE hList,long House, long Munic, long ZIP,BOOL BlockCenter, LPSHORT pNumMatch,LPHANDLE phMatch,BOOL UsePointBased,BOOL UseNetBased,HWND hWndDlg,int nControls,LPINT Controls);
BOOL MatchAddress2 (int MatchCode,long StreetNum,long House,long Munic,long ZIP, BOOL BlockCenter,LPSHORT pNumMatch, LPHANDLE phMatch,BOOL UsePointBased,BOOL UseNetBased);
BOOL MatchAddress3 (int MatchCode,long StreetNum,long House,long ZIP, long OrigZIP,long Munic, BOOL BlockCenter,LPSHORT pNumMatch, LPHANDLE phMatch,BOOL UsePointBased,BOOL UseNetBased);
BOOL BuildSegMaxIndex (HWND hWnd);
BOOL OpenSegMaxIndex (LPBOOL pOpened);
void CloseSegMaxIndex (BOOL Opened);
//BOOL ConvertGeospanData (HWND hWnd);  
BOOL GetSegDataGM (long TLID,LPSEGDATAGM pSegdata);  
double GetSegmentSpeed (LPSEGDATAGM pSegdata,double AtCost);
double GetIntersectionCost (LPSEGDATAGM pSegdataAt,LPSEGDATAGM pSegdataNext,BOOL ReverseNextSeg,int NumPicked);  
double GetTurnCost (long FromRef,short FromEnd,long ToRef,short ToEnd);
//BOOL AddSpeed (void);    
BOOL DumpUserAdd (void);
BOOL SaveStreetPolys (void);
BOOL OpenStreetPolys (LPBOOL pOpened);
void CloseStreetPolys (BOOL Opened);
BOOL UnloadStreets (HWND hWnd);
BOOL ReloadStreets (HWND hWnd);
void UpdateSNamesFromHlt (void);
BOOL CreateTurnTable (LPSTR NetworkDir);
BOOL OpenTurnTable (BOOL Update,LPBOOL Opened);
void CloseTurnTable (BOOL Opened);
BOOL CreatePointAddressTable (LPSTR Dir);
void InitAddMatchDir (LPSTR AddMatchDir);  
//BOOL ConvertSSTV1toV2 (LPSTR File);
//void OpenAddressFiles (HWND hWnd);
BOOL OpenAddressFilesPID (HWND hWnd);
void CloseAddressFilesPID (void);
void CloseAddressFiles (BOOL Final);
//void MarkStreetFile (HWND hWnd);
void HighlightStreet (HWND hWnd, long StrNum);
long PIDAddRefno (long Offset,LPSTR AddUDI);
BOOL CreateStreetSegmentTable (LPSTR File);
void SetAddEditValues (LPSTR House,LPSTR Street,LPSTR City,LPSTR ZIP,LPSTR OutVar,LPSTR Macro,LPADDMATCH pMatch);
void ShowStreetMatches (LPSTR StreetBuf,HWND hWndDlg,short idc_list,LPMNMXCORD pBounds);
long ChangeSimilarStreets (LPSTR DestName,LPSTR OrigName,long RecordNum,LPHANDLE hChangeRecs);
short GetAllInts (short MatchCode, short nList1,HANDLE hList1,long Munic,LPHANDLE hMatch,HWND hwnddlg1,HWND hwnddlg2);
short MatchIntLists (short MatchCode, short nList1,short nList2,HANDLE hList1,HANDLE hList2,long Munic,LPHANDLE hMatch,HWND hwnddlg1,HWND hwnddlg2);
short GetNameTypeList (short Type,LPSHORT nList,HANDLE hList,
								  LPSTR STDNAM,LPSTR NRONAM,LPSTR NMONLY,
                            	  LPSTR SANSCH,LPSTR NANDCH,LPSTR NCMPNM,LPSTR ORIGNM, LPSTR SANSCP, LPSTR SANSCS);
short GetMatchingNames (short NameType, LPSTR Name,LPSHORT nList,HANDLE hList);
void DisplayCurStreets (BOOL Clear,int Flash);
long AddToCurStreets (HWND hWndDlg,long CurPath,LPMNMXCORD TotMinMax,LPINT pDisplayedStreets);
BOOL GetMunicName (long MCD,LPSTR Name,LPSTR Abv);
BOOL GetZIPCenter (long ZIP,LPDPOINT Point);    
long GetMunicFromName (LPSTR MunName);
long GetStreetNumFromRawName (LPSTR Name,LPSTR TrueName);
long StandardizedMunic (long Munic);
BOOL SeparateIntStreets (LPSTR Street,LPSTR Street1,LPSTR Street2);
void SetSecondaryIntInput(HWND hWnd);
void SetSecondaryAddInput(HWND hWnd);
void SetSecondaryTAGInput(HWND hWnd);
void SetIntMatchControl(HWND hWndDlg, UINT control);
void SetAddMatchControl(HWND hWndDlg, UINT control);
void SetTAGMatchControl(HWND hWndDlg, UINT control);
BOOL SeparateOnFromToStreets(LPSTR StreetIN, LPSTR OnStreet, LPSTR FromStreet, LPSTR ToStreet);
BOOL AddToUserDefinedAddress (LPUSERLOCATEDADDRESSKEY pULAddKey, LPADDMATCH	pAM);
BOOL AddToUserDefinedAddressFromFile (LPSTR File);
BOOL FoundUserAssignedAddress (long IHouse,LPSTR Street,long IMunic,BOOL BlockCenter,LPHANDLE phMatch);
BOOL CreateUserDefinedAddress (LPSTR AddressDir,LPSTR Ver);
BOOL OpenUserDefinedAddress (BOOL Update,LPBOOL Opened);
void CloseUserDefinedAddress (BOOL Opened);
short GetNearHouse (long SNum,long WantHouse,long Munic,long ZIP,LPLONG pNearHouse,LPDPOINT pPoint);
void DumpIntersectionStreets (LPSTR Name,BOOL EliminateDuplicateStreets);
BOOL OpenZIPBounds (LPBOOL pOpened);
BOOL GetZIPBounds (long Zip,LPMNMXCORD pBounds);
void CloseZIPBounds (BOOL Opened);
long DrawStreet2 (long StreetNum,long WantZIP,COLORREF Color,short Width,BOOL HighlightStreet,LPMNMXCORD pTotMinMax,BOOL LimToMinMax);
int GetNumStreetSegs (long StreetNum,long WantZIP,LPMNMXCORD pTotMinMax,BOOL LimToMinMax, LPMNMXCORD pBounds, int DrawingOption,COLORREF Color,int Width);
BOOL CreateStnameFilesUpdate (void);
void ReadSegData (long Offset, LPSEGDATA pSegdata);
//void WriteSegData (long Offset, LPSEGDATA pSegdata);
//BOOL UpdateSegData (long TLID, LPSEGDATA pSegdata); 
LPSTR GetHouseAndStreet (LPSTR pStreet,LPSTR HouseNum);
long DumpMunicNameTable (LPSTR FileName);
BOOL ReloadMunicNameTable (LPSTR FileName);
BOOL AddUserAddress (LPSTR Address,LPSTR Munic,DPOINT Point,BOOL Unmatchable);
short AddMatchSingle (LPSTR HouseAndStreet,LPSTR City,LPSTR ZIP,LPSTR OutAddress,LPDPOINT OutCoord,LPADDMATCH pMatchOut,int matchOpt);
BOOL CreateAddMatchByArea (LPSTR InFile,LPSTR OutFile,LPSTR AreaName);
BOOL CreateSEGBETWEEN_INT_MATCHTable (LPSTR File,char OrigKeyType,short OrigKeyLen);
BOOL CreateSEGBETWEEN_INT_SEGS (LPSTR File,char OrigKeyType,short OrigKeyLen);
COLORREF GetRoadColor (int n);
BOOL AddZipCenterMatch (HWND hWndDlg,UINT icntl,long ZipCode);
short AddPrecinctCenterMatch (HWND hWndDlg,UINT icntl,long ZipCode);
int GetMapQuestLocation (LPSTR FullAddress,LPSTR Quality,LPDPOINT pPoint,int MaxAcceptableQuality);
int GetGoogleLocation(LPSTR FullAddressIN, int wantMatch, LPSTR formattedAddress, LPDPOINT pLocPoint, LPBOOL pHaveVPPoints, LPDPOINT pVPPoints, LPSTR locType, LPSTR types);

BOOL AddMapQuestMatch(HWND hWndDlg, UINT icntl, LPSTR FullAddress);
BOOL ReverseGeocodeCommand (int nArgs,LPSTR *Arg,LPSTR OutLoc);
BOOL GeocodeAlltypes(HWND hWnd,LPSTR OutLoc, LPSTR Arg1);

#endif









