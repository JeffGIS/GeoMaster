#include <windows.h>  
#include <float.h>   
#include <math.h>
#include <stdio.h> 
#ifndef _WIN32_WCE 
#include <io.h>
#include <dos.h>
#include <conio.h>
#include <time.h>
#include <errno.h>
#endif
#include <sql.h>
#include <winsock.h>
#include <limits.h>

#include <string.h>
#include <memory.h>
#include <stdlib.h>     
#include <direct.h>
#include <stdio.h>
#include <ctype.h>
#include <dlgs.h>    
#include <fcntl.h>
#include <tchar.h>
#include <malloc.h>   

#include <toolhelp.h>
#include <compobj.h>
#include <scode.h>
#include <bios.h>
#include <print.h>
#include <sqlext.h>
#include <mmsystem.h> 
#include <assert.h>
#include <shellapi.h>  
#include <dde.h>



//suspect


#include "gssitype.h"
#include "compute.h"


#define	OF_CREATE_NODELETE	0x1111      

#define UP_3D		1
#define DOWN_3D		2
#define	NO_3D		3

#define GSSI_REINITDIALOG       0x5110  
#define GSSI_ADDGF 0x5111
#define GSSI_REPOSITION 0x5112

#define MK_DIGITIZER_BUTTON	    0x0020

#define GetWValue(rgb)  ((BYTE)((rgb)>>24))
#define RGBW(r,g,b,w)   ((COLORREF)(((BYTE)(r)|((WORD)(g)<<8))|(((DWORD)(BYTE)(b))<<16)|(((DWORD)(BYTE)(w))<<24)))

typedef struct {double a,b,c,d,e,f,xc,yc;} TRN; 
typedef TRN FAR *LPTRN;

#define	MAXFILEHANDLES	80   

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
#define  RADIAN    5.729577951308232e1
#define  SFTSM     9.2903411613274e-2
#define  SMSFT     1.076386736111111e1
#define  TWOPI     6.283185307179586e0
#define  ACRSM     4046.872609874253e0  
#define  INCHES_PER_CM  0.3937e0
#define  MAX_GAP_BTWN_LINKS    0.1e0 
#define  ZERO$  -2146450000 
#define  TMPRF$ -2146483000   
#define	 NULLREF 0

#define  GOOD_SOUND 1
#define  BAD_SOUND  2  
    
BOOL FAR PASCAL DATAFILEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam, WORD cntlSQL,
                                WORD cntlSET_FILE, WORD cntlDATABASE_LIST, WORD cntlTABLE_NAMES,WORD cntlTABLE_NAMES_TITLE,
                                LPWORD pcntlFIELD_NAMES,short NumFieldLists,
                                LPSTR DataFile, short *DataFileType, HANDLE *hThemeDB,
                                LPINT pFieldListIsCB, BOOL WantBrackets);
BOOL FAR PASCAL FIELDSMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL FAR PASCAL CENFIELDSMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
void ClientRectToScreenRect (HWND hWnd,LPRECT pRect);
void ScreenRectToClientRect (HWND hWnd,LPRECT pRect);
BOOL FAR PASCAL DIALOGSTYLEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL SetDynDlgData (HWND hWndDlg,LPSTR Name,LPUSHORT pcntls,short lncntls);
BOOL rread (LPSTR str, LPDOUBLE lpRval, LPINT lpNdp);
double dread (LPSTR str, short nbytes);
long ldread (LPSTR str, short nbytes);
void RWRITE (double RVAL, short NDP, LPSTR OutLoc);
void RWRITEZ (double RVAL, short NDP, short len,LPSTR OutLoc); 
short Strip(LPSTR str, char chr);
LPSTR ReplaceChar (LPSTR str, char from, char to);
LPSTR RemoveDoubleQuotes (LPSTR str);
LPSTR ConvertCharBtwnDoubleQuotes (LPSTR str, char from, char to);
short FillRectPoly(HDC hDC, LPRECT Rect, COLORREF Color);
short DrawRectPoly(HDC hDC, LPRECT Rect, HPEN hPen);
BOOL LoadGlobalInit (LPSTR File,BOOL First);
HGLOBAL GSSiGlobAlloc(USHORT From,UINT fuAlloc, long cbAlloc);
void SECLIN8(const double *X1,const double *Y1,const double *A1,const double *X2,
             const double *Y2,const double *A2,double *X3,double *Y3,short *K);
