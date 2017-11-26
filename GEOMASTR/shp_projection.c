#include "graphint.h"
#include "translat.h"
#include "extrndb.h"
#include "shapefil.h"
#include "shpgeo.h"

LPSTR SHPGetNVP(LPSTR ShpFileName)
{
	char	prjFileName[MAX_PATH] = "";
	LPSTR	pDot;
	HFILE	fid;
	LPSTR	rtn = 0;

	if (!GetGlobalLVal2("[%USEOSRLIB]", FALSE))
		return 0;

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
				int ln = strlen (proj4def);
				rtn = malloc(ln + 4);
				strcpy(rtn, proj4def);
			}
			GSSiGlobUlFree(&hDef);
			GSSiGlobUlFree(&hMem);
		}
	}
	return rtn;
}
int SHPOpenPrj(LPSTR ShpFileName, int projectionID)
{
	int rtn = 0;
	LPSTR nvp = SHPGetNVP(ShpFileName);
	if (nvp)
	{
		LoadProjection(projectionID, nvp);
		free(nvp);
		rtn = 1;
	}
	return rtn;
}

int TransformShapeFile(LPSTR InFile, LPSTR OutFile, LPSTR InPrj, LPSTR OutPrj)
{
	int rtn = 0;
	char * args[5];
	char inprj_utm[] = { "+proj=utm +zone=15 +datum=NAD83 +units=m +no_defs" };
	char inprj_hc[] = { "+proj=lcc +lat_1=45.13333333 +lat_2=44.88333333 +lat_0=44.79111111 +lon_0=-93.38333333 +x_0=152400.3048006096 +y_0=30480.06096012192 +a=6378418.941 +b=6357033.31 +units=us-ft +no_defs" };
	LPSTR nvp = malloc(4096);
	char outprj[] = "+proj=latlong +datum=WGS84";
	args[1] = (char *)InFile;
	args[2] = OutFile;
	nvp = SHPGetNVP(InFile);
	if (nvp)
		args[3] = nvp;
	else if (InPrj && *InPrj)
		args[3] = InPrj;
	else
		args[3] = inprj_utm;
	if (OutPrj && *OutPrj)
		args[4] = OutPrj;
	else
		args[4] = outprj;
	makedirectories(OutFile, FALSE, FALSE);
	rtn = TransformSHP(5, args);
	if (nvp)
		free(nvp);
	return rtn;
}

BOOL GetShapeBounds(LPSTR file, LPMNMXCORD pbounds)
{
	BOOL rtn = FALSE;
	SHPHandle	hSHP;
	DBFHandle   old_DBF, new_DBF;
	int		nShapeType, nEntities;
	double	minBounds[4], maxBounds[4];

	hSHP = SHPOpen(file, "rb");
	if (hSHP)
	{
		SHPGetInfo(hSHP, &nEntities, &nShapeType, minBounds, maxBounds);
		SHPClose(hSHP);
		DBoundsInit(pbounds);
		AddDPointToMinMax(minBounds, pbounds);
		AddDPointToMinMax(maxBounds, pbounds);
		rtn = TRUE;
	}

	return rtn;
}
