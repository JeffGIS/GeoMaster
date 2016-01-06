#include <windows.h>
#include <winbase.h>
#include <winnt.h> 
//include <windowsx.h>  
#ifdef	_WIN32_WCE
#include <winuser.h>
#endif
#include <float.h>   
#include <math.h>
#include <stdio.h> 
#include <io.h>
#include <dos.h>
#include <conio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <shlwapi.h>
#include <limits.h>
#include <memory.h>
#include <stdlib.h>     
#include <fcntl.h>
#include <direct.h>
#include <stdio.h>
#include <ctype.h>
//#include <winsock2.h>
#include <dlgs.h>    
//#include <print.h>
#include <sql.h>
//include <sqlext.h>
//#include <mmsystem.h> 
#include <shellapi.h>  
#include <assert.h>
#include <malloc.h>   
#include <dde.h>
#include <ddeml.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <fcntl.h>
#include "FreeImage.h"
#include <process.h>
//include <compobj.h>
//include <scode.h>
//#include <bios.h>

#include <tchar.h>
#include <strsafe.h>
//#include <toolhelp.h>

#include "gssitype.h"
#include "compute.h"
#include "dibapi.h"
#include "sqlite3.h"

#define ORIGINALPROC(hWnd) (WNDPROC) MAKELONG( \
    GetProp(hWnd, "PrLO"), GetProp(hWnd, "PrHI") )

#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

#define	OF_CREATE_NODELETE	0x1111      

#define UP_3D		1
#define DOWN_3D		2
#define	NO_3D		3

#define VARSPACE_LOCAL	1
#define VARSPACE_GLOBAL	2

#define GSSI_REINITDIALOG       0x5110  
#define GSSI_ADDGF 0x5111
#define GSSI_REPOSITION 0x5112

#define GSSI_GPSX	0x5113
#define GSSI_GPSY	0x5114
#define GSSI_GPSZ	0x5115
#define GSSI_GPSwnd	0x5116
#define GSSi_DimMenu 0x5117
#define GSSI_GPS	0x5118

#define GSSI_GMEDITCOMPLETE 0x5119


#define MK_DIGITIZER_BUTTON	    0x0020

#define GetWValue(rgb)  ((BYTE)((rgb)>>24))
#define RGBW(r,g,b,w)   ((COLORREF)(((BYTE)(r)|((WORD)(g)<<8))|(((DWORD)(BYTE)(b))<<16)|(((DWORD)(BYTE)(w))<<24)))

typedef struct {double a,b,c,d,e,f,xc,yc;} TRN; 
typedef TRN FAR *LPTRN;

#define	MAXFILEHANDLES	950  
#define JOURNAL_BLOCK_SIZE	1024	

#define  CFTCM   2.83170164938e-2
#define  CMCFT   3.5314454833912e1
#define  CMCYD   1.3079427716264e0
#define  CYDCM   7.645594553315e-1
#define  FTM     3.04800609601219e-1
#define  HAFSEC    1.388888888888889e-5
#define  HALFPI    1.570796326794896e0
#define  MFT       3.280833333333333e0
#define  PIHALF    4.712388980384689e0
#define  PY        3.141592653589793e0
#define  RADDEG    1.74532925199433e-2
#define  DEGRAD    5.729577951308232e1
#define  SFTSM     9.2903411613274e-2
#define  SMSFT     1.076386736111111e1
#define  TWOPI     6.283185307179586e0
#define  ACRSM     4046.872609874253e0  
#define  INCHES_PER_CM  0.3937e0
#define  MAX_GAP_BTWN_LINKS    0.1e0 
#define  ZERO$  -2146450000 
#define  TMPRF$ -2146483000   
#define	 NULLREF 0
#define RADtoDEG 57.295779513082322
#define DEGtoRAD 0.017453292519943296

#define  GOOD_SOUND 1
#define  BAD_SOUND  2  

#define BA_EXPANDTEXT	1
#define BA_FUNCTION		2
#define BA_BEGINBLOCK	3
#define BA_NEXTPOS		4
#define BA_NEXTLINE		5
#define BA_NEXTBP		6
#define BA_SHOWFUN		7
#define BA_RETURN		8

#define OFS_MAXPATHNAMEGM 256
typedef struct _OFSTRUCTGM {
	BYTE cBytes;
	BYTE fFixedDisk;
	WORD nErrCode;
	WORD Reserved1;
	WORD Reserved2;
	CHAR szPathName[OFS_MAXPATHNAMEGM];
} OFSTRUCTGM, *LPOFSTRUCTGM, *POFSTRUCTGM;

HWND WindowExists(HWND hWnd);
HWND FindWindowByName (LPSTR WindowName);
HWND GetTopParent(HWND	hWnd);
void ShowHideWindows(LPSTR WindowName, UINT fun);
void ShowHideChildren(HWND hWndPar, UINT fun);

int SQLOK(int sqlReturn, sqlite3* database, char *method, char ** error);

HBITMAP GetToolBitmap (LPSTR BMPath);
BOOL WaitForProcessToEnd (DWORD pID,LPINT pMaxWait);
void UnixTimeToFileTime(time_t t, LPFILETIME pft);
void UnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst);
__int64 HighLowToint64(DWORD HighPart,DWORD LowPart);
__int64 FileTimeToint64(FILETIME ft);
long Time64toTime32 (time_t time64);
int	GSSiEnterProg (int progid);
int	GSSiExitProg (int progid);
int SetLastMessage (long mes);
int checkcpl (int i);
int abcd (int i);
BOOL ValidateLicense(HWND hWnd);
COLORREF ConvertColor (COLORREF Color,int UseHalfTone);
BOOL CopyFileExtended (LPSTR ToFile,LPSTR FromFile);
BOOL CopyFileToCache (LPSTR ToFile, LPSTR FromFile);
int DisplayBitmapOnButton (HDC hDCBtn,LPRECT pRectBtn,HBITMAP hBM);
void HandleFocusState(LPDRAWITEMSTRUCT lpdis);
void Draw3DFrame(HWND hWnd, int iStyle);
int TB_SetupTable(HWND hWndDlg,HWND Tablehwnd,HWND Tableheadhwnd,HWND Tablefoothwnd,LPARAM rows,int cols,
							int far* colwidths,int interspace,int usersize);
int   TB_FitToWindow(HWND tblhwnd,int numcols);
int   TB_BestFit(HWND tblhwnd,int extra,LPARAM count,FARPROC fp);
int  TB_RedrawTable(HWND hWnd);
POINT POINTStoPOINT(POINTS points);
POINTS POINTtoPOINTS(POINT point);    
BOOL FAR PASCAL DATAFILEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam, UINT cntlSQL,
                                UINT cntlSET_FILE, UINT cntlDATABASE_LIST, UINT cntlTABLE_NAMES,UINT cntlTABLE_NAMES_TITLE,
                                LPUINT pcntlFIELD_NAMES,int NumFieldLists,
                                LPSTR DataFile, short *DataFileType, HANDLE *hThemeDB,
                                LPBOOL pFieldListIsCB, BOOL WantBrackets);
BOOL FAR PASCAL FIELDSMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL CENFIELDSMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
void ClientRectToScreenRect (HWND hWnd,LPRECT pRect);
void ScreenRectToClientRect (HWND hWnd,LPRECT pRect);
BOOL SetDynDlgData (HWND hWndDlg,LPSTR Name,LPUINT pcntls,short lncntls);
BOOL ProcessDlgEdit (HWND hWndDlg,HWND hWndEdit,WPARAM wParam,LPARAM lParam); 
BOOL ProcessDynEdit (HWND hWndDlg,HWND hWndEdit,WPARAM wParam,LPARAM lParam); 
BOOL ShowDlgUpdateOptions (HWND hWndEdit,HWND hWndInput);
BOOL ShowDynDlgUpdateOptions (HWND hWndEdit,HWND hWndInput);
BOOL GetDynDlgHandle (HWND hWndDlg,LPHANDLE phDynDlgControls);
BOOL WriteDynDlgData (HWND hWndDlg);
extern	BOOL CALLBACK EnumCtrlProc(HWND hCtrl,LONG lParam); 
extern	BOOL CALLBACK EnumCtrlProcDynDialog(HWND hCtrl,LONG lParam); 
LONG FAR PASCAL Draw3DDownDynDialog(HWND hWnd, unsigned uiMsg,WORD wParam, LONG lParam);
LONG FAR PASCAL Draw3DUpDynDialog (HWND hWnd, unsigned uiMsg,WORD wParam, LONG lParam);
LONG FAR PASCAL Draw3DDown(HWND hWnd, unsigned uiMsg,WORD wParam, LONG lParam);
LONG FAR PASCAL Draw3DUp (HWND hWnd, unsigned uiMsg,WORD wParam, LONG lParam);
LONG FAR PASCAL ComboColor(HWND hWnd, UINT uiMsg,WORD wParam, LONG lParam);
LONG FAR PASCAL ComboColorDynDialog(HWND hWnd, UINT uiMsg,WORD wParam, LONG lParam);
USHORT	AddStringToList (LPSTR str,LPSTR pStrings,LPUSHORT plen);
BOOL GetFieldValFromSetList (LPSTR List,LPSTR Name,LPSTR Val);
BOOL rread (LPSTR str, LPDOUBLE lpRval, LPINT lpNdp);
double dread (LPSTR str, int nbytes);
long ldread (LPSTR str, int nbytes);
void RWRITE (double RVAL, int NDP, LPSTR OutLoc);
void RWRITEZ (double RVAL, int NDP, int len,LPSTR OutLoc); 
short Strip(LPSTR str, char chr);
LPSTR ReplaceChar (LPSTR str, char from, char to);
LPSTR RemoveDoubleQuotes (LPSTR str);
LPSTR ConvertCharBtwnDoubleQuotes (LPSTR str, char from, char to);
short FillRectPoly(HDC hDC, LPRECT Rect, COLORREF Color);
short DrawRectPoly(HDC hDC, LPRECT Rect, HPEN hPen);
int FillRectColor (HDC hDC,LPRECT Rect,COLORREF Color);
BOOL LoadGlobalInit (LPSTR File,BOOL First);
HGLOBAL GSSiGlobAlloc(int From,UINT fuAlloc, long cbAlloc);
HGLOBAL GSSiGlobalReAlloc (USHORT From,HGLOBAL hGlob, long cbAlloc,UINT fuAlloc);
void GSSiGlobFree (LPHANDLE pHandle); 
void GSSiGlobUlFree (LPHANDLE pHandle);    
void SECLIN8(const double *X1,const double *Y1,const double *A1,const double *X2,
             const double *Y2,const double *A2,double *X3,double *Y3,short *K);
double  AZDF(double AZC, double AZP, double RL);
double DSIGN(const double Value, const double Sign);
//void ERROr( long istat, char *message);
LPSTR LastChr (LPSTR str);
short FillList (HWND hWndDlg,UINT Control,LPSTR file,LPSTR DefaultVal,LPRECT pRect);
short FillCBList (HWND hWndDlg,UINT Control,LPSTR file, short InitVal,LPSTR DefaultVal);
UINT ItemInList (int i,UINT nElements,HANDLE hElements);
BOOL GetListValue (LPSTR file, long ID, LPSTR Val);
long GetListNum (LPSTR file, LPSTR Val);
void ShowProcessingMessage (LPSTR Mess); 
void ClearProcessingMessage (void);
BOOL GetCacheFile (LPSTR UseFile, LPSTR DiskFile, BOOL Add, HWND StatusWnd);
short DeleteCacheDir (void);
short GetCacheInfo (LPSTR CacheDir);
BOOL RemoveCacheFile (LPSTR File);
short DeleteDirAndContents (LPSTR Name);
short IsValidDir(LPSTR Name); 
void GSSisplitpath (LPSTR InPath,LPSTR Drive,LPSTR Dir,LPSTR Leaf,LPSTR Ext);
long SearchFilesInDir (LPSTR CurDir, LPSTR Ext, HFILE OutFile,LPLONG TotFiles,LPSTR WildCard,int Lev,BOOL WantSub,BOOL fileNameOnly);
long SearchDirectoriesInDir (LPSTR CurDirIN, HFILE OutFile,LPLONG TotFiles,LPSTR WildCard,int Lev);
int WINAPI DlgDirList32 (HWND hWndDlg, LPSTR cType, int LBCNTL, int CurrentDirCNTL, UINT Flags);
BOOL ExistFile(LPSTR Name);
int GetPathType(LPSTR InName);
int GetPathType2(LPSTR InName);
short FileType(LPSTR Name);
short FileType_old(LPSTR Name);
BOOL IsLocalFile(LPSTR Name);
HANDLE GetFilesToClose (HANDLE hSQL);
BOOL DeleteFilesInList (HANDLE hList);
void Sound (short Type);
void SetTrace (BOOL On); 
void TraceInWindow (LPSTR str);
void GSSiTrace (LPSTR str,short From);
void GSSiTraceLev (LPSTR str,short lev,short From);
BOOL FileErrMess (HFILE Fid,LPSTR Name,LPOFSTRUCTGM pOFStruct,UINT Mode);
HFILE OpenFileGM(
	_In_    LPCSTR lpFileName,
	_Inout_ LPOFSTRUCTGM lpReOpenBuff,
	_In_    UINT uStyle
	);