double  AZDF(double AZC, double AZP, double RL);
double DSIGN(const double Value, const double Sign);
void ERROr( long istat, char *message);
LPSTR LastChr (LPSTR str);
short FillList (HWND hWndDlg,short Control,LPSTR file,LPSTR DefaultVal,LPRECT pRect);
short FillCBList (HWND hWndDlg,short Control,LPSTR file, short InitVal,LPSTR DefaultVal);
UINT ItemInList (short i,UINT nElements,HANDLE hElements);
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
long SearchFilesInDir (LPSTR CurDir, LPSTR Ext, HFILE OutFile,LPLONG TotFiles,LPSTR WildCard,short Lev);
long SearchDirectoriesInDir (LPSTR CurDirIN, HFILE OutFile,LPLONG TotFiles,LPSTR WildCard,short Lev);
BOOL ExistFile(LPSTR Name);
short FileType(LPSTR Name);
void Sound (short Type);
void SetTrace (BOOL On); 
void TraceInWindow (LPSTR str);
void GSSiTrace (LPSTR str,short From);
void GSSiTraceLev (LPSTR str,short lev,short From);
BOOL FileErrMess (HFILE Fid,LPSTR Name,LPOFSTRUCT pOFStruct,UINT Mode);
HFILE GSSiOpenFile (LPSTR Name,LPOFSTRUCT pOFStruct,UINT Mode);
HFILE GSSiClose (HFILE Fid);
void LogOpenFilesOpen (UINT Mode,HFILE Fid,LPOFSTRUCT pOFStruct);
void LogOpenFilesClose (HFILE Fid);
BOOL RequestFileClose (HFILE Fid);  
void CloseAllRequestedFiles (BOOL FirstOnly); 
HFILE FileAlreadyOpen (LPSTR Name,UINT Mode,LPOFSTRUCT pOFStruct);
BOOL GSSiChangeLength (HFILE Fid,long NewLength);      
void CreateFidSmall (void);
void CloseFidSmall (void);
BOOL EditTextFile (HWND hWnd,LPSTR Name);
void CloseVars (void);
short OpenDataFile (LPSTR Name, LPSTR SQL, short Access, HANDLE *hDB);
BOOL CloseDataFile (BOOL Final,HANDLE *hDB);
HANDLE GetDBHandleFromSQL (HANDLE hSQL);
BOOL FetchDBRec (HANDLE hSQLPtr);
BOOL SaveDBRec (HANDLE hSQLPtr);
BOOL SetGMDField (HANDLE hSQLPtr,LPSTR Name,LPSTR Value);
HANDLE GetDBByIDName (LPSTR IDName);
short WantGMDRecord (LPOPENFILEDATA FilePtr,LPOPENSQLDATA SQLPtr,LPVOID lpGWDHead,short cond,BOOL HaveOtherFields);
short ProcessDelimTextHeader(LPSTR str,LPHANDLE phDLT);
BOOL GetDelimTextData(LPSTR str,HANDLE hDLT);
BOOL GetDLTVarName (short ivar,HANDLE hDLT, LPSTR Name);
HANDLE	GetDLTVarHandle (short ivar,HANDLE hDLT);
BOOL UpdateGlobalFile (LPSTR RptFile,LPSTR VarName,LPSTR str);
HANDLE FindVar (LPSTR Name);
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
void SetGlobalValue2 (HANDLE handle, LPSTR Value, short Index);
void SetGlobalValue3 (LPSTR Name, LPSTR Value, short Index, BOOL FoundLit);
void SetGlobalValueLen (LPSTR Name, LPSTR Value, short len);
void SetGlobalValueReal (LPSTR Name, double val);
void SetGlobalValueLong (LPSTR Name, long val);
void SetGlobalValueLong (LPSTR Name, long val);
void SetGlobalValueHandle (LPSTR Name, long val);
void SetGlobalValueDPoint (LPSTR Name, DPOINT Point);  
void SetGlobalValueBounds (LPSTR Name, LPMNMXCORD pBounds);
void SetGlobalValueRect (LPSTR Name, RECT Rect);  
BOOL SetGlobalFromCheckBox (HWND hWndDlg,UINT Control,LPSTR VarName);
void SetVarChangeTimes (short opt);
short   GetValFromOpenFiles (LPSTR VarName,LPSTR Value);
void CloseMacroFiles (long ThisMacro);
BOOL GetFieldDefFromOpenFiles (LPSTR VarName,LPFIELDINFO lpField);
BOOL AddToOpenFileList (HANDLE hFile);
void ExpandGlobalRaw (LPSTR VarName,LPSTR Value, short maxlen);
void RemoveFromOpenFileList (HANDLE hFile);
LPOPENFILEDATA InOpenFileList (LPSTR Name);
LPSTR ExpandText (LPSTR InText);
LPSTR ExpandText2 (LPSTR InText);
void ProcessText (LPSTR InText);
void ProcessGlobal (LPSTR Global);
HANDLE ProcessIF (LPSTR InLoc,LPSHORT pErr);
LPSTR MatchLev (LPSTR InStr, char MatchChar);  
BOOL StringEndsWith (LPSTR str,LPSTR endstr);
BOOL GetVal (LPSTR VarName, LPSTR OutStr);
void RestoreScreenRect (HDC hDC, HBITMAP hSavedBM, RECT Rect, RECT NewRect);
void TRAN (void *source, void *dest, size_t count);
long  NCHK(LPSTR NAME,  long i,  long PART_LN);
void  STRIPR(LPSTR NAME, short *NLEN,LPSTR out_with,  short llen); 
long  IREAD (LPSTR NAME, long NUMDIG,long *IRC);  
BOOL IWRITE (long ival,LPSTR str ,short len);  
BOOL IWRITEZ (long ival,LPSTR str ,short len);
LPSTR REPLAC (LPSTR STRING, LPSTR OLD, LPSTR NEW, short MAXLEN);
void RestoreScreen (HDC hDC, HBITMAP hSavedBM, RECT Rect);
void RestoreScreen2 (HDC hDC, HANDLE hSavedScreen,long ID,BOOL Clip);
HBITMAP SaveScreen (HDC hDC, RECT Rect);   
HANDLE SaveScreen2 (HDC hDC, RECT Rect, LPVOID pVP,LPLONG pID); 
void DestroySavedScreen (LPHANDLE hSavedScreen,long ID);
void RegisterSavedScreen (HANDLE hSavedScreen);
void UnRegisterSavedScreen (HANDLE hSavedScreen);
BOOL ScreenIsRegistered (HANDLE hSavedScreen,long ID);
void ClearSavedScreens(LPVOID pVP,LPRECT pRect);
void ResaveSavedScreens(void);
void SetBit (short ibit, LPSTR lpBytes, BOOL setto);
void SetBit2 (short ibit, LPSTR lpBytes, BOOL setto);
BOOL GetBit (short ibit, LPSTR lpBytes);
BOOL GetBitH (long ibit, HPSTR lpBytes);
void SetBitH (long ibit, HPSTR lpBytes, BOOL setto);
void GetRunDate (LPSTR RunID, LPSTR Date);
COLORREF SetColor (LPSTR pColor);
long GSSifilelength (HFILE Fid);
short WaitCursor (short iWait);
short LoadStrings (short nstrings ,HANDLE *hstrings,LPSTR *pNextLine,LPINT LineNum);
BOOL GWCheckMenuItem(HWND hWnd, WORD wItem);
LPSTR StrEnd (LPSTR str);
short EndStr (LPSTR str);
LPSTR fgetss (LPSTR lpStr, short len, FILE *Fid);
void fputss (LPSTR lpStr, FILE *Fid);
BOOL fputstring(LPSTR lpStr, HFILE Fid);
LPSTR fgetstring (LPSTR lpStr, USHORT len, HFILE Fid);
float MULREG (double Y[], double X1[], double X2[], short N,
              LPDOUBLE A,LPDOUBLE B, LPDOUBLE C);
