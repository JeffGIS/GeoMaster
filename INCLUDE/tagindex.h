#pragma once

#define TAGINDEX_BTREE	1
#define TAGINDEX_SHP	2

typedef struct {
	int type;
	HANDLE hBT;
	sqlite3 *sltdb;
	char TAGPrefix[16];
	char TAGFile[MAX_PATH];
}TAGINDEX;
typedef TAGINDEX *LPTAGINDEX;

BOOL OpenTAGIndex(BOOL Delete, BOOL StoreBounds, LPSTR ReopenName);
void CloseTAGIndex(void);