BOOL FileOpenForWrite (HFILE Fid);
HFILE OpenFileGSSi (LPSTR Name,LPOFSTRUCTGM pOFStruct,UINT opt,UINT ShareOpt);
HFILE GSSiOpenFile (LPSTR Name,LPOFSTRUCTGM pOFStruct,UINT Mode);
BOOL EditLastTextFile (void);
BOOL EditLastMenuFile (void);
LPSTR GetLastPathname (void);
LPSTR GetLastAccessedFile (void);
HFILE GSSiClose (HFILE Fid);
HFILE GSSiClose2 (LPHFILE pFid);
int StoreTestToProduction(LPSTR testDir, int option);
HFILE OpenTempNamedFile (void);
int GSSifstat (int Fid, struct _stati64 * pstat);
BOOL GSSifileinfo (int Fid, LPBY_HANDLE_FILE_INFORMATION pstat);
BOOL CloseAndDeleteFile (LPHFILE pFid);
HFILE LogOpenFilesOpen (UINT Mode,HFILE Fid,LPOFSTRUCTGM pOFStruct);
void LogOpenFilesClose (HFILE Fid);
BOOL RequestFileClose (HFILE Fid);  
void CloseAllRequestedFiles (BOOL FirstOnly); 
void DumpOpenFiles (LPSTR File);
HFILE FileAlreadyOpen (LPSTR Name,UINT Mode,LPOFSTRUCTGM pOFStruct);
BOOL GSSiChangeLength (HFILE Fid,long NewLength);      
void CreateFidSmall (void);
void CloseFidSmall (void);
BOOL GMEdit (HWND hWnd,LPSTR file);
void GMEditReturn(void);
void GMEditSetFile(LPSTR file, LPSTR bpid,int bploc);
void GMEditGetFile(LPSTR file);
BOOL EditTextFile(HWND hWnd, LPSTR Name);
void CloseVars (void);
void GMDFieldTypeToSQL (LPSTR gmd,LPSTR sql);
void SQLFieldTypeToGMD (LPSTR sql,LPSTR gmd);
short OpenDataFile (LPSTR InName, LPSTR SQL, short Access, HANDLE *hDB);
HANDLE	OpenSQLDatabase (LPSTR Name,LPSTR SQL);
HANDLE	OpenLISTVARDatabase (LPSTR Name);
BOOL GetCurrentPNDBName (LPSTR Name);
BOOL GetCurrentGraphicsDBName (LPSTR Name,short Type);
int GetNumDBFields (HANDLE hDB);
BOOL GetNextPipeDelimitedValue (LPSTR str,LPINT pPos,LPSTR Value);
BOOL SetSQLDatabaseFields (LPSTR Name);
HANDLE GetOpenDatabaseFromID (LPSTR id);
BOOL CloseDataFile (BOOL Final,HANDLE *hDB);
HANDLE GetDBHandleFromSQL (HANDLE hSQL);
BOOL GetFieldInfoFromName (HANDLE hDB,LPSTR FldName,LPSTR pFldType,LPSHORT pFldLen);
BOOL FetchDBRec (HANDLE hSQLPtr);
int GetDBPos(HANDLE hSQLPtr);
BOOL SaveDBRec (HANDLE hSQLPtr);
BOOL SetGMDField (HANDLE hSQLPtr,LPSTR Name,LPSTR Value);
HANDLE GetDBByIDName (LPSTR IDName);
short WantGMDRecord (LPOPENFILEDATA FilePtr,LPOPENSQLDATA SQLPtr,LPVOID lpGWDHead,short cond,BOOL HaveOtherFields);
short ProcessDelimTextHeader(LPSTR INstr, LPSTR File, HFILE Fid, LPHANDLE phDLT, char InDelim, LPSTR IDName);
BOOL GetDelimTextData(LPSTR str,HANDLE hDLT);
BOOL GetDLTVarName (int ivar,HANDLE hDLT, LPSTR Name);
HANDLE	GetDLTVarHandle (int ivar,HANDLE hDLT);
BOOL UpdateGlobalFile (LPSTR RptFile,LPSTR VarName,LPSTR str);
HANDLE FindVar (LPSTR Name);
int	GetParmLoc (int MaxParm,char delim,LPSTR str,LPSTR *pLoc);
void CreateInternalGlobals (void);
void ListGlobals (HWND hWndDlg,WORD Control,BOOL Expand);
void dumpvars (LPSTR Name);
HANDLE  AllocateVar (LPSTR Name);
HANDLE  AllocateTypeVar (LPSTR Name,short Type,BOOL Save); 
void UpdateVarHandle (HANDLE OldHandle,HANDLE NewHandle);
void SetVarSaveStatus (LPSTR Name,BOOL Save);
BOOL GetVarSaveStatus (LPSTR Name);
void SetUDIValue (LPSTR Name, LPSTR Value);
void SetUDIValueLen (LPSTR Name, LPSTR Value, short len);
void SetGlobalValue (LPSTR Name, LPSTR Value); 
void SetGlobalValue4(LPSTR InName, LPSTR InValue, BOOL Raw, LPBREAKPOINT pBrkPt, int bpOffset, int bplen);
void SetGlobalValue2 (HANDLE handle, LPSTR Value, short Index);
void SetGlobalValue3 (LPSTR Name, LPSTR Value, short Index, BOOL FoundLit);
void SetGlobalValueLen (LPSTR Name, LPSTR Value, short len);
void SetGlobalValueReal (LPSTR Name, double val);
void SetGlobalValueLong (LPSTR Name, long val);
void SetGlobalValueHandle (LPSTR Name, HANDLE val);
void SetGlobalValueDPoint (LPSTR Name, DPOINT Point);  
void SetGlobalValueBounds (LPSTR Name, LPMNMXCORD pBounds);
void SetGlobalValueBool (LPSTR Name, BOOL val);
void SetGlobalValueRect (LPSTR Name, RECT Rect);  
BOOL SetGlobalFromCheckBox (HWND hWndDlg,UINT Control,LPSTR VarName);
BOOL SetGlobalFromTextBox (HWND hWndDlg,UINT Control,LPSTR VarName,BOOL ConvertReturns);
void SetVarChangeTimes (short opt);
HANDLE GetOpenHandleFromID (LPSTR ID,int WantType);
void CheckOpenDataFiles (void);
int GetValFromOpenFiles (LPSTR VarName,LPSTR Value,int maxlval);
void SetDLGList (HWND hWndDlg,UINT dlgList);
void SetUseOnlyOneDBHandle (HANDLE handle);
void CloseMacroFiles (long ThisMacro);
BOOL GetFieldDefFromOpenFiles (LPSTR VarName,LPFIELDINFO lpField);
BOOL AddToOpenFileList (HANDLE hFile);
void ExpandGlobalRaw (LPSTR VarName,BOOL AddEscapes,LPSTR Value, short maxlen);
void RemoveFromOpenFileList (HANDLE hFile);
LPOPENFILEDATA InOpenFileList (LPSTR Name,int Type,int Access);
LPSTR ExpandText (LPSTR InText);
LPSTR ExpandTextDB(LPSTR InText, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen);
LPSTR ExpandText2 (LPSTR InText);
void ProcessText (LPSTR InText);
void ProcessTextDB(LPSTR InText, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen);
void ProcessGlobal(LPSTR Global);
HANDLE ProcessIF (LPSTR InLoc,LPBOOL pErr);
int charLevel(LPSTR pchar, LPSTR inStr);
LPSTR MatchLev(LPSTR InStr, char MatchChar);
short NumMatchChar (LPSTR str1,LPSTR str2,int maxchar);  
BOOL StringEndsWith (LPSTR str,LPSTR endstr);
BOOL ResetFileChangeTime (HANDLE hDB);
BOOL GetFileChangeTime (VARPNT VarPtr);
BOOL GetVal (LPSTR VarName, LPSTR OutStr);
void RestoreScreenRect (HDC hDC, HBITMAP hSavedBM, RECT Rect, RECT NewRect);
void TRAN (void *source, void *dest, size_t count);
long  NCHK(LPSTR NAME,  long i,  long PART_LN);
void  STRIPR(LPSTR NAME, int *NLEN,LPSTR out_with,  int llen); 
long  IREAD (LPSTR NAME, long NUMDIG,long *IRC);  
BOOL IWRITE (long ival,LPSTR str ,short len);  
BOOL IWRITEZ (long ival,LPSTR str ,short len);
LPSTR REPLAC (LPSTR STRING, LPSTR OLD, LPSTR NEW, int MAXLEN);
void RestoreScreen (HDC hDC, HBITMAP hSavedBM, RECT Rect);
void RestoreScreen2 (HDC hDC, HANDLE hSavedScreen,long ID,BOOL Clip);
HBITMAP SaveScreen (HDC hDC, RECT Rect);   
HANDLE SaveScreen2 (HWND hWnd,HDC hDC, RECT Rect, LPVOID pVP,LPLONG pID); 
void DestroySavedScreen (LPHANDLE hSavedScreen,long ID);
void RegisterSavedScreen (HANDLE hSavedScreen);
void UnRegisterSavedScreen (HANDLE hSavedScreen);
BOOL ScreenIsRegistered (HANDLE hSavedScreen,long ID);
void ClearSavedScreens(HWND hWnd,LPVOID pVP,LPRECT pRect);
void ResaveSavedScreens(void);
void dumpmemdc(HDC hdc);
void wantnextblt(void);
void SetBit (int ibit, LPSTR lpBytes, BOOL setto);
void SetBit2 (int ibit, LPSTR lpBytes, BOOL setto);
BOOL GetBit (int ibit, LPSTR lpBytes);
BOOL GetBitH (long ibit, HPSTR lpBytes);
void SetBitH (long ibit, HPSTR lpBytes, BOOL setto);
void GetRunDate (LPSTR RunID, LPSTR Date);
COLORREF SetColor (LPSTR pColor);
long GSSifilelength (HFILE Fid);
void BlockText (HDC hDC,LPSTR str,int maxlen,float f);
void ConvertSpanishText (LPBYTE pTxt);
int WaitCursor (int iWait);
int LoadStrings (int nstrings ,HANDLE *hstrings,LPSTR *pNextLine,LPINT LineNum);
BOOL GWCheckMenuItem(HWND hWnd, int wItem);
LPSTR StrEnd (LPSTR str);
short EndStr (LPSTR str);
LPSTR fgetss (LPSTR lpStr, short len, FILE *Fid);
void fputss (LPSTR lpStr, FILE *Fid);
BOOL fputstring(LPSTR lpStr, HFILE Fid);
BOOL fputstring2(LPSTR lpStr, HFILE Fid);
LPSTR fgetstring (LPSTR lpStr, int len, HFILE Fid);
LPSTR fgetstring2 (LPSTR lpStr, int len, HFILE Fid);
float MULREG (double Y[], double X1[], double X2[], int N,
              LPDOUBLE A,LPDOUBLE B, LPDOUBLE C);