double AMEAN (double X[],short N);
__inline long IDNINT (double X);       
short IDNSHRT (double X);
HANDLE STRAN2 (double X1[], double Y1[],double X2[],double Y2[],short N, LPFLOAT RSQMIN, short Type,LPMNMXCORD pBounds);
void TRANS2 (double XIN,double YIN, LPDOUBLE XOUT,LPDOUBLE YOUT, HANDLE hlpTran);
void TRNPRO (double XIN,double YIN, LPDOUBLE XOUT,LPDOUBLE YOUT, HANDLE hlpTran);   
void CloseTRANS2 (LPHANDLE phlpTran); 
HANDLE STRANRect (LPRECT FromRect,LPRECT ToRect);
void TRANRect (LPRECT Rect,HANDLE hTran); 
HANDLE STRANBoundsToRect (LPMNMXCORD FromRect,LPRECT ToRect);
HANDLE STRANRectToBounds (LPRECT FromRect,LPMNMXCORD ToRect);
POINT TRANDPointToPoint (DPOINT Dpoint,HANDLE hTran);
HANDLE ReadTranData (HFILE Fid);
DPOINT TranPoint (LPDPOINT pPoint,HANDLE hTran);
HANDLE TRFTRI_SET (LPDOUBLE XT,LPDOUBLE YT,LPDOUBLE XT2,LPDOUBLE YT2,short NSETPT,
				   double BASX1, double BASY1,double BASX2, double BASY2, HANDLE hTran,LPMNMXCORD pTriBounds);
BOOL makedirectories (LPSTR Name,BOOL IsDir,BOOL Verify);   
short GSSiMakeDir (LPSTR Name);   
short GSSiRemove32 (LPSTR Name); 
short GSSiRemoveDir (LPSTR Name); 
BOOL GSSirenamefile (LPSTR OldName,LPSTR NewName);
BOOL GSSiCopyFile (LPSTR OldName,LPSTR NewName,BOOL Replace);
void WriteTranData (HFILE Fid,HANDLE hTran);
HANDLE LoadTranFile (LPSTR Name, short dir,short Type,LPSHORT pNumPoints,LPDOUBLE pRSQ);
MNMXCORD GetTranFileBounds (LPSTR Name,LPSTR Direction);
HANDLE LoadTranFileWithDandT (LPSTR Name);
long ConvertPoint (LPSTR CvtFile,LPDPOINT Point,short Direction);

short LINSEC (double X1IN,double Y1IN,double A1,double X2IN,double Y2IN,
            double A2,double *X3,double *Y3);
short LINSEC_TOL (double X1IN,double Y1IN,double A1,double X2IN,double Y2IN,
            double A2,double *X3,double *Y3,double INTOL);
short LIN_SEC (double X1IN,double Y1IN,double A1,double X2IN,double Y2IN,
             double A2,double *X3,double *Y3);
BOOL  POINT_IN_AREA (POINT PickPoint, DWORD nPoints, HPPOINT lpAreaPoints);
void ThinPoly (LPLONG pnPnts, HPDPOINT pPoints, double maxdist);
BOOL NextLine (LPSTR *lpText,LPSTR lpLine,short nAutoLines); 
LPSTR PadString (LPSTR str,char padchr,short padlen);
LPSTR Truncate(LPSTR str); 
LPSTR TruncateAt (LPSTR str,LPSTR EndChars);           
LPSTR FirstNonInt (LPSTR Text);  
LPSTR FirstAlpha (LPSTR Text);
void GSSiERROR (HWND hWnd, short ErrorNum, LPSTR ErrorMessage);
LPSTR IADDR (LPSTR InAdd, long Offset);
LPSTR NextFld (LPSTR pInRec);
LPSTR NextRec (short Fid);
BOOL CheckForContinue (void);
void Wait (long MicroSeconds);
BOOL WaitForKeystroke (BOOL UseGetMessage);
BOOL PctBox (HWND hWnd, DWORD MaxLen, DWORD Done, short Freq);  
BOOL IsInteger(LPSTR str);
HANDLE GetPrinterDC(void);
long BigWrite (HFILE Fid,HPSTR pBuf,DWORD isize,long seekloc);
long BigRead (HFILE Fid,HPSTR pBuf,long isize);
void farfree(void far *block);
BOOL CDInit (HWND hWnd, HANDLE Inst);
void CDClose (void);
BOOL GetColor (HWND hWnd,COLORREF *Color);  
COLORREF GetNextUniqueColor (LPLONG pColor);
void cwCenter(HWND, int);
void SetFilterString (short Filter); 
void PrintMessage (short    ViewID, LPSTR File, short record);
void PrintMessage2 (LPSTR line1, LPSTR line2, LPSTR line3);
BOOL GetOpenFileCD (HWND hWnd, LPSTR Name, LPSTR lpInitDir);
BOOL GetSaveFileCD (HWND hWnd, LPSTR Name, LPSTR lpInitDir);
BOOL GetFont (HWND ghWnd, LPLOGFONT lpLogFont, COLORREF *dwFontColor);
double idist(POINT Point1, POINT Point2);
double ldistp(DPOINT Point1, DPOINT Point2);
double ldistpp(LPDPOINT Point1, LPDPOINT Point2);
POINT MidPoint (POINT Point1, POINT Point2);
DPOINT MidPointD (DPOINT Point1, DPOINT Point2);
double getaz (POINT Point1, POINT Point2);
double getazd (HPDPOINT Point1, HPDPOINT Point2);
double LTWOPI (double AZ1);
double LGETAZ(double X1, double Y1, double X2, double Y2);
void AZToBear (double CurAZ,LPSTR PreDir,LPSTR DegC,LPSTR MinC,LPSTR SecC,LPSTR PostDir);
void AZToAZIM (double AZ,LPSTR DegC,LPSTR MinC,LPSTR SecC);
POINT newpt (POINT OldPoint, double AZM, double DIS);
DPOINT dnewpt (DPOINT OldPoint, double AZM, double DIS);  
void LNEWPT (double X,double Y,LPDOUBLE NewX,LPDOUBLE NewY, double AZM, double DIS);  
BOOL MemError (void);
#if WIN32
    BOOL WINAPI AbortProc ( HDC hPrinterDC, short nCode );
    BOOL WINAPI OpenGCTP32( long FAR *var1);
    BOOL WINAPI LoadGCTPLibrary32( void );
    BOOL  POINT_IN_AREAD (DPOINT PickPoint, DWORD nPoints, DPOINT * lpAreaPoints,LPDOUBLE pNextXIntersect);
