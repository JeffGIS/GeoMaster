#include "graphint.h"
#include "translat.h"
#include "extrndb.h"
#include "shapefil.h"
#include "shpgeo.h"

LPSTR SHPGetNVP(LPSTR ShpFileName,LPDOUBLE pFactor)
{
	char	prjFileName[MAX_PATH+12] = "";
	LPSTR	pDot;
	HFILE	fid;
	LPSTR	rtn = 0;
	int		outUnits = 0;
#define OUMETERS	1
#define OUFEET		2

	if (pFactor)
		*pFactor = 1.0;
	if (!GetGlobalLVal2("[%USEOSRLIB]", FALSE))
		return 0;

	strncpy0(prjFileName, ShpFileName,MAX_PATH+10);
	ExpandText(prjFileName);
	if ((pDot = strrchr(prjFileName, '.')))
	{
		LPSTR pParen = strrchr(pDot, '(');
		if (pParen)
		{
			*pParen++ = 0;
			if (!strnicmp(pParen, "METERS", 6))
				outUnits = OUMETERS;
			else if (!strnicmp(pParen, "FEET", 4))
				outUnits = OUFEET;
		}
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
			GSSiClose2 (&fid);
			if (!ConvertPRJtoProj4(pMem, proj4def))
			{
				int ln = strlen (proj4def);
				rtn = malloc(ln + 4);
				strcpy(rtn, proj4def);
				if (strstr(rtn, "units=us-ft"))
				{
					if (outUnits == OUMETERS)
					{
						if (pFactor)
							*pFactor = FTM;
					}
				}
				else
				{
					if (outUnits == OUFEET)
					{
						if (pFactor)
							*pFactor = MFT;
					}
				}
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
	double factor;
	LPSTR nvp = SHPGetNVP(ShpFileName,&factor);
	if (nvp)
	{
		LoadProjection(projectionID, nvp);
		free(nvp);
		rtn = 1;
	}
	return rtn;
}

BOOL IsProjectionFile(LPSTR File)
{
	char file[MAX_PATH + 12];
	strncpy0(file, File, MAX_PATH);
	LPSTR pDot = strrchr(file, '.');
	if (pDot)
	{
		LPSTR pParen = strrchr(pDot, '(');
		if (pParen)
			*pParen = 0;
		if (!stricmp(pDot, ".prj"))
			return TRUE;
	}
	return FALSE;
}

static LPSTR GetProjectionFile(LPSTR File)
{
	static char file[MAX_PATH + 12];
	LPSTR rtn = file;
	strncpy0(file, File, MAX_PATH + 10);
	LPSTR pDot = strrchr(file, '.');
	if (pDot)
	{
		LPSTR pParen = strrchr(pDot, '(');
		if (pParen)
			*pParen = 0;
	}
	return rtn;
}

int TransformShapeFile(LPSTR InFile, LPSTR OutFile, LPSTR InPrj, LPSTR OutPrj)
{
	int rtn = 0;
	BOOL isPrjFile = FALSE;
	char * args[5];
	double inFactor=1, outFactor=1;
	char inprj_utm[] = { "+proj=utm +zone=15 +datum=NAD83 +units=m +no_defs" };
	char inprj_hc[] = { "+proj=lcc +lat_1=45.13333333 +lat_2=44.88333333 +lat_0=44.79111111 +lon_0=-93.38333333 +x_0=152400.3048006096 +y_0=30480.06096012192 +a=6378418.941 +b=6357033.31 +units=us-ft +no_defs" };
	LPSTR nvp = malloc(4096);
	char outprj[] = "+proj=latlong +datum=WGS84";
	args[1] = (char *)InFile;
	args[2] = OutFile;
	nvp = SHPGetNVP(InFile,&inFactor);
	if (nvp)
		args[3] = nvp;
	else if (InPrj && *InPrj)
		args[3] = InPrj;
	else
		args[3] = inprj_utm;

	if (!stricmp(OutPrj, "GOOGLEMAPS"))
	{
		args[4] = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs";
	}
	else if (IsProjectionFile(OutPrj))
	{
		args[4] = SHPGetNVP(OutPrj,&outFactor);
		isPrjFile = TRUE;
	}
	else if (OutPrj && *OutPrj)
		args[4] = OutPrj;
	else
		args[4] = outprj;
	if (args[4])
	{
		makedirectories(OutFile, FALSE, FALSE);
		rtn = TransformSHP(5, args, &outFactor);
		if (rtn && isPrjFile)
		{
			char PrjFile[MAX_PATH + 1];
			strncpy0(PrjFile, OutFile, MAX_PATH);
			LPSTR pDot = strrchr(PrjFile, '.');
			if (pDot)
			{
				strcpy(pDot, ".prj");
				CopyFile(GetProjectionFile(OutPrj), PrjFile, FALSE);
			}
		}
	}
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
		AddDPointToMinMax((LPDPOINT)minBounds, pbounds);
		AddDPointToMinMax((LPDPOINT)maxBounds, pbounds);
		rtn = TRUE;
	}

	return rtn;
}
BOOL GetShapeType(LPSTR file, int * ptype)
{
	BOOL rtn = FALSE;
	SHPHandle	hSHP;
	int		nShapeType, nEntities;
	double	minBounds[4], maxBounds[4];

	hSHP = SHPOpen(file, "rb");
	if (hSHP)
	{
		SHPGetInfo(hSHP, &nEntities, &nShapeType, minBounds, maxBounds);
		SHPClose(hSHP);
		rtn = TRUE;
		*ptype = nShapeType;
	}

	return rtn;
}
BOOL GetShapeNumRecs(LPSTR file, int * nrecs)
{
	BOOL rtn = FALSE;
	SHPHandle	hSHP;
	int		nShapeType, nEntities;
	double	minBounds[4], maxBounds[4];

	hSHP = SHPOpen(file, "rb");
	if (hSHP)
	{
		SHPGetInfo(hSHP, &nEntities, &nShapeType, minBounds, maxBounds);
		SHPClose(hSHP);
		rtn = TRUE;
		*nrecs = nEntities;
	}

	return rtn;
}
BOOL CopySHPParm(LPSTR fromfile, LPSTR tofile, int startref)
{
	BOOL rtn = FALSE;
	char fromFile[MAX_PATH];
	char toFile[MAX_PATH];
	char line[MAX_PATH + 2];
	LPSTR pDot;
	HFILE Fid, FidTo;

	strcpy(fromFile, fromfile);
	strcpy(toFile, tofile);

	pDot = strrchr(fromFile, '.');
	if (!pDot)
		pDot = strchr(fromFile, 0);
	strcpy(pDot, ".gsp");
	pDot = strrchr(toFile, '.');
	if (!pDot)
		pDot = strchr(toFile, 0);
	strcpy(pDot, ".gsp");
	Fid = GSSiOpenFile(fromFile, 0, OF_READ);
	if (Fid != HFILE_ERROR)
	{
		FidTo = GSSiOpenFile(toFile, 0, OF_CREATE);
		if (FidTo != HFILE_ERROR)
		{
			fgetstring(line, MAX_PATH, Fid);//projection
			fputstring(line, FidTo);
			fgetstring(line, 32, Fid);//units
			fputstring(line, FidTo);
			fgetstring(line, MAX_PATH, Fid);//refno
			itoa(startref, line, 10);
			fputstring(line, FidTo);
			fgetstring(line, 32, Fid);//TAG
			fputstring(line, FidTo);
			fgetstring(line, 32, Fid);//indextype (not used)
			fputstring(line, FidTo);
			while (fgetstring(line, 256, Fid))
			{
				if (*line == '#')
					break;
				fputstring(line, FidTo);
			}
			if (fgetstring(line, 256, Fid)) //begin date
				fputstring(line, FidTo);
			if (fgetstring(line, 256, Fid)) //end date
				fputstring(line, FidTo);
			GSSiClose2 (&FidTo);
		}
		GSSiClose2 (&Fid);
	}

	return rtn;
}
