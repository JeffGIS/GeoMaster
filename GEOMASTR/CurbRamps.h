#ifndef CurbRamps_h
#define CurbRamps_h

#include "RampCompliance.h"
#include "MPIntersection.h"

#define DATABASEID_CURBRAMPS	1
#define DATABASEID_ALLEYWALLS	2

LPSTRD getSendToServerFile(int databaseID, int iPad);
BOOL executeAndSendCmd(int databaseID, LPSTR cmd, BOOL sendToServer);

BOOL openDatabaseID(int databaseID);
void closeDatabaseID(int databaseID, BOOL opened);

BOOL loadUpdate(int databaseID, LPSTR path);

int getLastDataUpdateNumber(int databaseID, int iPad, int *pictNum);
BOOL setLastDataAndPictUpdateNums(int databaseID, int lastNum, int pictNum, int iPad);
BOOL setLastDataUpdateNum(int databaseID, int lastNum, int iPad);
BOOL setLastPictUpdateNum(int databaseID, int lastNum, int iPad);
int incrementLastDataFileNum(int databaseID);
int incrementLastPictFileNumber(int databaseID);
int incrementLastPictFileNum(int databaseID);
int getLastPictUpdateNumber(int databaseID, int iPad);

LPSTRD outputDataFile(int databaseID, int fileNum, int iPad);
LPSTRD outputPictFile(int databaseID, int fileNum, int iPad);

void logToErrorFile(LPSTR error);
void logToErrorFileWithHeader(LPSTR error, LPSTR header);
void logToErrorFileIgnore(BOOL ignore);
void clearErrorFile(void);
LPSTR timeStamp(void);

HANDLE OpenServerFTP(LPSTR serverDir, int serverNumber, LPSTR errorVar);
BOOL CloseServerFTP(HANDLE hFTPStruct);

BOOL PushFileToServer(LPSTR fromFile, LPSTR toFileName, LPSTR fromDir, LPSTR toDir, BOOL deleteWhenDone, BOOL appendTempExtension, HWND *popoverView, LPSTR title, double showAfter, LPSTR errorVar);
BOOL GetFileFromServer(LPSTR fromFile, LPSTR toFileName, LPSTR fromDir, LPSTR toDir, HWND *popoverView, LPSTR title, double showAfter, LPSTR errorVar);
#endif /* CurbRamps_h */