#else       
    BOOL FAR PASCAL __export AbortProc ( HDC hPrinterDC, short nCode );
    void Beep( unsigned duration, unsigned frequency );
    void Sleep( clock_t _wait );
    short _far _pascal CopyFile (LPSTR FromFile, LPSTR ToFile, BOOL SameAtts);
    BOOL FAR PASCAL __export OpenGCTP32( long FAR *var1);
    BOOL  FAR PASCAL __export LoadGCTPLibrary32( void );
    BOOL  POINT_IN_AREAD (DPOINT PickPoint, DWORD nPoints, HPDPOINT lpAreaPoints,LPDOUBLE pNextXIntersect);
#endif

BOOL copyfile (LPSTR ToFile, LPSTR FromFile,short AppendOrReplace,long StartPos,long EndPos,HWND hWndDlg,UINT StatusCntl,long TotLen,LPLONG pCurLoc);
LPSTR FirstNonBlank(LPSTR Text);
LPSTR LastNonBlank(LPSTR Text);
LPSTR NextBlank(LPSTR Text);
short RemoveTrailingBlanks (LPSTR str);
UINT GetOPENERR00(void);    
double Round (double Value,double RoundTo);
LPSTR AddCommas (LPSTR VCBuff);
double DecDegFromDMS (LPSTR DMS,LPSHORT pErr);
BOOL GetDMS (double DecDegrees,LPSHORT Deg,LPSHORT Min,LPDOUBLE Sec);
LPSTR ValueConv (double Value, short ValConv,double RoundTo,BOOL Commas); 
HANDLE  BMPFromPCX (LPSTR ImageFile);  
HANDLE  BMPFromEXT (LPSTR ImageFile); 
BOOL  BMPToEXT (HANDLE hBMP,LPSTR ImageFile,DWORD Flag); 
DWORD  BMPFileFromEXT (LPSTR ImageFile,LPSTR BMPFile); 
short  DisplayBMFileInRect (HDC hDC,LPSTR ImageFile, RECT Rect, short MaintainAspect);
short  DisplayBMInRect (HDC hDC,LPBITMAPINFOHEADER pDibInfo,LPSTR pImage, RECT Rect, short MaintainAspect);
short  DisplayPCXFileInRect (HDC hDC,LPSTR ImageFile, RECT Rect, BOOL MaintainAspect);
BOOL  DisplayBMInRect2 (HDC hDC, HANDLE hBM, RECT Rect, double Factor,double VPct, double HPct);
short  DisplayBMInRect32 (HDC hDC,HDIB32 hDib, RECT Rect, short MaintainAspect);
BOOL DisplayBMInVP32 (HDC hDC,LPSTR BMFile);
BOOL CheckBoardBMInRect (HDC hDC,LPSTR ImageFile1, LPSTR ImageFile2,RECT Rect, BOOL MaintainAspect);
POINT RectMid (LPRECT rect);
void ComputeBMLoc (RECT Rect,LPBITMAPINFO pDibInfo,short MaintainAspect);  
BOOL GetDIBDimensions(LPSTR File,LPSHORT height,LPSHORT width);
BOOL LoadBitMap (LPSTR ImageFile,long frame, LPHANDLE phDib, LPSHORT pDeleteBM,
                 short BitCount, short width, short height);
BOOL ReadBitMapHeader (HFILE Fid, LPHANDLE phDibInfo, LPLONG ImageOffset);
BOOL SetupTIFHeader (HFILE Fid, LPHANDLE phDibInfo,LPSHORT pNumStrips, LPHANDLE phImageOffset,LPHANDLE phByteCounts,
					 LPSHORT pRowsPerStrip,LPSHORT pPhotoInterp,
					 LPSHORT pSamplesPerPixel,LPSHORT pPlanarConfig,LPSHORT TIFFCompression,
					 LPSHORT pBitsPerSample,LPSHORT pSkipBits);  
