#define PROJECTION_INCLUDE

#include "graphint.h"
#include <sys\stat.h>         
struct _stati64	OpenConfigStat;    
HBITMAP hPatBMP[5] = { 0 };
HFILE	OpenFileFid[MAXFILEHANDLES]; 
UINT	OpenFileCallID[MAXFILEHANDLES];
short	OpenFileUndoFileID[MAXFILEHANDLES];
HFILE	UndoFileFid[MAXUNDOFILES];
HFILE	JournalFileFid[MAXFILEHANDLES];
int		FidMemLen[MAXFILEHANDLES];
LONGLONG	OriginalFileLength[MAXFILEHANDLES];
LONGLONG	OpenFileLength[MAXFILEHANDLES];
HANDLE	JournalFileIndex[MAXFILEHANDLES];
BYTE	JournalIsCompleteFile[MAXFILEHANDLES];
HANDLE	hFilesWithJournals=0;
int		lFilesWithJournals=0;
HANDLE	fidLogFileUse = (HANDLE)HFILE_ERROR;
short	HltFetchPos=BT_FIRST;
short	HltFetchSort=0;
double	CurrentAZ=HALFPI;
short	RouteType=TWOPOINTROUTE;
#undef PROJECTION_INCLUDE

