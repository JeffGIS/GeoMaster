#include "graphint.h"
#include "translat.h"
#include "extrndb.h"

int SHPOpenPrj( LPSTR ShpFileName,int projectionID )
{
    char	prjFileName[MAX_PATH] = "";
	LPSTR	pDot;
	HFILE	fid;
	int		rtn = 0;

	strcpy(prjFileName, ShpFileName);
	ExpandText(prjFileName);
	if ((pDot = strrchr(prjFileName, '.')))
	{
		strcpy(pDot, ".prj");
		fid = GSSiOpenFile(prjFileName, 0, OF_READ);
		if (fid != HFILE_ERROR)
		{
			int len = GSSifilelength(fid);
			HANDLE hMem = GSSiGlobAlloc(0, GMEM_MOVEABLE, len + 1);
			LPSTR pMem = GlobalLock(hMem);
			HANDLE hDef = GSSiGlobAlloc(0, GMEM_MOVEABLE, 4096);
			LPSTR proj4def = GlobalLock(hDef);

			BigRead(fid, pMem, len);
			pMem[len] = 0;
			GSSiClose(fid);
			if (!ConvertPRJtoProj4(pMem, proj4def))
			{
				LoadProjection(projectionID, proj4def);
				rtn = 1;
			}
			GSSiGlobUlFree(&hDef);
			GSSiGlobUlFree(&hMem);
		}
	}
	return rtn;
}