BOOL DecompressTIFF (HPSTR Data,long length,long explen,short type,short SkipBits);
HANDLE  BMPFromTIF (LPSTR TiffFile,BOOL Check);
short DisplayTIFFileInRect (HDC hDC,LPSTR ImageFile, RECT Rect, BOOL MaintainAspect);
BOOL Report (LPSTR Name, LPSTR ViewportName, LPSTR Prefix, LPSTR UDI, long ref,BOOL LoadOnly, BOOL FitToVP);
BOOL DisplayReport (HDC hDC, HANDLE hReport, RECT Rect, double Factor, long Refno,LPRECT SizeRect);
BOOL ReportToFile (LPSTR Name, LPSTR Prefix, LPSTR UDI, long ref,LPSTR File);
BOOL DisplayReport2 (HDC hDC, HANDLE hReport, RECT Rect, double Factor,BOOL CloseFiles,LPRECT SizeRect);
RECT SizeReport (HDC hDC,HANDLE hReport,RECT CurRect,BOOL FitToVP);
HANDLE LoadReport (LPSTR Name); 
void UnloadReport (LPHANDLE hReport);
BOOL OpenReportFiles (HANDLE hReport);
void CloseReportFiles (HANDLE hReport);
BOOL AddReportToPrintList (LPSTR ReportFile,long Refno,LPSTR Prefix, LPSTR UDI);  
BOOL GetNextPrintReport (BOOL First,LPSTR ReportName,LPLONG pRefno,LPSTR Prefix,LPSTR UDI);
BOOL PrintReport2 (HDC hPr,LPSTR ReportFile,long Refno,LPSTR Prefix,LPSTR UDI);
short   Signof (double val);
short   lSignof (long val);
double   dSignof (double val);
WORD ECRCMem(WORD *icrc, BYTE *icp, WORD icnt);
DWORD CRC16 (LPSTR Array, short nc);
BOOL GetSaveName (HWND hWnd,LPSTR Name, short StringID, LPSTR Ext);  
BOOL    GetSaveName2 (HWND hWnd,LPSTR Name, short StringID, LPSTR Ext,UINT FileNameID);
BOOL    GetSaveName3 (HWND hWnd,LPSTR Name, short StringID, LPSTR Ext,LPSTR Title,LPSTR VarName);
BOOL    GetFileName (HWND hWnd,LPSTR Name, short StringID);
BOOL    GetFileName2 (HWND hWnd,LPSTR Name, LPSTR Ext,UINT FileNameID);
BOOL    GetFileName3 (HWND hWnd,LPSTR Name,short StringID,UINT FileNameID);
BOOL    GetFileName4 (HWND hWnd,LPSTR Name,short StringID,LPSTR Ext,LPSTR Title,LPSTR VarName);
void SetOpenFlags (short Flags);
void SetCreateFile (void);
DWORD GetDlgItemPrompt (HWND hWnd); 
void flip (LPSTR In, short n);
void SetDlgPrompt (HWND hWnd,UINT Prompt,UINT More);
void InitDlgPrompts (HWND hWnd);
void ClearDlgPrompts (void);
void FillFontNameList (HWND hWndDlg,UINT ListBox);
void GMMessageBox (UINT Message, UINT Title, UINT Style);
long LoadProjection(long nj, LPSTR NAME); 
long TranProjection(long I, long O, double *x, double *y); 
extern long FAR PASCAL CloseLibrary(void);
extern BOOL FAR PASCAL OpenGCTPLibrary(void);
extern BOOL FAR PASCAL OpenGCTP(long far *d);   
BOOL ConvertCoordClose (void);
long ConvertCoord (LPDPOINT DPoint,short from, short to);
long ConvertCoordInit (void);
long ConvertAndTranCoord (LPDPOINT pPoint,HANDLE hTranFile);
void ConvertCoordError(long ErrNum);
BOOL GetCurVal (LPSTR Val,short maxval, UINT StringID);
void SetCurVal (LPSTR Val,UINT StringID);
void SaveGlobalVals (HFILE Fid);
BOOL AVIFrameToDIB (LPSTR File, long frame,LPHANDLE NewDIB,LPSHORT ShouldDeleteBM,
                    short Bitcount, short width, short height);
