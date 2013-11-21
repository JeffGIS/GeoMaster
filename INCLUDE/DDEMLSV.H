#include <windows.h>
#include <ddeml.h>


#define IDS_BADLENGTH   1

#define MAX_APPNAME     64
#define MAX_TOPIC       64
#define MAX_COMMENT     100
#define MAX_EXEC        40
#define CCHARS          80
#define CLINES          40

#define UM_CHGDATA      (WM_USER + 435)

#define CSYSTEMITEMS 4
#define CTESTITEMS  4
#define CTOPICS     2

//#include "dialog.h"

/****** Structrues ******/

// predefined format list item

typedef struct 
{
    ATOM atom;
    LPSTR sz;
} FORMATINFO;

#define CFORMATS 1


// this structure depends on the fact that EXPENTRY is defined as
// pascal calling convention.  This allows us to merely point to
// the last parameter of DdeCallback and cast the pointer to this
// structure to save stack space. (a common hack)

typedef struct _XFERINFO {
    DWORD lData2;
    DWORD lData1;
    HDDEDATA hData;
    HSZ hszItem;
    HSZ hszTopic;
    HCONV hConv;
    WORD wFmt;
    WORD wType;
} XFERINFO;
typedef XFERINFO *PXFERINFO;    // DS==SS!

typedef struct _ITEMLIST {
    HSZ hszItem;
    HDDEDATA (*npfnCallback)(PXFERINFO, WORD);
    LPSTR pszItem;
} ITEMLIST;

typedef struct _TOPICLIST {
    HSZ hszTopic;
    ITEMLIST *pItemList;
    WORD cItems;
    LPSTR pszTopic;
} TOPICLIST;

typedef struct {    // used to passinfo to/from TextDataEntryDlgProc and
    HDDEDATA hDdeData;
    WORD wFmt;
    HSZ hszItem;
} XACT;

HDDEDATA EXPENTRY DdeCallback(WORD wType, WORD wFmt, HCONV hConv, HSZ hszTopic,
        HSZ hszItem, HDDEDATA hData, DWORD lData1, DWORD lData2);
HDDEDATA TopicListXfer(PXFERINFO pXferInfo, WORD iFmt);
HDDEDATA ItemListXfer(PXFERINFO pXferInfo, WORD iFmt);
HDDEDATA sysFormatsXfer(PXFERINFO pXferInfo, WORD iFmt);
HDDEDATA GWIZXfer(PXFERINFO pXferInfo, WORD iFmt);
HDDEDATA VideoXfer(PXFERINFO pXferInfo, WORD iFmt);
HDDEDATA HelpXfer(PXFERINFO pXferInfo, WORD iFmt);
void Hszize(void);
void UnHszize(void);
void Delaydde(DWORD delay, BOOL fModal);


