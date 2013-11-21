#include "gmlimits.h"
extern LPSTR MapCopyProjection; 
extern LPSTR PickedOrthoName;
extern LPSTR CurrentSymName;
extern LPSTR CurrentPrefix;
extern LPSTR CurrentUDI;
extern LPSTR NewDescName;
extern LPSTR CurEditRectInfo;  
extern LPSTR CurHelpTopic;
extern LPSTR gszFilter;
extern LPSTR gszBuffer;
extern LPSTR curproject;
extern LPSTR curunits;  
extern LPSTR HLTOutSQL;
extern LPSTR UGridPickMacro;
extern LPSTR HardDrive;
extern LPSTR HardDrive2;
extern LPSTR MIFOutSQL;
extern LPSTR SymStuff;
extern LPSTR CurSymColor;
extern LPSTR CurSymName;
extern LPSTR CurParName;                          
extern LPSTR CurSymSize;
extern LPSTR CurSymRot;   

//directories
extern LPSTR MapCopyPath;
extern LPSTR CurDocDir;
extern LPSTR CurDir;  
extern LPSTR OriginalDir; 
extern LPSTR StartupDir;  
extern LPSTR NetworkDir;
extern LPSTR AddMatchDir;

//pathnames
extern LPSTR PltName;
extern LPSTR PickName;  
extern LPSTR PickDirectory;
extern LPSTR CfgName;
extern LPSTR Lev0CfgName;
extern LPSTR AutoExportName;  
extern LPSTR AutoMapIndexName;
extern LPSTR PMDataFile;
extern LPSTR PMMacroFile; 
extern LPSTR FullBM;  
extern LPSTR EditName;
extern LPSTR ThemeDB; 
extern LPSTR TagFile;
extern LPSTR CurODBCFile;
extern LPSTR MemMapName;
extern LPSTR HLTOutDataFile;
extern LPSTR HLTOutPath;
extern LPSTR HLTOutPathScreen;
extern LPSTR DataFile;
extern LPSTR MIFOutDataFile;
extern LPSTR LoadName;
extern LPSTR IMDataFile; 
extern LPSTR DestName;        
extern LPSTR NoCache;  
extern LPSTR UserName;
extern LPSTR NodeName;  
extern LPSTR MenuCFGName;
extern LPSTR AccessCode;
extern LPSTR SerialNumber;  
extern LPSTR CurMovie;
extern LPSTR IconDict;
extern LPSTR AttImportDataFile;
extern LPSTR MaskAreaFile;
extern LPSTR GMIni;



extern	char FontNames[MAXFONTS][lnFontNames];  
extern	COLORREF	FontColors[MAXFONTS]; 
extern	double	FontWidthFactor[MAXFONTS];

extern	DPOINT	UserPoints[33];
extern	DPOINT	UserBounds[2];  
extern	short	NextCFGTAG[MAXCFGTAGS];
extern	RECT	ConfigRect[MAXCFGTAGS];
extern	HANDLE	hSavedConfig[MAXCFGTAGS];  

extern	LPHPEN		pens;
extern	LPHBRUSH	brushes;      

extern	LPDOUBLE	PossibleAz, PossibleMP, PossiblePCT, PossibleLength; 
extern	LPSHORT		PossibleDir; 
extern	LPLONG		PossibleTLID;  

