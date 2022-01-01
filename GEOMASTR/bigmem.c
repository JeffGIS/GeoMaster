#include <windows.h>   
#include <limits.h>  
#include <float.h>   
#include <string.h>
#include "gssitype.h"
#include "gmlimits.h"
#include "bigmemln.h"   
//#include "toolhelp.h"
HGLOBAL GSSiGlobAlloc (USHORT From,UINT fuAlloc, long cbAlloc);
void GSSiGlobUlFree (LPHANDLE pHandle);
#include "gmvardef.h"

LPSTR MapCopyProjection; 
LPSTR PickedOrthoName;
LPSTR CurrentSymName;
LPSTR CurrentPrefix;
LPSTR CurrentUDI;
LPSTR NewDescName;
LPSTR UnusedCurEditRectInfo;  
LPSTR CurHelpTopic;
LPSTR gszFilter;
LPSTR gszBuffer;
LPSTR curproject;
LPSTR curunits;  
LPSTR HLTOutSQL;
LPSTR UGridPickMacro;
LPSTR HardDrive;
LPSTR HardDrive2;
LPSTR MIFOutSQL;
LPSTR SymStuff;
LPSTR CurSymColor=0;
LPSTR CurSymName;
LPSTR CurParName;                          
LPSTR CurSymSize=0;
LPSTR CurSymRot=0;    
LPSTR IMDataFile; 
LPSTR DestName; 
LPSTR NoCache;  
LPSTR UserName;
LPSTR NodeName;

//directories
LPSTR MapCopyPath;
LPSTR CurDocDir;
LPSTR CurDir;  
LPSTR OriginalDir;
LPSTR StartupDir;
LPSTR NetworkDir;
LPSTR AddMatchDir;

//pathnames
LPSTR PltName;
LPSTR PickName;
LPSTR PickDirectory;
LPSTR CfgName;
LPSTR Lev0CfgName;
LPSTR AutoExportName;  
LPSTR AutoMapIndexName;
LPSTR PMDataFile;
LPSTR PMMacroFile; 
LPSTR FullBM;  
LPSTR TagFile;   
LPSTR EditName;
LPSTR ThemeDB; 
LPSTR CurODBCFile;
LPSTR MemMapName;
LPSTR HLTOutDataFile;
LPSTR HLTOutPath;
LPSTR HLTOutPathScreen;
LPSTR DataFile;
LPSTR MIFOutDataFile;
LPSTR LoadName;  
LPSTR MenuCFGName;  
LPSTR AccessCode;
LPSTR SerialNumber;
LPSTR CurMovie;
LPSTR IconDict;
LPSTR AttImportDataFile;
LPSTR MaskAreaFile;
LPSTR GMIni;


//LPSTR FontNames; 
//LPDOUBLE	FontWidthFactor;   

LPDOUBLE	PossibleAz, PossibleMP, PossiblePCT, PossibleLength; 
LPSHORT		PossibleDir; 
LPLONG		PossibleTLID;  

char FontNames[MAXFONTS][lnFontNames]; 
COLORREF	FontColors[MAXFONTS];
double	FontWidthFactor[MAXFONTS];
DPOINT	UserPoints[33];
DPOINT	UserBounds[2];  
short	NextCFGTAG[MAXCFGTAGS];
RECT	ConfigRect[MAXCFGTAGS];
HANDLE	hSavedConfig[MAXCFGTAGS];  
LPORTHO	CurOrtho, OrthoBuffers[MAXORTHOBUFS];

LPHPEN		pens;
LPHBRUSH	brushes;
 


HPSTR	pCommonMem; 
long	lCommonMem=((long)USHRT_MAX)*8L;
LPTHEME	CurTheme=0;  
LPVIEWPORT  CurView=0;
LPVIEWPORT	FAR	*pViewports,	*pViewportsArray[2];
LPVIEWPORT	FAR	*pViewportsD,	*pViewportsDArray[2];
LPHANDLE hViewports,		hViewportsArray[2];  
short	CurrentConfig=0;
short	NumViewportsArray[2]={0,0};  
short	CommandViewportArray[2]={0,0};
LPSHORT	pNumViewports, pCommandViewport;  
RECT	ConfigDisplayRect={0,0,0,0};

char	AppName[16];
char	szAppName[20];
char    AddPrefix[4][10] = { 0 };
char	AddUDI[34];
char	AddUDIVar[32];  
char	CurrentOrthoOrigName[MAX_PATH];
char	TagLocPrefix[10];   
char	TagLocViewport[32]="";
char	TagLocLayer[32]="";
char	CurrentStreetName[34];
short	TXPntrSym[10];
char	LastFunctionName[24];
char	IncludeFileArg[32]="";    
char	ConvertTextFrom[16]="";  
//char	DLLDir[64]="";
char	ConvertTextTo[16];
char	ImageExtension[6]=".gci";
char	AreaUnitOpts[6][10]={"SQRFEET","SQRMETERS","SQRYARDS","SQRMILES","SQRKILOS","ACRES"};
char	DistUnitOpts[6][12]={"FEET","METERS","YARDS","MILES","KILOMETERS","SYSTEM"};
char	DynDlgName[10];
char	NewTAG[80];