double AMEAN (double X[],int N);
long IDNINT (double X);       
short IDNSHRT (double X);
HANDLE STRAN2 (int ID,double X1[], double Y1[],double X2[],double Y2[],int N, LPFLOAT RSQMIN, int Type,LPMNMXCORD pBounds);
void TRANS2 (double XIN,double YIN, LPDOUBLE XOUT,LPDOUBLE YOUT, HANDLE hlpTran);
void TRNPRO (double XIN,double YIN, LPDOUBLE XOUT,LPDOUBLE YOUT, HANDLE hlpTran);   
void CloseTRANS2 (LPHANDLE phlpTran); 
BOOL WriteTranCHeader (HANDLE hTran,HFILE Fid);
HANDLE STRANRect (LPRECT FromRect,LPRECT ToRect);
void TRANRect (LPRECT Rect,HANDLE hTran); 
void TranToParcelPoints(LPDOUBLE XIN, LPDOUBLE YIN, LPDOUBLE  XOUT, LPDOUBLE  YOUT, LPPARCELTRAN pTran);
HANDLE STRANBoundsToRect (LPMNMXCORD FromRect,LPRECT ToRect);
HANDLE STRANRectToBounds (LPRECT FromRect,LPMNMXCORD ToRect);
HANDLE STRANBoundsToBounds (LPMNMXCORD FromRect,LPMNMXCORD ToRect);
HANDLE STRANPoints (int id,LPDPOINT FromPt,LPDPOINT ToPt,int nPt,LPFLOAT pRSQMIN,int Type,LPMNMXCORD pBounds);
POINT TRANDPointToPoint (HPDPOINT pDpoint,HANDLE hTran);
void TranPoints(LPDPOINT FromPt, LPDPOINT ToPt, int nPt, HANDLE hTran);
HANDLE DPolyToPPoly (LPINT pnpnts, HANDLE hDPoly,HANDLE hTran);
HANDLE ReadTranData (HFILE Fid);
BOOL DisplayTranTriangles(HANDLE hTran, LPVIEWPORT CurView);
POINT TranPoint16(POINT pPoint, HANDLE hTran);
DPOINT TranPoint (LPDPOINT pPoint,HANDLE hTran);  
void SetVPRotation (double AZ);
XFORM SetXFORMFromTRANS (HANDLE hlpTran);
HANDLE TRFTRI_SET (LPDOUBLE XT,LPDOUBLE YT,LPDOUBLE XT2,LPDOUBLE YT2,int NSETPT,
				   double BASX1, double BASY1,double BASX2, double BASY2, HANDLE hTran,LPMNMXCORD pTriBounds);
BOOL makedirectories (LPSTR Name,BOOL IsDir,BOOL Verify);   
short GSSiMakeDir (LPSTR Name,LPDWORD pErrCode);
short GSSiRemove32 (LPSTR Name); 
int GSSiRemoveDir (LPSTR Name); 
int ConvertToNewLocation (LPSTR Path, BOOL DoCopy);
BOOL ConvertFileNameToCacheFileName (LPSTR FileName);
DWORD GM32GetSpecialDirectory (LPSTR Path);
BOOL GSSirenamefile (LPSTR OldName,LPSTR NewName);
BOOL GSSiCopyFile (LPSTR OldName,LPSTR NewName,BOOL Replace);
BOOL CopyDirectory(LPSTR toDir, LPSTR fromDir, BOOL replace, LPSTR statusTitle);
BOOL NameContainsDL (LPSTR Name);
void WriteTranData (HFILE Fid,HANDLE hTran);
double GetTranAZ (HANDLE hTrans);
HANDLE LoadTranFile (LPSTR Name, int dir,int Type,LPSHORT pNumPoints,LPDOUBLE pRSQ);
HANDLE LoadTranFromBPW (LPSTR Name);
MNMXCORD GetTranFileBounds (LPSTR Name,LPSTR Direction);
HANDLE LoadTranFileWithDandT (LPSTR Name);
long ConvertPoint (LPSTR CvtFile,LPDPOINT Point,int Direction);
DWORD ConvertGeodeticToMGRS (LPDOUBLE pLat,LPDOUBLE pLon,DWORD Precision,LPSTR MGRS);
DWORD ConvertMGRSToGeodetic (LPDOUBLE pLat,LPDOUBLE pLon,LPSTR MGRS);

short LINSEC (double X1IN,double Y1IN,double A1,double X2IN,double Y2IN,
            double A2,double *X3,double *Y3);
short LINSEC_TOL (double X1IN,double Y1IN,double A1,double X2IN,double Y2IN,
            double A2,double *X3,double *Y3,double INTOL);
short LIN_SEC (double X1IN,double Y1IN,double A1,double X2IN,double Y2IN,
             double A2,double *X3,double *Y3);
BOOL  POINT_IN_AREAS (POINT PickPoint, DWORD nPoints, HPPOINTS lpAreaPoints);
BOOL  POINT_IN_AREA (POINT PickPoint, DWORD nPoints, HPPOINT lpAreaPoints);
HANDLE  PointInAreaAcceleratorSetup (DWORD nPoints, HPDPOINT pAreaPoints,int nPoly,HANDLE hPolyPartLen,HANDLE hMaskAccelerator); 
HANDLE  PointInAreaAcceleratorSetupMono (DWORD nPoints, HPDPOINT pAreaPoints, int nPoly,HANDLE hPolyPartLen,double Offset,LPMNMXCORD pBounds);
HANDLE  PointInAreaAcceleratorSetupWindow (DWORD nPoints, HPDPOINT pAreaPoints,HANDLE hMaskAccelerator); 
HANDLE  PCTInAreasInit (LPMNMXCORD pBounds,int Precision);
void PCTInAreasDestroy (HANDLE hPIA);
double  PCTInAreasLoad (HANDLE hPIA,int opt,int type,DWORD nPoints, HPDPOINT pAreaPoints, int nPoly,HANDLE hPolyPartLen,double Offset,LPMNMXCORD pBounds);
double PCTInAreas (HANDLE hPIA);
int	PointInAreaAccelerator (LPDPOINT Point,HANDLE hAccelerator);    
POINT PIAACenter (LPPIAAStruct pPIAA,LPLONG piCPDist,LPLONG pMaxn,BOOL UsePCTBox,LPBOOL pHaveCP);
POINT	DPointToPIAAPoint (HPDPOINT pDPoint,LPMNMXCORD pBounds,LPDOUBLE pFactor,short Offset,short Type);
DPOINT	PIAAPointToDPoint (POINT Point,LPMNMXCORD pBounds,LPDOUBLE pFactor,short Offset,short Type);
LPHANDLE GetPolygonPickAccelerator (int Opt);
void ThinPoly (LPLONG pnPnts, HPDPOINT pPoints, double maxdist);
int	ReducePolyPoints (int nPnts,LPHANDLE phPoly,int MaxPoints);
BOOL NextLine (LPSTR *lpText,LPSTR lpLine,short nAutoLines); 
LPSTR PadString (LPSTR str,char padchr,int padlen);
LPSTR Truncate(LPSTR str); 
LPSTR Truncate2(LPSTR Text, char c);
LPSTR TruncateAt (LPSTR str,LPSTR EndChars);           
LPSTR FirstNonInt (LPSTR Text);  
LPSTR FirstAlpha (LPSTR Text);
void GSSiERROR (HWND hWnd, short ErrorNum, LPSTR ErrorMessage);
LPSTR IADDR (LPSTR InAdd, long Offset);
long HADDR (HPSTR InAddress, long Offset);
//LPSTR NextFld (LPSTR pInRec);
//LPSTR NextRec (short Fid);
BOOL CheckForContinue (BOOL QuitOnEscapeOnly,LPBOOL pQuitProcessing);
void Wait (long MicroSeconds);
void Wait2 (long MicroSeconds);
BOOL WaitForKeystroke (BOOL UseGetMessage);
BOOL PctBox (HWND hWnd, DWORD MaxLen, DWORD Done, short Freq);  
BOOL IsInteger(LPSTR str);       
BOOL IsReal(LPSTR str);
HANDLE GetPrinterDC(void);
long BigWrite (HFILE Fid,LPVOID pBuf,DWORD isize,long seekloc);
long BigRead (HFILE Fid,LPVOID pBuf,long isize);
void farfree(void far *block);
BOOL CDInit (HWND hWnd, HINSTANCE Inst);
void CDClose (void);
BOOL GetColor (HWND hWnd,COLORREF *Color);  
int GetColorFromPalette (HWND hWnd,LPRGBTRIPLE pColors,int nColors);
COLORREF GetNextUniqueColor (LPLONG pColor);
void cwCenter(HWND hWnd, int top);
int nCwRegisterClasses(LPSTR Menu);
void CwUnRegisterClasses(void);
BOOL GSSIPeekMessage(
    __out LPMSG lpMsg,
    __in_opt HWND hWnd,
    __in UINT wMsgFilterMin,
    __in UINT wMsgFilterMax,
    __in UINT wRemoveMsg);
BOOL GSSiGetMessage(
    __out LPMSG lpMsg,
    __in_opt HWND hWnd,
    __in UINT wMsgFilterMin,
    __in UINT wMsgFilterMax);
void SetFilterString (UINT Filter); 
void PrintMessage (short    ViewID, LPSTR File, short record);
void PrintMessage2 (LPSTR line1, LPSTR line2, LPSTR line3);
BOOL GetOpenFileCD (HWND hWnd, LPSTR Name, int lname, LPSTR lpInitDir);
BOOL GetSaveFileCD (HWND hWnd, LPSTR Name, LPSTR lpInitDir);
BOOL GetFolderName (HWND hWnd,LPSTR startDir,LPSTR outDir,LPSTR title);
BOOL GetFont (HWND ghWnd, LPLOGFONT lpLogFont, COLORREF *dwFontColor, COLORREF *dwShadowColor,HFONT *phSelectedFont);
double square(double val);
double idist(POINT Point1, POINT Point2);
double ldistp(DPOINT Point1, DPOINT Point2);
double l2ddistfrom3d (DPOINT3D Point1, DPOINT3D Point2);
double ldistpp(LPDPOINT Point1, LPDPOINT Point2);
POINT MidPoint (POINT Point1, POINT Point2);
DPOINT MidPointD (DPOINT Point1, DPOINT Point2);
DPOINT3D MidPoint3D (DPOINT3D Point1, DPOINT3D Point2);
double getaz (POINT Point1, POINT Point2);
float fgetaz (FLTPOINT Point1, FLTPOINT Point2);
double getazl (LPOINT Point1, LPOINT Point2);
double getazd (HPDPOINT Point1, HPDPOINT Point2);
double get2dazfrom3d (HPDPOINT3D Point1, HPDPOINT3D Point2);
double LTWOPI (double AZ1);
double LGETAZ(double X1, double Y1, double X2, double Y2);
void AZToBear (double CurAZ,LPSTR PreDir,LPSTR DegC,LPSTR MinC,LPSTR SecC,LPSTR PostDir);
void AZToAZIM (double AZ,LPSTR DegC,LPSTR MinC,LPSTR SecC);
POINT newpt (POINT OldPoint, double AZM, double DIS);
POINT newptscreen (POINT OldPoint, double AZM, double DIS);
DPOINT dnewpt (DPOINT OldPoint, double AZM, double DIS);  
DPOINT dnewptproj (DPOINT OldPoint, double AZM, double DIS);  
void LNEWPT (double X,double Y,LPDOUBLE NewX,LPDOUBLE NewY, double AZM, double DIS);  
BOOL MemError (void);
#if WIN32
    BOOL WINAPI AbortProc ( HDC hPrinterDC, short nCode );
    BOOL WINAPI OpenGCTP32( long FAR *var1);
    BOOL WINAPI LoadGCTPLibrary32( void );
#else       
    BOOL FAR PASCAL __export AbortProc ( HDC hPrinterDC, short nCode );
    void Beep( unsigned duration, unsigned frequency );
    void Sleep( clock_t _wait );
    short _far _pascal CopyFile (LPSTR FromFile, LPSTR ToFile, BOOL SameAtts);
//    BOOL FAR PASCAL __export OpenGCTP32( long FAR *var1);
//    BOOL  FAR PASCAL __export LoadGCTPLibrary32( void );
#endif

