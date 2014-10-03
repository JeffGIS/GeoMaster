#include "graphint.h"   
#include "sqlite3.h"
//#include "sqlite3ext.h"

int SQLiteCmd(int nArgs, LPSTR *ARG)
{
	int rtn = 0;
	sqlite3 *db;

	if (!stricmp(ARG[1], "OPEN"))
	{
		rtn = sqlite3_open(ARG[2], &db);
		if (rtn == SQLITE_OK)
		{
			SetGlobalValueLong(ARG[3], (UINT)db);
		}
	}
	else if (!stricmp(ARG[1], "CLOSE"))
	{
		db = (sqlite3*)atoi(ARG[2]);
		rtn = sqlite3_close(db);
	}
	else if (!stricmp(ARG[1], "FROMGMD"))
	{
		db = (sqlite3*)atoi(ARG[2]);
		rtn = sqlite3_close(db);
	}
	return rtn;
}