char	CurSymbolFont[4][64]={"","","",""};
PICKDATA	PickList[MAXPICKITEMS+3];


static	HANDLE	hBigMem1=0, hBigMem2=0, hBigMem3=0, hBigMem4=0, hBigMem5=0, hBigMem6=0, hBigMem7=0;   

int SetConfig (int in)
{   
	short	ii;
	int	n = abs (in);
	
	if (InDisplayProcessing)
		ii = 1;
	if (!n)
		ii=1;
	else if (n!=1)
		n=1;
	pViewports = pViewportsArray[n];
	pViewportsD = pViewportsDArray[n];
	hViewports = hViewportsArray[n]; 
	pNumViewports = &NumViewportsArray[n];  
	pCommandViewport = &CommandViewportArray[n]; 
	if (*pNumViewports && *pCommandViewport && in >= 0)
		CurView = pViewports[*pCommandViewport-1];
	CurrentConfig = n;
	return *pNumViewports;
}

void CreateBigMem (void)
{   
    short	i;
	long memlen =     lnMapCopyProjection
					+ lnPickedOrthoName
					+ lnCurrentSymName
					+ lnCurrentPrefix
					+ lnCurrentUDI
					+ lnNewDescName
					+ lnCurEditRectInfo  
					+ lnCurHelpTopic
					+ lngszFilter
					+ lngszBuffer
					+ lncurproject
					+ lncurunits  
					+ lnHLTOutSQL
					+ lnUGridPickMacro
					+ lnHardDrive
					+ lnHardDrive2
					+ lnMIFOutSQL
					+ lnSymStuff
					+ lnCurSymColor
					+ lnCurSymName
					+ lnCurParName                          
					+ lnCurSymSize
					+ lnCurSymRot
					+ lnIMDataFile
					+ lnDestName   
					+ lnNoCache
					+ lnUserName
					+ lnNodeName 
					+ lnAccessCode
					+ lnSerialNumber;
				//	+ MAXFONTS * lnFontNames
				//	+ MAXFONTS * sizeof(double);  

//common memory
	hNulls = GSSiGlobAlloc(0, GHND, 128);
	hBigMem7 = GSSiGlobAlloc (1302, GHND,lCommonMem+32);
	pCommonMem = GlobalLock (hBigMem7);

	hBigMem1 = GSSiGlobAlloc (1297,GHND,memlen);
	
	MapCopyProjection = GlobalLock (hBigMem1);
	PickedOrthoName = MapCopyProjection + lnMapCopyProjection;
	CurrentSymName = PickedOrthoName + lnPickedOrthoName;
	CurrentPrefix = CurrentSymName + lnCurrentSymName;
	CurrentUDI = CurrentPrefix + lnCurrentPrefix;
	NewDescName = CurrentUDI + lnCurrentUDI;
	UnusedCurEditRectInfo = NewDescName + lnNewDescName;  
	CurHelpTopic= UnusedCurEditRectInfo + lnCurEditRectInfo;
	gszFilter = CurHelpTopic + lnCurHelpTopic;
	gszBuffer = gszFilter + lngszFilter;
	curproject = gszBuffer + lngszBuffer;
	curunits = curproject + lncurproject;  
	HLTOutSQL = curunits + lncurunits;
	UGridPickMacro = HLTOutSQL + lnHLTOutSQL;
	HardDrive = UGridPickMacro + lnUGridPickMacro;
	HardDrive2 = HardDrive + lnHardDrive;
	MIFOutSQL = HardDrive2 + lnHardDrive2;
	SymStuff = MIFOutSQL + lnMIFOutSQL;
	CurSymColor = SymStuff + lnSymStuff;
	CurSymName = CurSymColor + lnCurSymColor;
	CurParName = CurSymName + lnCurSymName;                          
	CurSymSize = CurParName + lnCurParName;
	CurSymRot = CurSymSize + lnCurSymSize;  
	IMDataFile = CurSymRot + lnCurSymRot; 
	DestName = IMDataFile + lnIMDataFile;   
	NoCache = DestName + lnDestName;   
	UserName = NoCache + lnNoCache;
	NodeName = UserName + lnUserName;  
	AccessCode = NodeName + lnNodeName;
	SerialNumber = AccessCode + lnAccessCode;
//	FontNames = DestName + lnDestName;  
//	FontWidthFactor = (LPDOUBLE)(FontNames + MAXFONTS * lnFontNames);
//	FontWidthFactor = (LPDOUBLE)(SerialNumber + lnSerialNumber);
    
    for (i=0;i<MAXFONTS;i++)
    {
    	_fstrcpy (FontNames[i],"Arial");
    	FontColors[i] = 0;  
    	FontWidthFactor[i]=0;
    }
    _fstrcpy (curproject,"BASEPROJ.CVT");
    _fstrcpy (curunits,"FEET");
//directories 
	memlen = MAX_PATH * 7;  
	hBigMem2 = GSSiGlobAlloc (1298,GHND,memlen);

	MapCopyPath = GlobalLock (hBigMem2);
 	CurDocDir = MapCopyPath + MAX_PATH;
 	CurDir = CurDocDir + MAX_PATH;  
 	OriginalDir = CurDir + MAX_PATH;  
 	StartupDir = OriginalDir + MAX_PATH;
	NetworkDir = StartupDir + MAX_PATH;
	AddMatchDir = NetworkDir + MAX_PATH;

//pathnames
	memlen = MAX_PATH * 29;   
	hBigMem3 = GSSiGlobAlloc (1299,GHND,memlen);
 	PltName = GlobalLock (hBigMem3);
	PickName = PltName + MAX_PATH;
	PickDirectory = PickName + MAX_PATH;
	CfgName = PickDirectory + MAX_PATH;
	Lev0CfgName = CfgName + MAX_PATH*2;
	AutoExportName = Lev0CfgName + MAX_PATH;
	AutoMapIndexName = AutoExportName + MAX_PATH;
	PMDataFile = AutoMapIndexName + MAX_PATH;
	PMMacroFile = PMDataFile + MAX_PATH; 
	FullBM = PMMacroFile + MAX_PATH;  
	TagFile = FullBM + MAX_PATH;   
	EditName = TagFile + MAX_PATH;
	ThemeDB = EditName + MAX_PATH;; 
	CurODBCFile = ThemeDB + MAX_PATH;
	MemMapName = CurODBCFile + MAX_PATH;
	HLTOutDataFile = MemMapName + MAX_PATH;
	HLTOutPath = HLTOutDataFile + MAX_PATH;
	HLTOutPathScreen = HLTOutPath + MAX_PATH;
	DataFile = HLTOutPathScreen + MAX_PATH;
	MIFOutDataFile = DataFile + MAX_PATH;
	LoadName = MIFOutDataFile + MAX_PATH; 
	MenuCFGName = LoadName + MAX_PATH;
	CurMovie = MenuCFGName + MAX_PATH;
	IconDict = CurMovie + MAX_PATH;
	AttImportDataFile = IconDict + MAX_PATH;
	MaskAreaFile = AttImportDataFile + MAX_PATH;
	GMIni = MaskAreaFile + MAX_PATH;


	memlen = MAXPENS * (sizeof(HPEN) + sizeof(HBRUSH));  
	hBigMem4 = GSSiGlobAlloc (1300,GHND,memlen);     
	pens = (LPHPEN)GlobalLock (hBigMem4);
	brushes = (LPHBRUSH) (pens + MAXPENS);
    
//viewport stuff
	memlen = MAX_VIEWPORTS * 2 * (sizeof (LPVOID) + sizeof (LPVOID) + sizeof (LPHANDLE));
	hBigMem5 = GSSiGlobAlloc (1301,GHND,memlen);
	pViewportsArray[0] = (LPVOID)GlobalLock (hBigMem5);
	pViewportsArray[1] = pViewportsArray[0] + MAX_VIEWPORTS;
	pViewportsDArray[0] = pViewportsArray[1] + MAX_VIEWPORTS;
	pViewportsDArray[1] = pViewportsDArray[0] + MAX_VIEWPORTS;
	hViewportsArray[0] = (LPHANDLE) (pViewportsDArray[1] + MAX_VIEWPORTS);
	hViewportsArray[1] = hViewportsArray[0] + MAX_VIEWPORTS;  
	
	memlen = sizeof(double) * 4 * 10 + sizeof(short) * 10 + sizeof(long) * 10;
	hBigMem6 = GSSiGlobAlloc (1302,GHND,memlen);
	PossibleAz = (LPDOUBLE)GlobalLock (hBigMem6);
	PossibleMP = PossibleAz + 10;
	PossiblePCT = PossibleMP + 10;
	PossibleLength = PossiblePCT + 10; 
	PossibleDir = (LPSHORT) (PossibleLength + 10); 
	PossibleTLID = (LPLONG) (PossibleDir + 10);  

	_fmemset (hSavedConfig,0,sizeof(hSavedConfig));
	SetConfig (1);

	return;
}

void FreeBigMem (void)
{   
	GSSiGlobUlFree (&hBigMem1);
	GSSiGlobUlFree (&hBigMem2);
	GSSiGlobUlFree (&hBigMem3);
	GSSiGlobUlFree (&hBigMem4); 
	GSSiGlobUlFree (&hBigMem5); 
	GSSiGlobUlFree (&hBigMem6); 
	GSSiGlobUlFree(&hBigMem7);
	GSSiGlobUlFree(&hNulls);
	return;
}