long NumDIBColors (HANDLE hDib);
void CloseOrthoAVI (void);
short AVIOut (LPSTR Name,LPBITMAPINFOHEADER alpbi,LPHANDLE hFile,LPLONG pFrame,BOOL UseExCmp);
void AVIOutClose (LPHANDLE phFile);
HANDLE ConvertBitmap16To24 (LPBITMAPINFOHEADER  lpbi);
HANDLE ConvertBitmap8To24 (LPBITMAPINFOHEADER  lpbi);
void GSSiGlobFree (LPHANDLE pHandle); 
void GSSiGlobUlFree (LPHANDLE pHandle);    
int	GSSiRemove (LPSTR Name);
long GSSiLength (LPSTR Name);
BOOL GSSiRename (LPSTR NameFrom, LPSTR NameTo);
HCURSOR GSSiSetCursor (HCURSOR hCursor);
POINT TAGPtToWinPt (DPOINT TagPoint);
DPOINT WinPtToTAGPt (POINT WinPoint);
float PixelsToTAGFontHt (LPLOGFONT lpFont,short CoordStyle);
double TAGFontHtToPixels (double TAGFontHeight,short CoordStyle);
DPOINT AverageDPoint (DPOINT Point1, DPOINT Point2);   
POINT AveragePoint (POINT Point1, POINT Point2);   
DPOINT AverageDPoints (HPDPOINT Point,long nPnts);
BOOL RemoveLine (LPSTR Name,LPSTR line);
BOOL DisplayTextFileInRect (HDC hDC,LPRECT Rect,LPSTR Name);
LPSTR UpcaseFirst (LPSTR s);
LPSTR OneSpace (LPSTR Arg1); 
LPSTR strncpy0(LPSTR Buff,LPSTR str, size_t n);
void SetFocusAndCursor (HWND hWnd);
BOOL GetTextString (HWND hWnd,LPSTR String,short lenstring, LPSTR Title,LPSTR ListFile,LPSTR InitVal,long AutoInc,BOOL DrowDown,BOOL Sorted); 
BOOL GetTextStringML (HWND hWnd,LPSTR String,short lenstring, LPSTR Title,LPSTR InitVal);
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
BOOL GetGlobalValRaw (LPSTR Global,LPSTR Val);
BOOL ExecuteCommandString (LPSTR pCommand);
HBITMAP PCXtoBMP (HDC hDC,LPSTR Name);   
long Factorial (long value);    
BOOL LoadTAGDef (void);
double ArcDistance(DPOINT p1, DPOINT p2);
DPOINT NewLatLong(double Lat,double Long,double Dist,double Direction);
short DrawCircle (HDC hDC,POINT Point,double Radius,short Width,COLORREF Color); 
void AddPointToRect (POINT pPoint,LPRECT pBounds);
void AddPointToMinMax (POINT pPoint,LPMINMAX pMinMax);
void AddDPointToMinMax (LPDPOINT Point,LPMNMXCORD pBounds);
void MinMaxInit (LPMINMAX pMinMax);
void AddPointToMinMaxL (POINT Point,LPMNMXCORL pBounds);
void RectInit (LPRECT pMinMax);
void BoundsToPoints (LPMNMXCORD pBounds, LPDPOINT Points);
BOOL DPointInBounds (LPDPOINT Point,LPMNMXCORD pBounds);
DPOINT FPointToDPoint (FPOINT point);
DPOINT PointToDPoint (POINT Point);
POINT DPointToPoint (DPOINT Point);
FPOINT DPointToFPoint (DPOINT DPoint);
void InflateBounds (LPMNMXCORD pBounds, double Value);
void InflateMinMax (LPMINMAX pBounds, short Value);
BOOL RectInRect(LPRECT pRectIn,LPRECT pRectTest);
BOOL IntersectBounds (LPMNMXCORD pBounds1,LPMNMXCORD pBounds2,LPMNMXCORD pBoundsInt);
void BlowOut (LPSTR Message, LPSTR Title);
void SubstituteDL (LPSTR Name); 
void SetReplacePath (LPSTR Name,short opt);
LPSTR ldelim (LPSTR str,char delim);
BOOL ProcessMacroReport (LPSTR Name, LPSTR Prefix, LPSTR UDI, long ref);
BOOL DisplayScrollReport (LPSTR Name, LPSTR Prefix, LPSTR UDI, long ref);
BOOL DisplayReportScroll (HWND hWndDlg, int ScrollCntl);
short GetLBSelectedItems (HWND hWndDlg,UINT controlid,LPHANDLE phItems);
BOOL EditDynamicDialog (HWND hWndDlg,LPSTR Name, LPSTR SQL, LPSTR InsertString);
BOOL AppendFile (LPSTR File,LPSTR Line);
void AddToMinMaxD (LPMNMXCORD mm1, LPDPOINT pPoint); 
BOOL BoundsInBounds (LPMNMXCORD pBounds1,LPMNMXCORD pBounds2,short Opt);
BOOL PointInBounds (DPOINT Point,LPMNMXCORD pBounds);
void DBoundsInit (LPMNMXCORD lpRect);           
void AdjustBounds (LPMNMXCORD Bounds, double AdjustX, double AdjustY);
void ExpandBounds (LPMNMXCORD Bounds, double Adjust);
void ExpandMinMax (LPMINMAX Bounds, short Adjust);
void ExpandMinMaxL (LPMNMXCORL Bounds, long Adjust);
void TranBounds (HANDLE TranID,LPMNMXCORD pBounds);
void DebugShowLine (LPDPOINT p1,LPDPOINT p2);
double MinAngleToTheRight (double AZ1,double AZ2);
short GetValListFieldLen (LPFIELDINFO pField);
BOOL CreateStatusWindow (HWND hWnd,short nStatusBars,LPSTR Title);
void StatusWindowUpdate (LPSTR Title, LPSTR Mess, DWORD Tot, DWORD Done);
void StatusWindowUpdate2 (LPSTR Mess, DWORD Tot, DWORD Done);
BOOL DestroyStatusWindow (void);
double TranAngle (HANDLE TranID,LPMNMXCORD pBounds);
double TranScale (HANDLE TranID,LPMNMXCORD pBounds);
void AddMinMax (LPMINMAX mm1, LPMINMAX mm2);
void AddMinMaxD (LPMNMXCORD mm1, LPMNMXCORD mm2);  
BOOL ValidBounds (LPMNMXCORD Bounds);
void DdeBye(void);
BOOL InitDdeStuff(HWND  In_handle, BOOL Highways);
void SetIntRefno (long Ref); 
void BufWrite (HPSTR *pBuf,LPLONG plBuf,HPSTR data,long ldata);
BOOL BackupFiles (LPSTR FileList,LPSTR BUDir);
void GSSiDeleteObject (HGDIOBJ *handle);  
short loadtabs (LPSHORT Tabs);
short GetDriveNum (char DriveLetter);
BOOL CreatePickData (void);
double ComputeAreaAreaD (HPDPOINT lpDPoints,long nPnts,LPDOUBLE pPerim);
double ComputeAreaAreaDH (HANDLE hPoints,long nPnts,LPDOUBLE pPerim);
DPOINT ComputeAreaMidpoint (HANDLE hPoints,long nPnts);
BOOL GetVolumeLabel(WORD wDrive, LPSTR lpBuff);
double	GetDriveFreeSpace (char Drive);
BOOL LoadLibraries(HINSTANCE hInst);
void FreeLibraries(void);
USHORT GetSQLType(LPSTR pstr,LPSTR lpBrack,int NumFields,LPFIELDINFO lpFldInfo,LPDWORD pPrecision,LPUSHORT pScale);
BOOL AddPWtoODBCFile (LPSTR lpcstring);
void CloseGCTPLibrary32(void);
HANDLE CopyDib (HANDLE hdib);
BOOL ReverseBOOL (BOOL In);
void hmemmove (HPSTR out,HPSTR in,DWORD len);
void hmemset (HPSTR out,char in,DWORD len);
BOOL SamePoint (POINT p1, POINT p2);
BOOL SameDPoint (LPDPOINT p1, LPDPOINT p2);
int	GetFunctionID (LPSTR str, LPSTR ParenLoc);
int	GetFunctionValue (int FunID,LPSTR Args, LPSTR OutLoc);
int	GetFunctionValue2 (int FunID,LPSTR Args, LPSTR OutLoc);
int	GetFunctionValue3 (int FunID,LPSTR Args, LPSTR OutLoc);
COLORREF ColorWOWidth (COLORREF InColor);
double FltAP (LPSTR INEXPR,LPSHORT IRC);
BOOL LogicP (LPSTR INEXPR,LPSHORT IRC);
void ConvertSQLToLogicP (LPSTR pLogicP,LPSTR pSQL);
double fltread (LPSTR Instr, LPSHORT IRC);
double ComputeFLTValue (double val1, double val2, char OpCode,LPSHORT IRC);
BOOL ComputeLogValue (LPSTR val1, LPSTR val2, UINT OpCode,LPSHORT IRC);
UINT GetBoolOpCode (LPSTR Input,LPSHORT pInc,LPSHORT pNeedArg1,LPSHORT pError);
short OpRankBool (UINT OpCode);
short DupDLLs (void);                            
HWND SendDDEInitiate(LPSTR szApplication, LPSTR szTopic);
void SendExecute(HWND hwndClientDDE, HWND hwndServerDDE,LPSTR szExecuteString);
void ClientTerminate(HWND hwndClientDDE,HWND hwndServerDDE);
void TerminateConversations(HWND hWndClient);
void SetPrompt (UINT PromptID,BOOL SetVP);
void SetPromptDlg (UINT PromptID);
POINT MinMaxMidPoint (LPMINMAX pBounds);
DPOINT MinMaxMidPointD (LPMNMXCORD pBounds);
DPOINT SubtractPoint (LPDPOINT pPoint1,LPDPOINT pPoint2);
int	SaveCurView (int opt);
short GSSiMessageBox (LPSTR Mess,LPSTR Title,UINT icon);
DPOINT EnlargedPoint (POINT Point);
BOOL EnlargeScreen (short factor,short width); 
BOOL atob (LPSTR Value); 
void btoa (BOOL Val,LPSTR str);
DPOINT atopt (LPSTR Value,LPSHORT pErr);
MNMXCORD atobounds (LPSTR Value,LPSHORT pErr);
RECT atorect (LPSTR Value,LPSHORT pErr); 
void boundstoa (LPSTR Value,LPMNMXCORD Bounds);
void recttoa (LPSTR Value,RECT Rect);
void dpointtoa (LPSTR Value,LPDPOINT pPoint);
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
BOOL LININT (double L1X1, double L1Y1, double L1X2, double L1Y2,
			 double L2X1, double L2Y1, double L2X2, double L2Y2);