BOOL  POINT_IN_AREAD (DPOINT PickPoint, DWORD nPoints, HPDPOINT lpAreaPoints,int nPoly,HANDLE hPolyPartLen,LPDOUBLE pNextXIntersect,LPHANDLE phAccelerator);
BOOL copyfile (LPSTR ToFile, LPSTR FromFile,short AppendOrReplace,long StartPos,long EndPos,HWND hWndDlg,UINT StatusCntl,long TotLen,LPLONG pCurLoc);
LPSTR FirstNonBlank(LPSTR Text);
LPSTR LastNonBlank(LPSTR Text);
LPSTR NextBlank(LPSTR Text);
short RemoveTrailingBlanks (LPSTR str);
UINT GetOPENERR00(void);    
double Round (double Value,double RoundTo);
LPSTR AddCommas (LPSTR VCBuff);
double DecDegFromDMS (LPSTR DMS,LPBOOL pErr);
BOOL GetDMS (double DecDegrees,LPSHORT Deg,LPSHORT Min,LPDOUBLE Sec);
LPSTR ValueConv (double Value, short ValConv,double RoundTo,BOOL Commas); 
HANDLE  BMPFromPCX (LPSTR ImageFile);  
HANDLE  BMPFromEXT (LPSTR ImageFile); 
BOOL SetDIBMonoColors(HDIB32 hDib,LPLONG Colors); 
BOOL  BMPToEXT (HANDLE hBMP,LPSTR ImageFile,DWORD Flag); 
BOOL  BMPToEXT32 (HDIB32 hDib,LPSTR ImageFile,DWORD Flag); 
DWORD  BMPFileFromEXT (LPSTR ImageFile,LPSTR BMPFile,double factor); 
int  DisplayBMFileInRect (HDC hDC,LPSTR ImageFile, RECT Rect, short MaintainAspect);
int  DisplayBMInRect (HDC hDC,LPBITMAPINFOHEADER pDibInfo,LPSTR pImage, RECT Rect, short MaintainAspect,LPRECT pOutRect);
short  DisplayPCXFileInRect (HDC hDC,LPSTR ImageFile, RECT Rect, BOOL MaintainAspect);
BOOL  DisplayBMInRect2 (HDC hDC, HANDLE hBM, RECT Rect, double Factor,double VPct, double HPct,LPRECT pOutRect);
int  DisplayBMInRect32 (HDC hDC,HDIB32 hDib, RECT Rect, short MaintainAspect);
int  DisplayBMInRect32_2 (HDC hDC,HDIB32 hDib, RECT Rect, short MaintainAspect,DWORD RasterOpt);
BOOL DisplayBMFileInVP32(HDC hDC, LPSTR BMFile, double RotationAZ, BOOL fitToVP);
BOOL DisplaySIDInVP32 (LPVIEWPORT pVP,LPSTR BMFile);
//BOOL CheckBoardBMInRect (HDC hDC,LPSTR ImageFile1, LPSTR ImageFile2,RECT Rect, BOOL MaintainAspect);
void CenterRectOnPoint(LPRECT pRect,POINT center);
POINT RectMid (LPRECT rect);
DPOINT RectMidD (LPRECT rect);
void ComputeBMLoc (RECT Rect,LPBITMAPINFO pDibInfo,short MaintainAspect);  
BOOL GetDIBDimensionsFromHandle(HDIB32	hDib32,LPINT height,LPINT width);
BOOL GetDIBDimensions(LPSTR File,LPINT height,LPINT width);
LPBITMAPINFO GetDibHeader (HDIB32 hDib);
BOOL hDibIs32Bit (HDIB32 hDib);
void hDibFree (LPHDIB32 phDib);
BOOL LoadBitMap (LPSTR ImageFile,long frame, LPHDIB32 phDib, LPSHORT pDeleteBM,
                 LPSHORT BitCount, LPSHORT width, LPSHORT height);
BOOL ReadBitMapHeader (HFILE Fid, LPHANDLE phDibInfo, LPLONG ImageOffset);
BOOL SetupTIFHeader (HFILE Fid, LPHANDLE phDibInfo,LPSHORT pNumStrips, LPHANDLE phImageOffset,LPHANDLE phByteCounts,
					 LPSHORT pRowsPerStrip,LPSHORT pPhotoInterp,
					 LPSHORT pSamplesPerPixel,LPSHORT pPlanarConfig,LPSHORT TIFFCompression,
					 LPSHORT pBitsPerSample,LPSHORT pSkipBits);  
BOOL DecompressTIFF (HPSTR Data,long length,long explen,short type);
HANDLE  BMPFromTIF (LPSTR TiffFile,BOOL Check);
short DisplayTIFFileInRect (HDC hDC,LPSTR ImageFile, RECT Rect, BOOL MaintainAspect);
BOOL Report (LPSTR Name, LPSTR ViewportName, LPSTR Prefix, LPSTR UDI, long ref,BOOL LoadOnly, BOOL FitToVP);
BOOL DisplayReport (HDC hDC, HANDLE hReport, RECT Rect, RECT ClipRect, double Factor, long Refno,LPRECT SizeRect);
BOOL ReportToFile (LPSTR Name, LPSTR Prefix, LPSTR UDI, long ref,LPSTR File);
BOOL DisplayReport2 (HDC hDC, HANDLE hReport, RECT Rect, double Factor,BOOL CloseFiles,LPRECT SizeRect);
//RECT SizeReport (HDC hDC,HANDLE hReport,RECT CurRect,BOOL FitToVP);
HANDLE LoadReport (LPSTR Name); 
void UnloadReport (LPHANDLE hReport);
BOOL OpenReportFiles (HANDLE hReport);
void CloseReportFiles (HANDLE hReport);
BOOL AddReportToPrintList (LPSTR ReportFile,long Refno,LPSTR Prefix, LPSTR UDI,LPSTR InitCmd);  
BOOL GetNextPrintReport (BOOL First,LPSTR ReportName,LPLONG pRefno,LPSTR Prefix,LPSTR UDI,LPSTR InitCmd);
BOOL PrintReport2 (HDC hPr,HDC PrinterDC,BOOL IsVirtPrinter,HDC mfDC,LPSTR ReportFile,long Refno,LPSTR Prefix,LPSTR UDI,LPSTR InitCmd);
short   Signof (double val);
short   lSignof (long val);
int NumCharInString (LPSTR str,char chr);
double   dSignof (double val);
WORD ECRCMem(WORD *icrc, BYTE *icp, WORD icnt);
DWORD CRC16 (LPSTR Array, short nc);
BOOL GetSaveName (HWND hWnd,LPSTR Name, UINT StringID, LPSTR Ext);  
BOOL    GetSaveName2 (HWND hWnd,LPSTR Name, UINT StringID, LPSTR Ext,UINT FileNameID);
BOOL    GetSaveName3 (HWND hWnd,LPSTR Name, UINT StringID, LPSTR Ext,LPSTR Title,LPSTR VarName);
BOOL    GetFileName (HWND hWnd,LPSTR Name, int lname, UINT StringID);
BOOL    GetFileName2 (HWND hWnd,LPSTR Name, LPSTR Ext,UINT FileNameID);
BOOL    GetFileName3 (HWND hWnd,LPSTR Name,UINT StringID,UINT FileNameID);
BOOL    GetFileName4 (HWND hWnd,LPSTR Name,UINT StringID,LPSTR Ext,LPSTR Title,LPSTR VarName);
BOOL    GetMultFiles (HWND hWnd,LPSTR Name,int lname,UINT StringID,UINT FileNameID);
void SetOpenFlags (short Flags);
void SetCreateFile (void);
DWORD GetDlgItemPrompt (HWND hWnd); 
void flip (LPSTR In, short n);
void SetDlgPrompt (HWND hWnd,UINT Prompt,UINT More);
void InitDlgPrompts (HWND hWnd);
void ClearDlgPrompts (void);
//void FillFontNameList (HWND hWndDlg,UINT ListBox);
void GMMessageBox (UINT Message, UINT Title, UINT Style);
long LoadProjection(long nj, LPSTR NAME); 
long TranProjection(long I, long O, double *x, double *y); 
int ConvertPRJtoProj4(char *in, char * out);

//extern long FAR PASCAL CloseLibrary(void);
//extern BOOL FAR PASCAL OpenGCTPLibrary(void);
//extern BOOL FAR PASCAL OpenGCTP(long far *d);   
BOOL ConvertCoordClose (void);
BOOL NeedToConvertCoord (int from, int to);
long ConvertCoord (LPDPOINT DPoint,int from, int to);
double ClipCoordToProjection (double val,int xory,int idFrom,int idTo);
long ConvertCoordInit (void);
long ConvertAndTranCoord (LPDPOINT pPoint,HANDLE hTranFile);
void ConvertCoordError(long ErrNum);
BOOL GetCurVal (LPSTR Val,short maxval, UINT StringID);
void SetCurVal (LPSTR Val,UINT StringID);
void SaveGlobalVals (HFILE Fid);
int	GetUpdateFieldType (LPSTR SetFieldName);
BOOL GetUpdateFieldValue (HWND hWndDlg,LPSTR SetFieldName,LPSTR NewValue);
BOOL AVIFrameToDIB (LPSTR File, long frame,LPHANDLE NewDIB,LPSHORT ShouldDeleteBM,
                    short Bitcount, short width, short height);
