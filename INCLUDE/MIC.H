#include "MIC2.H"
#define APPLICATION_TITLE		"MIC System II Video Window"
#define APPLICATION_CLASS		"SimpleVideoWndClass"
#ifdef MIC_VARIABLES
DWORD	MICId;
/*char		szMICCmdBuf[256];*/ 
char		str[256];
char		szMICRspBuf[256];
WORD		xVideoWin;
WORD		yVideoWin;
WORD		cxVideoWin;
WORD		cyVideoWin;
WORD		cxScreen;
WORD		cyScreen;
#else
extern  DWORD	    MICId;
extern  char		str[256];
extern  char		szMICRspBuf[256];
extern  WORD		xVideoWin;
extern  WORD		yVideoWin;
extern  WORD		cxVideoWin;
extern  WORD		cyVideoWin;
extern  WORD		cxScreen;
extern  WORD		cyScreen;
#endif
BOOL DeInitApplication(HANDLE hInstance);
VOID DeInitInstance(VOID);

DWORD __export CALLBACK GVInit(void);
DWORD __export CALLBACK GVConfirm(char FAR *rtlid, long rjpeg, 
                                  long rview, long FAR *rframe);
DWORD __export CALLBACK GVFind(char FAR *tlid, long FAR *vtframe, long mode, 
          long FAR *view, char FAR *drive, char FAR *runid, char FAR *file,
          long FAR *jpegframe); 
DWORD __export CALLBACK GVClose(void);

/****************************************************************************/
/*      Local defines.                                                      */
/****************************************************************************/              
#define MYLOGPALSIZE	16

/****************************************************************************/
/*      Local Variables.													*/
/****************************************************************************/              
//static HANDLE	hInst;

/****************************************************************************/
/*      Local Procedure Definitions.                                        */
/****************************************************************************/
BOOL AssignDVA(VOID);
VOID DeassignDVA(VOID);
VOID InitVideoWindow(HWND, WORD);
VOID SizeVideoWindow(WORD, WORD);
VOID MoveVideoWindow(WORD, WORD);
VOID DeInitVideoWindow(VOID);
VOID PlaySampleVideo();

