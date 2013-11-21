#define PROJECTION_INCLUDE

#include "graphint.h"
#include <sys\stat.h>         
struct	_stat	OpenConfigStat;    
UINT	PatBMP[5]={IDB_MONO4,IDB_MONO1,IDB_MONO2,IDB_MONO3,IDB_MONO5};
HFILE	OpenFileFid[MAXFILEHANDLES]; 
short	OpenFileUndoFileID[MAXFILEHANDLES];
HFILE	UndoFileFid[MAXUNDOFILES];
short	HltFetchPos=BT_FIRST;
double	CurrentAZ=HALFPI;
short	RouteType=TWOPOINTROUTE;
#undef PROJECTION_INCLUDE