long NumDIBColors (HANDLE hDib);
void CloseOrthoAVI (void);
short AVIOut (LPSTR Name,LPBITMAPINFOHEADER alpbi,LPHANDLE hFile,LPLONG pFrame,BOOL UseExCmp);
void AVIOutClose (LPHANDLE phFile);
HANDLE ConvertBitmap16To24 (LPBITMAPINFOHEADER  lpbi);
HANDLE ConvertBitmap8To24 (LPBITMAPINFOHEADER  lpbi);
BOOL SplitBitmap (LPSTR Infile,int numRows,int numCols,LPSTR OutDir,LPSTR OutExt,int OutOpt);
BOOL FixBitMapHeader (LPSTR FileName);
int GetOpenFileChecksum2 (HFILE Fid,int frombyte,int tobyte);
int GetOpenFileChecksum (HFILE Fid,int frombyte,int tobyte);
int GetFileChecksum (LPSTR File,int frombyte,int tobyte);
BYTE ComputeCheckSum (LPBYTE rec,DWORD l);
int	GSSiRemove (LPSTR Name);
int GSSiRemoveAndClear (LPSTR Name);
int GSSiRemove2 (LPSTR Name);
long GSSiLength (LPSTR Name);
BOOL GSSiRename (LPSTR NameFrom, LPSTR NameTo);
HCURSOR GSSiSetCursor (HCURSOR hCursor);
POINT TAGPtToWinPt (DPOINT TagPoint);
DPOINT WinPtToTAGPt (POINT WinPoint);
float PixelsToTAGFontHt (LPLOGFONT lpFont,short CoordStyle);
double TAGFontHtToPixels (double TAGFontHeight,short CoordStyle);
DPOINT AverageDPoint (DPOINT Point1, DPOINT Point2);   
POINT AveragePoint (POINT Point1, POINT Point2);   
LPOINT AveragePointL (LPOINT Point1, LPOINT Point2);
DPOINT AverageDPoints (HPDPOINT Point,long nPnts);
BOOL RemoveLine (LPSTR Name,LPSTR line);
BOOL DisplayTextFileInRect (HDC hDC,LPRECT Rect,LPSTR Name);
LPSTR UpcaseFirst (LPSTR s);
LPSTR OneSpace (LPSTR Arg1); 
LPSTR strncpy0(LPSTR Buff,LPSTR str, size_t n);
void SetFocusAndCursor (HWND hWnd);
BOOL GetTextString (HWND hWnd,LPSTR String,int lenstring, LPSTR Title,LPSTR ListFile,LPSTR InitVal,long AutoInc,BOOL DrowDown,BOOL Sorted); 
BOOL GetTextStringML (HWND hWnd,LPSTR String,int lenstring, LPSTR Title,LPSTR InitVal);
long GetGlobalLVal (LPSTR Global);
long GetGlobalLVal2 (LPSTR Global,long Default);
long GetGlobalLVal3 (LPSTR Global,long Default);
BOOL GetGlobalBVal (LPSTR Global);
BOOL GetGlobalBVal2 (LPSTR Global,BOOL Default);
BOOL GetGlobalPVal (LPSTR Global,LPDPOINT Default,LPDPOINT pPoint);
short GetGlobalVal (HANDLE hGlobal,LPSTR OutStr,long Index);
double GetGlobalDVal (LPSTR Global);
double GetGlobalDVal2 (LPSTR Global,double Default);
BOOL GetGlobalCVal (LPSTR Global,LPSTR Val,LPSTR Default);
RECT GetGlobalRectVal (LPSTR Global,LPRECT pDefault);
MNMXCORD GetGlobalBoundsVal (LPSTR Global,LPMNMXCORD pDefault); 
BOOL GetGlobalValRaw (LPSTR Global,LPSTR Val);
BOOL ExecuteCommandString (LPSTR pCommand);
//HBITMAP PCXtoBMP (HDC hDC,LPSTR Name);   
long Factorial (long value);    
BOOL LoadTAGDef (void);
double ArcDistance(DPOINT p1, DPOINT p2);
DPOINT NewLatLong(double Lat,double Long,double Dist,double Direction);
short DrawCircle (HDC hDC,POINT Point,double Radius,short Width,COLORREF Color); 
void AddPointToRect (POINT pPoint,LPRECT pBounds);
void AddPointToRect16 (POINT Point,LPRECT16 pBounds);
void AddPointToMinMax (POINT pPoint,LPMINMAX pMinMax);
void AddDPointToMinMax (HPDPOINT Point,LPMNMXCORD pBounds);
double BoundsWidth (LPMNMXCORD pBounds);
double BoundsHeight (LPMNMXCORD pBounds);
void MinMaxInit (LPMINMAX pMinMax);
void AddPointToMinMaxL (POINT Point,LPMNMXCORL pBounds);
void RectInit (LPRECT pMinMax);
void RectInit16 (LPRECT16 pMinMax);
void MinMaxInitL (LPMNMXCORL pMinMax);
void BoundsToPoints (LPMNMXCORD pBounds, LPDPOINT Points,LPDOUBLE pAZ);
void BoundsLToPoints (LPMNMXCORL pBounds, LPDPOINT Points,LPDOUBLE pAZ);
BOOL DPointInBounds (LPDPOINT Point,LPMNMXCORD pBounds);
DPOINT FPointToDPoint (FPOINT point);
POINT FPointToPoint (FPOINT point);
DPOINT SPointToDPoint (POINTS Point);
DPOINT PointToDPoint (POINT Point);
LPOINT PointToLPoint (POINT Point);
POINT DPointToPoint (DPOINT Point); 
HANDLE DPointsToPoints(HANDLE hDPoints, int np);
LPOINT DPointToLPoint (LPDPOINT pPoint);
DPOINT LPointToDPoint (LPOINT Point);
FPOINT DPointToFPoint (DPOINT DPoint);
FPOINT PointToFPoint (POINT DPoint);
DPOINT DPoint3DToDPoint (DPOINT3D DPoint);
DPOINT3D DPointToDPoint3D (DPOINT DPoint);
LPOINT DPointToFilePointL (LPDPOINT pDPoint,HANDLE hTran);
void RoundToPTOL (LPDPOINT Point);
void InflateBounds (LPMNMXCORD pBounds, double Value);
void InflateMinMax (LPMINMAX pBounds, int Value);
void InflateMinMaxL (LPMNMXCORL pBounds, int Value);
BOOL RectInRect(LPRECT pRectIn,LPRECT pRectTest);
BOOL RectCompletelyInRect(LPRECT pRectIn,LPRECT pRectTest);
BOOL IntersectBounds (LPMNMXCORD pBounds1,LPMNMXCORD pBounds2,LPMNMXCORD pBoundsInt);
void BlowOut (LPSTR Message, LPSTR Title);
void SubstituteDL (LPSTR Name,BOOL WantAt); 
void SetReplacePath (LPSTR Name,short opt);
LPSTR ldelim (LPSTR str,char delim);
BOOL ProcessMacroReport (LPSTR Name, LPSTR Prefix, LPSTR UDI, long ref);
BOOL DisplayScrollReport (LPSTR Name, LPSTR Prefix, LPSTR UDI, long ref);
BOOL DisplayReportScroll (HWND hWndDlg, int ScrollCntl);
int GetLBSelectedItems (HWND hWndDlg,UINT controlid,LPHANDLE phItems);
LPSTR TabFromEnd (LPSTR str, int n);
BOOL EditDynamicDialog (HWND hWndDlg,LPSTR Name, LPSTR SQL, LPSTR InsertString);
int AppendFile (LPSTR File,LPSTR Line);
int AppendFile2 (LPSTR InFile,LPSTR Line);
void AddToMinMaxD (LPMNMXCORD mm1, LPDPOINT pPoint); 
BOOL BoundsInBounds (LPMNMXCORD pBounds1,LPMNMXCORD pBounds2,short Opt);
BOOL Bounds4InBounds4 (LPMNMXCORL pBounds1,LPMNMXCORL pBounds2,short Opt);
BOOL PointInBounds (DPOINT Point,LPMNMXCORD pBounds);
BOOL PointInBoundsL (DPOINT Point,LPMNMXCORL pBounds);
void DBoundsInit (LPMNMXCORD lpRect);           
void AdjustBounds (LPMNMXCORD Bounds, double AdjustX, double AdjustY);
void ExpandBounds (LPMNMXCORD Bounds, double Adjust);
void ExpandMinMax (LPMINMAX Bounds, int Adjust);
void ExpandMinMaxL (LPMNMXCORL Bounds, long Adjust);
void TranBounds(HANDLE TranID, LPMNMXCORD pBounds);
void ConvertBounds(LPMNMXCORD pBounds, int from, int to);
void AdjustRectToRect(LPRECT pRectToAdjust, LPRECT pRect);
void DebugShowLine (LPDPOINT p1,LPDPOINT p2);
double MinAngleToTheRight (double AZ1,double AZ2);
short GetValListFieldLen (LPFIELDINFO pField);
BOOL CreateStatusWind (HWND hWnd,int nStatusBars,LPSTR Title);
BOOL StatusWindowUpdate (LPSTR Title, LPSTR Mess, DWORD Tot, DWORD Done);
void StatusExtraInfoUpdate(LPSTR mess);
BOOL StatusWindowUpdate2 (LPSTR Mess, DWORD Tot, DWORD Done);
BOOL DestroyStatusWindow (long Macro);
void TranBoundsToRect (HANDLE TranID,LPMNMXCORD pBounds,LPRECT pRect);
double TranAngle (HANDLE TranID,LPMNMXCORD pBounds);
double TranScale (HANDLE TranID,LPMNMXCORD pBounds);
void AddMinMax (LPMINMAX mm1, LPMINMAX mm2);
void AddMinMaxD (LPMNMXCORD mm1, LPMNMXCORD mm2);  
void AddMinMaxL (LPMNMXCORL mm1, LPMNMXCORL mm2);
BOOL ValidBounds (LPMNMXCORD Bounds);
BOOL ValidBounds2 (LPMNMXCORD Bounds);
void DdeBye(void);
BOOL InitDdeStuff(HWND  In_handle, BOOL Highways);
void SetIntRefno (long Ref); 
void SetSymNum (int SymNum);
void BufWrite (HPSTR *pBuf,LPLONG plBuf,HPSTR data,long ldata);
BOOL BackupFiles (LPSTR FileList,LPSTR BUDir);
void GSSiDeleteObject (HGDIOBJ *handle);  
short loadtabs (LPINT Tabs);
short GetDriveNum (char DriveLetter);
//BOOL CreatePickData (void);
double RectArea (LPRECT pRect);
double RectArea16 (LPRECT16 pRect);
double BoundsArea (LPMNMXCORD pRect);
double ComputeAreaAreaD (HPDPOINT lpDPoints,long nPnts,LPDOUBLE pPerim);
double ComputeAreaAreaDH (HANDLE hPoints,long nPnts,LPDOUBLE pPerim);
DPOINT ComputeAreaMidpoint2 (HPDPOINT pPoints,long nPnts);
DPOINT ComputeAreaMidpoint (HANDLE hPoints,long nPnts);
BOOL GetItemMidpoint (LPSTR TagOrRef,LPDPOINT pMidPoint);
DPOINT ComputePolylineMidpoint2 (HPDPOINT pPoints,long nPnts);
DPOINT ComputePolylineMidpoint (HANDLE hPoints,long nPnts);
BOOL GetVolumeLabel(LPSTR wDrive, LPSTR lpBuff);
double	GetDriveFreeSpace (LPSTR Drive);
//BOOL LoadLibraries(HINSTANCE hInst);
//void FreeLibraries(void);
USHORT GetSQLType(LPSTR pstr,LPSTR lpBrack,int NumFields,LPFIELDINFO lpFldInfo,LPDWORD pPrecision,LPUSHORT pScale);
BOOL AddPWtoODBCFile (LPSTR lpcstring);
//void CloseGCTPLibrary32(void);
HANDLE CopyDib (HANDLE hdib);
BOOL ReverseBOOL (BOOL In);
void hmemset (HPSTR out,char in,DWORD len);
BOOL SamePointS (POINTS p1, POINTS p2);
BOOL SamePoint (POINT p1, POINT p2);
BOOL SameLPoint (LPOINT p1, LPOINT p2);
BOOL SameDPoint (LPDPOINT p1, LPDPOINT p2);
int	GetFunctionID (LPSTR str, LPSTR ParenLoc);
int	GetFunctionValue(int FunID, LPSTR Args, LPSTR OutLoc, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen);
int	GetFunctionValue1(int FunID, LPSTR Args, LPSTR OutLoc, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen);
int	GetFunctionValue2(int FunID, LPSTR Args, LPSTR OutLoc, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen);
int	GetFunctionValue3(int FunID, LPSTR Args, LPSTR OutLoc, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen);
short GetFunArgs(LPSTR	Args, LPSTR *Arg, short MaxArgs, LPHANDLE phMem, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen);
COLORREF ColorWOWidth(COLORREF InColor);
COLORREF ColorWithWidth (COLORREF Color,int Width);
double FltAP (LPSTR INEXPR,LPBOOL IRC);
BOOL LogicP (LPSTR INEXPR,LPBOOL IRC);
BOOL LogicPBP(LPSTR INEXPR, LPBOOL IRC, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen);
BOOL LogicPFile(LPOPENSQLDATA	FilePtr, LPSTR SQL, LPBOOL pErr);
void ConvertSQLToLogicP (LPSTR pLogicP,LPSTR pSQL);
double fltread (LPSTR Instr, LPBOOL IRC);
double ComputeFLTValue (double val1, double val2, char OpCode,LPBOOL IRC);
BOOL ComputeLogValue (LPSTR val1, LPSTR val2, UINT OpCode,LPBOOL IRC);
UINT GetBoolOpCode (LPSTR Input,LPSHORT pInc,LPBOOL pNeedArg1,LPBOOL pError);
short OpRankBool (UINT OpCode);
//short DupDLLs (void);                            
HWND SendDDEInitiate(LPSTR szApplication, LPSTR szTopic);
void SendExecute(HWND hwndClientDDE, HWND hwndServerDDE,LPSTR szExecuteString);
void ClientTerminate(HWND hwndClientDDE,HWND hwndServerDDE);
void TerminateConversations(HWND hWndClient);
void MovePromptMessage (HWND hWnd,LPARAM lParam);
void SetPromptDlg (UINT PromptID);
POINT MinMaxMidPoint (LPMINMAX pBounds);
POINT MinMaxMidPointL (LPMNMXCORL pBounds);
DPOINT MinMaxMidPointD (LPMNMXCORD pBounds);
MNMXCORD FactorBounds (LPMNMXCORD pRect,double Factor);
DPOINT SubtractPoint (LPDPOINT pPoint1,LPDPOINT pPoint2);
int	SaveCurView (int opt);
short GSSiMessageBox (LPSTR Mess,LPSTR Title,UINT icon,LPSTR Position);
DPOINT EnlargedPoint (POINT Point);
BOOL EnlargeScreen (short factor,short width); 
BOOL atob (LPSTR Value); 
void btoa (BOOL Val,LPSTR str);
DPOINT atopt (LPSTR Value,LPBOOL pErr);
POINT atopt16 (LPSTR Value,LPBOOL pErr);
MNMXCORD atobounds (LPSTR Value,LPBOOL pErr);
MNMXCORD lboundstobounds (LPMNMXCORL plbounds);
RECT atorect (LPSTR Value,LPBOOL pErr); 
void boundstoa (LPSTR Value,LPMNMXCORD Bounds);
void lboundstoa (LPSTR Value,LPMNMXCORL Bounds);
void recttoa (LPSTR Value,RECT Rect);
void pttoa (LPSTR Value,POINT Point);
void dpointtoa (LPSTR Value,LPDPOINT pPoint);
void dpointtoatrunc (LPSTR Value,LPDPOINT pPoint);
void pointstoa (LPSTR Value,int nPoints,HANDLE hPoints,HANDLE hTran);
LPSTR ftoa (LPSTR Value,double DVal);
void NextPage (HDC hDC);  
short Expandicmp (LPSTR String1, LPSTR String2);
BOOL MakeMap (LPSTR Args);
BOOL WhereAt (LPSTR Args);  
BOOL NeedSQLValueQuote (HANDLE hSQL,LPSTR FieldName);
void SetLinkedVarTime (VARPNT VP); 
void LinkIncludedVars (HANDLE hVar,LPSTR Value);
void GetPatternOpt (HWND hDlg,LPPATBYTE pPatByte);
void SetPatternOpt (HWND hDlg,LPPATBYTE pPatByte);
HBRUSH CreatePatBrush (HDC hDC, COLORREF color, LPPATBYTE pPatByte);
void SetDlgItemTextGlobal (HWND hWndDlg,UINT icntl,LPSTR Global,LPSTR Init);
void GetDlgItemTextGlobal (HWND hWndDlg,UINT icntl,LPSTR Global);
void SetDlgItemTextFromField (HWND hWndDlg,UINT icntl,LPSTR FieldName,LPSTR Init);
BOOL SetDlgCheckboxFromField (HWND hWndDlg,UINT icntl,LPSTR FieldName,LPSTR Init);
BOOL SetDlgComboboxFromField (HWND hWndDlg,UINT icntl,LPSTR FieldName,LPSTR Init);
BOOL LININT (double L1X1, double L1Y1, double L1X2, double L1Y2,
			 double L2X1, double L2Y1, double L2X2, double L2Y2);