void DumpDCB (int Stream);
void SIOOpenError (LPSTR Port,int Error);
HANDLE ReadSavedScreen (LPSTR File,LPLONG pUpdateTime);
BOOL WriteSavedScreen (LPSTR File,HANDLE hSavedScreen); 
void Draw3DBorder(HDC hDC, LPRECT pRect,int iStyle, BOOL DoubleWidth);
BOOL FAR PASCAL EDITDYNDIALOGMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL LINFIT (HPDPOINT pPoint,LONG N,LPDOUBLE A,LPDOUBLE B,LPDOUBLE MAXDIF,LPLONG LOFMDF);
BOOL GetCDDriveForVol (LPSTR VolID,LPSTR Drive);
BOOL ConvertNameToCDName (LPSTR Name);     
BOOL CDIsInstalled (LPSTR CDName);
void OpenCDLookUpTable (void);
void CloseCDLookUpTable (void);
HANDLE	EnterBlockingWindow (HWND hWndDlg);
void LeaveBlockingWindow(HANDLE hSavedScreen);
void GSSiEndDialog(HWND hWndDlg,BOOL rtn,HANDLE hSaveBM);  
void ClearFullWindowBitmap (void);
short GetNumOpenFiles (void);
short GetOpenFileID (HFILE Fid);
short GenList (HWND hWndDlg,UINT CntlID,BOOL ComboBox,long Loc,LPSTR str,LPSTR CurrentValue,LPSHORT pUseBAR);
BOOL GetGenListVal (LPSTR GenListString,WORD Inc,LPSTR OutString);
void ClearCurrentCD (void);
BOOL CDInCancelledList (LPSTR CD);
void AddCDToCancelledList (LPSTR CD);
short OkToContinue (BOOL ForceCheck);
BOOL WindowIsCovered (HWND hWnd,short opt);
double GetDriveSize (char Drive);
long GetProcessorID (void);
BOOL NormalRect (LPRECT Rect); 
BOOL GetOpenFilePathname (HFILE Fid,LPSTR Name); 
BOOL MinimizeWindowIfOverMain (HWND hWnd);
double BoundsArea (LPMNMXCORD pBounds);
BOOL WriteErrorHandler (HFILE Fid,long ln,long loc);  
grp4decomp(HPBYTE indata,long inbytes,long width,long height,HPBYTE outdata,LPLONG outbytes,short SkipBits);
int SetLastMessage (long mes);
void ShowTextTrace (LPSTR str);
double ColorDist (COLORREF C1, COLORREF C2);
double RGBDist (RGBTRIPLE C1, RGBTRIPLE C2);
COLORREF NearestColor (COLORREF Color, COLORREF C1, COLORREF C2);
BOOL RectanglesAreEqual (LPRECT pRect1,LPRECT pRect2);
BOOL TABToComma (LPSTR InFile,LPSTR OutFile);  
LONG GSSillseek (HFILE Fid, LONG loc, int opt); 
DWORD GSSillseek2 (HFILE Fid, DWORD loc, int opt);
long  GSSilread(HFILE Fid, void _huge* ptr, long len);
UINT  WINAPI GSSilwrite(HFILE Fid, const void _huge* ptr, UINT len);
HFILE GSSiOpenFileMem (LPSTR InName); 
HFILE SetMemFile (HANDLE handle,long len);
double LDIST(double X1,double Y1,double X2,double Y2);
HANDLE GetBMPFromCache (LPSTR Name);
void AddBMPToCache (LPSTR Name,HANDLE hBMP);
BOOL hDIBInCache (HANDLE hBMP);
HDIB32 GetBMPFromCache32 (LPSTR Name);
BOOL hDIBInCache32 (HDIB32 hBMP);
void AddBMPToCache32 (LPSTR Name,HDIB32 hBMP);
short CacheAlreadyChecked (LPSTR Name,short lCacheDir,UINT Mode);
void ResetOriginalDrive (void);
long ConvertRectToArea (LPHANDLE phPoints);
HANDLE TransformBitmap (HANDLE hBMP,HANDLE hTran,HANDLE hTranBack,LPMNMXCORD pBounds, LPDOUBLE pRes,LPSTR Namelong, long nAreaPts,HANDLE hAreaPts,BOOL DisplayStatus);
void RectToPoints (LPRECT Rect,LPPOINT Points);
void RectToDPoints (LPRECT Rect,LPDPOINT Points);
int GSSiGetTempFileName (BYTE Drive,LPCSTR Pre,UINT Unique,LPSTR Name);
BOOL GetTempDir (LPSTR Dir);
short CompressCensusString (LPSTR pStr);
short ExpandCensusString (LPSTR pStr);
long CompressBinaryRecord (HPSTR pDecompressedRec,HPSTR pCompressedRec,long DecompressedLen);
long DecompressBinaryRecord (HPSTR pDecompressedRec,HPSTR pCompressedRec,long CompressedLen);		
void FileWillBeCreated (LPSTR Name);
void FileOpenForUpdate (LPSTR Name,HFILE Fid);
void SaveDataToUndoFile (short Type,short UndoFileID, long len, HPSTR pData,long SeekLoc);
BOOL CreateCheckPoint (LPSTR Name);
void RemoveCheckPoint (void);
long GetSizeAtLastCheckpoint (short UndoFileID);
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
void AdjustDIBColors (HANDLE hDib);
BOOL CopyDib2 (HANDLE hdib,LPHANDLE phDibInfo, LPHANDLE phImage);
void BiasPoly (HANDLE hDPoints,long np,double xbias,double ybias);
void GetPolyBounds (HANDLE hPoly,long np,LPMNMXCORD pBounds);
BOOL GetFile32 (short Opt,LPOPENFILENAME pof,DWORD lFilters);
BOOL GetShortPathName (LPSTR Name,short MaxLen);
BOOL GetLongPathName (LPSTR Name,short MaxLen);
BOOL LoadGM32Lib (BOOL Close);
BOOL GSSiGetFileName (short opt,HWND hWnd,HINSTANCE hInstance,LPSTR PathName,LPCSTR Filters,DWORD lFilters,LPCSTR InitialDir,LPCSTR Title,DWORD Flags);
DWORD CreateLongNameFile (LPSTR Name);
void CheckOpenFiles (void);   
BOOL GSSiPrintDlg (LPPRINTDLG p);
long GSSiGetDeviceCaps(HDC hPr, int opt); 
long StretchDIBits32 (HDC hDC,long destX,long destY,long destW,long destH,long xoff,long yoff,
					  LPBITMAPINFOHEADER pDibInfo, LPBYTE pImage, LPDOUBLE pFactor);