//void DumpDCB (int Stream);
void SIOOpenError (LPSTR Port,int Error);
HANDLE ReadSavedScreen (LPSTR File,LPLONG pUpdateTime,LPMNMXCORD pBounds);
BOOL WriteSavedScreen (LPSTR File,HANDLE hSavedScreen,LPMNMXCORD pBounds); 
void Draw3DBorder(HDC hDC, LPRECT pRect,int iStyle, BOOL DoubleWidth);
BOOL FAR PASCAL EDITDYNDIALOGMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL LINFIT (HPDPOINT pPoint,LONG N,LPDOUBLE A,LPDOUBLE B,LPDOUBLE MAXDIF,LPLONG LOFMDF);
BOOL GetCDDriveForVol (LPSTR VolID,LPSTR Drive);
BOOL ConvertNameToCDName (LPSTR Name);     
BOOL CDIsInstalled (LPSTR CDName);
void OpenCDLookUpTable (void);
void CloseCDLookUpTable (void);
void ShowCheck (HDC hDC,LPRECT pRect,BOOL Checked,LPRECT pOutRect);
HANDLE	EnterBlockingWindow (HWND hWndDlg);
void LeaveBlockingWindow(HANDLE hSavedScreen);
void GSSiEndDialog(HWND hWndDlg,BOOL rtn,HANDLE hSaveBM);  
void ClearFullWindowBitmap (HWND hWnd);
short GetNumOpenFiles (void);
short GetOpenFileID (HFILE Fid);
short GenList (HWND hWndDlg,UINT CntlID,BOOL ComboBox,long Loc,LPSTR str,LPSTR CurrentValue,LPBOOL pUseBAR,LPINT MaxLen,LPINT NumItems);
BOOL GetGenListVal (LPSTR GenListString,int Inc,LPSTR OutString);
void ClearCurrentCD (void);
BOOL CDInCancelledList (LPSTR CD);
void AddCDToCancelledList (LPSTR CD);
short OkToContinue (BOOL ForceCheck);
BOOL WindowIsCovered (HWND hWnd,short opt);
double GetDriveSize (char Drive);
long GetProcessorID (void);
BOOL NormalRect (LPRECT Rect); 
RECT FactorRect (LPRECT Rect,double Factor); 
BOOL GetOpenFilePathname (HFILE Fid,LPSTR Name); 
BOOL MinimizeWindowIfOverMain (HWND hWnd);
BOOL WriteErrorHandler (HFILE Fid,long ln,long loc);  
//grp4decomp(HPBYTE indata,long inbytes,long width,long height,HPBYTE outdata,LPLONG outbytes,short SkipBits);
//int SetLastMessage (long mes);
//void ShowTextTrace (LPSTR str);
double ColorDist (COLORREF C1, COLORREF C2);
double RGBDist (RGBTRIPLE C1, RGBTRIPLE C2);
double RGBQUADDist (RGBQUAD C1, RGBQUAD C2);
COLORREF NearestColor (COLORREF Color, COLORREF C1, COLORREF C2);
int NumBytesDifferent (LPSTR File1,LPSTR File2);
BOOL RectanglesAreEqual (LPRECT pRect1,LPRECT pRect2);
BOOL TABToComma (LPSTR InFile,LPSTR OutFile);  
__int64 llFileSeek (HANDLE hf, __int64 distance, DWORD MoveMethod);
LONG GSSillseek (HFILE Fid, LONG loc, int opt); 
DWORD GSSillseek2 (HFILE Fid, DWORD loc, int opt);
long  GSSilread(HFILE Fid, void _huge* ptr, long len);
UINT  WINAPI GSSilwrite(HFILE Fid, const void _huge* ptr, UINT len);
BOOL ConvertToMemFile (HFILE Fid,int MaxMem);
HFILE GSSiOpenFileMem (LPSTR InName,UINT mode,UINT maxlen); 
HFILE SetMemFile (HFILE Fid,HANDLE handle,long len);
BOOL IsMemFile (HFILE Fid);
double LDIST(double X1,double Y1,double X2,double Y2);
HANDLE GetBMPFromCache (LPSTR Name);
void AddBMPToCache (LPSTR Name,HANDLE hBMP);
BOOL hDIBInCache (HANDLE hBMP);
HDIB32 GetBMPFromCache32 (LPSTR Name);
BOOL hDIBInCache32 (HDIB32 hBMP);
void AddBMPToCache32 (LPSTR Name,HDIB32 hBMP);
BOOL RemoveBMPFromCache32 (LPSTR Name);
short CacheAlreadyChecked (LPSTR Name,int lCacheDir,UINT Mode);
void ResetOriginalDrive (void);
long ConvertRectToArea (LPHANDLE phPoints);
HANDLE TransformBitmap (HANDLE hBMP,HANDLE hTran,HANDLE hTranBack,LPMNMXCORD pBounds, LPDOUBLE pRes,LPSTR Namelong, long nAreaPts,HANDLE hAreaPts,BOOL DisplayStatus);
void RectToPoints (LPRECT Rect,LPPOINT Points);
void RectToDPoints (LPRECT Rect,LPDPOINT Points);
void RectToBounds (LPRECT Rect,LPMNMXCORD pBounds);
MNMXCORD RotateBounds (LPMNMXCORD pBounds,double Rotation);
int GSSiGetTempFileName (BYTE Drive,LPSTR Pre,UINT Unique,LPSTR Name);
BOOL GetTempDir (LPSTR Dir);
BOOL GetCacheDir (LPSTR Dir);
short CompressCensusString (LPSTR pStr);
short ExpandCensusString (LPSTR pStr);
long CompressBinaryRecord (LPBYTE pDecompressedRec,LPBYTE pCompressedRec,DWORD DecompressedLen);
long DecompressBinaryRecordUnsafe (LPBYTE pDecompressedRec,LPBYTE pCompressedRec,DWORD CompressedLen);	
int CompressByteArray (LPBYTE pMem,LPBYTE pMemCmp,int lMem);
int DeCompressByteArray (LPBYTE pMemCmp,LPBYTE pMem,int lMem);
int OpenJournal (LPSTR Name,HFILE Fid,UINT Mode);
BOOL CloseJournal (HFILE Fid);
BOOL ApplyJournal (LPSTR JournalFileName);
void FileWillBeCreated (LPSTR Name);
void FileOpenForUpdate (LPSTR Name,HFILE Fid,UINT Mode);
void SaveDataToUndoFile (short Type,short UndoFileID, long len, HPSTR pData,long SeekLoc);
void CheckPointBegin (void);
void CheckPointEnd (void);
BOOL CreateUndoPoint (LPSTR Name);
void RemoveUndoPoint (void);
long GetSizeAtLastUndoPoint (short UndoFileID);
BOOL UndoAction (HFILE FidUndo);
BOOL UndoChanges (HWND hWnd);
HFILE GetUndoFid (short UndoFileID);
BOOL GetUndoData (HFILE Fid,HPSTR pData,long length,long FirstPiece,BOOL Remove);
BOOL GetUndoFileNameFromUndoFileID (short UndoFileID,LPSTR Name);
short GetUndoFileIDFromName (LPSTR Name,BOOL Add);
short GetUndoFileIDFromFid (HFILE Fid);
void AddToChangedGlobalList (HANDLE handle);
void AddToUndoFreeSpace (HFILE Fid,long loc,long len);
DWORD NextVarTime(void);   
long WriteToUndoFile (HFILE Fid,HPSTR pData,long len);
BOOL OpenHighlightList(LPSTR Name1, LPSTR Name2);
void CloseHighlightList(void);
void AddFileToUndoFile (LPSTR Name,long BeginLoc,HFILE Fid);
void DisableUndo (BOOL Disable);
void UndoAddWindow (HWND hWnd);
void UndoRemoveWindow (HWND hWnd);
void AdjustDIBColors (HANDLE hDib);   
HDIB32 AdjustDIB32Colors (HDIB32 hDib32);   
COLORREF ConvertToGray (COLORREF);
RGBTRIPLE ConvertToGrayTriple (RGBTRIPLE color3);
double DistToGray (short R, short G, short B, short Gray);
//BOOL CopyDib2 (HANDLE hdib,LPHANDLE phDibInfo, LPHANDLE phImage);
void BiasPoly (HANDLE hDPoints,long np,double xbias,double ybias);
RECT GetPolyBounds (HANDLE hPoly,long np);
RECT GetPolyBounds2 (LPPOINT Poly,long np);
void GetPolyBoundsD (HANDLE hPoly,long np,LPMNMXCORD pBounds,int Type);
void GetPolyBoundsD2 (LPDPOINT Poly,long np,LPMNMXCORD pBounds,int Type);
HANDLE	GetOffsetPoly (LPDPOINT pPoints,int nPoints,LPINT pnOffPoints,double Offset);
BOOL GetFile32 (short Opt,LPOPENFILENAME pof,DWORD lFilters);
BOOL GetShortPathName2 (LPSTR Name,short MaxLen);
int GetLongPathName2 (LPSTR Name,short MaxLen);
int GetLongPathName3 (LPSTR Name,short MaxLen);
int GetLongPathFromBuffer (LPSTR Name,short MaxLen);
int FlushFile (HFILE Fid);
BOOL LoadGM32Lib (BOOL Close);
BOOL GSSiGetFileName (short opt,HWND hWnd,HINSTANCE hInstance,LPSTR PathName,LPCSTR Filters,DWORD lFilters,LPCSTR InitialDir,LPCSTR Title,DWORD Flags);
DWORD CreateLongNameFile (LPSTR Name);
void CheckOpenFiles (void);   
BOOL GSSiPrintDlg (LPPRINTDLG p,LPBOOL IsVirtualPrinter,LPHANDLE phVirtPrinter,HDC *PrinterDC);
HDC GM32LargeMemDC (HDC hDC16,LPDWORD pMemMapWidth,LPDWORD pMemMapHeight,LPDWORD phOldBitmap);
WORD GM32SaveDCBitMap (HDC hDC16,LPSTR OutFile,long Format,DWORD Flag,DWORD BackgroundColor,
					   HANDLE hOverViewBitmap16,
					   DWORD OverViewTileCol, DWORD OverViewTileRow,
					   DWORD OverViewTileWidth, DWORD OverViewTileHeight,
					   DWORD OverViewBitmapHeight);
DWORD GM32DeleteLargeMemDC (HDC hDC16,DWORD hOldBitmap);
DWORD GM32GetDIBPalette (HDIB32 hDIB,LPLONG pPalletSize,LPRGBQUAD pPalletIn);
DWORD GM32SetDIBPalette(HDIB32 hDIB,DWORD PalletSize,LPRGBQUAD pPalletIn);
DWORD GM32QuantizeDIB (HDIB32 hDIB,DWORD Flag);
DWORD GM32BitmapToDIB(DWORD hBitmap16);
WORD GM32SaveBitmap(HANDLE hBitmap16, LPSTR OutFile, long Format, DWORD Flag);
WORD GM32SaveDIB (HDIB32 hDIB,LPSTR OutFile,long Format,DWORD Flag);
HDIB32 GM32AllocateDIB (DWORD Width,DWORD Height,DWORD BitsPerPixel);
DWORD GM32PasteDIB (HDIB32 ToDIB,HDIB32 FromDIB,DWORD Left,DWORD Top,DWORD Alpha);
int ConvertBitmapColorsInRect (LPSTR BitmapPath,LPMNMXCORD pBounds,COLORREF FromColor,COLORREF ToColor,BOOL CountOnly);
int ConvertBitmapColorsInRange(LPSTR BitmapPath, LPSTR ToPath, COLORREF FromColor, COLORREF ToColor, double colordist, LPMNMXCORD pBounds);
DWORD GM32CompressFrame (LPBITMAPINFOHEADER	lpbiIn, LPBITMAPINFOHEADER lpbiOut);
BOOL GSSiGetGMCName (HWND hWnd,LPSTR PathName,LPSTR InitialDir,LPSTR Title,LPBOOL pStartInNewSession,LPBOOL pRetainZoom,LPBOOL pLinkZoom,LPSTR NetworkDir,LPSTR PersonalDir);
long GSSiGetDeviceCaps(HDC hPr, int opt); 
DWORD DrawLineWithFlatEnd (HDC hDC,DWORD npt,HPPOINT pPoints16,DWORD Width,COLORREF Color);
DWORD GM32DrawDibOpen (void);
DWORD GM32DrawDibClose (DWORD hdd);
HDC LargeMemDC (HDC hDC16,DWORD * MemMapWidth,DWORD * MemMapHeight,DWORD *pOldBitmap); 
BOOL DeleteLargeDC (HDC hDC16,DWORD hOldBitmap);  
BOOL SaveDCBitMap (HDC hDC16,LPSTR OutFile,long Format,DWORD Flag,COLORREF BackgroundColor,
						   	 HBITMAP hOverviewBitmap,
						   	 long VirtualPageRow,long VirtualPageCol,
							 long OverViewTileWidth, long OverViewTileHeight, long OverViewBitmapHeight);  
long StretchDIBits32 (HDC hDC,long destX,long destY,long destW,long destH,long xoff,long yoff,long bmwidth,long bmheight,
					  LPBYTE pImage, LPBITMAPINFOHEADER pDibInfo,long ColorType,long RastOpts,LPDOUBLE pFactor,BOOL Fast);
long StretchDIBitsFromHandle (HDC hDC,long destX,long destY,long destW,long destH,long source_xoff,long source_yoff,long sourcew,long sourceh,
					 		  HDIB32 hDib, DWORD ColorType,DWORD RasterOpt,double Factor);
int SetCurImage (LPSTR Name);
BOOL SaveBitmap (HBITMAP hBitmap,LPSTR OutFile,long Format,DWORD Flag);  
HDIB32 BitmapToDIB_32(HBITMAP hBitmap, HPALETTE hPal);
BOOL SaveDIB32(HDIB32 hBitmap, LPSTR OutFile, long Format, DWORD Flag);
HDIB32 AllocateDIB (DWORD Width,DWORD Height,DWORD BitsPerPixel);
BOOL PasteDIB (HDIB32 ToDIB,HDIB32 FromDIB,DWORD Left,DWORD Top,DWORD Alpha);
HDIB32 BitmapToDIB32 (HBITMAP hBitmap); 
HDIB32 QuantizeDib (HDIB32 hDib,DWORD Opt); 
HDIB32 QuantizeDibEx (HDIB32 hDib,DWORD Opt,DWORD PalSize,DWORD ResPalSize,RGBQUAD *Pallet);
BOOL GetDibPalette (HDIB32 hDib,LPLONG pNumColors,LPRGBQUAD pPalette);
BOOL SetDibPalette (HDIB32 hDib,DWORD NumColors,LPRGBQUAD pPalette);   
RGBQUAD RGBTripleToRGBQuad (RGBTRIPLE Trip);
COLORREF RGBTRIPLEToCOLORREF (RGBTRIPLE Trip);
COLORREF RGBQUADToCOLORREF (RGBQUAD Quad);
RGBTRIPLE RGBQuadToRGBTriple (RGBQUAD qcolor);
RGBTRIPLE RGBTRIPLEFromCOLORREF (COLORREF Color);
RGBQUAD RGBQUADFromCOLORREF (COLORREF Color);  
COLORREF COLORREFFromRGBQUAD (RGBQUAD rgbq);
COLORREF COLORREFFromRGBTRIPLE (RGBTRIPLE rgbt);
HDIB32 LoadDIB32(LPSTR lpFileName,BOOL AdjustColorsToVP);
HDIB32 GMDestroyDIB32 (HDIB32 hDib);
BOOL DestroyDIB32 (HDIB32 hDib,BOOL Force);
BOOL GetFreeImageVersionAndCopyright (LPSTR Version, LPSTR Copyright) ;
BOOL GMFIBMPFromEXT (LPSTR lpszPathName,LPSTR ToMem,DWORD * pSize);
DWORD GMFIGetVersionAndCopyright (LPSTR Version,LPSTR Copyright);
HDIB32  CopyBMP32 (HDIB32 hBitmap,DWORD left,DWORD right, DWORD top, DWORD bottom); 
BOOL GetBitmapInfoFromHandle (LPBITMAPINFOHEADER pDibInfo,HDIB32 hDib);
DWORD GM32QuantizeDIBEx (HDIB32 hDIB,DWORD Flag,DWORD PalSize, DWORD ResPalSize, LPRGBQUAD Pallet);
BOOL GetGeoTiffData (HDIB32 hBMP,LPDOUBLE pScaleX,LPDOUBLE pScaleY,LPDPOINT pBitmapPoint, LPDPOINT pWorldPoint); 
BOOL SetGeoTiffData (HDIB32 hBMP,LPDOUBLE pScaleX,LPDOUBLE pScaleY,LPDPOINT pBitmapPoint, LPDPOINT pWorldPoint); 
BOOL GetImageCoord (HANDLE hBMP, LPDPOINT pWorldPoint,LPSTR DateTaken);
HDIB32 Create8BitBMP (HDIB32 dibin,RGBQUAD	*rgbpal, LPLONG plPalette);
HDIB32 Create8BitBMPSixteenth (HDIB32 dibin,RGBQUAD	*rgbpal, LPLONG plPalette,int iSixteenth,LPBOOL pErr);
HDIB32 LoadImageFromMem (FREE_IMAGE_FORMAT fif, void *pMem, int MemLen,int flags);
BOOL SaveImageToMem (FREE_IMAGE_FORMAT fif, FIBITMAP *dib,void *pMem, int *MemLen,int flags);
HANDLE WriteDIBToMem (HDIB32 hDib,int Format, int flags,LPINT pSize);
DWORD GM32Remove (LPSTR Name);
DWORD GM32GetNodeInfo (LPSTR NodeName,LPSTR UserName,LPSTR Winver);
HDIB32 GMFIBMPHandleFromEXT (LPSTR lpszPathName);
BOOL GMFIBMPGetBitmapInfo (LPBITMAPINFOHEADER pDibInfoD, HANDLE hDib);
DWORD GM32StretchDIBitsFromHandle (HDC hDC16,long destX,long destY,long destW,long destH,long xoff,
						 long yoff,long sourcew, long sourceh, HANDLE Handle,DWORD ColorType,DWORD RasterOpt,LPDOUBLE pFactorD);
BOOL GMFIGetGeoTiffData (HANDLE hBMP,DWORD ShowTag,DWORD pScaleX, DWORD pScaleY, DWORD pBitmapPoint, DWORD pWorldPoint);
DWORD GM32DrawLineWithFlatEnd (HDC hDC16,DWORD npt,LPPOINT pPoints32,DWORD Width,DWORD Color);
HDIB32 LoadDIBFromMem (LPBYTE pMem,int MemLen,int Format,int flags);
HDIB32  BMPHandleFromEXT (LPSTR ImageFile); 
BOOL GMFISetGeoTiffData (DWORD hBMP,DWORD pScaleX, DWORD pScaleY, DWORD pBitmapPoint, DWORD pWorldPoint);
HDIB32 GMRotateImageClassic (HDIB32 hDib,double DegreesRotation);
HANDLE GMFreeImageRotateClassic (HANDLE hDIBIn,LPDOUBLE pRotate);
DWORD GMFICopy (DWORD hDIBIn,DWORD left,DWORD right, DWORD top, DWORD bottom);
BOOL GMFIBMPFileFromEXT (LPSTR lpszPathName,LPSTR ToFile,double factor);
BOOL GMFIBMPToEXT (LPSTR lpszPathName,LPSTR FromMem,DWORD * pSize,DWORD Flag);
BOOL GMFIBMPHandleToEXT (LPSTR lpszPathName,HANDLE hBMP,DWORD Flag);
BOOL GMSetDIBMonoColors (HANDLE hBMP,LPLONG Colors);
BOOL GM32GetGMCName(HWND hWnd16,LPSTR PathName,
								 LPSTR InitialDirIn,LPSTR Title,LPBOOL pStartInNewSession,LPBOOL pRetainZoom,LPBOOL pLinkZoom,LPSTR NetDir,LPSTR PersonalDir);
BOOL PickNearestCPTFile (DPOINT Point,LPSTR Filelist,LPSTR IgnoreName,double MaxDist,LPSTR OutLoc);
DWORD SearchDirectory32 (LPSTR Name,DWORD UseHandle,LPDWORD pType,WIN32_FIND_DATA	*pFindFileData);
DWORD GetSpecialDirectory (LPSTR Name);
BOOL GSSiGetNodeInfo (LPSTR CDriveSerno,LPSTR NodeName,LPSTR UserName,LPSTR Winver);
DWORD	GetDriveSerialNumber (LPSTR Drive);
BOOL GetDriveLetterFromLocalDriveName (LPSTR Name,LPSTR Drive);
BOOL	GetSDChipSerialNumber (LPSTR Drive);
BOOL DisplayHTMLHelp (LPSTR HelpFile,LPSTR Topic);
BOOL SizeDlgToRect (HWND hWndDlg,LPRECT pRect);
RECT Rect16ToRect32 (RECT16 rect16);
RECT16 Rect32ToRect16 (RECT rect16);
BOOL GM32NADCONINIT (LPSTR InitFile);
BOOL GMNADCON (LPDPOINT Point,DWORD Dir);
long NumRowsInTxtFile (HFILE Fid);
BOOL CompressFrame (LPBITMAPINFOHEADER	lpbiIn, LPBITMAPINFOHEADER lpbiOut);
BOOL CompressFrameEX (LPBITMAPINFOHEADER	lpbiIn, LPBITMAPINFOHEADER lpbiOut, long Quality);
BOOL SetupVirtualPrinter (HWND hWnd,LPSTR VPName);
int GSSiMsgBox (HWND hWnd, LPSTR Mess, LPSTR Title, UINT Flag,LPSTR Position);
int MessageBoxAtPosition (HWND hWnd, LPSTR Mess, LPSTR Title, UINT Flag,LPSTR Position);
BOOL GetPerpendicularIntersect (LPDPOINT Point,LPDPOINT Line,LPDPOINT IntPoint,BOOL IncludeDistToEPs);
BOOL GetPerpendicularIntersect2 (LPDPOINT Point,LPDPOINT LinePt,LPDOUBLE pLineAZ,LPDPOINT IntPoint);
BOOL GetPerpendicularOffsetToPoly (LPDPOINT Point,long nPolyPoints,HPDPOINT PolyPoints,LPDPOINT IntPoint,LPDOUBLE OffDist,LPDOUBLE PolyDist,double LastPolyDist);
void AreaFromLineAndOffset (LPPOINT Line,double Offset,LPPOINT AreaPoints);  
BOOL OpenGarminUSB (LPSTR ProductID);
BOOL CloseGarminUSB (LPSTR NullArg);
int	Insert (LPSTR Opt,LPSTR File,LPSTR Text);
//HPSTR malloc32 (DWORD Bytes);
//DWORD malloc32free (HPSTR pMem);
time_t GetLastFileWriteTime (LPSTR File,LPLONG pDiffTime);
time_t GetFileCreateTime (LPSTR File,LPLONG pTimeDiff);
BOOL GetDirectoryCreateTime (LPSTR File,LPDWORD pLowTime,LPDWORD pHighTime,LPDWORD pTimeDiff);
DWORD MrSidVersion (LPSTR Version);
HDIB32 MrSidOpen (LPSTR File);
DWORD MrSidClose (HDIB32 ImageHandle);
DWORD MrSidGetInfo (HDIB32 ImageHandle,
					LPDWORD pWidth,
					LPDWORD pHeight,
					LPDWORD	pColorSpace,
					LPDWORD	pNumBands,
					LPDWORD	pDataType,
					LPDOUBLE pMinMag,
					LPDOUBLE pMaxMag,
					LPDWORD	pIsLocked,
					LPDOUBLE pULX,
					LPDOUBLE pULY,
					LPDOUBLE pXres,
					LPDOUBLE pYres,
					LPDOUBLE pXRot,
					LPDOUBLE pYRot,
					LPDWORD pNumMetaRecords);
DWORD MrSidGetMetadataRecord(HDIB32 ImageHandle,
                               DWORD recordNum,
                               LPSTR ptag,
                               LPSHORT pdatatype,
                               LPSHORT pnumDims,
                               LPDWORD *ppdims,
                               LPVOID *ppdata);
HDIB32 GMMrSidGetImage (HDIB32 ImageHandle,
					   LPDOUBLE pxUpperLeft,
					   LPDOUBLE pyUpperLeft,
					   LPDWORD pWidth,
					   LPDWORD pHeight,
					   LPDOUBLE pMagnification,
					   DWORD	ConvertToGray,
					   DWORD	Intensity,
					   DWORD DisplayErrorMessage);