long StretchDIBitsFromHandle (HDC hDC,long destX,long destY,long destW,long destH,long source_xoff,long source_yoff,long sourcew,long sourceh,
					 		  HDIB32 hDib, double Factor);
HDIB32 FAR LoadDIB32(LPSTR lpFileName);
BOOL DestroyDIB32 (HDIB32 hDib,BOOL Force);
BOOL GetBitmapInfoFromHandle (LPBITMAPINFOHEADER pDibInfo,HDIB32 hDib);
HDIB32  BMPHandleFromEXT (LPSTR ImageFile); 
long DecompressBinaryRecord (HPSTR pDecompressedRec,HPSTR pCompressedRec,long CompressedLen);
long CompressBinaryRecord (HPSTR pDecompressedRec,HPSTR pCompressedRec,long DecompressedLen);
BOOL PickNearestCPTFile (DPOINT Point,LPSTR Filelist,LPSTR IgnoreName,double MaxDist,LPSTR OutLoc);
DWORD SearchDirectory32 (LPSTR Name,DWORD UseHandle,LPDWORD pType);
DWORD GetSpecialDirectory (LPSTR Name);
BOOL GSSiGetNodeInfo (LPSTR CDriveSerno,LPSTR NodeName,LPSTR Winver);
BOOL SizeDlgToRect (HWND hWndDlg,LPRECT pRect);
BOOL GM32NADCONINIT (LPSTR InitFile);
BOOL GM32NADCON (LPDPOINT Point,DWORD Dir);
long NumRowsInTxtFile (HFILE Fid);
BOOL CompressFrame (LPBITMAPINFOHEADER	lpbiIn, LPBITMAPINFOHEADER lpbiOut);
BOOL CompressFrameEX (LPBITMAPINFOHEADER	lpbiIn, LPBITMAPINFOHEADER lpbiOut, long Quality);
BOOL SetupVirtualPrinter (HWND hWnd,LPSTR VPName);
int GSSiMsgBox (HWND hWnd, LPCSTR Mess, LPCSTR Title, UINT Flag);
BOOL GetPerpendicularIntersect (LPDPOINT Point,LPDPOINT Line,LPDPOINT IntPoint);
BOOL GetPerpendicularIntersect2 (LPDPOINT Point,LPDPOINT LinePt,LPDOUBLE pLineAZ,LPDPOINT IntPoint);