HDIB32 MrSidGetImage (HDIB32 ImageHandle,
					LPDOUBLE pULX,
					LPDOUBLE pULY,
					LPDWORD pWidth,
					LPDWORD pHeight,
					LPDOUBLE	pMag,
					DWORD	ConvertToGray,
					DWORD		Intensity);
HDIB32 GMMrSidOpen (LPSTR MrSidFile);
DWORD GMMrSidClose (HDIB32 ImageHandle);
DWORD GMMrSidVersion (LPSTR Version);
DWORD GMMrSidGetImageInfo (HDIB32 ImageHandle,
						   DWORD pWidth,
						   DWORD pHeight,
						   DWORD pColorSpace,
						   DWORD pNumBands,
						   DWORD pDataType,
						   DWORD pMinMag,
						   DWORD pMaxMag,
						   DWORD pIsLocked,
						   DWORD pULX,
						   DWORD pULY,
						   DWORD pXres,
						   DWORD pYres,
						   DWORD pXRot,
						   DWORD pYRot,
						   DWORD pNumMetaRecords
						   );
long Convert_MGRS_To_Geodetic (char* MGRS, 
                               double *Latitude, 
                               double *Longitude);
long GMConvert_Geodetic_To_MGRS (double *Latitude,
                               double *Longitude,
                               long Precision,
                               char* MGRS);
DWORD GMURLDownloadToFile(LPSTR URL,LPSTR File);
BOOL GetTXTUniqueFieldValues (HANDLE hDB, LPSTR SQL,LPSTR FldName,short FieldLength, HANDLE hDBList);
BOOL GetDBUniqueFieldValues (HANDLE hDB, LPSTR SQL,LPSTR FldName, LPHANDLE phDBList);
DWORD SendEMail (LPSTR cmd);
BOOL URLToFile (LPSTR URL,LPSTR File);
LPSTR requestFromURL(LPSTR url);

  void CNGRNT(const double *BX1,const double *BY1,const double *EX1,const double *EY1,
                    const double *BX2,const double *BY2,const double *EX2,const double *EY2,
                    short *N1,short *N2, short *IRC);
                    

 void SECLIN8(const double *X1,const double *Y1,const double *A1,const double *X2,
                    const double *Y2,const double *A2,double *X3,double *Y3,short *K);
                    
 double DSPTLN(const double *X1,const double *Y1,const double *X2,const double *Y2,
                     const double *XP,const double *YP, short *N, short m1);
                      
  double DSTMIN(const double *X1,const double *Y1,const double *X2,const double *Y2,
                      const double *X3,const double *Y3,const double *XP,const double *YP,
                      short *N);
                      
 short INLNCK(const double *BX,const double *BY,const double *EX,const double *EY,
                  const double *PX8,const double *PY8); 
                    
  double DMNMX(const double *X1,const double *Y1,const double *X2,const double *Y2,
                     const double *X3,const double *Y3,const double *X4,const double *Y4,
                     short *N1,short *N2,short *IRC);
                                                                                  
 double DMNMX_TOL(const double *X1,const double *Y1,const double *X2,const double *Y2,
                        const double *X3,const double *Y3,const double *X4,const double *Y4,
                        short *N1,short *N2, const double TOL,short *IRC);
                        
 void XLL(const double *BX1, const double *BY1, const double *EX1, const double *EY1,
                const double *BX2, const double *BY2, const double *EX2, const double *EY2,
                double *X1, double *Y1, double *X2, double *Y2, double *X3, double *Y3, 
                short *N1, short *N2, double *DMN, short *ICD);
         
 void XLC(const double *ldax1,  const double *lday1,
                const double *ldax2,  const double *lday2, const double *ldalngth,
                const double *ldax21, const double *lday21, 
                const double *ldax22, const double *lday22,
                double far *x1, double far *y1, double far *x2,double far *y2,
                double far *x3, double far *y3, short *n1, short *n2,
                double *gap, short *i2);  

 void XCC(const double *c1x,const double *c1y,const double *r1x,const double *r1y,
                const double *cl1,const double *c2x,const double *c2y,const double *r2x,
                const double *r2y,const double *cl2,double *x1, double *y1, 
                double *x2, double *y2, double *x3, double *y3,
                short *n1, short *n2, double *dmn, short *icd);                        
                
 short     INCRVE(const double *CX,
                const double *CY,
                const double *RX,
                const double *RY,
                const double *CL,
                const double *PtX,
                const double *PtY);

HANDLE OpenComm (LPSTR CommID,int buf1,int buf2);
BOOL CloseComm (HANDLE Stream);
BOOL FoundModem (HANDLE Stream);
int ReadComm (HANDLE Stream,LPBYTE Buffer,int MaxRead);
int WriteComm (HANDLE Stream,LPBYTE Buffer,int MaxRead);
int LogCommIO (int From,LPBYTE Buffer,int BytesRead);
int GetCommEventMask(HANDLE Stream,int EvtToClear);
BOOL GetSystemErrMessage (DWORD errorcode,LPSTR Mess);
HWND CreateToolbarWnd (HWND hWnd); 
void AbendWriter (LPSTR Message,LPSTR Title,long at,int type);
int SysMonthFromSymTime (time_t systime);
BOOL CALLBACK CACHEFILEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam);
void __cdecl BackgroundCache (LPHANDLE phArgs);
BOOL StartBackgroundCache (void);
void StopBackgroundCache (void);
void RenameCachedFiles (LPSTR CacheDir);
HANDLE GetNextXMLElement (HANDLE FileHandle,LPINT pFileLoc,LPSTR TagID);
HANDLE LoadXMLFile (LPSTR File);
BOOL GetXMLElementAttribute (HANDLE hElem,LPSTR AttrName,LPSTR AttrValue);
HANDLE GetXMLElementValue (HANDLE hElement);

double ComputeMaxSlope (double frontSlope,double sideSlope);

//2D
float DotVec2(vec2* vec0, vec2* vec1);
void  LerpVec2(vec2* out_lerpedPos, vec2* p0, vec2* p1, float t);
void  NormalizeVec2(vec2* vec);
float MagnitudeVec2(vec2* vec);
void  SubVec2(vec2* out_resultVec, vec2* p0, vec2* p1);
void  SubVec2_Self(vec2* out_resultVec, vec2* p);
void  ScaleVec2_Self(vec2* vec, float scale);
void  ScaleVec2(vec2* out_resultVec, vec2* vec, float scale);
void  AddVec2_Self(vec2* out_resultVec, vec2* vec);
void  AddVec2(vec2* out_resultVec, vec2* vec0, vec2* vec1);
void  AddScaledVec2_Self(vec2* out_resultVec, vec2* vec, float scale);
void  AddScaledVec2(vec2* out_resultVec, vec2* vec0, vec2* vec1, float scale);
void  SubScaledVec2_Self(vec2* out_resultVec, vec2* vec, float scale);
void  SubScaledVec2(vec2* out_resultVec, vec2* vec0, vec2* vec1, float scale);
void  CopyVec2(vec2* out_result, vec2* point);
void  CreatePlaneFromPointsVec2(struct PlaneVec2* out_resultPlane, vec2* p0, vec2* p1);
BOOL  PointInsidePlaneVec2(vec2* point,struct PlaneVec2* plane);
BOOL  PointInsidePlaneVec2(vec2* point, struct PlaneVec2* plane);
float AngleBetweenVec2(vec2* vec0, vec2* vec1);

//3D
void  SetVec3(vec3* out_resultVec,float x, float y, float z);
void  CrossVec3(vec3* out_resultVec, const vec3* vec0, const vec3* vec1);
float DotVec3(const vec3* vec0, const vec3* vec1);
void  LerpVec3(vec3* out_lerpedPos, const vec3* p0, const vec3* p1, float t);
void  NormalizeVec3(vec3* vec);
float MagnitudeVec3(const vec3* vec);
BOOL  PosIsBehind(const vec3* frontPos, const vec3* frontAt, const vec3* behindPos);
float DistSqVec3(const vec3* vec0, const vec3* vec1);
float DistVec3_2D(const vec3* vec0, const vec3* vec1);
float DistVec3(const vec3* vec0, const vec3* vec1);
float MagnitudeSqVec3(const vec3* vec);
void  AddVec3_Self(vec3* out_resultVec, vec3* vec);
void  AddVec3(vec3* out_resultVec, vec3* vec0, vec3* vec1);
void  AddScaledVec3_Self(vec3* out_resultVec, const vec3* vec, float scale);
void  AddScaledVec3(vec3* out_resultVec, const vec3* vec0, const vec3* vec1, float scale);
void  SubScaledVec3_Self(vec3* out_resultVec, vec3* vec, float scale);
void  SubScaledVec3(vec3* out_resultVec, vec3* vec0, vec3* vec1, float scale);
void  SubVec3(vec3* out_resultVec, const vec3* p0, const vec3* p1);
void  ScaleVec3_Self(vec3* vec, float scale);
void  ScaleVec3(vec3* out_resultVec, const vec3* vec, float scale);
void  CopyVec3(vec3* out_result, const vec3* point);
float AngleBetweenVec3(vec3* vec0, vec3* vec1);
vec3 VectorFromPoints (vec3 * p1, vec3 * p2);
vec3 sVectorFromPoints (svec3 * p1, svec3 * p2);
vec3 GetTriangleNormal (vec3 * vertex1,vec3 * vertex2,vec3 * vertex3);
vec3 sGetTriangleNormal (svec3 * vertex1,svec3 * vertex2,svec3 * vertex3,double metersperpixel);

HANDLE FTPOpen (LPCSTR lpszServerName,LPCSTR lpszUsername,LPCSTR lpszPassword,LPCSTR directory,LPSTR errorVarName);
BOOL FTPClose (HANDLE h);
HANDLE ListFtpDir(HANDLE hConnection,HANDLE hFind,
				  LPSTR pWildCard,DWORD dwFindFlags,
				  LPSTR fileName,LPDWORD pFileSize,LPDWORD pLastUpdateTime,LPBOOL pisDirecotry,LPSTR errorVarName);
BOOL FTPGetFile(HANDLE hConnect,LPCTSTR lpszRemoteFile,LPCTSTR lpszNewFile,BOOL fFailIfExists,BOOL showStatus,LPSTR errorVarName);
BOOL FTPPutFile(HANDLE hConnect,LPCTSTR lpszRemoteFile,LPCTSTR lpszLocalfile,BOOL replace,BOOL showStatus,LPSTR errorVarName);
BOOL FTPDeleteFile(HANDLE hConnect,LPCTSTR lpszRemoteFile,LPSTR errorVarName);
BOOL FTPCreateDirectory(HANDLE hConnect,LPCTSTR lpszRemoteDir,LPSTR errorVarName);
BOOL FTPGetDirectory(HANDLE hConnect,LPSTR lpszRemoteDir,LPSTR errorVarName);
BOOL FTPSetDirectory(HANDLE hConnect,LPCTSTR lpszRemoteDir,LPSTR errorVarName);
__int64 FTPGetFileSize (HANDLE hConnect);


void  InFunction (int funid,LPSTR inString);
void OutFunction (int funid,LPSTR outString);
void SetFunctionDBIn(LPSTR InLoc);
void SetFunctionDBOut(LPSTR OutLoc);
void DebugReturn(LPSTR rtnValue);
HANDLE CreateVarSpace(int type);
void DestroyVarSpace(HANDLE hVarSpace);
void SetVarSpace(int type, HANDLE hVarSpace);
int AddToMacroStack(int from, int iCurrentMacro, LPSTR File, LPHANDLE phArgs, int NumArgs);
void RemoveFromMacroStack (int macroID);
BOOL AddBreakpoint(LPSTR macroFile, int insertLoc);
void setMacroBrkPtHandle(int macroID, HANDLE hBreakPoints, int lnBP);
void breakAtPos(int pos, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen, int from);
void AtBreakPoint(LPSTR Args,int bploc);
void SetDebug (BOOL state);
BOOL GetDebug (void);
BOOL getDebugMacro(int macroID);
void GetWindowsVersion(LPSTR Ver);
int ChassisType(void); // returns -1 if error, 1 for desktop, 2 for laptop, 3 for handheld and 4 for other
int MonitorType(int which,LPSTR monName);
int GetMouseType(void);
int GetNumMonitors(void);
HMONITOR GetOtherMonitor(POINT pt);
BOOL IsPointOnTouchScreen(HWND hWnd,POINT pt);

int wlanGetCurrentSSID(char * pSSID, int maxSSID, GUID *pInterfaceGuid);
int getipAddressForAdapter(LPSTR ipAddress, GUID *pInterfaceGuid);

int GetWifiName(LPSTR Name);
int GetWifiAddress(LPSTR address);

void Rotate256(int irot, LPDPOINT pt);




