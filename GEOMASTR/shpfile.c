#include "graphint.h"
#include "translat.h"
#include "extrndb.h"
#include "shapefil.h"
#include <commctrl.h>
#define MAXINDEXSYMBOLS	3200
#define MAXPOC	4096
#define MAXFGDBLEVS	8
#define MAX_CHILD_LENGTH 256

static	struct {float fcontour;
			float size;
			MNMXCORD bounds;
			int npnts;} conheader;

// Extended shape buffer format
			typedef enum{
				IsEmpty =	1,
				IsCCW =		1 << 3,
				IsMinor =	1 << 4,
				IsLine =	1 << 5,
				IsPoint =	1 << 6,
				DefinedIP = 1 << 7
			} arcBits;

			typedef enum
			{
				esriSegmentArc = 1,
				esriSegmentLine = 2,
				esriSegmentSpiral = 3,
				esriSegmentBezierCurve = 4,
				esriSegmentEllipticArc = 5
			} esriSegmentType;
			typedef struct
			{
				union {
					DPOINT centerPoint;
					double angles[2];
				};
				LONG Bits;
			} SegmentArc;
			typedef struct 
			{
				long startPointIndex,
					segmentType;
				union {
					SegmentArc			arc;
					//SegmentBesierCurve	bezierCurve;
					//SegmentEllipticArc	ellipticArc;
					//SegmentOther		otherKindsOfSegment;
				} segmentParams;
			}esriSegmentModifier;

static double			PGDBCnvFac=1;//FTM for henco
static SHPHEADER		SHPHeader; 
static SHPPOLYHEADER	SHPPolyHeader;  
static SHPPOINTREC		SHPPointRec;
static SHPPOINTZREC		SHPPointZRec;
static long				SHPBaseRefno=0;
static char				SHPRefno[256];
static long				SHPMaxRefPerFile=0;
static BOOL				SHPProjectionIsBase;
static short			NumSHPParms; 
static char				SHPParms[4096]="";  
static char				SHPTag[100], SHPTAG[100]; 
static char				LastSHPFile[MAX_PATH]=""; 
static HFILE			FidSmallDBF=HFILE_ERROR;
static char				SmallDBFName[MAX_PATH]="";
static MNMXCORD			SHPFileMNMX,CurrentSHPRecBounds;    
static HANDLE			hSHPIndexBlocks=0;
static long				NumSHPIndexBlocks; 
static time_t			SHPParmTime=0;  
static short			HaveSHPSym=-1; 
static short			NumIndexSyms=0;
static short			IndexSyms[MAXINDEXSYMBOLS]; 
static char				SHPBeginDate[256];
static char				SHPEndDate[256];
static BYTE				BlockInWBounds[1024];  
static double			PGDBGridOrigX,PGDBGridOrigY,PGDBGridSize;  
static long				ExtraBytes;
static short			WordLen[16];
static char				Words[16][128];
static long				POCPos[MAXPOC];
static DPOINT			POCCoord[MAXPOC];
static long				POCFlag[MAXPOC] = { 0 };
static int				NumPOC;
static HFILE			thinnedConFid=HFILE_ERROR;
static sqlite3			*SHPIndexHandle = 0;
static sqlite3_stmt		*SHPstatement = 0;
static double			ShapeXMin = 0, ShapeYMin = 0;
static double			ShapeXFactor = 1, ShapeYFactor = 1;

#include "gmextern.h"
#include <sys\types.h>
#include <sys\stat.h>         

typedef struct {
					long	Offset; 
					mnmxCor	MinMax;
					short	SymNum;
				}SHPINDEXRECORD;
typedef SHPINDEXRECORD	FAR	*LPSHPINDEXRECORD; 

static int minx=458000, miny=4989900, midx=464380, midy=4995700, nrows=0, ncols=0, iwidth=1160, nfiles;
static RECT ScreenRect;

sqlite3 * NVShapeIndexCreate(LPSTR IndexNameIN)
{
	char IndexName[MAX_PATH];
	char cmd[256];
	int rtn;
	sqlite3 * db=NULL;
	LPSTR pDot;

	return db;
	strcpy(IndexName, IndexNameIN);
	if ((pDot = strrchr(IndexName, '.')))
	{
		strcpy(pDot, ".nvi");
		GSSiRemove(IndexName);
		rtn = sqlite3_open(IndexName, &db);
		if (rtn == SQLITE_OK)
		{
			rtn = SQLOK(sqlite3_exec(db, "BEGIN", NULL, NULL, 0), db, "", 0);
			sprintf(cmd, "CREATE TABLE SHAPEINDEX (RECNUM INTEGER PRIMARY KEY, SymNum INT, offset INT);");
			rtn = sqlite3_exec(db, cmd, NULL, NULL, NULL);
			sprintf(cmd, "CREATE VIRTUAL TABLE SHAPEINDEX_index USING rtree(id,minX, maxX, minY, maxY);");
			rtn = sqlite3_exec(db, cmd, NULL, NULL, NULL);
		}
	}
	return db;
}

int NVShapeIndexAdd(sqlite3 * db, long long RECNUM, long long SHPRecOffset, LPMNMXCORD pBounds, int symnum)
{
	char cmd[256];
	int rtn;
	return 0;
	sprintf(cmd, "INSERT INTO SHAPEINDEX VALUES(%I64i,%i,%I64i)", RECNUM, symnum, SHPRecOffset);
	rtn = sqlite3_exec(db, cmd, NULL, NULL, NULL);
	sprintf(cmd, "INSERT INTO SHAPEINDEX_index VALUES(%I64i,%f,%f,%f,%f)", RECNUM, pBounds->xmn, pBounds->xmx, pBounds->ymn, pBounds->ymx);
	rtn = sqlite3_exec(db, cmd, NULL, NULL, NULL);
	return rtn;
}

int NVShapeIndexClose(sqlite3 * db)
{
	int rtn;
	return 0;
	rtn = SQLOK(sqlite3_exec(db, "COMMIT", NULL, NULL, 0), db, "", 0);
	rtn = sqlite3_close(db);
	return rtn;
}

BOOL OpenSHPFile (LPSTR SHPFileNameIN)
{
	int	i;
	MNMXCORD	FileMNMX;
	DPOINT		Points[4];
	char		 SHPFileName[MAX_PATH];
	OFSTRUCTGM	ofStructGM;

	strcpy(SHPFileName, SHPFileNameIN);
	ExpandText(SHPFileName);

	SHPFid = GSSiOpenFile (SHPFileName,&ofStructGM,OF_READ);
	if (SHPFid == HFILE_ERROR)
		return FALSE;
	SHPHandle	hSHP = SHPOpenGSSi(ofStructGM.szPathName, "rb");
	if (hSHP)
		SHPClose(hSHP);
	if (!(SHPType = ReadSHPHeader (SHPFid,&SHPFileMNMX,SHPFileName)))
    {
    	GSSiClose2 (&SHPFid);
    	return FALSE;
    }
	CloseTRANS2 (&hTranFileToBase); 
	CloseTRANS2 (&hTranBaseToFile);  
	CloseTRANS2 (&hTranFileToVP);  
	LoadSHPParm (SHPFileName,SHPType,CurView->hWnd);
	
	Points[0].x = ClipCoordToProjection (SHPFileMNMX.xmn,1,0,1);   
	Points[0].y = ClipCoordToProjection (SHPFileMNMX.ymn,2,0,1);   
	Points[1].x = ClipCoordToProjection (SHPFileMNMX.xmn,1,0,1);   
	Points[1].y = ClipCoordToProjection (SHPFileMNMX.ymx,2,0,1);   
	Points[2].x = ClipCoordToProjection (SHPFileMNMX.xmx,1,0,1);   
	Points[2].y = ClipCoordToProjection (SHPFileMNMX.ymx,2,0,1);   
	Points[3].x = ClipCoordToProjection (SHPFileMNMX.xmx,1,0,1);   
	Points[3].y = ClipCoordToProjection (SHPFileMNMX.ymn,2,0,1); 
	DBoundsInit (&FileMNMX);
	for (i=0;i<4;i++)
	{
		if (ConvertCoord(&Points[i],0,1))
		{   
		    MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
	    	GSSiClose2 (&SHPFid);
		    return FALSE;
		}
		AddDPointToMinMax (&Points[i],&FileMNMX); 
	}
	if (FileMNMX.xmx - FileMNMX.xmn >
		FileMNMX.ymx - FileMNMX.ymn)
	{
		MinMax.xmn = -32000;
		MinMax.xmx = 32000;
		MinMax.ymn = -32000 * ((FileMNMX.ymx - FileMNMX.ymn)/(FileMNMX.xmx - FileMNMX.xmn));
		MinMax.ymx = -MinMax.ymn;
	} 
	else
	{
		MinMax.ymn = -32000;
		MinMax.ymx = 32000;
		MinMax.xmn = -32000 * ((FileMNMX.xmx - FileMNMX.xmn)/(FileMNMX.ymx - FileMNMX.ymn));
		MinMax.xmx = -MinMax.xmn;
	} 
	CreateFileTran (&MinMax,&FileMNMX); 
	NextSHPRec = 0;
	OpenSHPFileIndex (SHPFileName,SHPFid);
	return TRUE;
} 

void CloseSHPFile (void)
{   
	GSSiClose2 (&SHPFid);
	OpenSHPFileIndex (0,HFILE_ERROR);
	return;
} 

void GetSHPName (LPSTR Name)
{
	_fstrcpy (Name,LastSHPFile);
	return;
}                            

/*void CreateFidDBF (void)
{   
	HANDLE	hMem;
	LPOFSTRUCT	pOFStruct;
	
	if (!*SmallDBFName)
		GSSiGetTempFileName (0,"gms",0,(LPSTR)SmallDBFName);
	hMem = GSSiGlobAlloc (1416,GMEM_MOVEABLE,sizeof(OFSTRUCTGM));
	pOFStruct = (LPOFSTRUCTGM)GlobalLock (hMem);	
   	FidSmallDBF = OpenFile (SmallDBFName,pOFStruct,OF_CREATE);
   	GSSiGlobUlFree (&hMem);
	return;
}  

BOOL CloseFidDBF (BOOL Final)
{   
	OFSTRUCTGM	OFStruct;
	HFILE		AvailableFid; 
	BOOL		UseSmall=FALSE;
	HANDLE		hMem;
	LPOFSTRUCT	pOFStruct; 
	LPSTR		TestName;
	
	if (FidSmallDBF == HFILE_ERROR)
		return FALSE; 
	if (Final)
	{
		_lclose (FidSmallDBF);
		remove (SmallDBFName);
		FidSmallDBF = HFILE_ERROR;  
		return TRUE;
	}
	hMem = GSSiGlobAlloc (1416,GMEM_MOVEABLE,sizeof(OFSTRUCTGM)+256);
	TestName = GlobalLock (hMem);
	pOFStruct = (LPOFSTRUCTGM)(TestName + 256);	
	GSSiGetTempFileName (0,"gms",0,(LPSTR)TestName);
	AvailableFid = OpenFile (TestName,pOFStruct,OF_CREATE); 
	if (AvailableFid > 20)
		UseSmall = TRUE;
	_lclose (AvailableFid);
	OpenFile (TestName,pOFStruct,OF_DELETE); 
	
	if (UseSmall)
	{
		_lclose (FidSmallDBF);
		remove (SmallDBFName);
		FidSmallDBF = HFILE_ERROR;
	}
   	GSSiGlobUlFree (&hMem);
	return UseSmall;
} */

BOOL SelectSHPFile (LPSTR File,LPSTR SymName)
{   
	if (!*LastSHPFile)
		return FALSE;
	_fstrcpy (File,LastSHPFile);
	return TRUE;
}  

int GetSHPSymListFilePath(LPSTR symlistFile, LPSTR SHPFileName)
{
	BOOL rtn = 0;
	char tempPath[MAX_PATH];
	LPSTR pLoc, pEnd;

	strcpy(tempPath, SHPFileName);
	strupr(tempPath);
	if ((pLoc = strstr(tempPath, ".GDB(")))
	{
		*pLoc = 0;
		pLoc += 5;
		if ((pEnd = strchr(pLoc, ')')))
			*pEnd = 0;
		sprintf(symlistFile, "%s_%s.symlist", tempPath, pLoc);
		rtn = MT_FILE_GEO_DB;
	}
	return rtn;
}

void CreateSHPSymlistFile(LPSTR SHPFileName, int NumSHPParms, LPSTR SymName)
{
	char symlistFile[MAX_PATH];
	HANDLE hStr = 0;
	LPSTR str, sql, sname, tableName, pTableName;
	int iType;
	HANDLE hDB = 0;
	HWND hWnd = 0;
	HFILE fid;

	if (CurView)
		hWnd = CurView->hWnd;

	if (NumSHPParms != 1 || *SymName != '[' || *LastChr(SymName) != ']')
		return;
	if ((iType = GetSHPSymListFilePath(symlistFile, SHPFileName)))
	{
		if (FileType(symlistFile) != 1)
		{
			switch (iType)
			{
			case MT_FILE_GEO_DB:
			{
				hStr = GSSiGlobAlloc(0, GMEM_MOVEABLE, 4096+1024);
				str = GlobalLock(hStr);
				sql = str + 4096;
				sname = sql + 512;
				tableName = sname + 256;
				sprintf(str, "FGDB=%s", SHPFileName);
				if ((pTableName = strrchr(str, '(')))
				{
					pTableName++;
					strcpy(tableName, pTableName);
					*LastChr(tableName) = 0;
				}
				if (!strnicmp(SymName, "[FGDB.", 6))
				{
					strcpy(sname, &SymName[6]);
					*LastChr(sname) = 0;
				}
				else if (*SymName == '[')
				{
					strcpy(sname, &SymName[1]);
					*LastChr(sname) = 0;
				}
				sprintf(sql, ":%s", sname);
				//sprintf(sql, "SELECT %s FROM %s:%s", sname, tableName,sname);
				if (OpenDataFile(str,sql,BT_READ,&hDB))
				{
					int ln = 66;
					int pos = BT_FIRST;
					int count, numSymbols;
					char value[256];
					HANDLE hSymbols;
					LPSTR pSymbols;
					int lSymbols = 0;
					HANDLE hBTDistinct = GetDistinctValues(hWnd, SymName, ln, hDB, 1024);
					numSymbols = BT_NUM_IN_INDEX(hBTDistinct);
					hSymbols = GSSiGlobAlloc(0,GHND, ln*numSymbols + 32);
					pSymbols = GlobalLock(hSymbols);
					while (!BT_FIND(hBTDistinct, value, pos, BT_ANY, (LPSTR)&count))
					{
						strcpy(&pSymbols[lSymbols], value);
						lSymbols += strlen(value) + 1;
						pos = BT_NEXT;
					}
					BT_CLOSEANDDELETE(&hBTDistinct);
					CloseDataFile(FALSE,&hDB);
					fid = GSSiOpenFile(symlistFile, 0, OF_CREATE);
					BigWrite(fid, pSymbols, lSymbols + 2,-1);
					GSSiClose2 (&fid);
					GSSiGlobUlFree(&hSymbols);
				}
				GSSiGlobUlFree(&hStr);
			}
				break;
			default:
				break;
			}
		}
	}
	return;
}

void DecodeSHPParam (LPSTR str,LPSTR cDesc,LPSTR cIF, LPSTR cColor, LPSTR cWidth, LPSTR cRot)
{ 
	char	nullchr=0;
	LPSTR	pIF=&nullchr, pColor=&nullchr, pWidth=&nullchr, pRot=&nullchr;
	
	if ((pIF = _fstrchr (str,';')))
	{
		*pIF++ = 0;
		if ((pColor = _fstrchr (pIF,';')))
		{
			*pColor++ = 0;
			if ((pWidth = _fstrchr (pColor,';')))
			{
				*pWidth++ = 0;
				if ((pRot = _fstrchr (pWidth,';')))
				{
					*pRot++ = 0;
				}
				else
					pRot = _fstrchr (str,0);
			}
			else
				pWidth = _fstrchr (str,0);
		}
		else
			pColor = pWidth = pRot = _fstrchr (str,0);
	}
	else
		pIF = pColor = pWidth = _fstrchr (str,0); 
	_fstrcpy (cDesc,str);
	_fstrcpy (cIF,pIF);
	_fstrcpy (cColor,pColor);
	_fstrcpy (cWidth,pWidth);
	_fstrcpy (cRot,pRot);
	return;
}

BOOL LoadSHPParm (LPSTR SHPFileName,long Type,HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (1374);
#endif
{   
	char	Name[MAX_PATH], str[260], Projection[MAX_PATH+2], Units[34];
	char	SymName[66], cWidth[64],cRot[64],cColor[64], cIF[128];
	LPSTR	pDot, pTAG,pWidth, pParm=SHPParms, pBS;  
	short	l;
	int		itype;
	HFILE	Fid; 
	BOOL	FileIsIndex;
	BOOL	havePrj = FALSE;
	struct _stati64    statParmFile; 

    if (!SHPFileName)
    {   
    	HaveIndexParmFile = FALSE;  
    	goto RtnTrue;
    }
    if (HaveIndexParmFile) 
    {
    	goto RtnTrue;
    }
	*SHPBeginDate = 0;
	*SHPEndDate = 0;
	SHPParmTime = 0;
	NumSHPParms = 0;  
	SHPProjectionIsBase = TRUE;
	_fmemset (SHPParms,0,sizeof(SHPParms)); 
	switch (Type)
	{
		case SHPT_POINT:
		case SHPT_POINTZ:
			GetGlobalCVal ("[%DefaultShapePointSymbol]",SHPParms,"DUMMYPT");  
			pWidth = _fstrchr (SHPParms,0)+4;
			GetGlobalCVal ("[%DefaultShapePointSize]",pWidth,"-5"); 
		break;
		
		case SHPT_ARC:
		case SHPT_ARCZ:
		case SHPT_ARCM:
		//case shapePolylineM:
		//case shapePolylineZM:
		case shapePolylineZ:
			GetGlobalCVal ("[%DefaultShapeLineSymbol]",SHPParms,"PEN1"); 
		break;
        
        case 4://personalgeodb area type???
      	case SHPT_TEXT:
		case SHPT_POLYGON:
		case SHPT_POLYGONM:
		case SHPT_POLYGONZ:  
		case SHPT_PGDB_POLYGONZ:
		//case shapePolygonM:
		//case shapePolygonZM:
		//case shapePolygonZ:
			GetGlobalCVal ("[%DefaultShapeAreaSymbol]",SHPParms,"PARCEL"); 
		break;
	}
	SHPBaseRefno = 0; 
	*SHPRefno = 0;
	_fstrcpy (Name,SHPFileName); 
	ExpandText (Name);
	l = _fstrlen (Name);
	pBS = strrchr(Name, '\\');
	if (l > 4 && !_fstricmp(&Name[l - 5], "INDEX"))
	{
		HaveIndexParmFile = FALSE; 
		FileIsIndex = TRUE; 
		pDot = &Name[l];
	}
	else if (pBS && !strnicmp(++pBS, "INDEX", 5))
	{
		HaveIndexParmFile = FALSE;
		FileIsIndex = TRUE;
		pDot = strchr(pBS, 0);
	}
	else  
	{
		FileIsIndex = FALSE;
        if ((pDot = _fstrrchr (Name,'.')))
        {
        	if (!_fstrnicmp (pDot,".mdb",4))
        	{
        		if (*(pDot+4) != '(')
        			goto RtnFalse;
        		_fstrcpy (PGDBTable,pDot+5);
        		*LastChr (PGDBTable) = 0; 
        		*pDot = 0;
        		sprintf (str,"%s_%s.",Name,PGDBTable);
		        _fstrcpy (Name,str);
		        pDot = LastChr (Name);
        	}
        	else if (!_fstrnicmp (pDot,".gdb",4))
        	{
        		if (*(pDot+4) != '(')
        			goto RtnFalse;
        		_fstrcpy (FGDBTable,pDot+5);
        		*LastChr (FGDBTable) = 0; 
        		*pDot = 0;
        		sprintf (str,"%s_%s.",Name,FGDBTable);
		        _fstrcpy (Name,str);
		        pDot = LastChr (Name);
        	}
        }
	}
	if (!pDot)
		goto RtnFalse;
	_fstrcpy (pDot,".gsp");
	Fid = GSSiOpenFile (Name,0,OF_READ); 
	if (Fid == HFILE_ERROR)  
	{
        DLGPROC lpfnSETSHAPEPARAMMsgProc;  

		_fstrcpy (LastSHPFile,SHPFileName);
		ExpandText (LastSHPFile); 
		if (!Type || FileIsIndex || !GetGlobalBVal2 ("[%AUTOSHPPARM]",TRUE))
			goto RtnFalse;   
		{
			lpfnSETSHAPEPARAMMsgProc = MakeProcInstance((DLGPROC)SETSHAPEPARAMMsgProc, hInst);
			DialogBox(hInst, (LPSTR)"SETSHAPEPARAM", hWnd, lpfnSETSHAPEPARAMMsgProc);
			FreeProcInstance(lpfnSETSHAPEPARAMMsgProc);
			Fid = GSSiOpenFile (Name,0,OF_READ);  
			if (Fid == HFILE_ERROR)  
	        	goto RtnFalse; 
	    }
	}
	if (FileIsIndex)
		HaveIndexParmFile = TRUE;
	else
	{
		if (!SHPOpenPrj(SHPFileName, 0))
		{
			GetGlobalCVal("[%DefaultShapeProjection]", Projection, "BASEPROJ");
			LoadProjection(0, Projection);
		}
		else
			havePrj = TRUE;
		GetGlobalCVal("[%DefaultShapeUnits]", Units, "FEET");
		PGDBCnvFac = 1;
		if (!_fstricmp(Units, "FEET"))
		{
			PRJ_UNITS[0] = 1;
			PGDBCnvFac = FTM;
		}
		else if (!_fstricmp(Units, "METERS"))
			PRJ_UNITS[0] = 2;
		else
			PRJ_UNITS[0] = 4;
	}
    GSSifstat (Fid,&statParmFile);
    SHPParmTime = statParmFile.st_mtime;
	fgetstring (Projection,MAX_PATH,Fid);
	if (!havePrj)
	{
		if (*Projection)
			LoadProjection(0, Projection);
		else
		{
			GetGlobalCVal("[%DefaultShapeProjection]", Projection, "BASEPROJ");
			LoadProjection(0, Projection);
		}
	}
	SHPProjectionIsBase = IS_BASE[0];
	fgetstring (Units,32,Fid);
	if (!*Units)
		GetGlobalCVal ("[%DefaultShapeUnits]",Units,"FEET"); 
	if (!havePrj)
	{
		if (!_fstricmp(Units, "FEET"))
			PRJ_UNITS[0] = 1;
		else if (!_fstricmp(Units, "METERS"))
			PRJ_UNITS[0] = 2;
		else
			PRJ_UNITS[0] = 4;
	}
	fgetstring (SHPRefno,255,Fid); 
	if (IndexEntryStartRef != LONG_MAX)
		SHPBaseRefno = IndexEntryStartRef;
	else
		SHPBaseRefno = atol (SHPRefno);
	fgetstring (SHPTAG,99,Fid); 
	fgetstring (str,32,Fid); 
	//SHPIndexType = atoi (str);
	_fmemset (SHPParms,0,sizeof(SHPParms));
	while (fgetstring (str,256,Fid))
	{   
		if (*str == '#')
			break;
		DecodeSHPParam (str,SymName,cIF,cColor,cWidth,cRot);
		_fstrcpy (pParm,SymName);
		l = _fstrlen (SymName);
		pParm += l+1;
		_fstrcpy (pParm,cIF);
		l = _fstrlen (cIF);
		pParm += l+1;
		_fstrcpy (pParm,cColor);
		l = _fstrlen (cColor);
		pParm += l+1;
		_fstrcpy (pParm,cWidth);
		l = _fstrlen (cWidth);
		pParm += l+1;
		_fstrcpy (pParm,cRot);
		l = _fstrlen (cRot);
		pParm += l+1;
		NumSHPParms++;
		switch (Type)
		{
		default:
		case SHPT_POINT:
		case SHPT_POINTZ:
		case SHPT_MULTIPOINT:
			itype = 1;
			break;
		case SHPT_ARC:
			itype = 2;
			break;
		case SHPT_POLYGON:
			itype = 3;
		}
		if (!GetOrCreateSym(hWnd, SymName, 0, 0, GetGlobalLVal2("[%ALLOWSYMBOLCREATION]", 0), itype))
		{
			ExpandText(SymName);
			MessageBox(0, "Symbol not found", SymName, MB_ICONEXCLAMATION);
		}
	}
	//CreateSHPSymlistFile(SHPFileName, NumSHPParms, SymName); could speed up file gdb and shp processing for multiple symbol files when symbol name contained in a variable
	if (fgetstring(str, 256, Fid))
		_fstrcpy (SHPBeginDate,str);
	if (fgetstring (str,256,Fid))
		_fstrcpy (SHPEndDate,str);
	
	GSSiClose2 (&Fid); 
RtnTrue:
{
#if ENABLETRACE
GSSiExitProg (1374);
#endif
	return TRUE;
}
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (1374);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

void AddSymToList (HWND hWndDlg, int DlgItemSym, int DlgItemPar, short idesc,HFILE FidSymList)
{   
	short	iparent; 
	char	DescName[100];
	
	iparent = GetDictSymParent (idesc); 
	GetDictSymName (idesc,DescName);
    ConvertSymName (DescName,1,FALSE,idesc);  
	if(GetVisibility(idesc))
		_fstrcat (DescName,"\t<on>\t");
	else
		_fstrcat (DescName,"\t\t");
	sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,idesc);
	if (FidSymList != HFILE_ERROR)
	{
		sprintf (strchr (DescName,0),"\t%i",CurView->CurFile);
		fputstring (DescName,FidSymList);
	}
	else
	{
		if (SendDlgItemMessage ((HWND)hWndDlg,DlgItemSym,LB_FINDSTRING,-1,(LPARAM) DescName) ==  LB_ERR)
			SendDlgItemMessage ((HWND)hWndDlg,DlgItemSym,LB_ADDSTRING,0,(LPARAM) DescName); 
		while (iparent)
		{   
			idesc = iparent;
			iparent = GetDictSymParent (idesc); 
			GetDictSymName (idesc,DescName);
			ConvertSymName (DescName,1,TRUE,idesc);  
    		if(GetVisibility(idesc))
    			_fstrcat (DescName,"\t<on>\t");
    		else
    			_fstrcat (DescName,"\t\t");
    		sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,idesc);
			if (SendDlgItemMessage ((HWND)hWndDlg,DlgItemPar,LB_FINDSTRING,-1,(LPARAM) DescName) ==  LB_ERR)
				SendDlgItemMessage ((HWND)hWndDlg,DlgItemPar,LB_ADDSTRING,0,(LPARAM) DescName); 
		}
	}
	return;
}

BOOL IsSHPFileVisible (void)
{
	LPSTR	pDesc, pClause, pC, pColor, pWidth, pRot; 
	short	idesc,i; 
	char	str[128]; 
//	return TRUE;
	if (NumIndexSyms)
	{    
		for (i=0;i<NumIndexSyms;i++)
			if (GetVisibility (IndexSyms[i]))
				return TRUE;  
	}
	if (!*SHPParms)
		return TRUE;
	pDesc = SHPParms;
	while (*pDesc)
	{
		pClause = _fstrchr (pDesc,0) + 1;
		pColor = _fstrchr (pClause,0) + 1;
		pWidth = _fstrchr (pColor,0) + 1;
		pRot = _fstrchr (pWidth,0) + 1;   
		_fstrcpy (str,pDesc);
		ExpandText (str);
		idesc = GetDictSymbolNumber (str); 
		if (GetVisibility (idesc))
			return TRUE;  
		pDesc = _fstrchr (pRot,0) + 1;
	}
	return FALSE;
}
void GetSHPTag(LPSTR tag)
{
	strcpy(tag, SHPTag);
	return;
}

BOOL SetSHPVis (HWND hWndDlg, int DlgItemSym, int DlgItemPar,HFILE FidSymList)
{
	LPSTR	pDesc, pClause, pC, pColor, pWidth, pRot; 
	short	idesc,i; 
	char	str[128]; 
	
	if (NumIndexSyms)
	{    
		for (i=0;i<NumIndexSyms;i++)
			AddSymToList (hWndDlg,DlgItemSym,DlgItemPar,IndexSyms[i],FidSymList);
		return TRUE;
	}
	pDesc = SHPParms;
	while (*pDesc)
	{
		pClause = _fstrchr (pDesc,0) + 1;
		pColor = _fstrchr (pClause,0) + 1;
		pWidth = _fstrchr (pColor,0) + 1;
		pRot = _fstrchr (pWidth,0) + 1;   
		_fstrcpy (str,pDesc);
		ExpandText (str);
		idesc = GetDictSymbolNumber (str);   
		AddSymToList (hWndDlg,DlgItemSym,DlgItemPar,idesc,FidSymList);
		pDesc = _fstrchr (pRot,0) + 1;
	}
/*	else if (parent) 
	{   
		if(GetVisibility(idesc))
		{
    		if (_fstricmp (DescName,"ALL"))   
           		sprintf (str,"(%s)",DescName);
    		SendDlgItemMessage ((HWND) hWndDlg, -DlgItemPar,
        				     	CB_ADDSTRING, 0, (LPARAM) str);
	    }
	}*/ 

	return TRUE;
}

BOOL SetSHPParms (long RecordNumber)
{   
	LPSTR	pDesc, pTAG, pClause, pC, pColor, pWidth, pRot; 
	BOOL	rc;
	char	str[1024];  
	BOOL	rtn = FALSE;
	
	CurrentSHPRec = RecordNumber;
	if (strchr (SHPRefno,'['))
	{
		BOOL err;

		strcpy (str,SHPRefno);
		CurrentRefno = FltAP(str, &err);
	}
	else
		CurrentRefno = SHPBaseRefno + RecordNumber + IndexEntryStartRecord;
	if (!hSHPDBF)
		return FALSE; 
	SetUseOnlyOneDBHandle (hSHPDBF);
	pTAG = SHPTAG;
	if (*pTAG && (pC = _fstrchr (pTAG,':')))
	{   
		HANDLE hTag = GSSiGlobAlloc(0,GMEM_MOVEABLE, 4096);
		LPSTR ptag = GlobalLock(hTag);
		_fstrcpy (ptag,pTAG); 
		ExpandText (ptag);
		strncpy0(SHPTag, ptag, sizeof(SHPTag)-1);
		GSSiGlobUlFree(&hTag);
		pC = _fstrchr (SHPTag,':');
		*pC++ = 0;
		strncpy0 (CurrentPrefix,SHPTag,MAX_PREFIX_LEN);
		strncpy0 (CurrentUDI,pC--,MAX_UDI_LEN);
		*pC = ':'; 
		ExpandText (CurrentUDI);
	}
	else if (!*SHPTAG)
	{
		sprintf(SHPTag, "REFNO:%i", CurrentRefno);
		strcpy(CurrentPrefix, "REFNO");
		sprintf(CurrentUDI, "%i", CurrentRefno);
	}
	else
	{   
		*SHPTag = 0;
		*CurrentPrefix = 0;
		*CurrentUDI = 0;
	} 
	if (HaveSHPSym < 0)
	{
    	LPOPENSQLDATA SQLPtr = (LPOPENSQLDATA)GlobalLock (hSHPDBF);
	    LPOPENFILEDATA FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
		pDesc = SHPParms;
		while (*pDesc)
		{
			pClause = _fstrchr (pDesc,0) + 1;
			pColor = _fstrchr (pClause,0) + 1;
			pWidth = _fstrchr (pColor,0) + 1;
			pRot = _fstrchr (pWidth,0) + 1;
	    	ConvertSQLToLogicP (str,pClause); 
			if (!*pClause || LogicPFile (SQLPtr,str,&rc))
			{
				break;
			}
			else
				pDesc = _fstrchr (pRot,0) + 1;
		}
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (hSHPDBF);   
		_fstrcpy (str,pDesc);
		ExpandText (str);
		CurrentDesc = GetDictSymbolNumber (str); 
		if (!CurrentDesc)
		{
			if (GetGlobalLVal2 ("[%ALLOWSYMBOLCREATION]",0))
			{
				int itype=0;
				switch (SHPType)
				{
				case SHPT_TEXT:
				case SHPT_POINT:
				case SHPT_MULTIPOINTZ:
				case SHPT_MULTIPOINTM:
				case SHPT_POINTZ:
					itype = 1;
					break;
				case SHPT_ARC:
				case SHPT_ARCM:
				case SHPT_ARCZ:
				case SHPT_POLYLINE_WITHCURVES:
					itype = 2;
					break;
				case SHPT_POLYGON_PGDB:
				case SHPT_POLYGON:
				case SHPT_POLYGON_WITHCURVES:
				case SHPT_POLYGONZ:
					itype = 3;
					break;
				}
				if (itype)
					CurrentDesc = GetOrCreateSym (0,str,0,0,TRUE,itype);
			}
		}
		if (*pRot && (SHPType == SHPT_POINT ||
					  SHPType == SHPT_POINTZ ||
					  SHPType == SHPT_TEXT ||
					  SHPType == SHPT_MULTIPOINTZ ||
					  SHPType == SHPT_MULTIPOINTM))
		{
			PTRot = atof (pRot);
			if (*LastChr (pRot) == 'D')
				PTRot *= RADDEG;
		}
		if (!*pDesc)
			goto Exit;
		if (*pColor) 
		{
			_fstrcpy (str,pColor);
			ExpandText (str);
			SHPColor = ConvertColor (atol (str),CurrentDesc);
		}
		else
			SHPColor = -1;
		if (*pWidth) 
		{
			_fstrcpy (str,pWidth);
			ExpandText (str);
			SHPWidth = atof (str);
			if (SHPWidth == -1)
				SHPWidth = GetSymbolWidth (CurrentDesc);
			switch (*LastChr (str)) 
			{
				case 'P':
				case 'p':
					SHPWidth = -SHPWidth;
					break;
				case 'F':
				case 'f':
					SHPWidth *= FTM;
					break;
			}
			if (SHPType == SHPT_POINT || SHPType == SHPT_POINTZ)
				CurPointSize = SHPWidth;
		}
		else
			SHPWidth = 0;
	}
	else
		CurrentDesc = HaveSHPSym; 
	if (*SHPBeginDate)
	{
		_fstrcpy (str,SHPBeginDate);
		ExpandText (str);
		GRStartTime  = GREndTime  = atol(str);
	}
	if (*SHPEndDate)
	{
		_fstrcpy (str,SHPEndDate);
		ExpandText (str);
		GREndTime  = atol(str);
	}
	rtn = TRUE;   
Exit: 
	SetUseOnlyOneDBHandle (0);
	return rtn;
}

HFILE CreateSHPFileIndex (LPSTR IndexName,LPSTR SHPFileName)
{   
	HFILE	Fid, FidIdx;   
	long	i, Offset,ii;
	SHPINDEXRECORD	IndexRecord; 
	short	SaveIndexType = SHPIndexType;
	MNMXCORD	Bounds;
	HANDLE	hIndexBlocks;
	LPMINMAX BlockMinMax;
	long	RecsPerBlock = 100;
	long	NumIndexBlocks = max (2,min(1024,1+(NumSHPRecs-1)/RecsPerBlock)); 
	long	BlockID; 
	short	Version=2;
	char	txt[16]; 
	BOOL	ValidRec; 
	char	Name[MAX_PATH]; 
	LPSTR	pName;
	sqlite3 *db;

	NumIndexSyms = 0;
	RecsPerBlock = 1+NumSHPRecs/(NumIndexBlocks-1);
	
	_fstrcpy (Name,SHPFileName);
	ExpandText (Name);
	Fid = GSSiOpenFile (Name,0,OF_READ);
	if (Fid == HFILE_ERROR)
		return Fid;   
	hIndexBlocks = GSSiGlobAlloc (0,GMEM_MOVEABLE,NumIndexBlocks*sizeof(mnmxCor));
	BlockMinMax = (LPMINMAX)GlobalLock (hIndexBlocks); 
	for (i=0;i<NumIndexBlocks;i++)
		MinMaxInit (&BlockMinMax[i]);
	DisableHalt = TRUE;
	CreateStatusWind (CurView->hWnd,1,"Creating GeoMaster Shape File Index");      
	DisableHalt = FALSE;
	SHPIndexType = SHP_INDEX_STANDARD;
	CurView->PassID = 4;
	FidIdx = GSSiOpenFile (IndexName,0,OF_CREATE);
	db = NVShapeIndexCreate(IndexName);
	BigWrite (FidIdx,(HPSTR)&Version,2,-1); 
	BigWrite (FidIdx,(HPSTR)&NumIndexBlocks,4,-1); 
	BigWrite (FidIdx,(HPSTR)&RecsPerBlock,4,-1); 
	BigWrite (FidIdx,(HPSTR)BlockMinMax,NumIndexBlocks*sizeof(mnmxCor),-1);
	for (CurrentSHPRec=0;CurrentSHPRec<NumSHPRecs;CurrentSHPRec++)
	{   
    	SHPRecOffset = GetSHPRecordOffset (CurrentSHPRec,FALSE);
		if (SHPRecOffset == 65918)
			ii=1;

		IndexRecord.Offset = SHPRecOffset;
		ValidRec = ReadSHPRecordHeader (Fid,SHPRecOffset,&Bounds);
	    SetSHPParms (CurrentSHPRec); 
		BlockID = CurrentSHPRec/RecsPerBlock;
	    if (ValidRec)
	    { 
			IndexRecord.SymNum = CurrentDesc;  
			for (i=0;i<NumIndexSyms;i++)
				if (CurrentDesc == IndexSyms[i])
					goto HaveSym; 
			if (NumIndexSyms >= MAXINDEXSYMBOLS-1) 
			{
				GSSiMessageBox (0,"Maximum symbols exceeded",IndexName,MB_ICONEXCLAMATION,0); 
				break;
			}
			IndexSyms[NumIndexSyms++] = CurrentDesc;
	HaveSym:
			BoundsToMinMax (&IndexRecord.MinMax,&Bounds);
			AddMinMax (&BlockMinMax[BlockID], &IndexRecord.MinMax); 
		}
		else
		{
			IndexRecord.SymNum = -1;
			IndexRecord.Offset = -1;
		}
		BigWrite (FidIdx,(HPSTR)&IndexRecord,sizeof(IndexRecord),-1); 
		NVShapeIndexAdd(db, CurrentSHPRec,SHPRecOffset, &Bounds, IndexRecord.SymNum);
//		ltoa (CurrentSHPRec,txt,10);
		if (!(pName = strrchr (SHPFileName,'\\')))
			pName = SHPFileName;
		else
			pName++;
		StatusWindowUpdate (0,pName,NumSHPRecs,CurrentSHPRec); 
		if (!ContinueProcessing)
			break;
	} 
	SetContinueProcessing ( TRUE);  
	BigWrite (FidIdx,(HPSTR)IndexSyms,NumIndexSyms*2,-1);
	BigWrite (FidIdx,(HPSTR)&NumIndexSyms,2,-1);
	GSSillseek (FidIdx,10,0);
	BigWrite (FidIdx,(HPSTR)BlockMinMax,NumIndexBlocks*sizeof(mnmxCor),-1);
	GSSiClose2 (&Fid);   
	GSSiClose2 (&FidIdx);
	NVShapeIndexClose(db);
	FidIdx = GSSiOpenFile (IndexName,0,OF_READ);  
	SHPIndexType = SaveIndexType;
	GSSiGlobUlFree (&hIndexBlocks);
	DestroyStatusWindow(0); 
	return FidIdx;
}

int GetSHPIndexType(LPSTR Name)
{
#define DEFAULT_SHP_INDEX_TYPE	SHP_INDEX_SLT
	char name[MAX_PATH+1];
	strncpy0(name, Name,MAX_PATH);
	int type = SHP_INDEX_STANDARD;
	LPSTR pDot = strrchr(name, '.');
	if (pDot)
	{
		strcpy(pDot, ".nvi");
		if (GSSiLength(name)>0)
			type = SHP_INDEX_SLT;
		else
		{
			strcpy(pDot, ".gsi");
			if (GSSiLength(name) > 0)
				type = SHP_INDEX_SIMPLE;
			else
				type = DEFAULT_SHP_INDEX_TYPE;
		}
	}
	return type;
}
SHPHandle SHPOpenGSSi(const char * pszShapeFile, const char * pszAccess)
{
	SHPHandle hSHP = SHPOpen(pszShapeFile, pszAccess);

	if (hSHP)
	{
		ShapeXMin = hSHP->adBoundsMin[0];
		ShapeYMin = hSHP->adBoundsMin[1];
		ShapeXFactor = INT_MAX / (hSHP->adBoundsMax[0] - hSHP->adBoundsMin[0]);
		ShapeYFactor = INT_MAX / (hSHP->adBoundsMax[1] - hSHP->adBoundsMin[1]);
	}
	return hSHP;
}
MNMXCORL AdjustShapeBounds(LPMNMXCORD pBounds,BOOL Insert)
{
	MNMXCORL AdjustedBounds = { 0 };
	long halflong = INT_MAX / 2;

	AdjustedBounds.xmn = IDNINT((pBounds->xmn - ShapeXMin) * ShapeXFactor - halflong);
	AdjustedBounds.ymn = IDNINT((pBounds->ymn - ShapeYMin) * ShapeYFactor - halflong);
	AdjustedBounds.xmx = IDNINT((pBounds->xmx - ShapeXMin) * ShapeXFactor - halflong);
	AdjustedBounds.ymx = IDNINT((pBounds->ymx - ShapeYMin) * ShapeYFactor - halflong);
	if (!Insert)
	{
		double queryAdjustment = 1.00000012;
		double xMid = ((double)AdjustedBounds.xmn + (double)AdjustedBounds.xmx) / 2.0;
		double yMid = ((double)AdjustedBounds.ymn + (double)AdjustedBounds.ymx) / 2.0;
		double xWidth = queryAdjustment * ((double)AdjustedBounds.xmx - (double)AdjustedBounds.xmn) / 2.0;
		double yWidth = queryAdjustment * ((double)AdjustedBounds.ymx - (double)AdjustedBounds.ymn) / 2.0;
		AdjustedBounds.xmn = IDNINT(xMid - xWidth);
		AdjustedBounds.xmx = IDNINT(xMid + xWidth);
		AdjustedBounds.ymn = IDNINT(yMid - yWidth);
		AdjustedBounds.ymx = IDNINT(yMid + yWidth);
	}
	return AdjustedBounds;
}
BOOL OpenSHPFileIndex(LPSTR SHPFileName, HFILE SHPFid)
{ 
	char	Name[MAX_PATH];  
	LPSTR	pDot; 
	short	Version;   
	short	ii;
	BOOL	rtn = TRUE;
	BOOL haveSLTIndex = FALSE;

	if (!SHPFileName)
	{
		if (SHPIDXFid != HFILE_ERROR)
			GSSiClose2 (&SHPIDXFid);
		SHPIDXFid = HFILE_ERROR;
		GSSiGlobFree(&hSHPIndexBlocks);
		GetSHPRecordOffset(-1, FALSE);
		CloseDataFile(TRUE, &hSHPDBF);
		if (SHPIndexType == SHP_INDEX_SLT)
		{
			if (SHPstatement)
			{
				sqlite3_finalize(SHPstatement);
				SHPstatement = 0;
			}
			if (SHPIndexHandle)
				sqlite3_close(SHPIndexHandle);
			SHPIndexHandle = 0;
		}
		return TRUE;
	} 
	_fstrcpy (Name,SHPFileName);
	ExpandText (Name); 
	_fstrcpy (LastSHPFile,Name);
	pDot = _fstrrchr (Name,'.');
	if (!pDot)
		return FALSE;
	_fstrcpy (pDot,".shx");
	SHPIDXFid = GSSiOpenFile (Name,0,OF_READ);  
	NumSHPRecs = (GSSifilelength (SHPIDXFid) - 100) / 8; 
	sprintf (Name,"SHP=%s",SHPFileName);
	{
		long SaveSHPRec = CurrentSHPRec; 
		
		OpenDataFile (Name,"",BT_READ,&hSHPDBF); 
		CurrentSHPRec = SaveSHPRec;
	}
	SHPIndexType = GetSHPIndexType(LastSHPFile);
	if (SHPIndexType == SHP_INDEX_SIMPLE)
	{ 
		HFILE	TMPFid;
		
		_fstrcpy (Name,SHPFileName);  
		ExpandText (Name);
		pDot = _fstrrchr (Name,'.');
		if (!pDot)
			return FALSE;
		_fstrcpy (pDot,".gsi");
		TMPFid = GSSiOpenFile (Name,0,OF_READ); 
		if (TMPFid == HFILE_ERROR)
			TMPFid = CreateSHPFileIndex (Name,SHPFileName);
		else
		{
		    struct _stati64 stat; 
		    long	IndexTime;
		    double	dtime;
		     
		    GSSifstat (TMPFid,&stat); 
		    IndexTime = stat.st_mtime;
		    dtime = difftime (stat.st_mtime,SHPParmTime);  
		    if (dtime < 0)
		    {
		    	GSSiClose2 (&TMPFid);
				TMPFid = CreateSHPFileIndex (Name,SHPFileName);  
			}
			else
			{
				BOOL DoClose = TRUE;

				if (SHPFid == HFILE_ERROR)
					SHPFid=GSSiOpenFile (SHPFileName,0,OF_READ); 
				else
					DoClose = FALSE;
				
			    ii=GSSifstat (SHPFid,&stat);
			    dtime = difftime (stat.st_mtime,IndexTime);  
				if (DoClose)
					GSSiClose2 (&SHPFid);
			    if (dtime > 0)
			    {
			    	GSSiClose2 (&TMPFid);
					TMPFid = CreateSHPFileIndex (Name,SHPFileName); 
				}  
			}	
		}
		GSSiClose2 (&SHPIDXFid); 
		GetSHPRecordOffset (-1,FALSE);
		if (TMPFid == HFILE_ERROR)
			return FALSE; 
		SHPIDXFid = TMPFid;
		BigRead (SHPIDXFid,(HPSTR)&Version,2);
		if (Version != 2)
		{
			GSSiClose2 (&SHPIDXFid); 
			TMPFid = CreateSHPFileIndex (Name,SHPFileName);
			GSSiClose2 (&SHPIDXFid); 
			GetSHPRecordOffset (-1,FALSE);
			SHPIDXFid = TMPFid;
		}
		else
		{
			GSSillseek (SHPIDXFid,-2,2);
			GSSilread (SHPIDXFid,(HPSTR)&NumIndexSyms,2);
			GSSillseek (SHPIDXFid,-(NumIndexSyms+1)*2,2);
			GSSilread (SHPIDXFid,(HPSTR)&IndexSyms,NumIndexSyms*2);
			GSSillseek (SHPIDXFid,2,0);
		}
	}
	else if (SHPIndexType == SHP_INDEX_SLT)
	{
		HFILE	TMPFid;
		OFSTRUCTGM OFStruct;

		if (SHPIndexHandle)
			sqlite3_close(SHPIndexHandle);
		SHPIndexHandle = 0;

		_fstrcpy(Name, SHPFileName);
		ExpandText(Name);
		pDot = _fstrrchr(Name, '.');
		if (!pDot)
			return FALSE;
		_fstrcpy(pDot, ".nvi");
		TMPFid = GSSiOpenFile(Name, &OFStruct, OF_READ);
		if (TMPFid == HFILE_ERROR)
			haveSLTIndex = CreateShapeFileIndexSLT(SHPFileName,SHPTAG);
		else
		{
			struct _stati64 stat;
			long	IndexTime;
			double	dtime;

			GSSifstat(TMPFid, &stat);
			IndexTime = stat.st_mtime;
			dtime = difftime(stat.st_mtime, SHPParmTime);
			if (dtime < 0 || IndexTime < 1559413642 /*time when index changed to int32 coord*/)
			{
				GSSiClose2 (&TMPFid);
				GSSiRemove(Name);
				haveSLTIndex = CreateShapeFileIndexSLT(SHPFileName,SHPTAG);
			}
			else
			{
				BOOL DoClose = TRUE;

				if (haveSLTIndex)
					SHPFid = GSSiOpenFile(SHPFileName, 0, OF_READ);
				else
					DoClose = FALSE;

				ii = GSSifstat(SHPFid, &stat);
				dtime = difftime(stat.st_mtime, IndexTime);
				if (DoClose)
					GSSiClose2 (&SHPFid);
				if (dtime > 0)
				{
					GSSiClose2 (&TMPFid);
					GSSiRemove(Name);
					haveSLTIndex = CreateShapeFileIndexSLT(SHPFileName,SHPTAG);
				}
				else
					haveSLTIndex = TRUE;
			}
		}
		GSSiClose2 (&SHPIDXFid);
		GetSHPRecordOffset(-1, FALSE);
		if (TMPFid == HFILE_ERROR)
			return FALSE;
		SHPIDXFid = TMPFid;
	}
	if (haveSLTIndex)
	{
		//LONGLONG numRows=0;
		if (sqlite3_open(Name, &SHPIndexHandle) == SQLITE_OK)
		{
			/*if ((numRows = GetSQLITENumRows(SHPIndexHandle, "SHP", "")))
			{
			if (numRows != NumSHPRecs)
			ii = 1;
			}*/
		}
		else
			rtn = FALSE;
	}
	return rtn;
} 

long GetSHPRecordOffset (long record,BOOL UseBounds)
{
	long	loc, BlockID,ii;  
	static	HANDLE	hOffsets=0;
	static	HANDLE	hBlocks=0;
	static	long	FirstLoc, LastLoc,NumBlocks,RecsPerBlock, FirstRecLoc;
	long	Len,rtn;
	long	MaxLen= (((long)USHRT_MAX-100) / sizeof(SHPINDEXRECORD)) * sizeof(SHPINDEXRECORD); 
	LPSTR	pLoc, pLen;
	LPSHPINDEXRECORD	pIndexRec;
	LPMINMAX BlockMinMax;
	
	HaveSHPSym = -1;
	if (record < 0) 
	{
		GSSiGlobFree (&hOffsets);
		GSSiGlobFree (&hBlocks); 
		NumIndexSyms = 0;
		return -1; 
	} 
	if (!SHPProjectionIsBase && SHPIndexType != SHP_INDEX_SLT)
		UseBounds = FALSE;
	if (!UseBounds && record > NumSHPRecs - 1)
		return -1;
	if (record == 8190)
		ii=1; 
	if (CurView->PassID && CurView->PassID < 4)
	{
		if ((SHPType == SHPT_POLYGON || SHPType == SHPT_POLYGONM || SHPType == SHPT_POLYGONZ) && CurView->PassID != 2)
			return -1; 
		if ((SHPType == SHPT_POINT || SHPType == SHPT_POINTZ || SHPType == SHPT_ARC || SHPType == SHPT_ARCZ || SHPType == SHPT_ARCM) && CurView->PassID == 2)
			return -1; 
	}
	ItemSeg = record;  
	switch (SHPIndexType)
	{
	case SHP_INDEX_STANDARD:
//			MessageBox (0,"case 0","",MB_OK);
			loc = 100 + record * 8; 
			if (!hOffsets || loc < FirstLoc || loc > LastLoc)
			{   
				GSSiGlobFree (&hOffsets);
				hOffsets = GSSiGlobAlloc (1720,GMEM_MOVEABLE,MaxLen);
				pLoc = (LPSTR)GlobalLock (hOffsets);
				GSSillseek (SHPIDXFid,loc,0);
				Len = GSSilread (SHPIDXFid,pLoc,MaxLen); 
				GlobalUnlock (hOffsets);
				if (Len < 8)
				{
					GSSiGlobFree (&hOffsets);
					return -1;               
				}
				FirstLoc = loc;
				LastLoc = FirstLoc + Len - 1;
			}
			loc -= FirstLoc;
			pLoc = (LPSTR)GlobalLock (hOffsets);
			pLoc += loc;
			pLen = pLoc + 4;  
			loc = *(LPLONG)pLoc; 
			Len = *(LPLONG)pLen;
			GlobalUnlock (hOffsets);
			flip ((LPSTR)&loc,4); 
			flip ((LPSTR)&Len,4); 
			rtn = loc*2;
			if (rtn < 0)
				rtn = -2;
			return rtn;   
		
	case SHP_INDEX_SIMPLE:
			if (!hBlocks)
			{   
				ii=GSSillseek (SHPIDXFid,-2,2);
				GSSilread (SHPIDXFid,(HPSTR)&NumIndexSyms,2);
				GSSillseek (SHPIDXFid,-(NumIndexSyms+1)*2,2);
				GSSilread (SHPIDXFid,(HPSTR)&IndexSyms,NumIndexSyms*2);
				GSSillseek (SHPIDXFid,2,0);
				GSSilread (SHPIDXFid,(HPSTR)&NumBlocks,4);
				GSSilread (SHPIDXFid,(HPSTR)&RecsPerBlock,4);
				hBlocks = GSSiGlobAlloc (1721,GMEM_MOVEABLE,NumBlocks*sizeof(mnmxCor));
				BlockMinMax = (LPMINMAX)GlobalLock (hBlocks); 
				GSSilread (SHPIDXFid,(HPSTR)BlockMinMax,NumBlocks*sizeof(mnmxCor));
                GlobalUnlock (hBlocks); 
                FirstRecLoc = GSSillseek (SHPIDXFid,0,1); 
                _fmemset (BlockInWBounds,0,(int)NumBlocks);
            }
		Top:
			if (record == 652)
				ii=1;
			if (record > NumSHPRecs-1)
				return -1;
/*			{
				char	str[32];
				ltoa (record,str,10);
				SetWindowText (hWndMain,str);
			}*/
			ItemSeg = record; 
			if (!RecsPerBlock)
				return 0;
			BlockID = record / RecsPerBlock;
			loc = FirstRecLoc + record * sizeof(SHPINDEXRECORD); 
			if (UseBounds)
			{   
				if (!BlockInWBounds[BlockID])
				{
					BlockMinMax = (LPMINMAX)GlobalLock (hBlocks); 
					if (BlockInWindow (&BlockMinMax[BlockID],0)) 
						BlockInWBounds[BlockID]=1;
					else
						BlockInWBounds[BlockID]=2;
					GlobalUnlock (hBlocks);
				} 
				if (BlockInWBounds[BlockID] == 2)
				{
					record = (BlockID + 1) * RecsPerBlock;
					goto Top;
				}
			}
			if (!hOffsets || loc < FirstLoc || loc > LastLoc)
			{   
				GSSiGlobFree (&hOffsets);
				hOffsets = GSSiGlobAlloc (1422,GMEM_MOVEABLE,RecsPerBlock*sizeof(SHPINDEXRECORD));
				pLoc = (LPSTR)GlobalLock (hOffsets);
				GSSillseek (SHPIDXFid,loc,0);
				Len = GSSilread (SHPIDXFid,(HPSTR)pLoc,RecsPerBlock*sizeof(SHPINDEXRECORD)); 
				GlobalUnlock (hOffsets);
				if (Len < sizeof(SHPINDEXRECORD))
				{
					GSSiGlobFree (&hOffsets);
					return -1;               
				}
				FirstLoc = loc;
				LastLoc = FirstLoc + Len - 1;
			}
			loc -= FirstLoc;
			pLoc = (LPSTR)GlobalLock (hOffsets);
			pLoc += loc; 
			pIndexRec = (LPSHPINDEXRECORD) pLoc; 
			loc = pIndexRec->Offset;
			if (UseBounds)
			{ 
				if (!GetVisibility (pIndexRec->SymNum) || !BlockInWindow (&pIndexRec->MinMax,0))
				{
					GlobalUnlock (hOffsets); 
					record++;
					goto Top;
				}
//				else
//					HaveSHPSym = pIndexRec->SymNum; doesnt process size rot or color if set here
			}
			GlobalUnlock (hOffsets); 
			CurrentSHPRec = record;
			if (loc < 0)
				loc = -2;
			return loc;
			
		case SHP_INDEX_SLT:
		{
			loc = -1;
			LONGLONG rtn = 0;
			int symnum = 0;
			static int nread = 0;

			if (SHPIndexHandle)
			{
				if (!SHPstatement)
				{
					char cmd[1024];
					//LONGLONG count = GetSQLITENumRows(SHPIndexHandle, "SHP", "");
					nread = 0;
					if (UseBounds)
					{
						MNMXCORD shpBounds = CurView->WBounds;

						ConvertBounds(&shpBounds, 1, 0);
						MNMXCORL shpBoundsL = AdjustShapeBounds(&shpBounds,FALSE);

						sprintf(cmd, "SELECT RECNUM, symnum, offset FROM SHP, SHP_index WHERE SHP.RECNUM = SHP_index.id AND maxX >= %i AND minX <= %i AND maxY >= %i AND minY <= %i", shpBoundsL.xmn, shpBoundsL.xmx, shpBoundsL.ymn, shpBoundsL.ymx);
					}
					else
						sprintf(cmd, "SELECT RECNUM, symnum, offset FROM SHP WHERE RECNUM = %i", record);
					if (!SQLOK(sqlite3_prepare_v2(SHPIndexHandle, cmd, -1, &SHPstatement, 0), SHPIndexHandle, "get record offset", 0) == SQLITE_OK)
						return loc;
				}
				if (sqlite3_step(SHPstatement) == SQLITE_ROW)
				{
					CurrentSHPRec = sqlite3_column_int(SHPstatement, 0);
					ItemSeg = CurrentSHPRec;
					symnum = sqlite3_column_int(SHPstatement, 1);
					loc = sqlite3_column_int(SHPstatement, 2);
					nread++;
				}
				if (!UseBounds || loc < 0)
				{
					sqlite3_finalize(SHPstatement);
					SHPstatement = 0;
				}
			}
			else
				ii = 1;
			return loc;
		}
		default:
			return 0;
	}
	
}

BOOL ReadSHPRecordHeader (HFILE FidSHP,long RecordOffset,LPMNMXCORD pMinMaxCoord)
{   
	DPOINT		Points[4];  
	UINT		i;
	long		RecLen;
    
	GSSillseek (FidSHP,RecordOffset+4,0);
	BigRead (FidSHP,(HPSTR)&RecLen,4); 
	flip ((LPSTR)&RecLen,4);  
	switch (SHPType)
	{
		case SHPT_POINT:
	        if (BigRead (FidSHP,(HPSTR)&SHPPointRec,sizeof(SHPPointRec)) != sizeof(SHPPointRec))
	        	return FALSE; 
			if (!SHPPointRec.Type)
				return FALSE;
            if (pMinMaxCoord)
            { 
				DBoundsInit (pMinMaxCoord);
				Points[0] = SHPPointRec.Point; 
				if (ConvertCoord(&Points[0],0,1)) 
					return FALSE;
				AddDPointToMinMax (&Points[0],pMinMaxCoord);
			}  
		break;
		case SHPT_POINTZ:
	        if (BigRead (FidSHP,(HPSTR)&SHPPointZRec,sizeof(SHPPointZRec)) != sizeof(SHPPointZRec))
	        	return FALSE; 
			if (!SHPPointZRec.Type)
				return FALSE;
            if (pMinMaxCoord)
            { 
				DBoundsInit (pMinMaxCoord);
				Points[0] = SHPPointZRec.Point; 
				if (ConvertCoord(&Points[0],0,1)) 
					return FALSE;
				AddDPointToMinMax (&Points[0],pMinMaxCoord);
			}  
		break;
		
      	case SHPT_TEXT:
		case SHPT_ARC: 
		case SHPT_ARCM:   
		case SHPT_ARCZ:
		case 4:
		case SHPT_POLYGON:
		case SHPT_POLYGONZ:
//			if (RecLen < sizeof(SHPPolyHeader))
//				return FALSE;
            if (BigRead (FidSHP,(HPSTR)&SHPPolyHeader,sizeof(SHPPolyHeader)) != sizeof(SHPPolyHeader))
            	return FALSE; 
			if (!SHPPolyHeader.Type)
				return FALSE;
			if (!SHPPolyHeader.NumPoints)
				return FALSE;
            	
            	
            if (pMinMaxCoord)
            { 
				DBoundsInit (pMinMaxCoord);
				Points[0].x = SHPPolyHeader.Xmin;   
				Points[0].y = SHPPolyHeader.Ymin;   
				Points[1].x = SHPPolyHeader.Xmin;   
				Points[1].y = SHPPolyHeader.Ymax;   
				Points[2].x = SHPPolyHeader.Xmax;   
				Points[2].y = SHPPolyHeader.Ymax;   
				Points[3].x = SHPPolyHeader.Xmax;   
				Points[3].y = SHPPolyHeader.Ymin; 
				for (i=0;i<4;i++)
				{
					if (ConvertCoord(&Points[i],0,1))
					{   
	//				    MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
					    return FALSE;
					}
					AddDPointToMinMax (&Points[i],pMinMaxCoord);
				} 
			}
		break;
		case SHPT_MULTIPOINT:
		break;
	}  
	if (pMinMaxCoord)
	{
		GetFileMinMax(&CurrentItemMinMax, pMinMaxCoord);
		CurrentSHPRecBounds = *pMinMaxCoord;
	}
	return TRUE;
} 

BOOL ValidDPoint (LPDPOINT pVal)
{
	BYTE bytes[8];
	char	CVal[34];
	int	sign,dec;
	
	_fmemmove (bytes,&pVal->x,8);  
	_fstrcpy (CVal,_ecvt (pVal->x,32,&dec,&sign));
	if (_fstrstr (CVal,"QNAN"))
		return FALSE;
	return TRUE;
}

BOOL ReadPGDBRecordHeader (LPMNMXCORD pMinMaxCoord)
{   
	DPOINT		Points[4];  
	UINT		i;
	long		RecLen;  
	char		str[64];     
	HANDLE		hRec;
	HPSTR		pRec;
    
    sprintf (str,"[PGDB.%s]",ShapeFieldName);
    ExpandText (str);  
    hRec = (HANDLE) atol (str);   
    if (!hRec)
    	return FALSE; 
	switch (SHPType)
	{
		case SHPT_POINT:
//	        if (BigRead (FidSHP,&SHPPointRec,sizeof(SHPPointRec)) != sizeof(SHPPointRec))
//	        	return FALSE; 
		    pRec = GlobalLock (hRec);
		    hmemmove ((HPSTR)&SHPPointRec,pRec,sizeof(SHPPointRec));
		    GlobalUnlock (hRec);
			if (!SHPPointRec.Type || !ValidDPoint (&SHPPointRec.Point))
				return FALSE;
            if (pMinMaxCoord)
            { 
				DBoundsInit (pMinMaxCoord);
				Points[0] = SHPPointRec.Point; 
				if (ConvertCoord(&Points[0],0,1)) 
					return FALSE;
				AddDPointToMinMax (&Points[0],pMinMaxCoord);
			}  
		break;
		
		case 4:
      	case SHPT_TEXT:
		case SHPT_ARC:
		case SHPT_POLYGON:
		case SHPT_POLYGONZ:
//			if (RecLen < sizeof(SHPPolyHeader))
//				return FALSE;
//            if (BigRead (FidSHP,&SHPPolyHeader,sizeof(SHPPolyHeader)) != sizeof(SHPPolyHeader))
//            	return FALSE; 
		    pRec = GlobalLock (hRec);
		    hmemmove ((HPSTR)&SHPPolyHeader,pRec,sizeof(SHPPolyHeader));
		    GlobalUnlock (hRec);
		    switch (SHPPolyHeader.Type)
		    {
		    	case 0:
		    		return FALSE;
				case SHPT_POLYGON:
				case SHPT_POLYGONZ:
				case SHPT_PGDB_POLYGONZ:
				case SHPT_POLYGONM:
				case SHPT_ARC:
				case SHPT_ARCM: 
				case SHPT_ARCZ:
				case SHPT_POLYLINE_WITHCURVES: 
				case -1610612685:
				//return FALSE;
					break;
				case SHPT_POLYGON_WITHCURVES:
					break;
				default:
					return FALSE;
			} 
			if (!SHPPolyHeader.NumPoints)
				return FALSE;
            	
            	
            if (pMinMaxCoord)
            { 
				DBoundsInit (pMinMaxCoord);
				Points[0].x = SHPPolyHeader.Xmin;   
				Points[0].y = SHPPolyHeader.Ymin;   
				Points[1].x = SHPPolyHeader.Xmin;   
				Points[1].y = SHPPolyHeader.Ymax;   
				Points[2].x = SHPPolyHeader.Xmax;   
				Points[2].y = SHPPolyHeader.Ymax;   
				Points[3].x = SHPPolyHeader.Xmax;   
				Points[3].y = SHPPolyHeader.Ymin; 
				for (i=0;i<4;i++)
				{
					if (ConvertCoord(&Points[i],0,1))
					{   
	//				    MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
					    return FALSE;
					}
					AddDPointToMinMax (&Points[i],pMinMaxCoord);
				} 
			}
		break;
		case SHPT_MULTIPOINT:
		break;
	}  
	GetFileMinMax (&CurrentItemMinMax,pMinMaxCoord); 
	return TRUE;
} 

BOOL ReadFGDBRecordHeader (LPMNMXCORD pMinMaxCoord)
{   
	DPOINT		Points[4];  
	UINT		i;
	long		RecLen, rectype;  
	char		str[128];     
	HANDLE		hRec;
	LPBYTE		pRec;
	LPFILEGDBRECHEADER pFGDBRecHeader;
	LPCURVAL	pCurVal;

    strcpy (str,"[FGDB.Shape]");
    ExpandText (str);  
    hRec = (HANDLE) atol (str);  
    if (!hRec)
    	return FALSE; 
	pCurVal = GlobalLock (hRec);
    pRec = &pCurVal->Value;
	pFGDBRecHeader = (LPFILEGDBRECHEADER)pRec; //sizeof(FILEGDBRECHEADER)
	pRec += sizeof (FILEGDBRECHEADER);//sizeof(pRec) (pRec+48)
	rectype = *(LPLONG)pRec;
	switch(rectype)
	{
		default:
//			ii=1;
//			break;
		case shapePolylineZ:
		case SHPT_POLYGON:
		case SHPT_POLYGONZ:
		case SHPT_POINT:
		case SHPT_PGDB_POLYGONZ:
		case SHPT_POLYGONM:
		case SHPT_ARC:
		case SHPT_ARCM: 
		case SHPT_ARCZ:
		case SHPT_POLYLINE_WITHCURVES:
		case SHPT_POLYLINE_WITHCURVESANDZ:
		case SHPT_POLYGON_WITHCURVES:
		case  shapeGeneralPolyline:
		case -1610612685:
		case SHPT_MULTIPOINT:
			ii=0;
			break;
	}
	switch (pFGDBRecHeader->geometryType)
	{
  //geometryNull        = 0,
		case geometryPoint:
  //geometryMultipoint  = 2,
  //geometryMultiPatch  = 9,
//		case SHPT_POINT:
//	        if (BigRead (FidSHP,&SHPPointRec,sizeof(SHPPointRec)) != sizeof(SHPPointRec))
//	        	return FALSE; 
		    hmemmove ((HPSTR)&SHPPointRec,pRec,sizeof(SHPPointRec));
		    GlobalUnlock (hRec);
			if (!SHPPointRec.Type || !ValidDPoint (&SHPPointRec.Point))
				return FALSE;
            if (pMinMaxCoord)
            { 
				DBoundsInit (pMinMaxCoord);
				Points[0] = SHPPointRec.Point; 
				if (ConvertCoord(&Points[0],0,1)) 
					return FALSE;
				AddDPointToMinMax (&Points[0],pMinMaxCoord);
			}  
		break;
		
		case geometryPolyline:
		case geometryPolygon:
		//case SHPT_TEXT:
		//case SHPT_ARC:
		//case SHPT_POLYGON:
		//case SHPT_POLYGONZ:
//			if (RecLen < sizeof(SHPPolyHeader))
//				return FALSE;
//            if (BigRead (FidSHP,&SHPPolyHeader,sizeof(SHPPolyHeader)) != sizeof(SHPPolyHeader))
//            	return FALSE; 
		    hmemmove ((HPSTR)&SHPPolyHeader,pRec,sizeof(SHPPolyHeader));
		    GlobalUnlock (hRec);
		    switch (SHPPolyHeader.Type)
		    {
		    	case 0:
		    		return FALSE;
				case shapePolylineZ:
				case SHPT_POLYGON:
				case SHPT_POLYGONZ:
				case SHPT_PGDB_POLYGONZ:
				case SHPT_POLYGONM:
				case SHPT_ARC:
				case SHPT_ARCM: 
				case SHPT_ARCZ:
				case SHPT_POLYLINE_WITHCURVESANDZ:
				case SHPT_POLYLINE_WITHCURVES:
				case  shapeGeneralPolyline:
				case -1610612685:
				//return FALSE;
					break;
				case SHPT_POLYGON_WITHCURVES:
					break;
				default:
					break;
//					return FALSE;
			} 
			if (!SHPPolyHeader.NumPoints)
				return FALSE;
            	
            	
            if (pMinMaxCoord)
            { 
				DBoundsInit (pMinMaxCoord);
				Points[0].x = SHPPolyHeader.Xmin;   
				Points[0].y = SHPPolyHeader.Ymin;   
				Points[1].x = SHPPolyHeader.Xmin;   
				Points[1].y = SHPPolyHeader.Ymax;   
				Points[2].x = SHPPolyHeader.Xmax;   
				Points[2].y = SHPPolyHeader.Ymax;   
				Points[3].x = SHPPolyHeader.Xmax;   
				Points[3].y = SHPPolyHeader.Ymin; 
				for (i=0;i<4;i++)
				{
					if (ConvertCoord(&Points[i],0,1))
					{   
	//				    MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
					    return FALSE;
					}
					AddDPointToMinMax (&Points[i],pMinMaxCoord);
				} 
			}
		break;
		case SHPT_MULTIPOINT:
		break;
	}  
	GetFileMinMax (&CurrentItemMinMax,pMinMaxCoord); 
	return TRUE;
} 

BOOL ProcessSHPRecord (HDC hDC,HFILE FidSHP,long RecordNumber)
{
    HPDPOINT    pPoints, pFirstPoint;
    HPLONG      pPartIndex;   
    ULONG		i;
    short		Type, ltag;
    HANDLE		hPoints, hPartIndex;  
    LPINT		pNumPoints;
    long		NumPoints; 
    HPEN		hOldPen=0, hDeletePen=0, hTempPen=0;
	HBRUSH		hOldBrush=0, hTempBrush=0;  
	DPOINT		DPoint,DynSegEndPoint;
	double		size;
    static		short	dbugid=6;
    short		ii;
    static		int	nrecs=0;
	int			BorderSymNum;

	nrecs++;
	if (RecordNumber == 66)
		ii = 1;
    if (CurView->ID == dbugid)
    	ii=1;
	if (Pick)
		ii=1;
	ShowValue (hDC,FALSE);  
	InGraphicsProcessor = TRUE;
	ItemIsDeleted = FALSE;
	ItemIsRemoved = FALSE;              	
	InitRecord (hDC); 
	PolyIsHiPrecis = TRUE;
    SetSHPParms (RecordNumber);  
    if (!CurrentDesc)
    	goto RtnFalse;
	if (GRStartTime > TimeRangeEnd || GREndTime < TimeRangeBeg)
		goto RtnFalse;
    ltag = _fstrlen (SHPTag);
	SetSymNum (CurrentDesc);
	if (!ProcessRefAndTAG (GetVisibility (CurrentDesc),SHPTag,ltag))
		goto RtnFalse;
	if (!Pick && hDC && Display)
	{
		SetROP2(hDC,DisplayRasterOpt);
		if (HaveVarFillColor)
			SetTextColor (hDC,ConvertColor(GlobalColors[0],CurrentDesc));
		else
			SetTextColor (hDC,ConvertColor(DefaultTextColor,CurrentDesc));
		SelectObject (hDC,GetStockObject(BLACK_PEN));
		GSSiDeleteObject (&hBlackPen);
		hBlackPen = CreatePen (PS_SOLID,0,ConvertColor(0,CurrentDesc));		
		SelectObject (hDC,hBlackPen);
	}
	HiPrecis = TRUE; 
	CurrentPen = 0;
	switch (SHPType)
	{
		case SHPT_POINTZ:
			if (!SHPPointZRec.Type)
				goto RtnFalse;
			DPoint = SHPPointZRec.Point; 
		case SHPT_POINT:
		{
			int		SDCrtn=0;

			if (SHPType == SHPT_POINT)
			{
				if (!SHPPointRec.Type)
					goto RtnFalse;
				DPoint = SHPPointRec.Point; 
			}
			if (ConvertCoord(&DPoint,0,1)) 
				goto RtnFalse;
	        lpDCurPoints = &DPoint;  
			CurrentPoint = CurPointLocD = DPoint;
	        LastElementBeginPoint = LastElementEndPoint = DPoint;
	        nPnts = 1; 
			CurPointLoc = BasePtToWinPt(lpDCurPoints); 
    		if (!PointInMaskAreaWinCoord (CurPointLoc))
    			goto RtnFalse;
    		if (PointIsBlocked (&CurPointLocD,CurrentDesc))
    			goto RtnFalse;   
       		HaveTXLoc = TRUE;
    		CurrentType = GF_POINT; 
			if (CurPointSize < 0)
				CurPointSize = -CurPointSize * DeviceToScreenFactor();
			else
				CurPointSize /= CurView->BaseUnitsPerPixel;
			if ((Pick||PickingByRefno) && GetTypeVisibility(TYPE_POINT))
			{
				if (SetDisplayChar(hDC, GF_POINT, CurrentRefno, CurrentDesc, CurrentPrefix, CurrentUDI) > 0)
					PickPointItemD(lpDCurPoints, (CurPointSize*ThemeWidthFactor*GraphicsPointFactor)*CurView->BaseUnitsPerPixel, PTRot, CurrentDesc);
			}
			else if (GetTypeVisibility(TYPE_POINT))
			{   
				short	iDesc=CurrentDesc;
						
				if (CopyRec)
				{   
				    AddPointToBuffer (CurPointLocD,CurrentRefno,0,CurrentDesc,10, PTRot,0,0,0,
									  CurrentPrefix,CurrentUDI,-1,-1,-1,TRUE,&hUpdateBuf,&lUpdateBuf,0);
				}
				else
				{
	 				if (CurrentDesc > 0 && CurrentDesc < 3201)   
	 				{
						if (TSize)
							CurView->CurVisType[CurrentDesc]=5;
						else
							CurView->CurVisType[CurrentDesc]=4;
					}
					HighlightPointSym=FALSE;
					if (SHPColor >= 0)
					{  
						GlobalColors[0]=SHPColor;  
						HaveVarFillColor = TRUE;
					}
					if (!GetTypeVisibility(6) && SymbolIsVisible (iDesc)) 
					{
						CurPointSize = 10*DeviceToScreenFactor();
						iDesc = InvisiblePointSymbol;
					}
					if ((SDCrtn = SetDisplayChar (hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI)) > 0)
					{   
						if (ThemePointSym)
						{
							iDesc = ThemePointSym;
							if (ThemePointSize < 0)
								size = -ThemePointSize *DeviceToScreenFactor();
							else
								size = ThemePointSize / CurView->BaseUnitsPerPixel; 
							size *= ThemeWidthFactor;
							size = min(max (size,1),MaxPointSize);
						}
						else
							size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor; 
						{
							long	DisplayedWidth=0;

							DisplayPointItem (hDC,CurPointLoc,size,PTRot,iDesc,&DisplayedWidth);
							CurView->MaxSymbolWidth = max (CurView->MaxSymbolWidth,DisplayedWidth);
							CurView->MaxFileDisplayedPointWidth[FileNum] = max (CurView->MaxFileDisplayedPointWidth[FileNum],(DisplayedWidth/ FileDistToWinDist)-(((long)CurrentItemMinMax.xmx)-CurrentItemMinMax.xmn));
						}
		       			HaveTXLoc = 1;
					}
				}
			}
   			TXLoc = CurPointLocD; 
			if (SDCrtn < 0)
       			HaveTXLoc = 1;
		}
		break;
		
       	case SHPT_TEXT:
		case SHPT_ARC: 
		case SHPT_ARCM:
		case SHPT_ARCZ:
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=2; 
    		CurrentType = GF_POLYLINE; 
    		if (!GetTypeVisibility(TYPE_LINECURVE))
    			break;
			goto DoPoly;  
		case 4:
		case SHPT_POLYGON:
		case SHPT_POLYGONZ: 
		case SHPT_POLYGONM:  
    		if (!GetTypeVisibility(TYPE_AREA))
    			break;
    		CurrentType = GF_AREA; 
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=3; 
DoPoly:
			if (SHPColor >= 0)
			{  
				GlobalColors[0]=SHPColor;  
				HaveVarFillColor = TRUE;
			}
	        if (SHPPolyHeader.NumPoints > USHRT_MAX)
	        	ii=1;
	        	//goto RtnFalse;
	        if (!SHPPolyHeader.NumPoints)
	        	goto RtnFalse;
	        if (!SHPPolyHeader.Type)
	        	goto RtnFalse;
	        nPoly = SHPPolyHeader.NumParts; 
	        if (nPoly > 1)
	        	ii=1; 
	        NumPoints = SHPPolyHeader.NumPoints; 
	        if (SHPHeader.ShapeType == SHPT_POLYGON || SHPHeader.ShapeType == SHPT_POLYGONZ || SHPHeader.ShapeType == SHPT_POLYGONM)
	        	NumPoints = NumPoints+nPoly-1;
	        hPartIndex = GSSiGlobAlloc (1418,GMEM_MOVEABLE,sizeof(long)*(nPoly+1));
	        hPolyPartLen = GSSiGlobAlloc (1419,GMEM_MOVEABLE,sizeof(int)*(nPoly+1));
			hPoints = GSSiGlobAlloc (1420,GMEM_MOVEABLE,sizeof(DPOINT)*NumPoints); 
	        pPartIndex = (HPLONG)GlobalLock (hPartIndex); 
	        BigRead (FidSHP,(HPSTR)pPartIndex,(long)nPoly*sizeof(long)); //pPartIndex[0]
	        pPartIndex[SHPPolyHeader.NumParts]= SHPPolyHeader.NumPoints;
	                    
            pPoints = pFirstPoint = (LPDPOINT)GlobalLock (hPoints); 
            pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
	        for (i=0;i<nPoly;i++)
	        {   
	            long    numpoints, startpoint,ii; 
	                        
	            startpoint = *pPartIndex++;
	            numpoints = *pPartIndex-startpoint; 
	            if (numpoints > (long)USHRT_MAX)
					ii=1;
	            *pNumPoints++ = numpoints; 
	            if (BigRead (FidSHP,(HPSTR)pPoints,numpoints * sizeof(DPOINT)) != numpoints * sizeof(DPOINT))
	            	goto RtnFalse;
	            pPoints += numpoints;   
	            if (i && (SHPHeader.ShapeType == SHPT_POLYGON || SHPHeader.ShapeType == SHPT_POLYGONZ || SHPHeader.ShapeType == SHPT_POLYGONM))
	            	*pPoints++ = *pFirstPoint; 
	        } 
	        GlobalUnlock (hPoints); 
	        GlobalUnlock (hPolyPartLen);
	        GSSiGlobUlFree (&hPartIndex);  
			if (NeedToConvertCoord (0,1))
			{
				UINT	i;

				pPoints = (HPDPOINT)GlobalLock (hPoints);
				for (i=0;i<NumPoints;i++)
					ConvertCoord(&pPoints[i],0,1);
				GlobalUnlock (hPoints); 
			}
			if (Display && nPoly > 1 && !BoundsInBounds (&CurrentSHPRecBounds,&CurView->WBounds,0))
			{
				int	nPolyNew = 0, nPointsNew=0;
				HANDLE	hPolyPartLenNew = GSSiGlobAlloc (1783,GMEM_MOVEABLE,sizeof(int)*(nPoly+1));
				HANDLE	hPointsNew = GSSiGlobAlloc (1784,GMEM_MOVEABLE,sizeof(DPOINT)*NumPoints); 
				HPDPOINT	pPointsNew = (HPDPOINT)GlobalLock (hPointsNew);
				LPLONG		pNumPointsNew = (LPLONG)GlobalLock (hPolyPartLenNew);
				DPOINT	FirstPointNew;

				pPoints = (HPDPOINT)GlobalLock (hPoints);
	            pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
	            for (i=0;i<nPoly;i++)
	            { 
					MNMXCORD	IslandBounds;
						
					GetPolyBoundsD2 (pPoints,pNumPoints[i],&IslandBounds,0);
					if (BoundsInBounds (&IslandBounds,&CurView->WBounds,1))
					{
						if (!nPolyNew && (SHPHeader.ShapeType == SHPT_POLYGON || SHPHeader.ShapeType == SHPT_POLYGONZ || SHPHeader.ShapeType == SHPT_POLYGONM))
							FirstPointNew = *pPoints;
						pNumPointsNew[nPolyNew] = pNumPoints[i];
						hmemmove ((HPSTR)pPointsNew,(HPSTR)pPoints,sizeof(DPOINT)*((long)pNumPoints[i]));
						pPointsNew += pNumPoints[i];
						if (nPolyNew && (SHPHeader.ShapeType == SHPT_POLYGON || SHPHeader.ShapeType == SHPT_POLYGONZ || SHPHeader.ShapeType == SHPT_POLYGONM))
						{
	            			*pPointsNew++ = FirstPointNew; 
							nPointsNew++;
						}
						nPointsNew += pNumPoints[i];
						nPolyNew++;
					}
					pPoints += pNumPoints[i];
					if (i && (SHPHeader.ShapeType == SHPT_POLYGON || SHPHeader.ShapeType == SHPT_POLYGONZ || SHPHeader.ShapeType == SHPT_POLYGONM))
						pPoints++;
	            } 

		        GlobalUnlock (hPoints); 
		        GlobalUnlock (hPolyPartLen);
		        GlobalUnlock (hPointsNew); 
		        GlobalUnlock (hPolyPartLenNew);
				if (nPolyNew < nPoly)
				{
					GSSiGlobFree (&hPoints);
					GSSiGlobFree (&hPolyPartLen);
					hPoints = hPointsNew;
					hPolyPartLen = hPolyPartLenNew;
					nPoly = nPolyNew;
					NumPoints = nPointsNew;
				}
				else
				{
					GSSiGlobFree (&hPointsNew);
					GSSiGlobFree (&hPolyPartLenNew);
				}
			}
			pPoints = (HPDPOINT)GlobalLock (hPoints);
			if (NumPoints)
			{
				if (SHPHeader.ShapeType == SHPT_ARC || SHPHeader.ShapeType == SHPT_ARCM || SHPHeader.ShapeType == SHPT_ARCZ) 
				{
					if (Pick || PickingByRefno)
					{   
						pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
	        			for (i=0;i<nPoly;i++)
	        			{ 
	        				HPDPOINT	SavelpDCurPoints = lpDCurPoints;
							int	PolyID=i;

							if (nPoly > 1)
								PolyID++;
							nPnts = *pNumPoints;  
							PickPolylineD (pPoints,nPnts,PolyID,2,0,0,0);
							pPoints+=*pNumPoints++; 
						}   
						GlobalUnlock (hPolyPartLen); 
					}
					else if (CopyRec)
					{
						pNumPoints = (LPINT)GlobalLock(hPolyPartLen);//pNumPoints[1]
						if (nPoly > 1)
						{
							HANDLE	hhPoly = GSSiGlobAlloc(418, GMEM_MOVEABLE, sizeof(HANDLE)*nPoly);
							LPHANDLE phPoly = (LPHANDLE)GlobalLock(hhPoly);
							HPDPOINT	pPoints1 = (HPDPOINT)GlobalLock(hPoints), pPoints2;
							
							for (i = 0; i<nPoly; i++)
							{
								phPoly[i] = GSSiGlobAlloc(420, GMEM_MOVEABLE, sizeof(DPOINT)*(long)pNumPoints[i]);
								pPoints2 = (HPDPOINT)GlobalLock(phPoly[i]);
								hmemmove((HPSTR)pPoints2, (HPSTR)pPoints1, sizeof(DPOINT)*(long)pNumPoints[i]);
								GlobalUnlock(phPoly[i]);
								pPoints1 += pNumPoints[i];
							}
							GlobalUnlock(hPoints);
							AddPolyToBuffer(nPoly, pNumPoints, phPoly, TYPE_POLYLINE, CurrentRefno, 0, -1, CurrentDesc, 0, CurrentPrefix, CurrentUDI,
								-1, -1, 0, 0, 0, 0, 0, TRUE, &hUpdateBuf, &lUpdateBuf);
							for (i = 0; i<nPoly; i++)
								GSSiGlobFree(&phPoly[i]);
							GSSiGlobUlFree(&hhPoly);
						}
						else
							AddPolyToBuffer(nPoly, pNumPoints, &hPoints, TYPE_POLYLINE, CurrentRefno, 0, -1, CurrentDesc, 0, CurrentPrefix, CurrentUDI,
							-1, -1, 0, 0, 0, 0, 0, TRUE, &hUpdateBuf, &lUpdateBuf);
						GlobalUnlock(hPolyPartLen);
					}
					else
					{    
						lpDCurPoints = pPoints;
						nPnts = nCurPoints = NumPoints;

						if (PolyInMaskAreaFileCoord (GF_LINE,&NumPoints,0,0,&pPoints,TRUE))
						{   
							HPEN hNewPen=0;
							
							hOldPen  = SelectObject(hDC,GetStockObject(BLACK_PEN));
							TempLineColor = SHPColor; 
							TempLineWidth = SHPWidth; 
							if (TempLineColor >= 0 || TempLineWidth != 0)
							{
								hNewPen = CreatePen (PS_SOLID,(short)IDNINT(TempLineWidth*DeviceToScreenFactor()),TempLineColor);
								SelectObject(hDC,hNewPen);
							}
	//						if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
	//						{   
								pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
			        			for (i=0;i<nPoly;i++)
			        			{ 
			        				HPDPOINT	SavelpDCurPoints = lpDCurPoints;
  
	//								GWPolylineD (hDC,lpDCurPoints,*pNumPoints,CurrentDesc); 
	//								lpDCurPoints+=*pNumPoints++;
			        				nPnts = *pNumPoints;  
			        				InDynamicSegmentation = FALSE;  
			        				DoDynamicFixedSegmentation (-1,0,0);
									while (nPnts > 1)
									{ 
										if (!WantSegmentID || WantSegmentID-1 == i)
										{
											if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0)
											{   
												GWPolylineD (hDC,lpDCurPoints,NumDynSegPoints,CurrentDesc); 
											}
										}
										else
											NumDynSegPoints = nPnts;
										DynSegEndPoint = lpDCurPoints[NumDynSegPoints-1];  
										lpDCurPoints += (NumDynSegPoints - 2);
										nPnts = NumDynSegPointsRemaining;
										if (nPnts > 1)
										{   
											lpDCurPoints[0] = DynSegEndPoint;
											lpDCurPoints[1] = DynSegSavePoint;  
											InDynamicSegmentation = TRUE;
										}	
									}
			        				InDynamicSegmentation = FALSE;
			        				lpDCurPoints = SavelpDCurPoints;
									lpDCurPoints+=*pNumPoints++; 
				        		}   
			        			GlobalUnlock (hPolyPartLen); 
	//						} 
							if (hNewPen)
							{
								SelectObject(hDC,GetStockObject(BLACK_PEN)); 
								GSSiDeleteObject (&hNewPen);
							}
						}
					}
				}
				else
				{   
					BOOL	ShowBorder = GetBit (5,(LPSTR)&CurVis->WantType[7]); 
					HPEN	hBorderPen=0;
					
					if (Pick)
					{   
						PickPolygonD (pPoints,NumPoints,nPoly,hPolyPartLen,9999999,0);
					}
					else if (CopyRec)  
					{
						pNumPoints = (LPINT)GlobalLock (hPolyPartLen);//pNumPoints[1]
						if (nPoly > 1)
						{
							HANDLE	hhPoly = GSSiGlobAlloc ( 418,GMEM_MOVEABLE,sizeof(HANDLE)*nPoly); 
							LPHANDLE phPoly = (LPHANDLE)GlobalLock (hhPoly);
							HPDPOINT	pPoints1=(HPDPOINT)GlobalLock (hPoints), pPoints2;
                        
							for (i=0;i<nPoly;i++)
							{   
	                   			phPoly[i] = GSSiGlobAlloc ( 420,GMEM_MOVEABLE,sizeof(DPOINT)*(long)pNumPoints[i]);
								pPoints2= (HPDPOINT)GlobalLock (phPoly[i]);  
								hmemmove ((HPSTR)pPoints2,(HPSTR)pPoints1,sizeof(DPOINT)*(long)pNumPoints[i]);
								GlobalUnlock (phPoly[i]); 
								pPoints1 += pNumPoints[i];
								if (i)
	                        		pPoints1++;
							} 
							GlobalUnlock (hPoints);
							AddPolyToBuffer (nPoly,pNumPoints,phPoly,TYPE_AREA,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
											 -1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
							for (i=0;i<nPoly;i++)
		                		GSSiGlobFree (&phPoly[i]);
							GSSiGlobUlFree (&hhPoly);
						}
						else
							AddPolyToBuffer (nPoly,pNumPoints,&hPoints,TYPE_AREA,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
                                     		-1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
						GlobalUnlock (hPolyPartLen);
					}
					else
					{ 
						
						if (Display)
						{
							hOldPen  = SelectObject(hDC,h0Pen);
							if (ShowBorder && !GetTypeVisibility(TYPE_SYMBOL))
		    					hBorderPen = hAreaBorderPen[TRUE];
			        		if (GetInVisibility(CurrentDesc))
			        		{
			        			if (CurView->HaveLayerColor[FileNum])
			        			{
			        				hTempBrush = CreateSolidBrush(CurView->LayerColor[FileNum]);
									hOldBrush = SelectObject (hDC,hTempBrush);
									hTempPen = CreatePen (PS_SOLID,0,CurView->LayerColor[FileNum]);
									SelectObject (hDC,hTempPen);
								}
			        			else
									hOldBrush = SelectRandomBrush (hDC,CurrentRefno,&hBorderPen,CurrentDesc); 
							}
							//if (hBorderPen)
							//	SelectObject(hDC,hBorderPen); 
						}
						lpDCurPoints = pPoints;
						nPnts = nCurPoints = NumPoints;

						if (PolyInMaskAreaFileCoord (GF_AREA,&NumPoints,0,0,&pPoints,TRUE))
						{
							int	SDCrtn = SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI);
							
							if (SDCrtn > 0)
								ProcessPolygon (hDC,ShowBorder,PltType,hBorderPen,hTempPen,0);
							else if (SDCrtn < 0)
								HaveTXLoc = TRUE;
							/*
							if (SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0)
							{   
								BOOL	DoBorder = GetBit (5,(LPSTR)&CurVis->WantType[7]);
								
								if (Display)
								{
									int	StartPoly = 0;

									if (nPoly > 1 && !ShowLinkLines)
										SelectObject(hDC,h0Pen); 
									if (FillAreas && (GetBit (7,(LPSTR)&CurVis->WantType[7]) ||
													  GetBit (6,(LPSTR)&CurVis->WantType[7])))
									{
										hDeletePen = GWPolygonD (hDC,pPoints, NumPoints, nPoly, hPolyPartLen,CurrentDesc,ShowBorder,TRUE,0);
										// cant remember what this is for //StartPoly = 1;
									}
									else
									{
										DoBorder = TRUE;
										if (!StartPoly)
											SetAreaPenAndBrush (hDC,0,CurrentDesc,ItemIsHighlighted,ShowBorder,&hDeletePen,0,&BorderSymNum);
									}
									if ((DoBorder || nPoly > 1) && (hBorderPen || hDeletePen))
									{   
										SetROP2(hDC,DisplayRasterOpt);
										if (hDeletePen)
											SelectObject(hDC,hDeletePen); 
										else
											SelectObject(hDC,hBorderPen); 
										pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
					        			for (i=0;i<nPoly;i++)
					        			{   
											if (i >= StartPoly)
												GWPolylineD (hDC,lpDCurPoints,*pNumPoints,0); 
											lpDCurPoints+=*pNumPoints++;
											if (i)
												lpDCurPoints++;
					        			}   
					        			GlobalUnlock (hPolyPartLen); 
					        		}
								}
							}*/
						}
					}
				}
			}
			GSSiGlobFree (&hPolyPartLen);
			GSSiGlobUlFree (&hPoints);
		break;
		
		case SHPT_MULTIPOINT:
		break;
	}
	if (hOldBrush)
		SelectObject (hDC,hOldBrush);
	if (hOldPen)
		SelectObject (hDC,hOldPen); 
   	if (hDeletePen != h0Pen)
   		GSSiDeleteObject (&hDeletePen);
   	GSSiDeleteObject (&hTempBrush);
   	GSSiDeleteObject (&hTempPen);
	ShowValue (hDC,FALSE);
	InGraphicsProcessor = FALSE;
	return TRUE;  
RtnFalse:
	InGraphicsProcessor = FALSE;
	return FALSE;
}

long ReadSHPHeader (HFILE FidSHP,LPMNMXCORD pMinMaxCoord,LPSTR FileName)
{
	DPOINT		Points[4];  
	UINT		i;  
	int			lRead;
	char		ShpType[16]="";
	
	//int ipos2,ipos1 = GSSillseek (FidSHP,0,1);

	GSSillseek (FidSHP,0,0);
	lRead = BigRead (FidSHP,(HPSTR)&SHPHeader,(UINT)sizeof(SHPHeader)); 
	//ipos2 = GSSillseek (FidSHP,0,1);
	if (lRead == sizeof(SHPHeader))
	{
		flip ((LPSTR)&SHPHeader.FileCode,4);
		flip ((LPSTR)&SHPHeader.FileLength,4);  

		switch (SHPHeader.ShapeType)
		{
			case SHPT_NULL: 
				_fstrcpy (ShpType,"NULL");
				break;
			case SHPT_MULTIPOINT:
				_fstrcpy (ShpType,"MULTIPOINT");
				break;
			//case SHPT_POINTZ:
			//	_fstrcpy (ShpType,"POINTZ");
			//	break;
			case SHPT_MULTIPOINTZ:
				_fstrcpy (ShpType,"MULTIPOINTZ");
				break;
			case SHPT_POINTM:
				_fstrcpy (ShpType,"POINTM");
				break;
			case SHPT_MULTIPOINTM:
				_fstrcpy (ShpType,"MULTIPOINTM");
				break;
			case SHPT_MULTIPATCH:
				_fstrcpy (ShpType,"MULTIPATCH");
				break;
		}
	}
	else
		strcpy (ShpType,"Invalid file");
	if (*ShpType) 
	{   
		char	mess[280];
			
		sprintf (mess,"Shape File: %s\r\nShape file type %s not yet implemented",FileName,ShpType);
		GSSiMessageBox (0,mess,0,MB_ICONEXCLAMATION,0);
		return FALSE;
	}
	flip ((LPSTR)&SHPHeader.FileCode,4);
	flip ((LPSTR)&SHPHeader.FileLength,4);  
	if (pMinMaxCoord)
	{
		DBoundsInit (pMinMaxCoord);
		Points[0].x = SHPHeader.Xmin;   
		Points[0].y = SHPHeader.Ymin;   
		Points[1].x = SHPHeader.Xmin;   
		Points[1].y = SHPHeader.Ymax;   
		Points[2].x = SHPHeader.Xmax;   
		Points[2].y = SHPHeader.Ymax;   
		Points[3].x = SHPHeader.Xmax;   
		Points[3].y = SHPHeader.Ymin; 
		for (i=0;i<4;i++)
		{
			AddDPointToMinMax (&Points[i],pMinMaxCoord); 
		} 
	}
	return SHPHeader.ShapeType;
}   

BOOL OpenFGDB (LPSTR DBName,LPSTR Table,LPSTR SQL)
{   
	BOOL	rtn=TRUE;
	char	dbName[MAX_PATH+8];
	
    CloseDataFile (TRUE, &FGDBHandle); 
    hSHPDBF = 0; 
	if (DBName)
    {  
    	if (Table)
        	sprintf (dbName,"FGDB=%s(%s)",DBName,Table);
        else
			sprintf (dbName,"FGDB=%s",DBName);
        rtn = OpenDataFile (dbName,SQL,BT_READ,&FGDBHandle); 
        if (rtn)
        	hSHPDBF = FGDBHandle;
    }
    return rtn;
}   
   

BOOL OpenPGDB (LPSTR DBName,LPSTR Table,LPSTR SQL)
{   
	BOOL	rtn=TRUE;
	
    CloseDataFile (TRUE, &PGDBHandle); 
    hSHPDBF = 0; 
	if (DBName)
    {  
    	BOOL	SaveUseLongVarBinary = UseLongVarBinary;  
    	
    	UseLongVarBinary = TRUE;   
    	if (Table)
        	sprintf (PGDBName,"PGDB=ODBC|MS Access Database;DBQ=%s|%s",DBName,Table);
        else
        	sprintf (PGDBName,"PGDB=ODBC|MS Access Database;DBQ=%s",DBName);
        rtn = OpenDataFile (PGDBName,SQL,BT_READ,&PGDBHandle); 
        UseLongVarBinary = SaveUseLongVarBinary;    
        if (rtn)
        	hSHPDBF = PGDBHandle;
    }
    return rtn;
}   


void SetPGDB_SQL (LPSTR SQL)
{
	LPOPENSQLDATA	SQLPtr;
	LPOPENFILEDATA	FilePtr;

	_fstrcpy (PGDB_SQL,SQL);
	if (PGDBHandle)
	{
		SQLPtr = (LPOPENSQLDATA)GlobalLock(PGDBHandle);
		FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
		ProcessFileSQL (SQLPtr,FilePtr,SQL);   
		SQLPtr->lastreadtime = 0;                                     
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (PGDBHandle);
	}
	return;
}

void SetFGDB_SQL (LPSTR SQL)
{
	LPOPENSQLDATA	SQLPtr;
	LPOPENFILEDATA	FilePtr;

	_fstrcpy (FGDB_SQL,SQL);
/*	if (FGDBHandle)
	{
		SQLPtr = (LPOPENSQLDATA)GlobalLock(FGDBHandle);
		FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
		ProcessFileSQL (SQLPtr,FilePtr,SQL);   
		SQLPtr->lastreadtime = 0;                                     
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (FGDBHandle);
	}*/
	return;
}

BOOL OpenPGDBFileIndex (LPSTR DBNameIN,LPMNMXCORD WBounds)
{
	char	DBName[512],str[512];
    LPSTR	pPar;
    
	_fstrcpy (DBName,DBNameIN);
    if ((pPar = _fstrrchr (DBName,'(')))
    {
    	*pPar++ = 0; 
    	_fstrcpy (PGDBTable,pPar);
    	*LastChr (PGDBTable) = 0;
    }
    else
    	*PGDBTable = 0;
    if (!*PGDBTable)
    	return FALSE;  
    _fstrcpy (LastSHPFile,DBNameIN);
    if (WBounds)
    {   
    	BOOL	st;  
    	long	MinGx,MaxGx,MinGy,MaxGy;
		LPSTR	pFields;
    	
    	sprintf (str,"TABLENAME = '%s'",PGDBTable);
		st = OpenPGDB (DBName,"GDB_GeomColumns",str); 
		hSelectFields = GSSiGlobAlloc (GMEM_MOVEABLE,0,1024);
		pFields = GlobalLock (hSelectFields);
		strcpy (pFields,"TableName,FieldName,ShapeType,ExtentLeft,ExtentRight,ExtentBottom,ExtentTop,IdxOriginX,IdxOriginY,IdxGridSize");
		GlobalUnlock (hSelectFields);
		_fstrcpy (str,"[PGDB.IdxOriginX]");
		ExpandText (str);
		PGDBGridOrigX = atof (str);
		_fstrcpy (str,"[PGDB.IdxOriginY]");
		ExpandText (str);
		PGDBGridOrigY = atof (str);
		_fstrcpy (str,"[PGDB.IdxGridSize]");
		ExpandText (str);
		PGDBGridSize = atof (str); 
		MinGx = ((WBounds->xmn - PGDBGridOrigX) / PGDBCnvFac) / PGDBGridSize;
		MaxGx = ((WBounds->xmx - PGDBGridOrigX) / PGDBCnvFac) / PGDBGridSize;
		MinGy = ((WBounds->ymn - PGDBGridOrigY) / PGDBCnvFac) / PGDBGridSize;
		MaxGy = ((WBounds->ymx - PGDBGridOrigY) / PGDBCnvFac) / PGDBGridSize;
		sprintf (str,  
					"(SELECT * FROM %s_SHAPE_Index INNER JOIN %s ON %s_SHAPE_Index.IndexedObjectID = %s.ObjectID WHERE MaxGX >= %ld AND MinGX <= %ld AND MaxGY >= %ld AND MinGY <= %ld)",
					PGDBTable,PGDBTable,PGDBTable,PGDBTable,MinGx,MaxGx,MinGy,MaxGy);	
		SetPGDB_SQL (str);	
		return OpenPGDB (DBName,0,PGDB_SQL);
    } 
    else
		return OpenPGDB (DBName,PGDBTable,PGDB_SQL);
}  

BOOL OpenFGDBFileIndex (LPSTR DBNameIN,LPMNMXCORD WBounds)
{
	char	DBName[512],str[512];
    LPSTR	pPar;
    
	_fstrcpy (DBName,DBNameIN);
    if ((pPar = _fstrrchr (DBName,'(')))
    {
    	*pPar++ = 0; 
    	_fstrcpy (FGDBTable,pPar);
    	*LastChr (FGDBTable) = 0;
    }
    else
    	*FGDBTable = 0;
    if (!*FGDBTable)
    	return FALSE;  
    _fstrcpy (LastSHPFile,DBNameIN);
    if (WBounds)
    { 
		MNMXCORD	bounds;

		ConvertRectCoord (&bounds, &CurView->WBounds, 1,0);
		strcpy (str,"BOUNDS=(");
		boundstoa (strchr(str,0),&bounds);
		strcat (str,")");
		SetFGDB_SQL (str);	
		return OpenFGDB (DBName,FGDBTable,FGDB_SQL);
    } 
    else
		return OpenFGDB (DBName,FGDBTable,FGDB_SQL);
}  

BOOL PGDBTableIsText (LPSTR DBName,LPSTR TableName)
{
    BOOL	rtn=FALSE;
    
	{ 
		LPOPENFILEDATA	FilePtr;
		LPOPENSQLDATA	SQLPtr; 
		LPFIELDINFO		pField; 
		UINT	j;
			
		SQLPtr = (LPOPENSQLDATA)GlobalLock (PGDBHandle);
        FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);  
        pField = &FilePtr->FldInfo;
        for (j=0;j<FilePtr->NumFields;j++,pField++)
        	if (!_fstricmp (pField->name,"AnnotationClassID"))
        		rtn=TRUE;
        GlobalUnlock (SQLPtr->OFHandle); 
		GlobalUnlock (PGDBHandle);
	}
    return rtn;
}

BOOL FGDBTableIsText (LPSTR DBName,LPSTR TableName)
{
    BOOL	rtn=FALSE;
    
	{ 
		LPOPENFILEDATA	FilePtr;
		LPOPENSQLDATA	SQLPtr; 
		LPFIELDINFO		pField; 
		UINT	j;
			
		SQLPtr = (LPOPENSQLDATA)GlobalLock (FGDBHandle);
        FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);  
        pField = &FilePtr->FldInfo;
        for (j=0;j<FilePtr->NumFields;j++,pField++)
        	if (!_fstricmp (pField->name,"AnnotationClassID"))
        		rtn=TRUE;
        GlobalUnlock (SQLPtr->OFHandle); 
		GlobalUnlock (FGDBHandle);
	}
    return rtn;
}

long ReadPGDBHeader (LPSTR DBNameIN,LPMNMXCORD pMinMaxCoord)
#if ENABLETRACE
{GSSiEnterProg (1376);
#endif
{ 
	DPOINT		Points[4];  
	UINT		i;  
	char		ShpType[16]="";
	char	DBName[512],SQL[128],str[128];
	LPSTR	pPar; 
	short	ShapeType; 
	double	Xmin,Xmax,Ymin,Ymax;
	LPSTR	pFields;

	_fstrcpy (DBName,DBNameIN); 
	ExpandText (DBName);
    if ((pPar = _fstrrchr (DBName,'(')))
    {
    	*pPar++ = 0; 
    	_fstrcpy (PGDBTable,pPar);
    	*LastChr (PGDBTable) = 0;
    }
    else
    	*PGDBTable = 0;
    if (!*PGDBTable)
    	goto RtnFalse;
    sprintf (SQL,"TableName = '%s'",PGDBTable);

	if (!OpenPGDB (DBName,"GDB_GeomColumns",SQL))
		goto RtnFalse;
	hSelectFields = GSSiGlobAlloc (GMEM_MOVEABLE,0,1024);
	pFields = GlobalLock (hSelectFields);
	strcpy (pFields,"TableName,FieldName,ShapeType,ExtentLeft,ExtentRight,ExtentBottom,ExtentTop");
	GlobalUnlock (hSelectFields);
	if (!FetchDBRec (PGDBHandle))
		goto RtnFalse;
	_fstrcpy (str,"[PGDB.ShapeType]"); 
	ExpandText (str);
	ShapeType = atoi (str);
	_fstrcpy (ShapeFieldName,"[PGDB.FieldName]"); 
	ExpandText (ShapeFieldName);
	switch (ShapeType)
	{
		case SHPT_NULL: 
			_fstrcpy (ShpType,"NULL");
			break;
		case SHPT_MULTIPOINT:
			_fstrcpy (ShpType,"MULTIPOINT");
			break;
		case SHPT_POINTZ:
			_fstrcpy (ShpType,"POINTZ");
			break;
		case SHPT_MULTIPOINTZ:
			_fstrcpy (ShpType,"MULTIPOINTZ");
			break;
		case SHPT_POINTM:
			_fstrcpy (ShpType,"POINTM");
			break;
		case SHPT_MULTIPOINTM:
			_fstrcpy (ShpType,"MULTIPOINTM");
			break;
		case SHPT_MULTIPATCH:
			_fstrcpy (ShpType,"MULTIPATCH");
			break;
	}
	if (*ShpType) 
	{   
		char	mess[64];
			
		sprintf (mess,"Shape file type %s not yet implemented",ShpType);
		GSSiMessageBox (0,mess,0,MB_ICONEXCLAMATION,0);
		OpenPGDB (0,0,0);
		goto RtnFalse;
	}
	if (pMinMaxCoord)
	{
		DBoundsInit (pMinMaxCoord); 
		_fstrcpy (str,"[PGDB.ExtentLeft]");
		ExpandText (str);
		Xmin = atof (str);
		_fstrcpy (str,"[PGDB.ExtentRight]");
		ExpandText (str);
		Xmax = atof (str);
		_fstrcpy (str,"[PGDB.ExtentBottom]");
		ExpandText (str);
		Ymin = atof (str);
		_fstrcpy (str,"[PGDB.ExtentTop]");
		ExpandText (str);
		Ymax = atof (str);
		if (Xmin > Xmax || Ymin > Ymax)
		{
			OpenPGDB (0,0,0);
			goto RtnFalse;
		}
		Points[0].x = Xmin;   
		Points[0].y = Ymin;   
		Points[1].x = Xmin;   
		Points[1].y = Ymax;   
		Points[2].x = Xmax;   
		Points[2].y = Ymax;   
		Points[3].x = Xmax;   
		Points[3].y = Ymin; 
		for (i=0;i<4;i++)
		{
			AddDPointToMinMax (&Points[i],pMinMaxCoord); 
		} 
	}
	if (OpenPGDB (DBName,PGDBTable,""))
	{ 
		if (PGDBTableIsText (DBName,PGDBTable))
			ShapeType = SHPT_TEXT;  
	}
	OpenPGDB (0,0,0); 
{
#if ENABLETRACE
GSSiExitProg (1376);
#endif
	return ShapeType;
}
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (1376);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
}
long ReadFGDBHeader (LPSTR DBNameIN,LPMNMXCORD pMinMaxCoord)
#if ENABLETRACE
{GSSiEnterProg (1376);
#endif
{ 
	DPOINT		Points[4];  
	UINT		i;  
	char		ShpType[16]="";
	char	DBName[512],SQL[128],str[128];
	LPSTR	pPar; 
	int		ShapeType; 
	double	Xmin,Xmax,Ymin,Ymax;
	LPSTR	pFields;
	int		iDB;
	BOOL	rc;

	_fstrcpy (DBName,DBNameIN); 
	ExpandText (DBName);
    if ((pPar = _fstrrchr (DBName,'(')))
    {
    	*pPar++ = 0; 
    	_fstrcpy (FGDBTable,pPar);
    	*LastChr (FGDBTable) = 0;
    }
    else
    	*FGDBTable = 0;
    if (!*FGDBTable)
    	goto RtnFalse;

	iDB = OpenFGDB2 (DBName,"","");
	if (iDB < 1)
		goto RtnFalse;
	rc = FGDBGetTableInfo (iDB,FGDBTable,&ShapeType,0,pMinMaxCoord);
	CloseFGDB (iDB);
	if (!rc)
		goto RtnFalse;

	switch (ShapeType)
	{
		case SHPT_NULL: 
			_fstrcpy (ShpType,"NULL");
			break;
		case SHPT_MULTIPOINT:
			_fstrcpy (ShpType,"MULTIPOINT");
			break;
		case SHPT_POINTZ:
			_fstrcpy (ShpType,"POINTZ");
			break;
		case SHPT_MULTIPOINTZ:
			_fstrcpy (ShpType,"MULTIPOINTZ");
			break;
		case SHPT_POINTM:
			_fstrcpy (ShpType,"POINTM");
			break;
		case SHPT_MULTIPOINTM:
			_fstrcpy (ShpType,"MULTIPOINTM");
			break;
		case SHPT_MULTIPATCH:
			_fstrcpy (ShpType,"MULTIPATCH");
			break;
	}
	if (*ShpType) 
	{   
		char	mess[64];
			
		sprintf (mess,"Shape file type %s not yet implemented",ShpType);
		GSSiMessageBox (0,mess,FGDBTable,MB_ICONEXCLAMATION,0);
		OpenPGDB (0,0,0);
		goto RtnFalse;
	}
{
#if ENABLETRACE
GSSiExitProg (1376);
#endif
	return ShapeType;
}
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (1376);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
}


long ListPGDBTables (LPSTR DBNameIN,HWND hWndDlg,UINT ListCntl,int ListType)
{
	UINT		i;  
	char		ShpType[16]="";
	char	DBName[512],str[256], TableName[128], TableType[64];
	LPSTR	pPar, pTab; 
	short	ShapeType; 
	double	Xmin,Xmax,Ymin,Ymax;  
	short	Num=0;        
	long	NumRows; 
	int   	TabStops[3]={150,200,250};
	LPSTR	pFields;
 
   	SendDlgItemMessage (hWndDlg,ListCntl,LB_SETTABSTOPS,3,(LPARAM)&TabStops); 
	_fstrcpy (DBName,DBNameIN); 
	ExpandText (DBName);
    if ((pPar = _fstrrchr (DBName,'(')))
    	*pPar++ = 0; 
	if (!OpenPGDB (DBName,"GDB_GeomColumns",""))
		return 0;
	hSelectFields = GSSiGlobAlloc (GMEM_MOVEABLE,0,1024);
	pFields = GlobalLock (hSelectFields);
	strcpy (pFields,"TableName,FieldName,ShapeType,ExtentLeft,ExtentRight,ExtentBottom,ExtentTop,IdxOriginX,IdxOriginY,IdxGridSize");
	GlobalUnlock (hSelectFields);
	while (FetchDBRec (PGDBHandle))
	{
		_fstrcpy (str,"[PGDB.ShapeType]"); 
		ExpandText (str);
		ShapeType = atoi (str);
		_fstrcpy (ShapeFieldName,"[PGDB.FieldName]"); 
		ExpandText (ShapeFieldName);    
		_fstrcpy (TableName,"[PGDB.TableName]"); 
		ExpandText (TableName);    
		*ShpType = 0;  
		
		switch (ShapeType)
		{
			case 1:
				_fstrcpy (TableType,"Point");
				break;
			case 3:
				_fstrcpy (TableType,"Line");  
				break;
			case 4:
				_fstrcpy (TableType,"Area");
				break;
			default:
				sprintf (TableType,"Unknown(%i)",ShapeType);
				break;
		}
		Num++;   
		sprintf (str,"%s\t%s",TableName,TableType);
		SendDlgItemMessage ((HWND)hWndDlg,ListCntl,LB_ADDSTRING,0,(LPARAM) str); 
	}
	OpenPGDB (0,0,0); 
	for (i=0;i<Num;i++)
	{ 
        SendDlgItemMessage(hWndDlg,ListCntl,LB_GETTEXT,i,(LPARAM)str);   
        pTab = _fstrchr (str,'\t');
        *pTab++ = 0;      
        _fstrcpy (TableType,pTab); 
		if (OpenPGDB (DBName,str,""))
		{
			NumRows = NumSQLRows (PGDBHandle);  
	        if (PGDBTableIsText (DBName,str))
		   		_fstrcpy (TableType,"Text"); 
		}
		else  
		{
			_fstrcpy (TableType,"Unknown");
			NumRows = 0;
		}
		OpenPGDB (0,0,0);  
		sprintf (_fstrchr(str,0),"\t%s\t%ld",TableType,NumRows);
		SendDlgItemMessage ((HWND)hWndDlg,ListCntl,LB_DELETESTRING,i,(LPARAM)0); 
		SendDlgItemMessage ((HWND)hWndDlg,ListCntl,LB_INSERTSTRING,i,(LPARAM) str); 
	}
	return Num;
}
//AddItemToTree - adds items to a tree view control. 
// Returns the handle to the newly added item. 
// hwndTV - handle to the tree view control. 
// lpszItem - text of the item to add. 
// nLevel - level at which to add the item. 
HTREEITEM AddItemToTree(HWND hwndTV, LPSTR lpszItem, int nLevel)
{ 
   TVITEM tvi; 
   TVINSERTSTRUCT tvins; 
   static HTREEITEM hPrev = (HTREEITEM) TVI_FIRST; 
   static HTREEITEM hPrevRootItem = NULL; 
   static HTREEITEM hPrevLev2Item = NULL; 
   HTREEITEM hti; 

   if (nLevel > 1)
	   tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE; 
   else
   {
	   tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
	   tvi.state = TVIS_BOLD;
   }


   // Set the text of the item. 
   tvi.pszText = lpszItem; 
   tvi.cchTextMax = lstrlen(lpszItem); 

   // Save the heading level in the item's application-defined 
   // data area. 
   tvi.lParam = (LPARAM) nLevel; 
   tvi.iImage = nLevel-1;

   tvins.item = tvi; 
   tvins.hInsertAfter = hPrev; 

   // Set the parent item based on the specified level. 
   if (nLevel == 1) 
       tvins.hParent = TVI_ROOT; 
   else if (nLevel == 2) 
       tvins.hParent = hPrevRootItem; 
   else 
       tvins.hParent = hPrevLev2Item; 

   hPrev = TreeView_InsertItem(hwndTV,&tvins); 
  // Save the handle to the item. 
   if (nLevel == 1) 
       hPrevRootItem = hPrev; 
   else if (nLevel == 2) 
       hPrevLev2Item = hPrev; 

   return hPrev; 
} 

LPSTR ShapeTypeName (int iType)
{
	static char typeName[32];

	switch (iType)
	{
	default:
		sprintf (typeName,"Unknown (%i)",iType);
		break;
/*#define SHPT_POINTM	21
#define SHPT_ARCM	23
#define SHPT_POLYGONM	25
#define SHPT_MULTIPOINTM 28
#define SHPT_MULTIPATCH 31*/
	case SHPT_POINT:
	case SHPT_POINTZ:
		strcpy (typeName,"Point");
		break;
	case SHPT_ARC:
	case SHPT_ARCZ:
	case SHPT_POLYLINE_WITHCURVES:
		strcpy (typeName,"Arc");
		break;
	case SHPT_TEXT:
		strcpy (typeName,"Annotation");
		break;
	case SHPT_POLYGON_PGDB:
	case SHPT_PGDB_POLYGONZ:
	case SHPT_POLYGON_WITHCURVES:
	case SHPT_POLYGON:
	case SHPT_POLYGONZ:
		strcpy (typeName,"Polygon");
		break;
	case SHPT_MULTIPOINT:
	case SHPT_MULTIPOINTZ:
		strcpy (typeName,"Multi-Point");
		break;
	}
	return typeName;
}

void GetFGDBLev (HWND hWndDlg,UINT ListCntl,int hDB,LPSTR Under,int iLev)
{
	char	UnderNext[256];
	char	tabs[MAXFGDBLEVS*6];
	char	str[256];
	HANDLE	hList;
	int		Num;
	UINT	i;
	int	lnLev1 = strlen (Under);

	Num = FGDBGetChildList (hDB,Under,3,MAX_CHILD_LENGTH,&hList);
	if (Num)
	{
		LPSTR	pList = GlobalLock (hList);

		for (i = 0; i<Num; i++, pList += MAX_CHILD_LENGTH)
		{
			int inc = *pList == '\\';

			memset (tabs,' ',sizeof(tabs));
			tabs[iLev*6] = 0;
			sprintf (str,"%s%s",tabs,pList+inc);
			SendDlgItemMessage ((HWND)hWndDlg,ListCntl,LB_ADDSTRING,0,(LPARAM) str); 
			GetFGDBLev (hWndDlg,ListCntl,hDB,pList,iLev+1);
		}
		GSSiGlobUlFree (&hList);
	}
	Num = FGDBGetChildList(hDB, Under, 2, MAX_CHILD_LENGTH, &hList);
	if (Num)
	{
		LPSTR	pList = GlobalLock (hList);

		memset (tabs,' ',sizeof(tabs));
		tabs[iLev*6] = 0;
		for (i = 0; i<Num; i++, pList += MAX_CHILD_LENGTH)
		{
			int		type=0, nrows=0;
			int inc = *(pList+lnLev1) == '\\';

			FGDBGetTableInfo (hDB,pList,&type,&nrows,0);
			if (!nrows)
				continue;
			sprintf (str,"%s%s\t%s\t%i",tabs,pList+lnLev1+inc,ShapeTypeName(type),nrows);
			SendDlgItemMessage ((HWND)hWndDlg,ListCntl,LB_ADDSTRING,0,(LPARAM) str); 
		}
		GSSiGlobUlFree (&hList);
	}
	return;
}

long ListFGDBTables (LPSTR DBNameIN,HWND hWndDlg,UINT ListCntl,int ListType)
{
	UINT	i,j;  
	char	ShpType[16]="";
	char	DBName[512],str[256], TableName[128], TableType[64];
	LPSTR	pPar, pTab; 
	short	ShapeType; 
	double	Xmin,Xmax,Ymin,Ymax;  
	short	Num=0, Num2;        
	long	NumRows; 
	LPSTR	pFields;
	int		hDB=0;
	int   	TabStops[3]={270,320,370};
 
   	SendDlgItemMessage (hWndDlg,ListCntl,LB_SETTABSTOPS,3,(LPARAM)&TabStops); 
	_fstrcpy (DBName,DBNameIN); 
	ExpandText (DBName);
    if ((pPar = _fstrrchr (DBName,'(')))
    	*pPar++ = 0; 
	hDB = OpenFGDB2 (DBName,"","");
	if (hDB < 1)
		return 0;
	GetFGDBLev (hWndDlg,ListCntl,hDB,"\\",0);
	CloseFGDB (hDB);
/*	strcpy (TableName,"gdb_items");
	if (!OpenFGDB (DBName,TableName,""))
		return 0;
	hSelectFields = GSSiGlobAlloc (GMEM_MOVEABLE,0,1024);
	pFields = GlobalLock (hSelectFields);
	strcpy (pFields,"Name,DatasetSubtype1,DatasetSubtype2");
	GlobalUnlock (hSelectFields);
	while (FetchDBRec (FGDBHandle))
	{
		_fstrcpy (str,"[FGDB.DatasetSubtype2]"); 
		ExpandText (str);
		ShapeType = atoi (str);
		_fstrcpy (TableName,"[FGDB.Name]"); 
		ExpandText (TableName);    
		*ShpType = 0;  
		
		switch (ShapeType)
		{
			case 1:
				_fstrcpy (TableType,"Point");
				break;
			case 3:
				_fstrcpy (TableType,"Line");  
				break;
			case 4:
				_fstrcpy (TableType,"Area");
				break;
			default:
				sprintf (TableType,"Unknown(%i)",ShapeType);
				break;
		}
		Num++;   
		sprintf (str,"%s\t%s",TableName,TableType);
		SendDlgItemMessage ((HWND)hWndDlg,ListCntl,LB_ADDSTRING,0,(LPARAM) str); 
	}
	OpenFGDB (0,0,0); 
	for (i=0;i<Num;i++)
	{ 
        SendDlgItemMessage(hWndDlg,ListCntl,LB_GETTEXT,i,(LPARAM)str);   
        pTab = strchr (str,'\t');
        *pTab++ = 0;      
        strcpy (TableType,pTab); 
		if (OpenFGDB (DBName,str,""))
		{
			NumRows = NumSQLRows (FGDBHandle);  
	        if (FGDBTableIsText (DBName,str))
		   		strcpy (TableType,"Text"); 
		}
		else  
		{
			strcpy (TableType,"Unknown");
			NumRows = 0;
		}
		OpenFGDB (0,0,0);  
		sprintf (_fstrchr(str,0),"\t%s\t%ld",TableType,NumRows);
		SendDlgItemMessage ((HWND)hWndDlg,ListCntl,LB_DELETESTRING,i,(LPARAM)0); 
		SendDlgItemMessage ((HWND)hWndDlg,ListCntl,LB_INSERTSTRING,i,(LPARAM) str); 
	}*/
	return Num;
}

BOOL FAR PASCAL SELECTPGDBMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1107);
#endif
{   
	static	HANDLE	hSaveBM;
	char	str[260], str2[512]; 
    int     TabStops[2]={2000,2100};
	LPSTR pBS;
	
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
                       
        SetDlgItemText (hWndDlg,IDC_LISTHEADER,"Table Name                                                              Type               Count");
		SetDlgItemText (hWndDlg,IDC_STARTREF,"1");
		if (StringEndsWith (PGDBFile,".GDB"))
			ListFGDBTables (PGDBFile,hWndDlg,IDC_LIST,1);
		else
			ListPGDBTables (PGDBFile,hWndDlg,IDC_LIST,1);
		
		 break;                              
    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            break; 
            
            case IDC_LIST:
				switch(HIWORD(wParam))
				{
					case LBN_SELCHANGE:
						EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);  
						break;
				 	case LBN_DBLCLK:
				     	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
				 	break;
				}
			break;
			 
            case IDOK: 
            {
                HANDLE	hItems;
                LPINT	lpItems;
				short	nItems = GetLBSelectedItems (hWndDlg,IDC_LIST,&hItems);

				if (!nItems)
					break; 
                lpItems = (LPINT) GlobalLock(hItems);
				if (nItems == 1)  
				{
		            SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,*lpItems,(LPARAM)str);  
		            if ((pBS = _fstrchr (str,'\t')))
		            	*pBS=0;  
		            SubstituteDL (str,FALSE);
		            sprintf (_fstrchr(PGDBFile,0),"(%s)",str);
				}
				else
				{   
					HFILE	FidFL;
					char	FileList[MAX_PATH], File[MAX_PATH];
					UINT	i;
					LPSTR	pBS;
					long	StartRef, Count;
					BOOL	Error;

					StartRef = GetDlgItemInt(hWndDlg,IDC_STARTREF,&Error,TRUE); 
					_fstrcpy (FileList,PGDBFile);   
					_fstrcpy (File,PGDBFile);
					pBS = _fstrrchr (FileList,'\\');
					_fstrcpy (pBS+1,"filelist.txt");
					FidFL=GSSiOpenFile (FileList,0,OF_CREATE);
					for (i=0; i<nItems; i++,lpItems++)
					{   
			            SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,*lpItems,(LPARAM)str);  
			            if ((pBS = _fstrrchr (str,'\t')))
			            	*pBS++=0;
						Count = atol (pBS);
			            if ((pBS = _fstrchr (str,'\t')))
			            	*pBS++=0;
			            sprintf (PGDBFile,"%s(%s)\t%ld",File,str,StartRef);
			            ExpandText (PGDBFile);  
			            SubstituteDL (PGDBFile,FALSE); 
			            fputstring (PGDBFile,FidFL);
						StartRef += Count;
					}
					GSSiClose2 (&FidFL);
					_fstrcpy (PGDBFile,FileList);  
		            SubstituteDL (PGDBFile,FALSE); 
				}
				GSSiGlobUlFree (&hItems);
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
            }
            break;    
            
         }
         break; 

    default:
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}
BOOL FAR PASCAL SELECTFGDBMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1107);
#endif
{   
	static	HANDLE	hSaveBM;
	char	str[260], str2[512]; 
    int     TabStops[2]={2000,2100};
	LPSTR pBS;
	
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
                       
        SetDlgItemText (hWndDlg,IDC_LISTHEADER,"Table Name                                                                                                                                                       Type               Count");
		SetDlgItemText (hWndDlg,IDC_STARTREF,"1");
		ListFGDBTables (FGDBFile,hWndDlg,IDC_LIST,1);
		 break;                              
    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            break; 
            
            case IDC_LIST:
				switch(HIWORD(wParam))
				{
					case LBN_SELCHANGE:
						EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);  
						break;
				 	case LBN_DBLCLK:
				     	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
				 	break;
				}
			break;
			 
            case IDOK: 
            {
                HANDLE	hItems;
                LPINT	lpItems;
				short	nItems = GetLBSelectedItems (hWndDlg,IDC_LIST,&hItems);

				if (!nItems)
					break; 
                lpItems = (LPINT) GlobalLock(hItems);
				if (nItems == 1)  
				{
		            SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,*lpItems,(LPARAM)str);  
		            if ((pBS = _fstrchr (str,'\t')))
		            	*pBS=0;  
		            SubstituteDL (str,FALSE);
		            sprintf (_fstrchr(FGDBFile,0),"(%s)",FirstNonBlank(str));
				}
				else
				{   
					HFILE	FidFL;
					char	FileList[MAX_PATH], File[MAX_PATH], File2[MAX_PATH];
					UINT	i;
					LPSTR	pBS, pDot;
					long	StartRef, Count;
					BOOL	Error;

					StartRef = GetDlgItemInt(hWndDlg,IDC_STARTREF,&Error,TRUE); 

					_fstrcpy (FileList,FGDBFile);   
					_fstrcpy (File,FGDBFile);
					pBS = _fstrrchr (FileList,'\\');
					_fstrcpy (pBS+1,"filelist.txt");
					FidFL=GSSiOpenFile (FileList,0,OF_CREATE);
					for (i=0; i<nItems; i++,lpItems++)
					{   
			            SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,*lpItems,(LPARAM)str);  
			            if ((pBS = _fstrrchr (str,'\t')))
			            	*pBS++=0;
						Count = atol (pBS);
			            if ((pBS = _fstrchr (str,'\t')))
			            	*pBS++=0;
			            sprintf (File2,"%s(%s)\t%ld",File,FirstNonBlank(str),StartRef);
			            ExpandText (File2);  
			            SubstituteDL (File2,FALSE); 
			            fputstring (File2,FidFL);
						StartRef += Count;
					}
					GSSiClose2 (&FidFL);
					_fstrcpy (FGDBFile,FileList);  
		            SubstituteDL (FGDBFile,FALSE); 
				}
				GSSiGlobUlFree (&hItems);
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
            }
            break;    
            
         }
         break; 

    default:
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}
void DisplayFGDBItemMenu (HWND hWnd,BOOL IsTable)
{
	HMENU	hMenu=CreatePopupMenu();   
	POINT	position;

	//AppendMenu (hMenu,MF_ENABLED|MF_STRING,65001,"Expand");
	if (IsTable)
		AppendMenu (hMenu,MF_ENABLED|MF_STRING,65002,"View");

	AppendMenu (hMenu,MF_ENABLED|MF_STRING,65003,"Cancel");
	GetCursorPos (&position);
	TrackPopupMenu (hMenu,TPM_LEFTBUTTON|TPM_CENTERALIGN|TPM_VCENTERALIGN,position.x,position.y,0,hWnd,0);
	DestroyMenu (hMenu); 
	return;
}

BOOL FAR PASCAL SELECTFGDBMsgProc_TV(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1107);
#endif
{   
	static	HANDLE	hSaveBM;
	char	str[260], str2[512]; 
    int     TabStops[2]={2000,2100};
	LPSTR	pBS;
	int		rc;
	static	char	itemText[256];
	static	char	fgdbTable[128];
	
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
                       
        SetDlgItemText (hWndDlg,IDC_LISTHEADER,"Table Name                                                                             Type               Count");
		SetDlgItemText (hWndDlg,IDC_STARTREF,"1");
		ListFGDBTables (FGDBFile,hWndDlg,IDC_FGDB_TREE,1);
		
		 break;                              

	case WM_NOTIFY:
		{
			// first cast lParam to NMHDR* to know what event is
			NMHDR* nmhdr = (NMHDR*)lParam;

			// TreeView notifications start with TVN_
			switch(nmhdr->code)
			{
			// drag-and-drop operation has begun
			case TVN_BEGINDRAG:
				// cast again lParam to NMTREEVIEW*
				break;

			// drag-and-drop operation using right mouse button has begun
			case TVN_BEGINRDRAG:
				break;

			// label editing has begun
			case TVN_BEGINLABELEDIT:
				// cast again lParam to NMTVDISPINFO*
				break;

			// label editing has ended
			case TVN_ENDLABELEDIT:
				// cast again lParam to NMTVDISPINFO*
				break;

			// an item has been deleted
			case TVN_DELETEITEM:
				break;

			// TreeView needs info(such as item text) to display an item
			case TVN_GETDISPINFO:
				ii=1;
				break;

			// parent window must update the item information
			case TVN_SETDISPINFO:
				ii=1;
				break;

			// list of items was expanded or collapsed
			case TVN_ITEMEXPANDED:
				ii=1;
				break;

			// list of items are about to be expanded or collapsed
			case TVN_ITEMEXPANDING:
				ii=1;
				break;

			// a keyboard event has occurred
			case TVN_KEYDOWN:
				{
				// When the TreeView control is contained in a dialog box,
				// IsDialogMessage() processes the ESC and ENTER keystrokes and
				// does not pass them on to the edit control that is created by
				// the TreeView control. The result is that the keystrokes have
				// no effect.
				// Cast again lParam to NMTVKEYDOWN*
					NMTVKEYDOWN* nmtvkeydown = (NMTVKEYDOWN*)lParam;

					switch (nmtvkeydown->wVKey)
					{
					case 'A':
						ii=1;
						break;
					}

				}
				break;

			// the item selection has changed
			case TVN_SELCHANGED:
				{
					LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) lParam; 
					HTREEITEM hTV = TreeView_GetSelection(GetDlgItem (hWndDlg,IDC_FGDB_TREE));
					TVITEM tvItem;
					LPSTR	pPar;
					BOOL	isTable=FALSE;
					
					memset (&tvItem,0,sizeof (tvItem));
					tvItem.hItem = hTV;
					tvItem.pszText = itemText;
					tvItem.cchTextMax = 256;
					tvItem.mask = TVIF_STATE | TVIF_TEXT;
					rc = TreeView_GetItem(GetDlgItem (hWndDlg,IDC_FGDB_TREE),&tvItem);
					pPar = strchr (tvItem.pszText,'(');
					if (pPar)
					{
						*--pPar = 0;
						strcpy (fgdbTable,itemText);
						isTable = TRUE;
						PostMessage (hWndDlg,WM_COMMAND,65002,0);
						//DisplayFGDBItemMenu (hWndDlg,isTable);
					}
				}
				break;

			// the item selection is about to change
			case TVN_SELCHANGING:
				ii=1;
				break;

			default:
				break;
			}

		}
		break;

	case WM_RBUTTONDOWN:
		ii=1;
		break;

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
 			case 65001:
				break;
			case 65002:
				{
					char	cmd[512];

					sprintf (cmd,"$FIELDS(%s(%s),,%s)",FGDBFile,fgdbTable,fgdbTable);
					ProcessText (cmd);
				}
				break;

           case IDCANCEL: 
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            break; 
            
            case IDC_FGDB_TREE:
				switch(HIWORD(wParam))
				{
					case LBN_SELCHANGE:
						EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);  
						break;
				 	case LBN_DBLCLK:
				     	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
				 	break;
				}
			break;
			 
            case IDOK: 
            {
				HTREEITEM	hItems[8]={0}, hSelItem;
				HWND	hTreeWnd = GetDlgItem(hWndDlg,IDC_FGDB_TREE);
				TVITEM tvItem;
				int		nItems=0;
				char	txt[256];
	
				hItems[0] = TreeView_GetRoot (hTreeWnd);

				while (hItems[0])
				{
					hItems[1] = TreeView_GetChild (hTreeWnd,hItems[0]);
					while (hItems[1])
					{
						if (TreeView_GetCheckState(hTreeWnd,hItems[1]) > 0)
						{
							hSelItem = hItems[1];
							nItems++;
						}
						hItems[1] = TreeView_GetNextSibling (hTreeWnd,hItems[1]);
					}
					hItems[0] = TreeView_GetNextSibling (hTreeWnd,hItems[0]);
				}
				if (nItems == 1)  
				{
		          	memset (&tvItem,0,sizeof (tvItem));
					tvItem.hItem = hSelItem;
					tvItem.pszText = itemText;
					tvItem.cchTextMax = 256;
					tvItem.mask = TVIF_STATE | TVIF_TEXT;
					rc = TreeView_GetItem(GetDlgItem (hWndDlg,IDC_FGDB_TREE),&tvItem);
					strcpy (str,tvItem.pszText);
		            if ((pBS = strstr (str," (")))
		            	*pBS=0;  
		            SubstituteDL (str,FALSE);
		            sprintf (_fstrchr(FGDBFile,0),"(%s)",str);
				}
				else
				{   
					HFILE	FidFL;
					char	FileList[MAX_PATH], File[MAX_PATH];
					UINT	i;
					LPSTR	pBS;
					long	StartRef, Count;
					BOOL	Error;

					StartRef = GetDlgItemInt(hWndDlg,IDC_STARTREF,&Error,TRUE); 
					_fstrcpy (FileList,FGDBFile);   
					_fstrcpy (File,FGDBFile);
					pBS = _fstrrchr (FileList,'\\');
					_fstrcpy (pBS+1,"filelist.txt");
					FidFL=GSSiOpenFile (FileList,0,OF_CREATE);
					hItems[0] = TreeView_GetRoot (hTreeWnd);

					while (hItems[0])
					{
						hItems[1] = TreeView_GetChild (hTreeWnd,hItems[0]);
						while (hItems[1])
						{
							//if (TreeView_GetCheckState(hTreeWnd,hItems[1]) > 0)
							{
								hSelItem = hItems[1];
		          				memset (&tvItem,0,sizeof (tvItem));
								tvItem.hItem = hItems[1];
								tvItem.pszText = itemText;
								tvItem.cchTextMax = 256;
								tvItem.mask = TVIF_STATE | TVIF_TEXT;
								rc = TreeView_GetItem(GetDlgItem (hWndDlg,IDC_FGDB_TREE),&tvItem);
								strcpy (str,tvItem.pszText);
								if ((pBS = strstr (str," (")))
			            			*pBS++=0;
								Count = atol (pBS+1);
								if ((pBS = _fstrchr (str,'\t')))
			            			*pBS++=0;
								sprintf (FGDBFile,"%s(%s)\t%ld",File,str,StartRef);
								ExpandText (FGDBFile);  
								SubstituteDL (FGDBFile,FALSE); 
								fputstring (FGDBFile,FidFL);
								StartRef += Count;
							}
							hItems[1] = TreeView_GetNextSibling (hTreeWnd,hItems[1]);
						}
						hItems[0] = TreeView_GetNextSibling (hTreeWnd,hItems[0]);
					}
					GSSiClose2 (&FidFL);
					_fstrcpy (FGDBFile,FileList);  
		            SubstituteDL (FGDBFile,FALSE); 
				}
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
            }
            break;    
            
         }
         break; 

    default:
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetPGDBTable (HWND hWnd,LPSTR File)
{
	BOOL	rtn;
	DLGPROC lpfnSELECTITEMSMsgProc;
     
    _fstrcpy (PGDBFile,File); 
	lpfnSELECTITEMSMsgProc = MakeProcInstance((DLGPROC)SELECTPGDBMsgProc, hInst);
	rtn = DialogBox(hInst, (LPSTR)"SELECTPGDB",hWnd, lpfnSELECTITEMSMsgProc);
	FreeProcInstance(lpfnSELECTITEMSMsgProc); 
	if (rtn)
	    _fstrcpy (File,PGDBFile); 
	return rtn;
}   
BOOL GetFGDBTable (HWND hWnd,LPSTR File)
{
	BOOL	rtn;
	DLGPROC lpfnSELECTITEMSMsgProc;
     
    _fstrcpy (FGDBFile,File); 
	lpfnSELECTITEMSMsgProc = MakeProcInstance((DLGPROC)SELECTFGDBMsgProc, hInst);
	rtn = DialogBox(hInst, (LPSTR)"SELECTFGDB",hWnd, lpfnSELECTITEMSMsgProc);
	FreeProcInstance(lpfnSELECTITEMSMsgProc); 
	if (rtn)
	    _fstrcpy (File,FGDBFile); 
	return rtn;
}   

int	GetWords (LPSTR txt)
{
	int	nWords=0; 
	LPSTR	pLoc=txt;
	LPSTR	pEnd;
	
	pLoc = FirstNonBlank (pLoc);
	while (*pLoc)
	{
		pEnd = NextBlank (pLoc);
		if (*pEnd)
			*pEnd++ = 0; 
		_fstrcpy (Words[nWords],pLoc);
		WordLen[nWords++] = _fstrlen (pLoc);
		pLoc = pEnd;
		if (*pLoc)
			pLoc = FirstNonBlank (pLoc);
	}
	return nWords;
} 

BOOL CheckCode (int Recnum,LPSTR pElement,int BinSizeE,int code, int val)
{
	if (code != val)
	{
		char File[MAX_PATH];
#if CHECKMEM
		sprintf (File,"c:\\badrecs\\%i.bin",Recnum);
		{
			HFILE FidDB=GSSiOpenFile (File,0,OF_CREATE);
			BigWrite (FidDB,pElement,BinSizeE,-1);
			GSSiClose2 (&FidDB);
		}
#endif
		return FALSE;
	}
	return TRUE;
}

int FileForPoint (HPDPOINT p,int minx,int miny,int iwidth,int ncols)
{
	double	xmin = p->x - minx;
	double	ymin = p->y - miny;
	int	icol = xmin / iwidth;
	int	irow = ymin / iwidth;
	int	ifile = irow * ncols + icol;

	return ifile;
}

HANDLE ConvertToIPoint (int np,HPDPOINT pPoints)
{
	int i;
	HANDLE handle=GSSiGlobAlloc (0,GMEM_MOVEABLE,np*sizeof(POINT));
	LPPOINT p=GlobalLock (handle);

	for (i=0;i<np;i++)
	{
		p[i].x = (pPoints[i].x - midx) * 512 + 0.5;
		p[i].y = (pPoints[i].y - midy) * 512 + 0.5;
	}
	GlobalUnlock (handle);
	return handle;
}

BOOL SaveContours (int NumPoints,HPDPOINT pPoints,LPSTR CurrentUDI)
{
	BOOL rtn=FALSE;
	static HANDLE hfid;
	HFILE * pfid;
	int i, ifile, irow, icol, irow2, icol2;
	double xmin, ymin, xmax, ymax;
	HANDLE hIPoints;
	LPPOINT pIPoints;

	if (NumPoints == -1)
	{
		LPINT pInit = (LPINT)pPoints;

		minx = pInit[0];
		miny = pInit[1];
		nrows = pInit[2];
		ncols = pInit[3];
		iwidth = pInit[4];
		midx = minx + (ncols * iwidth) / 2;
		midy = miny + (nrows * iwidth) / 2;
		nfiles = nrows*ncols;
		if (nfiles <= 0)
			return FALSE;
		hfid = GSSiGlobAlloc (1779,GMEM_MOVEABLE,nfiles*sizeof(HFILE));
		pfid = GlobalLock (hfid);
		for (i=0;i<nfiles;i++)
		{
			char filename[MAX_PATH];
			OFSTRUCTGM OFStruct;

			sprintf (filename,"%s\\file%4.4i.bin",CurrentUDI,i+1);
			pfid[i] = GSSiOpenFile (filename,&OFStruct,OF_CREATE);
			//GSSiClose2 (&pfid[i]);
			//pfid[i] = HFILE_ERROR;
		}
		GlobalUnlock (hfid);
		return TRUE;
	}
	if (NumPoints == -2)
	{
		pfid = GlobalLock (hfid);
		nrows = 0;
		for (i=0;i<nfiles;i++)
			GSSiClose2 (&pfid[i]);
		GSSiGlobUlFree (&hfid);
		return TRUE;
	}
	if (nrows > 0 && NumPoints > 0)
	{
		float fContour;
		LPSTR pPipe = strchr (CurrentUDI,'|');
		float size=0;
		MNMXCORD bounds={SHPPolyHeader.Xmin,SHPPolyHeader.Ymin,SHPPolyHeader.Xmax,SHPPolyHeader.Ymax};
		
		pPoints[NumPoints].x = 0;
		pPoints[NumPoints].y = 0;
		pfid = GlobalLock (hfid);
		if (pPipe)
		{
			*pPipe++ = 0;
			size = atof (pPipe);
		}
		fContour = atof (CurrentUDI);
		rtn = TRUE;
		xmin = bounds.xmn - minx;
		ymin = bounds.ymn - miny;
		xmax = bounds.xmx - minx;
		ymax = bounds.ymx - miny;
		icol = xmin / iwidth;
		icol2 = xmax / iwidth;
		irow = ymin / iwidth;
		irow2 = ymax / iwidth;
		if (icol == icol2 && irow == irow2 && NumPoints <= maxContourPoints)
		{
			ifile = irow * ncols + icol;
			BigWrite (pfid[ifile],&fContour,sizeof(float),-1);
			size = (float)GetPolyLengthD (&pPoints[0],NumPoints);
			BigWrite (pfid[ifile],&size,sizeof(float),-1);
			BigWrite (pfid[ifile],&bounds,sizeof(MNMXCORD),-1);
			BigWrite (pfid[ifile],&NumPoints,sizeof(int),-1);
			//BigWrite (pfid[ifile],pPoints,sizeof(DPOINT)*NumPoints,-1);
			hIPoints = ConvertToIPoint (NumPoints,pPoints);
			pIPoints = GlobalLock (hIPoints);
			BigWrite (pfid[ifile],pIPoints,sizeof(POINT)*NumPoints,-1);
			GSSiGlobUlFree (&hIPoints);
		}
		else
		{
			int npInFile = 1, ifile2, ibeg=0;
			DPOINT intPoint;
			
			ifile = FileForPoint (&pPoints[npInFile-1],minx,miny,iwidth,ncols);
			while (ibeg < NumPoints)
			{
				ifile2 = FileForPoint (&pPoints[ibeg+npInFile],minx,miny,iwidth,ncols);
				while (ifile == ifile2 && npInFile < maxContourPoints)
				{
					npInFile++;
					ifile2 = FileForPoint (&pPoints[ibeg+npInFile],minx,miny,iwidth,ncols);
				}
				if (ibeg+npInFile < NumPoints)
				{
					intPoint = MidPointD (pPoints[ibeg+npInFile-1],pPoints[ibeg+npInFile]);
					pPoints[ibeg+npInFile-1] = intPoint;
				}
				BigWrite (pfid[ifile],&fContour,sizeof(float),-1);
				size = (float)GetPolyLengthD (&pPoints[ibeg],npInFile);
				BigWrite (pfid[ifile],&size,sizeof(float),-1);
				GetPolyBoundsD2 (&pPoints[ibeg],npInFile,&bounds,1);
				BigWrite (pfid[ifile],&bounds,sizeof(MNMXCORD),-1);
				BigWrite (pfid[ifile],&npInFile,sizeof(int),-1);
				hIPoints = ConvertToIPoint (npInFile,&pPoints[ibeg]);
				pIPoints = GlobalLock (hIPoints);
				BigWrite (pfid[ifile],pIPoints,sizeof(POINT)*npInFile,-1);
				GSSiGlobUlFree (&hIPoints);
				ibeg += npInFile;
				npInFile = 1;
				ifile = ifile2;
			}
		}
		GlobalUnlock (hfid);
	}
	return rtn;
}

BOOL PointInScreenRect (LPPOINT p)
{
	return PtInRect (&ScreenRect,*p);
}

LPPOINT MergeContourPoints (LPHANDLE phPoints,LPINT pnpnts)
{
	HANDLE hNewPoints = GSSiGlobAlloc (1780,GMEM_MOVEABLE,((*pnpnts)+1) * sizeof(POINT));
	LPPOINT newp = GlobalLock (hNewPoints);
	LPPOINT p;
	unsigned int i;
	int nnewp=1, nummerged=0;

	GlobalUnlock (*phPoints);
	p = GlobalLock (*phPoints);

	newp[0] = p[0];
	for (i=1;i<*pnpnts;i++)
	{
		if (abs (newp[nnewp-1].x - p[i].x) +
			abs (newp[nnewp-1].y - p[i].y) <= contourPointMergeDist)
		{
			nummerged++;
			newp[nnewp-1].x = (newp[nnewp-1].x * nummerged + p[i].x) / (nummerged + 1);
			newp[nnewp-1].y = (newp[nnewp-1].y * nummerged + p[i].y) / (nummerged + 1);
		}
		else
		{
			nummerged = 0;
			newp[nnewp++] = p[i];
		}
	}
	GSSiGlobUlFree (phPoints);
	*phPoints = hNewPoints;
	trimmedContourPoints += *pnpnts - nnewp;
	*pnpnts = nnewp;
	return newp;
}

float GetSlope (LPFLTPOINT p1,LPFLTPOINT p2)
{
	float dx = p2->x - p1->x;
	float dy = p2->y - p1->y;

	if (dx == 0.0f)
		dx = contourPointLinearSmooth;
	return (float)dy/(float)dx;

}


void GetSlopeRange (LPPOINT p1,LPPOINT p2,float *sloperange)
{
	FLTPOINT pa={p1->x,p1->y}, pb={p2->x,p2->y};
	float	slope;

	sloperange[0] = sloperange[1] = GetSlope (&pa,&pb);
	pa.x = p1->x + contourPointLinearSmooth;
	pa.y = p1->y + contourPointLinearSmooth;
	pb.x = p2->x - contourPointLinearSmooth;
	pb.y = p2->y - contourPointLinearSmooth;
	slope = GetSlope (&pa,&pb);
	sloperange[0] = min (sloperange[0],slope);
	sloperange[1] = max (sloperange[1],slope);
	pa.x = p1->x - contourPointLinearSmooth;
	pa.y = p1->y - contourPointLinearSmooth;
	pb.x = p2->x + contourPointLinearSmooth;
	pb.y = p2->y + contourPointLinearSmooth;
	slope = GetSlope (&pa,&pb);
	sloperange[0] = min (sloperange[0],slope);
	sloperange[1] = max (sloperange[1],slope);
		
	return;
}

BOOL SlopeRangesOverlap (float *sloperange1,float *sloperange2)
{
	BOOL rtn=TRUE;

	if (sloperange1[0] > sloperange2[1] ||
		sloperange1[1] < sloperange2[0])
		rtn = FALSE;
	return rtn;
}

BOOL PointsWithinMaxOffset (LPPOINT ip,int ibeg,int iend)
{
	double az, lineLength;
	DPOINT pb, pe, p,intPoint;
	int i;

	if (iend - ibeg < 2)
		return TRUE;
	pb.x = ip[ibeg].x;
	pb.y = ip[ibeg].y;
	pe.x = ip[iend].x;
	pe.y = ip[iend].y;
	az = getazd (&pb,&pe);
	lineLength = ldistpp (&pb,&pe);
	for (i=ibeg+1;i<iend;i++)
	{
		p.x = ip[i].x;
		p.y = ip[i].y;
		if (GetPerpendicularIntersect2 (&p,&pb,&az,&intPoint))
		{
			double d = ldistpp (&p,&intPoint);

			if (d > contourPointLinearSmooth)
				return FALSE;
			if (ldistpp (&pb,&intPoint) > lineLength)
				return FALSE;
			if (ldistpp (&pe,&intPoint) > lineLength)
				return FALSE;
		}
		else
			return FALSE;
	}
	return TRUE;
}


LPPOINT SmoothContourPoints (LPHANDLE phPoints,LPINT pnpnts)
{
	HANDLE hNewPoints = GSSiGlobAlloc (1780,GMEM_MOVEABLE,((*pnpnts)+1) * sizeof(POINT));
	LPPOINT newp = GlobalLock (hNewPoints);
	LPPOINT p;
	int i, ibeg;
	int nnewp=1, nummerged=0;
	float sloperange1[2], sloperange2[2];
	double az1, az2, daz;

	GlobalUnlock (*phPoints);
	p = GlobalLock (*phPoints);

	newp[0] = p[0];
	ibeg = 0;
	for (i=1;i<*pnpnts;i++)
	{
		if (!PointsWithinMaxOffset (p,ibeg,i))
		{
			ibeg = i - 1;
			newp[nnewp++] = p[ibeg];
		}
	}
	newp[nnewp++] = p[*pnpnts-1];
	GSSiGlobUlFree (phPoints);
	*phPoints = hNewPoints;
	trimmedContourPoints += *pnpnts - nnewp;
	*pnpnts = nnewp;
	return newp;
}

LPPOINT SmoothContourPoints_old (LPHANDLE phPoints,LPINT pnpnts)
{
	HANDLE hNewPoints = GSSiGlobAlloc (1780,GMEM_MOVEABLE,*pnpnts * sizeof(POINT));
	LPPOINT newp = GlobalLock (hNewPoints);
	LPPOINT p;
	unsigned int i;
	int nnewp=1, nummerged=0;
	float sloperange1[2], sloperange2[2];

	GlobalUnlock (*phPoints);
	p = GlobalLock (*phPoints);

	newp[0] = p[0];
	GetSlopeRange (&p[0],&p[1],sloperange1);
	for (i=2;i<*pnpnts;i++)
	{
		GetSlopeRange (&newp[nnewp-1],&p[i],sloperange2);
		if (!SlopeRangesOverlap (sloperange1,sloperange2))
		{
			newp[nnewp++] = p[i-1];
			GetSlopeRange (&p[i-1],&p[i],sloperange1);
		}
		else
			memcpy (sloperange1,sloperange2,sizeof(sloperange1));
	}
	newp[nnewp++] = p[*pnpnts-1];
	GSSiGlobUlFree (phPoints);
	*phPoints = hNewPoints;
	trimmedContourPoints += *pnpnts - nnewp;
	*pnpnts = nnewp;
	return newp;
}

int GetNumThinnedContourRecs (void)
{
	int rtn = -1;
	
	if (thinnedConFid != HFILE_ERROR)
		rtn = GSSifilelength (thinnedConFid);
	return rtn;
}

int GetThinnedContourPos (void)
{
	return GSSillseek (thinnedConFid,0,1);
}

BOOL GetNextThinnedContour (float *contourElev,int *nconPnts,HANDLE *hConPnts)
{
	HANDLE hPoints;
	HPPOINT pPoints;
	HPDPOINT pPointsD;
	LPMNMXCORD pBounds;
	int npnts,i;

	if (thinnedConFid == HFILE_ERROR)
		return FALSE;
Top:
	if (BigRead (thinnedConFid,&conheader,sizeof(conheader)) != sizeof(conheader))
		return FALSE;
	hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,conheader.npnts*sizeof(POINT));
	pPoints = GlobalLock (hPoints);

	BigRead (thinnedConFid,pPoints,conheader.npnts*sizeof(POINT));
	npnts = conheader.npnts;

	if (conheader.size <= removeContourSize && SamePoint (pPoints[0],pPoints[npnts-1]))
	{
		GSSiGlobUlFree (&hPoints);
		goto Top;
	}
	if (contourPointMergeDist > 0)
		pPoints = MergeContourPoints (&hPoints,&npnts);
	if (contourPointLinearSmooth > 0)
		pPoints = SmoothContourPoints (&hPoints,&npnts);
	*hConPnts = GSSiGlobAlloc (1782,GMEM_MOVEABLE,npnts*sizeof(DPOINT)+sizeof(MNMXCORD));
	*nconPnts = npnts;
	*contourElev = conheader.fcontour;
	pBounds = GlobalLock (*hConPnts);
	pPointsD = (HPDPOINT)(pBounds+1);
	DBoundsInit (pBounds);
	for (i=0;i<npnts;i++)
	{
		pPointsD[i].x = (double)pPoints[i].x / 512.0 + midx;
		pPointsD[i].y = (double)pPoints[i].y / 512.0 + midy;
		AddDPointToMinMax (&pPointsD[i],pBounds);
	}
	GSSiGlobUlFree (&hPoints);
	GlobalUnlock (*hConPnts);
	return TRUE;
}

BOOL OpenThinnedContours (LPSTR UDI)
{
	HFILE rtn=FALSE;
	char	filename[MAX_PATH];
	int		ifile;

	if (!UDI)
	{
		GSSiClose2 (&thinnedConFid);
		thinnedConFid = HFILE_ERROR;
		return TRUE;
	}
	ifile = atoi (&UDI[4]);
	sprintf (filename,"%s\\file%4.4i.bin",saveContoursDir,ifile);
	thinnedConFid = GSSiOpenFile (filename,0,OF_READ);
	return thinnedConFid != HFILE_ERROR;
}

void DisplayContours (HDC hDC,LPSTR UDI)
{
	HBRUSH hRedBrush;
	HPEN hRedPen;
	HFILE fid;
	int	psize;
	static int maxPnts=0;
	float RSQMIN;
	int i, npnts, ifile, idesc=GetSymbolNum ("LKC000");
	char	filename[MAX_PATH];

	ifile = atoi (&UDI[4]);
	sprintf (filename,"%s\\file%4.4i.bin",saveContoursDir,ifile);
	fid = GSSiOpenFile (filename,0,OF_READ);
	if (fid == HFILE_ERROR)
		return;

    SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);
   	hRedBrush = CreateSolidBrush(RGB(255,   0,   0));
	hRedPen = CreatePen (PS_SOLID,3,RGB(255,   0,   0));

	{
		XFORM	xForm;
		double	bmx[4], bmy[4], vpx[4], vpy[4];
		double	wx[4], wy[4];
		HANDLE	hTran;

		vpx[0] = CurView->DrawRect.left;
		vpx[1] = CurView->DrawRect.left;
		vpx[2] = CurView->DrawRect.right;
		vpx[3] = CurView->DrawRect.right;
		vpy[0] = CurView->DrawRect.bottom;
		vpy[1] = CurView->DrawRect.top;
		vpy[2] = CurView->DrawRect.top;
		vpy[3] = CurView->DrawRect.bottom;
		wx[0] = (CurView->WBounds.xmn- midx) * 512;
		wy[0] = (CurView->WBounds.ymn- midy) * 512;
		wx[1] = (CurView->WBounds.xmn- midx) * 512;
		wy[1] = (CurView->WBounds.ymx- midy) * 512;
		wx[2] = (CurView->WBounds.xmx- midx) * 512;
		wy[2] = (CurView->WBounds.ymx- midy) * 512;
		wx[3] = (CurView->WBounds.xmx- midx) * 512;
		wy[3] = (CurView->WBounds.ymn- midy) * 512;
		hTran = STRAN2 (1651,wx,wy,vpx,vpy,-4,&RSQMIN,1,NULL);
		ScreenRect.left = wx[0];
		ScreenRect.right = wx[2];
		ScreenRect.top = wy[0];
		ScreenRect.bottom = wy[2];
		xForm = SetXFORMFromTRANS (hTran);

		ii = SetGraphicsMode(hDC, GM_ADVANCED);
		ii = SetWorldTransform(hDC, &xForm); 
	    CloseTRANS2 (&hTran);
		psize = IDNINT (2 * ((CurView->WBounds.xmx - CurView->WBounds.xmn) * 1000) / (CurView->DrawRect.right - CurView->DrawRect.left));

	}


	SelectObject (hDC,GetStockObject(BLACK_PEN));
	while (CheckForContinue(TRUE, 0) && BigRead(fid, &conheader, sizeof(conheader)) == sizeof(conheader))
	{
		if (BoundsInBounds (&conheader.bounds,&CurView->WBounds,1))
		{
			HANDLE hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,conheader.npnts*sizeof(POINT));
			HPPOINT pPoints = GlobalLock (hPoints);

			BigRead (fid,pPoints,conheader.npnts*sizeof(POINT));
			npnts = conheader.npnts;
			maxPnts = max (maxPnts,npnts);

			SelectObject (hDC,hRedPen);
			if (displayOriginalContours)
				Polyline (hDC,pPoints,npnts);
			SelectObject (hDC,GetStockObject(BLACK_PEN));
			totContourPoints += conheader.npnts;
			if (conheader.size <= removeContourSize && SamePoint (pPoints[0],pPoints[npnts-1]))
			{
				trimmedContourPoints += npnts;
			}
			else
			{
				if (contourPointMergeDist > 0)
					pPoints = MergeContourPoints (&hPoints,&npnts);
				if (contourPointLinearSmooth > 0)
					pPoints = SmoothContourPoints (&hPoints,&npnts);
				Polyline (hDC,pPoints,npnts);
				if (DisplayLinePoints)
				{ 
					HBRUSH hOldBrush = SelectObject (hDC,hRedBrush);
					HBRUSH hOldPen = SelectObject (hDC,hRedPen);
					for (i=0;i<npnts;i++)
					{
						if (PointInScreenRect (&pPoints[i]))
						{
							POINT p[4];
							char id[8];

							//itoa (i,id,10);
							//TextOut (hDC,pPoints[i].x,pPoints[i].y,id,strlen(id));
							p[0].x = pPoints[i].x-psize;
							p[0].y = pPoints[i].y-psize;
							p[1].x = pPoints[i].x-psize;
							p[1].y = pPoints[i].y+psize;
							p[2].x = pPoints[i].x+psize;
							p[2].y = pPoints[i].y+psize;
							p[3].x = pPoints[i].x+psize;
							p[3].y = pPoints[i].y-psize;
							Polygon(hDC,p,4);
						}
					}
					SelectObject (hDC,hOldBrush);
					SelectObject (hDC,hOldPen);
				}
			}
			//GWPolylineD (hDC, pPoints, header.npnts,idesc);
			GSSiGlobUlFree (&hPoints);
		}
		else
			GSSillseek (fid,conheader.npnts*sizeof(POINT),1);
	}
	DeleteObject (hRedBrush);
	DeleteObject (hRedPen);
	RestoreDC (hDC,-1);
	GSSiClose2 (&fid);
	return;
}
void DisplayContours_d (HDC hDC,LPSTR UDI)
{
	HFILE fid;
	int i, npnts, ifile, idesc=GetSymbolNum ("LKC000");
	char	filename[MAX_PATH];
	struct {int icontour;
			float size;
			MNMXCORD bounds;
			int npnts;} header;

	ifile = atoi (&UDI[4]);
	sprintf (filename,"%s\\file%4.4i.bin",saveContoursDir,ifile);
	fid = GSSiOpenFile (filename,0,OF_READ);
	if (fid == HFILE_ERROR)
		return;

    SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);


	SelectObject (hDC,GetStockObject(BLACK_PEN));
	while (BigRead (fid,&header,sizeof(header)) == sizeof(header))
	{
		if (BoundsInBounds (&header.bounds,&CurView->WBounds,1))
		{
			HANDLE hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,header.npnts*sizeof(DPOINT));
			HPDPOINT pPoints = GlobalLock (hPoints);

			BigRead (fid,pPoints,header.npnts*sizeof(DPOINT));
			{
				HANDLE hScreenPoints = GSSiGlobAlloc ( 0,GMEM_MOVEABLE,header.npnts * (long)sizeof(POINT));
				HPPOINT	pScreenPoints = GlobalLock (hScreenPoints);
				int nPnts = header.npnts;
				
				totContourPoints += header.npnts;
				trimmedContourPoints += nPnts;

				for (i=0;i<nPnts;i++)
					pScreenPoints[i] = BasePtToWinPt (&pPoints[i]);   
				Polyline (hDC,pScreenPoints,nPnts);
				if (DisplayLinePoints)
				{   
					for (i=0;i<header.npnts;i++)
					{
						DisplayPoint (hDC,pScreenPoints[i]); 
					}
				}
				GSSiGlobUlFree (&hScreenPoints);
			}
			//GWPolylineD (hDC, pPoints, header.npnts,idesc);
			GSSiGlobUlFree (&hPoints);
		}
		else
			GSSillseek (fid,header.npnts*sizeof(DPOINT),1);
	}
	RestoreDC (hDC,-1);
	GSSiClose2 (&fid);
	return;
}
void DisplayContours_old (HDC hDC,LPSTR UDI)
{
	HFILE fid;
	int i, npnts, ifile, idesc=GetSymbolNum ("LKC000");
	char	filename[MAX_PATH];
	struct {int icontour;
			float size;
			MNMXCORD bounds;
			int npnts;} header;

	ifile = atoi (&UDI[4]);
	sprintf (filename,"%s\\file%4.4i.bin",saveContoursDir,ifile);
	fid = GSSiOpenFile (filename,0,OF_READ);
	if (fid == HFILE_ERROR)
		return;

    SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);
	SelectObject (hDC,GetStockObject(BLACK_PEN));
	while (BigRead (fid,&header,sizeof(header)) == sizeof(header))
	{
		if (BoundsInBounds (&header.bounds,&CurView->WBounds,1))
		{
			HANDLE hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,header.npnts*sizeof(DPOINT));
			HPDPOINT pPoints = GlobalLock (hPoints);

			BigRead (fid,pPoints,header.npnts*sizeof(DPOINT));
			{
				HANDLE hScreenPoints = GSSiGlobAlloc ( 0,GMEM_MOVEABLE,header.npnts * (long)sizeof(POINT));
				HPPOINT	pScreenPoints = GlobalLock (hScreenPoints);
				int nPnts = header.npnts;
				
				totContourPoints += header.npnts;
				trimmedContourPoints += nPnts;

				for (i=0;i<nPnts;i++)
					pScreenPoints[i] = BasePtToWinPt (&pPoints[i]);   
				Polyline (hDC,pScreenPoints,nPnts);
				if (DisplayLinePoints)
				{   
					for (i=0;i<header.npnts;i++)
					{
						DisplayPoint (hDC,pScreenPoints[i]); 
					}
				}
				GSSiGlobUlFree (&hScreenPoints);
			}
			//GWPolylineD (hDC, pPoints, header.npnts,idesc);
			GSSiGlobUlFree (&hPoints);
		}
		else
			GSSillseek (fid,header.npnts*sizeof(DPOINT),1);
	}
	RestoreDC (hDC,-1);
	GSSiClose2 (&fid);
	return;
}
int AdjustPointIndex(int startPointIndex,int nPoly, HANDLE hPolyPartLen, int geometryType)
{
	LPINT pPartLen;
	int n = startPointIndex;
	int nTot = 0;
	int iPoly;

	if (geometryType != geometryPolygon)
		return n;
	pPartLen = GlobalLock (hPolyPartLen);
	for (iPoly = 0; iPoly < nPoly; iPoly++)
	{
		nTot += pPartLen[iPoly];
		if (iPoly)
		{
			nTot++;
			if (n >= nTot-1)
				n++;
			else
				break;
		}
	}
	
	GlobalUnlock(hPolyPartLen);
	return n;
}
BOOL ProcessFGDBRecord (HDC hDC,long RecordNumber)
#if ENABLETRACE
{GSSiEnterProg (1375);
#endif
{ 
    HPDPOINT    pPoints, pFirstPoint;
    HPLONG      pPartIndex;   
    ULONG		i;
    short		Type, ltag;
    HANDLE		hPoints, hPartIndex;  
    LPINT		pNumPoints, pNumPointsNew;
    long		NumPoints; 
    HPEN		hOldPen=0, hDeletePen=0, hTempPen=0;
	HBRUSH		hOldBrush=0, hTempBrush=0;  
	DPOINT		DPoint,DynSegEndPoint;
	double		size;
    static		short	dbugid=6;
    short		ii;
    LPBYTE		pRec,pElement;
    HANDLE		hRec,hElement=0;
    char		str[512];  
    long		recloc=sizeof(SHPPOLYHEADER); 
    long		BinSizeR,BinSizeE;  
    static	long	DebugRecNum=21291;
	static	int	nCurveText=0;
	BOOL	save=FALSE;
	static	BOOL	db_dopoly=TRUE,displayTextPoly=FALSE;
	int		opt;
	HANDLE	hPolyPartLenNew=0;
	char	FileID[2][6]={"PGDB","FGDB"};
	LPFILEGDBRECHEADER pFGDBRecHeader;
	int		IsFGDB = TRUE;
	BOOL	doWindowCheck;
	int		structuralPart;
    
    if (CurView->ID == dbugid)
    	ii=1; 
   // if (RecordNumber == DebugRecNum)
   // 	save = TRUE;
    SHPHeader.ShapeType = SHPType;//SHPT_POLYGONZ;
    sprintf (str,"[%s.%s]",FileID[IsFGDB],ShapeFieldName);
    ExpandText (str);
    hRec = (HANDLE) atol (str); 
    BinSizeR = GlobalSize (hRec);
    pRec = GlobalLock (hRec);  
	{
		LPCURVAL pCurVal = (LPCURVAL)pRec;

		pRec = (LPBYTE)&pCurVal->Value;
		pFGDBRecHeader = (LPFILEGDBRECHEADER)pRec;
		pRec += sizeof (FILEGDBRECHEADER);
		BinSizeR -= (sizeof (FILEGDBRECHEADER) + sizeof(int));
		if (pFGDBRecHeader->hasCurves)
			ii=1;
	}
	if (SHPType == SHPT_TEXT)
		pFGDBRecHeader->shapeType = SHPType;
	else
		SHPHeader.ShapeType = SHPType = pFGDBRecHeader->shapeType;
    sprintf (str,"[%s.ELEMENT]",FileID[IsFGDB]);
    ExpandText (str);
	hElement = (HANDLE) atol (str);      
	if (hElement)
	{
		BinSizeE = GlobalSize (hElement);
		pElement = GlobalLock (hElement);
		if (save)
		{
			HFILE FidDB=GSSiOpenFile ("c:\\temp.bin",0,OF_CREATE);
			BigWrite (FidDB,pElement,BinSizeE,-1);
			GSSiClose2 (&FidDB);
		}
	}
	ShowValue (hDC,FALSE);  
	InGraphicsProcessor = TRUE;
	ItemIsDeleted = FALSE;
	InitRecord (hDC); 
    SetSHPParms (RecordNumber);  
	ItemSeg = RecordNumber;  
	if (pFGDBRecHeader->shapeType == SHPT_TEXT && CurView->PassID == 3)
			;  
	else if (CurView->PassID && CurView->PassID < 4)
	{
		if ((pFGDBRecHeader->geometryType == geometryPolygon) && CurView->PassID != 2)
			goto RtnFalse;  
		if ((pFGDBRecHeader->geometryType == geometryPoint || pFGDBRecHeader->geometryType == geometryMultipoint|| pFGDBRecHeader->geometryType == geometryPolyline) && CurView->PassID == 2)
			goto RtnFalse;
	}
    if (!CurrentDesc)
    	goto RtnFalse;
	if (GRStartTime > TimeRangeEnd || GREndTime < TimeRangeBeg)
		goto RtnFalse;
    ltag = _fstrlen (SHPTag);
	SetSymNum (CurrentDesc);
	if (!ProcessRefAndTAG (GetVisibility (CurrentDesc),SHPTag,ltag))
		goto RtnFalse;
	if (!Pick && hDC && Display)
	{
		SetROP2(hDC,DisplayRasterOpt);
		if (HaveVarFillColor)
			SetTextColor (hDC,ConvertColor(GlobalColors[0],CurrentDesc));
		else
			SetTextColor (hDC,ConvertColor(DefaultTextColor,CurrentDesc));
		SelectObject (hDC,GetStockObject(BLACK_PEN));
		GSSiDeleteObject (&hBlackPen);
		hBlackPen = CreatePen (PS_SOLID,0,ConvertColor(0,CurrentDesc));		
		SelectObject (hDC,hBlackPen);
	}
	HiPrecis = TRUE; 
	CurrentPen = 0;
	SetGlobalValueReal("%FGDBZMin",0);
	SetGlobalValueReal("%FGDBZMax",0);

	switch (pFGDBRecHeader->geometryType)
	{
		case geometryPoint:
		{
			int	SDCrtn = 0;

			if (!SHPPointRec.Type)
				goto RtnFalse;
			DPoint = SHPPointRec.Point; 
			if (ConvertCoord(&DPoint,0,1)) 
				goto RtnFalse;
	        lpDCurPoints = &DPoint;  
			CurrentPoint = CurPointLocD = DPoint;
	        LastElementBeginPoint = LastElementEndPoint = DPoint;
	        nPnts = 1; 
			CurPointLoc = BasePtToWinPt(lpDCurPoints); 
    		if (!PointInMaskAreaWinCoord (CurPointLoc))
    			goto RtnFalse;
    		if (PointIsBlocked (&CurPointLocD,CurrentDesc))
    			goto RtnFalse;   
       		HaveTXLoc = TRUE;
    		CurrentType = GF_POINT; 
			if (CurPointSize < 0)
				CurPointSize = -CurPointSize * DeviceToScreenFactor();
			else
				CurPointSize /= CurView->BaseUnitsPerPixel;
			if ((Pick||PickingByRefno) && GetTypeVisibility(TYPE_POINT))
			{
				PickPointItemD(lpDCurPoints, (CurPointSize*ThemeWidthFactor*GraphicsPointFactor)*CurView->BaseUnitsPerPixel, PTRot, CurrentDesc);
			} 
			else if (GetTypeVisibility(TYPE_POINT))
			{   
				short	iDesc=CurrentDesc;

				if (CopyRec)
				{   
				    AddPointToBuffer (CurPointLocD,CurrentRefno,0,CurrentDesc,10, PTRot,0,0,0,
									  CurrentPrefix,CurrentUDI,-1,-1,-1,TRUE,&hUpdateBuf,&lUpdateBuf,0);
				}
				else
				{
	 				if (CurrentDesc > 0 && CurrentDesc < 3201)   
	 				{
						if (TSize)
							CurView->CurVisType[CurrentDesc]=5;
						else
							CurView->CurVisType[CurrentDesc]=4;
					}
					HighlightPointSym=FALSE;
					if (SHPColor >= 0)
					{  
						GlobalColors[0]=SHPColor;  
						HaveVarFillColor = TRUE;
					}
					if (!GetTypeVisibility(6) && SymbolIsVisible (iDesc)) 
					{
						CurPointSize = 10*DeviceToScreenFactor();
						iDesc = InvisiblePointSymbol;
					}
					if ((SDCrtn = SetDisplayChar (hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI)) > 0)
					{   
						if (ThemePointSym)
						{
							iDesc = ThemePointSym;
							if (ThemePointSize < 0)
								size = -ThemePointSize *DeviceToScreenFactor();
							else
								size = ThemePointSize / CurView->BaseUnitsPerPixel; 
							size *= ThemeWidthFactor;
							size = min(max (size,1),MaxPointSize);
						}
						else
							size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor; 
						{
							long	DisplayedWidth=0;

							DisplayPointItem (hDC,CurPointLoc,size,PTRot,iDesc,&DisplayedWidth);
							CurView->MaxSymbolWidth = max (CurView->MaxSymbolWidth,DisplayedWidth);
							CurView->MaxFileDisplayedPointWidth[FileNum] = max (CurView->MaxFileDisplayedPointWidth[FileNum],(DisplayedWidth/ FileDistToWinDist)-(((long)CurrentItemMinMax.xmx)-CurrentItemMinMax.xmn));
						}
						HaveTXLoc = 1;
					} 
				}
			}
   			TXLoc = CurPointLocD; 
       		if (SDCrtn < 0)
				HaveTXLoc = 1;
		}
		break;
		
       	case SHPT_TEXT: 
//       		GetValFromOpenFiles ("pgdb.TEXTSTRING",str); 
//       		GetValFromOpenFiles ("pgdb.FONTNAME",str); 
//       		GetValFromOpenFiles ("pgdb.ANGLE",str);    
			goto DoPoly;
		break;
		case geometryPolyline:
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=2; 
    		CurrentType = GF_POLYLINE; 
    		if (!GetTypeVisibility(TYPE_LINECURVE))
    			break;
			goto DoPoly; 
		case geometryPolygon:
    		CurrentType = GF_AREA; 
			if (SHPType == SHPT_TEXT)
			{
				CurrentType = GF_TEXT;
    			if (!GetTypeVisibility(TYPE_TEXT))
    				break;
			}
			else if (!GetTypeVisibility(TYPE_AREA))
    			break;
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=3; 
DoPoly:
	        if (SHPPolyHeader.NumPoints > USHRT_MAX)
	        	ii=1;
	        	//goto RtnFalse;
	        if (!SHPPolyHeader.NumPoints)
	        	goto RtnFalse;
	        if (!SHPPolyHeader.Type)
	        	goto RtnFalse;
	        nPoly = SHPPolyHeader.NumParts;  
	        NumPoints = SHPPolyHeader.NumPoints; 
			structuralPart = SHPPolyHeader.Type & esriShapeBasicTypeMask;
			if (structuralPart == 50)
				SHPPolyHeader.Type = SHPT_ARC;
			if (structuralPart == 51)
				SHPPolyHeader.Type = SHPT_POLYGON;

	        if (pFGDBRecHeader->shapeType != SHPT_TEXT && (SHPPolyHeader.Type == SHPT_POLYGON || SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES || SHPPolyHeader.Type == SHPT_POLYGONZ || SHPPolyHeader.Type == SHPT_POLYGONM || SHPPolyHeader.Type == SHPT_PGDB_POLYGONZ))
	        	NumPoints = NumPoints+nPoly-1;
	        hPartIndex = GSSiGlobAlloc (1418,GMEM_MOVEABLE,sizeof(long)*(nPoly+1));
	        hPolyPartLen = GSSiGlobAlloc (1785,GMEM_MOVEABLE,sizeof(int)*(nPoly+1));
	        hPolyPartLenNew = GSSiGlobAlloc (1786,GMEM_MOVEABLE,sizeof(int)*(nPoly+1));
			hPoints = GSSiGlobAlloc (1420,GMEM_MOVEABLE,sizeof(DPOINT)*(NumPoints+1)); 
	        pPartIndex = (HPLONG)GlobalLock (hPartIndex); 
	        hmemmove ((HPSTR)pPartIndex,&pRec[recloc],nPoly*sizeof(long));   //pPartIndex[5]
	        recloc += nPoly*sizeof(long);
	        pPartIndex[SHPPolyHeader.NumParts]= SHPPolyHeader.NumPoints;
	                    
            pPoints = pFirstPoint = (LPDPOINT)GlobalLock (hPoints); 
            pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
            pNumPointsNew = (LPINT)GlobalLock (hPolyPartLenNew);
	        for (i=0;i<nPoly;i++)
	        {   
	            long    numpoints, startpoint,ii; 
	                        
	            startpoint = *pPartIndex++;
	            numpoints = *pPartIndex-startpoint; 
	            //if (numpoints > (long)USHRT_MAX)
	            //	MessageBox (0,"Shape file record contains too many points",0,MB_ICONEXCLAMATION);
	            *pNumPoints++ = numpoints; 
				*pNumPointsNew++ = numpoints;
	            hmemmove ((HPSTR)pPoints,&pRec[recloc],numpoints * sizeof(DPOINT));     //pPoints[1]
	            recloc += numpoints * sizeof(DPOINT); //pPoints[2]
	            pPoints += numpoints;   
				//if (i && pFGDBRecHeader->shapeType != SHPT_TEXT && ((SHPPolyHeader.Type == SHPT_POLYGON || SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES || SHPPolyHeader.Type == SHPT_POLYGONZ || SHPPolyHeader.Type == SHPT_POLYGONM || SHPPolyHeader.Type == SHPT_PGDB_POLYGONZ)))
				if (i && pFGDBRecHeader->shapeType != SHPT_TEXT && pFGDBRecHeader->geometryType == geometryPolygon)
					*pPoints++ = *pFirstPoint;
	        } 
	        ExtraBytes = BinSizeR - recloc; 
/*	        if (ExtraBytes>1)
	        {
	        	HPDPOINT pPoints = (HPDPOINT)&pRec[recloc];//pPoints[3]
	        	HPLONG pPointsl = (HPLONG)&pRec[recloc];//pPointsl[3]
	        	HPFLOAT pPointsf = (HPFLOAT)&pRec[recloc];//pPointsf[2]  
	        	HPSHORT	pPointss = (HPSHORT)&pRec[recloc];//pPointss[7]
				HFILE	Fid;
				char	BinFileName[128];

				sprintf (BinFileName,"c:\\curvedecode\\%i_%i-%i.bin",ExtraBytes,NumPoints,CurrentRefno);
				Fid = GSSiOpenFile (BinFileName,0,OF_CREATE);
				BigWrite (Fid,&pRec[recloc],ExtraBytes,-1);
				GSSiClose2 (&Fid);
				if (ExtraBytes != 33)
					ii=1;
				else
					db_dopoly=TRUE;
	        	 ii = 1;

	        }*/
			if (pFGDBRecHeader->hasZs)
			{
				double zMin = *(LPDOUBLE)&pRec[recloc];
				double zMax = *(LPDOUBLE)&pRec[recloc + sizeof(double)];
				SetGlobalValueReal("%FGDBZMin", zMin);
				SetGlobalValueReal("%FGDBZMax", zMax);
				recloc += (2 + SHPPolyHeader.NumPoints) * sizeof (double);
			}
			if (pFGDBRecHeader->hasMs)
			{
				double mMin = *(LPDOUBLE)&pRec[recloc];
				double mMax = *(LPDOUBLE)&pRec[recloc + sizeof(double)];
				recloc += (2 + SHPPolyHeader.NumPoints) * sizeof (double);
			}
			NumPOC = 0;
			if (pFGDBRecHeader->hasCurves)
	        {
				int	reclocSave = recloc;
				int iSegMod;
				long nSegmentModifiers = *(LPLONG)&pRec[recloc];
				esriSegmentModifier * pSegmentModifier;

				recloc += 4;
				for (iSegMod = 0; iSegMod < nSegmentModifiers; iSegMod++)
				{
					pSegmentModifier = (esriSegmentModifier*)&pRec[recloc];
					recloc += sizeof (esriSegmentModifier);
					switch (pSegmentModifier->segmentType)
					{
					case esriSegmentArc:
					{
						SegmentArc * parc = (SegmentArc*)&pSegmentModifier->segmentParams;


						if (parc->Bits & IsEmpty)
						{
							ii = 1;
						}
						else if (parc->Bits &  IsLine)
						{
							ii = 1;
						}
						else if (parc->Bits & IsPoint)
						{
							ii = 1;
						}
						else if (parc->Bits & DefinedIP)
						{
							POCPos[NumPOC] = AdjustPointIndex(pSegmentModifier->startPointIndex, nPoly, hPolyPartLen, pFGDBRecHeader->geometryType);
							POCCoord[NumPOC] = pSegmentModifier->segmentParams.arc.centerPoint;
							ConvertCoord(&POCCoord[NumPOC++], 0, 1);
						}
						else
						{
							ii = 1;
						}
					}
						break;
					default:
						break;
					}
				}
/*				NumPOC = *(LPLONG)&pRec[recloc];
				recloc += 4;
				for (i=0;i<NumPOC;i++)
				{
					POCPos[i] = *(LPLONG)&pRec[recloc];
					recloc += 4;
					POCFlag[i] = *(LPLONG)&pRec[recloc];
					recloc += 4;
					POCCoord[i].x = *(LPDOUBLE)&pRec[recloc];
					recloc += 8;
					POCCoord[i].y = *(LPDOUBLE)&pRec[recloc];
					recloc += 8;
					recloc += 4;
					ConvertCoord(&POCCoord[i],0,1);
				}*/
				recloc = reclocSave;
			}
	        GlobalUnlock (hPoints); 
	        GlobalUnlock (hPolyPartLen);
	        GlobalUnlock (hPolyPartLenNew);
	        GSSiGlobUlFree (&hPartIndex);  
			//if (nPoly > 1)
			//	NumPOC = 0;
			pPoints = (HPDPOINT)GlobalLock (hPoints);//pPoints[9]
			doWindowCheck =  (SHPPolyHeader.Type == SHPT_ARC);
			for (i=0;i<NumPoints;i++)
			{
				ConvertCoord(&pPoints[i],0,1);  
				if (doWindowCheck && PtInWBounds (&pPoints[i]))
					doWindowCheck = FALSE;
			}
			if (doWindowCheck)
			{
				GSSiGlobUlFree (&hPoints);
				GSSiGlobFree (&hPolyPartLen);
				GSSiGlobFree (&hPolyPartLenNew);
				break;
			}
			if (SaveContours (NumPoints,pPoints,CurrentUDI))
			{
				GSSiGlobUlFree (&hPoints);
				GSSiGlobFree (&hPolyPartLen);
				GSSiGlobFree (&hPolyPartLenNew);
				break;
			}
			//if (SHPPolyHeader.Type == SHPT_POLYLINE_WITHCURVES || SHPPolyHeader.Type == SHPT_POLYLINE_WITHCURVESANDZ || SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES)
			if (NumPOC)
			{
				DPOINT	RP, BP, EP, POC;  //pPoints[3]
				//LPDPOINT RP = (LPDPOINT)(&pRec[recloc]+12);		//(LPDPOINT)(&pRec[recloc]+28)
				double	Radius, AZ, BackAZ;
				LPDPOINT	pPoints2;
				HANDLE		hPoints2;
				long		NumPoints2, NumPointsOrig, NumPointsAdded;// , nAddedPoints = 0;
				UINT		ipoc;
				LPINT		pNumPointsChk = (LPINT)GlobalLock (hPolyPartLen);
				int			totpChk = 0;

				db_dopoly=TRUE;
//SHPPolyHeader.Type == SHPT_POLYLINE_WITHCURVES;
	
				hPoints2 = GSSiGlobAlloc (0,GMEM_MOVEABLE,(1024*NumPOC+NumPoints*2)*sizeof(DPOINT));
				pPoints2 = (HPDPOINT)GlobalLock (hPoints2); 
				NumPoints2 = 0;
				for (i=0;i<NumPoints;i++)
				{
char	str[32]="";
/*if (dbug)
{
	DisplayMarkers = TRUE;
	itoa(i, str, 10);
	SetTextColor(CurView->hDC, RGB(255, 0, 0));
	DisplayMarker(pPoints[i], 2, str, 0.16, 0, RGB(255, 0, 0), FALSE, FALSE, NULL, NULL, 0, 0, 0);
}*/

				/*	if (i >= totpChk + *pNumPointsChk)
					{
						totpChk += *pNumPointsChk;
						pNumPointsChk++;
						if (nAddedPoints)
							totpChk++;
						nAddedPoints++;
					}*/
					for (ipoc = 0;ipoc < NumPOC;ipoc++)
					{
						if (POCPos[ipoc] == i)//-nAddedPoints)
						{
							BP = pPoints[i];
							EP = pPoints[i+1];
							switch (POCFlag[ipoc])
							{
							case 0:
							case 1:
								POC = POCCoord[ipoc];
								break;
							case 5:
								{
									DPOINT	MidPoint = MidPointD (BP,EP);

									RP = POCCoord[ipoc];
									Radius = ldistpp (&RP,&BP); 
									AZ = getazd (&RP,&MidPoint);
									POC = dnewpt (RP,AZ, Radius);  
								}
								break;
							default:
								ii=1;
							}
							NumPointsOrig = NumPoints2;
							/*if (dbug)
							{
								itoa(ipoc, str, 10);
								SetTextColor(CurView->hDC, 0);
								DisplayMarker(POC, 2, str, 0.16, 0, 0, FALSE, FALSE, NULL, NULL, 0, 0, 0);
							}*/
							if (!Display || FastMapCopy)
								CurvePointsD(&BP,&POC,&EP, &NumPoints2, &pPoints2,&BackAZ,1020,CurveChordDist,1);
							else
                				CurvePointsD(&BP,&POC,&EP, &NumPoints2, &pPoints2,&BackAZ,1020,CurView->BaseUnitsPerPixel,1); 
							NumPointsAdded = NumPoints2 - NumPointsOrig - 1;//sub 1 to account for PT
							if (NumPointsAdded > 0)
							{
								UINT	j;
								int		totp=0;

								pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
								pNumPointsNew = (LPINT)GlobalLock (hPolyPartLenNew);
								for (j=0;j<nPoly;j++,pNumPoints++,pNumPointsNew++)
								{
									if (i >= totp && i<totp + *pNumPoints)
									{
										*pNumPointsNew  += NumPointsAdded;
									}
									totp += *pNumPoints;
								}
								GlobalUnlock (hPolyPartLen);
								GlobalUnlock (hPolyPartLenNew);
							}
							goto NextPt;
 						}
					}
					*pPoints2++ = pPoints[i];
					NumPoints2++;
NextPt:;
				}
			//	Radius = ldistpp (RP,&BP); 
			//	AZ = getazd (RP,&MidPoint);
			//	POC = dnewpt (*RP,AZ, Radius);  
			//	POC = *RP;
				GSSiGlobUlFree (&hPoints);
                GlobalUnlock (hPoints2);
				GSSiGlobUlFree(&hPolyPartLen);
				hPolyPartLen = hPolyPartLenNew;
				hPolyPartLenNew = 0;
				hPoints = hPoints2;
				NumPoints = NumPoints2;
				pPoints = (HPDPOINT)GlobalLock (hPoints);
//				if (Pick)
//					PickCurve (lpDCurPoints,nPnts,&BP,&POC,&EP);
				if (nPoly != 1)
					ii = 1;
				if (SHPPolyHeader.Type == SHPT_POLYLINE_WITHCURVES || SHPPolyHeader.Type == SHPT_POLYLINE_WITHCURVESANDZ)
					SHPPolyHeader.Type = SHPT_ARC;
				else if (SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES)
					SHPPolyHeader.Type = SHPT_POLYGON;	

			}
			else if (SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES)
			{   
				LPLONG	pNumRP = (LPLONG)(&pRec[recloc]);
				short	irp;   
				typedef struct	{long	InsertAfter,RP2;
								 DPOINT	RP;
								 long	RP3;} RPBLOCK;
				typedef	RPBLOCK	FAR	*LPRPBLOCK;
				
				LPRPBLOCK	pRPBlock;
				
				recloc += 4;   
				pRPBlock = (LPRPBLOCK)(&pRec[recloc]);
				for (irp = 0;irp < *pNumRP; irp++)
				{
					pRPBlock++;
				}
			/*	DPOINT	MidPoint = MidPointD (BP,EP);
				double	Radius, AZ, BackAZ;
				
				ConvertCoord(RP,0,1);
				Radius = ldistpp (RP,&BP); 
				AZ = getazd (RP,&MidPoint);
				POC = dnewpt (*RP,AZ, Radius);  
				GSSiGlobUlFree (&hPoints);
				hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,4090*sizeof(DPOINT));
				pPoints = (HPDPOINT)GlobalLock (hPoints); 
				NumPoints = 0;
				if (!Display)   
                    CurvePointsD(&BP,&POC,&EP, &NumPoints, &pPoints,&BackAZ,4090,CurveChordDist,1);
                else
                	CurvePointsD(&BP,&POC,&EP, &NumPoints, &pPoints,&BackAZ,4090,DisplayCurveFactor,1); 
                GlobalUnlock (hPoints);
				pPoints = (HPDPOINT)GlobalLock (hPoints);
				pNumPoints = (LPWORD)GlobalLock (hPolyPartLen);
				*pNumPoints = NumPoints;
		        GlobalUnlock (hPolyPartLen);
				nPoly = 1;*/
				SHPPolyHeader.Type = SHPT_POLYGON;	
			}
			else if (SHPPolyHeader.Type == -1610612685)
			{
				double	Radius = ldistpp (&pPoints[0],&pPoints[1]);
				LPDPOINT	pUSPoints;
		        
				pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
				if (Radius)
				{
					HANDLE hCoords = CreateCirclePoly (pPoints[0],Radius,pNumPoints,0);
					GSSiGlobUlFree (&hPoints);
					pPoints = (HPDPOINT)GlobalLock (hCoords);
					hPoints = hCoords;
				}
				GlobalUnlock (hPolyPartLen);
				SHPPolyHeader.Type = SHPT_POLYGONM;	
			}

			if (pFGDBRecHeader->shapeType == SHPT_TEXT)
			{
				GRTEXTHEADER	GRTextHeader;  
	            DPOINT	MidPoint;// = MinMaxMidPointD ((LPMNMXCORD)&SHPPolyHeader.Xmin);  
	            double	MidPointAZ; 
	            char	txt[512]="[pgdb.TEXTSTRING]";
	            char	word[256];
	            short	nchar; 
	            double	THeight, AZ;
				int		MDPointID;
	            char	THeightC[32]="[pgdb.FONTSIZE]";   
	            char	AZC[32]="[pgdb.ANGLE]";
				int		type;
       			int		nWords, nWords2, code, lProjection, Curloc;
				DPOINT	Points2[4];
				BOOL	CurveText = FALSE;
				DPOINT	PC,POC,PT;
				short	vjus=1, hjus=0;
	            
				sprintf (txt,"[%s.TEXTSTRING]",FileID[IsFGDB]);
				sprintf (AZC,"[%s.ANGLE]",FileID[IsFGDB]);
				sprintf (THeightC,"[%s.FONTSIZE]",FileID[IsFGDB]);
		    	ExpandText (THeightC);
				if (!*THeightC)
				{
					strcpy(THeightC, "[%FONTSIZE]");
					ExpandText(THeightC);
				}
		    	THeight = atof (THeightC)*NonPltFileDistToBaseDist * 1.55165; 
				pNumPoints = (LPINT)GlobalLock (hPolyPartLen);    
				lpDCurPoints = pPoints;
	    		ExpandText (txt);
	    		ExpandText (AZC);
				if (hElement)
				{
					type = *(LPSHORT)(pElement + 4);
					if (type && type != 2)
						ii=1;
					code = *(LPSHORT)(pElement + 24);
					if (BinSizeE > 26 && code == 124)
						CurveText = TRUE;
				}
				else
					type = 1;
				if (CurveText || type)
					nWords = 1;
				else
					nWords = *(LPLONG)(pElement + 24);
				if (nPoly > 1 && type)
					debugvalue++;
				else if (type == 2 && *pNumPoints != 5)
					ii = 1;
				if (displayTextPoly)
        		for (i=0;i<nPoly;i++)
        		{   
        			int	np=*pNumPoints;
					GWPolylineD (hDC,lpDCurPoints,np,0); 
					lpDCurPoints+=*pNumPoints++;
				}
				GlobalUnlock (hPolyPartLen);
 				pNumPoints = (LPINT)GlobalLock (hPolyPartLen);    
				lpDCurPoints = pPoints;
	       		for (i=0;i<nWords;i++)
        		{   
        			int	np=*pNumPoints;
        			int	nchr = (np-1)/4,j;
        			
					nPnts = 1;
        			TLSet = FALSE;
        			if (type)
					{
       					_fstrcpy (word,txt);
					}
        			else
        			{
						vjus = 0;
						lpDCurPoints = Points2;
	        			if (!i)
						{
							if (CurveText)
							{
								char	NewName[64];
								LPDPOINT	pDPoint, pDPointBeg;
								HANDLE	hCurve;
								double	BackAZ;
								DPOINT	P[2], MP[2];
								double	d, az;

								hjus = vjus = 1;
								strcpy (word,txt);
							/*	nCurveText++;
								sprintf (NewName,"c:\\curvtext%i.bin",nCurveText);
								GSSiRename ("c:\\temp.bin",NewName);*/
								Curloc = 32;
								P[0].x = *(LPDOUBLE)(pElement + Curloc) * PGDBCnvFac;
								P[0].y = *(LPDOUBLE)(pElement + Curloc + 8) * PGDBCnvFac;
								P[1].x = *(LPDOUBLE)(pElement + Curloc + 16) * PGDBCnvFac;
								P[1].y = *(LPDOUBLE)(pElement + Curloc + 24) * PGDBCnvFac;
					//GWPolylineD (hDC,P,2,0); 
								Curloc = 76;
								PC.x = *(LPDOUBLE)(pElement + Curloc) * PGDBCnvFac;
								PC.y = *(LPDOUBLE)(pElement + Curloc + 8) * PGDBCnvFac;
								PT.x = *(LPDOUBLE)(pElement + Curloc + 16) * PGDBCnvFac;
								PT.y = *(LPDOUBLE)(pElement + Curloc + 24) * PGDBCnvFac;
								P[0] = PC;
								P[1] = PT;
								MP[0] = MidPointD (P[0],P[1]);
					//GWPolylineD (hDC,P,2,0); 
								Curloc = 120;
								P[0].x = *(LPDOUBLE)(pElement + Curloc) * PGDBCnvFac;
								P[0].y = *(LPDOUBLE)(pElement + Curloc + 8) * PGDBCnvFac;
								P[1].x = *(LPDOUBLE)(pElement + Curloc + 16) * PGDBCnvFac;
								P[1].y = *(LPDOUBLE)(pElement + Curloc + 24) * PGDBCnvFac;
								MP[1] = MidPointD (P[0],P[1]);
					//GWPolylineD (hDC,P,2,0); 
								d = ldistp (MP[0],MP[1]);
								az = getazd (&MP[1],&MP[0]);
								POC = dnewpt (MP[1],az,d*0.27);
								hCurve = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(DPOINT)*4096);
								pDPoint = pDPointBeg = GlobalLock (hCurve);
								np = 0;
								CurvePointsD(&PC,&POC,&PT, &np, &pDPointBeg,&BackAZ,4090,DisplayCurveFactor,1);
					//GWPolylineD (hDC,pDPoint,np,0); 
								GSSiGlobUlFree (&hCurve);
								CurrentType = GF_CURVE;
	    						PTRot = getazd (&PC,&PT); 
								BP = PC;
								EP = PT;
								CurPOCW = POC;
								goto ExitText;
							}
							else
							{
			   					nWords2 = GetWords (txt);
								code = *(LPSHORT)(pElement + 44);
								if (!CheckCode (RecordNumber,pElement,BinSizeE,code,3)) goto badcode;
								code = *(LPSHORT)(pElement + 92);
								if (!CheckCode (RecordNumber,pElement,BinSizeE,code,8)) goto badcode;
								lProjection = *(LPLONG)(pElement + 100);
								if (!CheckCode (RecordNumber,pElement,BinSizeE,lProjection,544)) goto badcode;
								Curloc = 104;
								Curloc += lProjection;
								Curloc += 42;
								code = *(LPLONG)(pElement + Curloc);Curloc += 4;
								if (!CheckCode (RecordNumber,pElement,BinSizeE,code,64)) goto badcode;
								Curloc += 12;
								code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						// can be 63 or 65		if (!CheckCode (RecordNumber,pElement,BinSizeE,code,65)) goto badcode;
								Curloc += 53;
							}
						}
						strcpy (word,Words[i]);
			   		/*	for (j=0;j<nWords;j++)
	        			{ 
	        				if (WordLen[j] == nchr)
	        					goto HaveWord;	
	        			}
	        			j = 0;
	       	HaveWord:   _fstrcpy (word,Words[j]);
	   					nWords--;
	       				if (j < nWords)
	       				{
	       					_fmemmove (&WordLen[j],&WordLen[j+1],(nWords-j)*2);
	       					_fmemmove (&Words[j],&Words[j+1],(nWords-j)*128);
	       				}*/
						code = *(LPSHORT)(pElement + Curloc);Curloc += 2;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,2)) goto badcode;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,80)) goto badcode;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,3)) goto badcode;
						lpDCurPoints[0].x = *(LPDOUBLE)(pElement + Curloc) * PGDBCnvFac;
						lpDCurPoints[0].y = *(LPDOUBLE)(pElement + Curloc + 8) * PGDBCnvFac;
						lpDCurPoints[1].x = *(LPDOUBLE)(pElement + Curloc + 16) * PGDBCnvFac;
						lpDCurPoints[1].y = *(LPDOUBLE)(pElement + Curloc + 24) * PGDBCnvFac;
					//GWPolylineD (hDC,lpDCurPoints,2,0); 
						Curloc += 32;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,1)) goto badcode;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,2)) goto badcode;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,0)) goto badcode;
						lpDCurPoints[0].x = *(LPDOUBLE)(pElement + Curloc) * PGDBCnvFac;
						lpDCurPoints[0].y = *(LPDOUBLE)(pElement + Curloc + 8) * PGDBCnvFac;
						lpDCurPoints[1].x = *(LPDOUBLE)(pElement + Curloc + 16) * PGDBCnvFac;
						lpDCurPoints[1].y = *(LPDOUBLE)(pElement + Curloc + 24) * PGDBCnvFac;
					//GWPolylineD (hDC,lpDCurPoints,2,0); 
						Curloc += 32;
						Curloc += 18;
						lProjection = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,lProjection,544)) goto badcode;
						Curloc += lProjection;
						Curloc += 42;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,64)) goto badcode;
						Curloc += 12;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
					// can be 63 or 65	if (!CheckCode (RecordNumber,pElement,BinSizeE,code,65)) goto badcode;
						Curloc += 53;
	       			}	
        			MidPoint =  lpDCurPoints[0];//MidPointD (lpDCurPoints[0],lpDCurPoints[nchr*2]);   
				//	RemoveDupPolyPoints (&np,lpDCurPoints,THeight/10);
					if (!type)
					{
						AZ = getazd (&lpDCurPoints[0],&lpDCurPoints[1]);
					}
					else
					{
	        			// removed for MSP data THeight = ldistpp (&lpDCurPoints[0],&lpDCurPoints[1]); 
						AZ = getazd (&lpDCurPoints[1],&lpDCurPoints[nchr*2]);
					}
					//DisplayPoint (hDC,BasePtToWinPt (&MidPoint));
					//if (ConvertCoord(&MidPoint,0,1)) 
					//	goto RtnFalse;
					CurPointLocD.x = MidPoint.x; 
					CurPointLocD.y = MidPoint.y;
					TXLoc = CurPointLocD; 
		    		LastElementBeginPoint = CurPointLocD;     
		    		//MidPointAZ = RADDEG * atof (AZC);
		    		MidPointAZ = AZ; 
		    		PTRot = MidPointAZ; 

ExitText:
					HaveTXLoc = TRUE;
		    		nchar = _fstrlen (word);
		    		_fmemset (&GRTextHeader,0,sizeof(GRTextHeader));
		    		GRTextHeader.lText = nchar;
					GRTextHeader.FontNum = 8;  
				 
					GRTextHeader.vJust=vjus;//0=above,1=baseline,2=center,3=below
					GRTextHeader.hJust=hjus;
					SetTextHeadSize (&GRTextHeader,THeight);
				    SetTextLocVars (&TLSet); 
				    if (CopyRec)
				    {   
				    	HANDLE hGRText=0;
				                
						if (UpdateItem == 20)
		        			CurPointLocD = NewPointD;
						if (UpdateItem == 201)
		        			AdjustPointRotation (&PTRot);
						if (UpdateItem == 19)
						{   
							LPGRTEXTHEADER pGRTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);	 
							LPSTR	pText = (LPSTR) (pGRTextHeader+1);
									
				    		nchar = pGRTextHeader->lText; 
							pGRTextHeader->UltiMapStyle = FALSE;
							pGRTextHeader->hJust += 2;
		            		hGRText = GRTextFromTextHeader (pGRTextHeader,pText);
							GlobalUnlock (hPickedTextHeader);	
		            	}
		            	else
						{
							GRTextHeader.UltiMapStyle = FALSE;
							GRTextHeader.hJust += 2;
		            		hGRText = GRTextFromTextHeader (&GRTextHeader,word);
						}
						if (CurveText)
						{
							int	npnts = 3;
							HANDLE	hPoints=GSSiGlobAlloc (0,GMEM_MOVEABLE,3*sizeof(DPOINT));
							LPDPOINT Points = GlobalLock (hPoints);

							Points[0] = BP;
							Points[1] =	CurPOCW;
							Points[2] = EP;

							AddPolyToBuffer (1,&npnts,&hPoints,3,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
                                     		 -1,-1,0,hGRText,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
							GSSiGlobUlFree (&hPoints);
						}
						else
						{
								if (nWords == 1)
									opt = 0;
								else if (!i)
									opt = 1;
								else if (i+1 == nWords)
									opt = 3;
								else
									opt = 2;
				    			AddPointToBuffer (CurPointLocD,CurrentRefno,0,CurrentDesc,10, PTRot,0,hGRText,0,
												  CurrentPrefix,CurrentUDI,-1,-1,-1,TRUE,&hUpdateBuf,&lUpdateBuf,opt);
						}
						GSSiGlobFree (&hGRText);
					}
		        	else if (CurVis->WantType[2] && HaveTXLoc && (CurView->PassID || Pick)) 
		        	{   
		        		COLORREF	OldColor = -1;
				        		
		        		if (hPickedTextHeader)
					    {
					    	LPGRTEXTHEADER pPickedTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);
					    	_fmemmove (pPickedTextHeader,&GRTextHeader,sizeof(GRTEXTHEADER)); 
					    	pPickedTextHeader++;
					    	_fstrcpy ((LPSTR)pPickedTextHeader,txt);
					    	GlobalUnlock (hPickedTextHeader);
					    }
				
						ProcessTextObject (hDC,&GRTextHeader,word,nchar,0,0,0,0,0);
						if (OldColor >= 0)
							SetTextColor (hDC,OldColor); 
						if (CurVis->WantType[8])
							DisplayPointItem (hDC,BasePtToWinPt(&CurPointLocD),10,PTRot,InvisiblePointSymbol,0); 
	        		}
					lpDCurPoints+=*pNumPoints++;
					/*					if (i)
											lpDCurPoints++;*/

				}
badcode:
       			GlobalUnlock (hPolyPartLen); 
			} 
			else if (SHPPolyHeader.Type == SHPT_ARC || SHPPolyHeader.Type == SHPT_ARCZ || SHPPolyHeader.Type == shapePolylineZ) 
			{
				if (Pick)
				{   
					PickPolylineD (pPoints,NumPoints,0,2,0,0,0);
				}
				else if (CopyRec)  
				{
					pNumPoints = (LPINT)GlobalLock (hPolyPartLen);//pNumPoints[1]
					if (nPoly > 1)
					{
						HANDLE	hhPoly = GSSiGlobAlloc ( 418,GMEM_MOVEABLE,sizeof(HANDLE)*nPoly); 
						LPHANDLE phPoly = (LPHANDLE)GlobalLock (hhPoly);
                        HPDPOINT	pPoints1=(HPDPOINT)GlobalLock (hPoints), pPoints2;
                        
	                    for (i=0;i<nPoly;i++)
	                    {   
	                   		phPoly[i] = GSSiGlobAlloc ( 420,GMEM_MOVEABLE,sizeof(DPOINT)*(long)pNumPoints[i]);
		                    pPoints2= (HPDPOINT)GlobalLock (phPoly[i]);  
		                    hmemmove ((HPSTR)pPoints2,(HPSTR)pPoints1,sizeof(DPOINT)*(long)pNumPoints[i]);
	                        GlobalUnlock (phPoly[i]); 
	                        pPoints1 += pNumPoints[i];
	                     //   if (i)
	                     //   	pPoints1++;
	                    } 
	                    GlobalUnlock (hPoints);
						AddPolyToBuffer (nPoly,pNumPoints,phPoly,TYPE_POLYLINE,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
	                                     -1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
		                for (i=0;i<nPoly;i++)
		                	GSSiGlobFree (&phPoly[i]);
	                    GSSiGlobUlFree (&hhPoly);
					}
					else
						AddPolyToBuffer (nPoly,pNumPoints,&hPoints,TYPE_POLYLINE,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
                                     	-1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
                    GlobalUnlock (hPolyPartLen);
				}
				else
				{    
					lpDCurPoints = pPoints;
					nPnts = nCurPoints = NumPoints;

					if (PolyInMaskAreaFileCoord (GF_LINE,&NumPoints,0,0,&pPoints,TRUE))
					{   
						HPEN hNewPen=0;
						
						hOldPen  = SelectObject(hDC,GetStockObject(BLACK_PEN));
						TempLineColor = SHPColor; 
						TempLineWidth = SHPWidth; 
						if (TempLineColor >= 0 || TempLineWidth != 0)
						{
							hNewPen = CreatePen (PS_SOLID,(short)IDNINT(TempLineWidth*DeviceToScreenFactor()),TempLineColor);
							SelectObject(hDC,hNewPen);
						}
//						if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
//						{   
							pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
			        		for (i=0;i<nPoly;i++)
			        		{ 
								LPDPOINT savelpDCurPoints = lpDCurPoints;

//								GWPolylineD (hDC,lpDCurPoints,*pNumPoints,CurrentDesc); 
//								lpDCurPoints+=*pNumPoints++;
			        			nPnts = *pNumPoints;  
			        			InDynamicSegmentation = FALSE;  
			        			DoDynamicFixedSegmentation (-1,0,0);
								while (nPnts > 1)
								{ 
									if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0)
									{   
										if (db_dopoly)
											GWPolylineD (hDC,lpDCurPoints,NumDynSegPoints,CurrentDesc); //lpDCurPoints[2]
									}
									DynSegEndPoint = lpDCurPoints[NumDynSegPoints-1];  
									lpDCurPoints += (NumDynSegPoints - 2);
									nPnts = NumDynSegPointsRemaining;
									if (nPnts > 1)
									{   
										lpDCurPoints[0] = DynSegEndPoint;
										lpDCurPoints[1] = DynSegSavePoint;  
										InDynamicSegmentation = TRUE;
									}	
								}
								lpDCurPoints = savelpDCurPoints;
			        			InDynamicSegmentation = FALSE;
								lpDCurPoints+=*pNumPoints++; 
								//if (i)
								//	lpDCurPoints++;
			        		}   
			        		GlobalUnlock (hPolyPartLen); 
//						} 
						if (hNewPen)
						{
							SelectObject(hDC,GetStockObject(BLACK_PEN)); 
							GSSiDeleteObject (&hNewPen);
						}
					}
				}
			}
			else
			{   
				BOOL	ShowBorder = GetBit (5,(LPSTR)&CurVis->WantType[7]); 
				HPEN	hBorderPen=0;
				
				if (Pick)
				{   
					PickPolygonD (pPoints,NumPoints,nPoly,hPolyPartLen,9999999,0);
				}
				else if (CopyRec)  
				{
					pNumPoints = (LPINT)GlobalLock (hPolyPartLen);//pNumPoints[1]
					if (nPoly > 1)
					{
						HANDLE	hhPoly = GSSiGlobAlloc ( 418,GMEM_MOVEABLE,sizeof(HANDLE)*nPoly); 
						LPHANDLE phPoly = (LPHANDLE)GlobalLock (hhPoly);
                        HPDPOINT	pPoints1=(HPDPOINT)GlobalLock (hPoints), pPoints2;
                        
	                    for (i=0;i<nPoly;i++)
	                    {   
	                   		phPoly[i] = GSSiGlobAlloc ( 420,GMEM_MOVEABLE,sizeof(DPOINT)*(long)pNumPoints[i]);
		                    pPoints2= (HPDPOINT)GlobalLock (phPoly[i]);  
		                    hmemmove ((HPSTR)pPoints2,(HPSTR)pPoints1,sizeof(DPOINT)*(long)pNumPoints[i]);
	                        GlobalUnlock (phPoly[i]); 
	                        pPoints1 += pNumPoints[i];
	                        if (i)
	                        	pPoints1++;
	                    } 
	                    GlobalUnlock (hPoints);
						AddPolyToBuffer (nPoly,pNumPoints,phPoly,TYPE_AREA,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
	                                     -1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
		                for (i=0;i<nPoly;i++)
		                	GSSiGlobFree (&phPoly[i]);
	                    GSSiGlobUlFree (&hhPoly);
					}
					else
						AddPolyToBuffer (nPoly,pNumPoints,&hPoints,TYPE_AREA,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
                                     	-1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
                    GlobalUnlock (hPolyPartLen);
                }
				else
				{    
					if (Display)
					{
						hOldPen  = SelectObject(hDC,h0Pen);
						if (ShowBorder)
		    				hBorderPen = hAreaBorderPen[TRUE];
			        	if (GetInVisibility(CurrentDesc))
			        	{
			        		if (CurView->HaveLayerColor[FileNum])
			        		{
			        			hTempBrush = CreateSolidBrush(CurView->LayerColor[FileNum]);
								hOldBrush = SelectObject (hDC,hTempBrush);
								hTempPen = CreatePen (PS_SOLID,0,CurView->LayerColor[FileNum]);
                                SelectObject (hDC,hTempPen);
                            }
			        		else
								hOldBrush = SelectRandomBrush (hDC,CurrentRefno,&hBorderPen,CurrentDesc); 
						}
					//	if (hBorderPen)
					//		SelectObject(hDC,hBorderPen); 
					}
					/*lpDCurPoints = pPoints;
					nPnts = nCurPoints = NumPoints;

					if (PolyInMaskAreaFileCoord (GF_AREA,&NumPoints,0,0,&pPoints,TRUE))
					{
						if (SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0)
						{   
							BOOL DoBorder = FALSE;
							
							if (Display)
							{
								if (nPoly > 1 && !ShowLinkLines)
									SelectObject(hDC,h0Pen); 
								if (FillAreas && (GetBit (7,(LPSTR)&CurVis->WantType[7]) ||
												  GetBit (6,(LPSTR)&CurVis->WantType[7])))
									hDeletePen = GWPolygonD (hDC,pPoints, NumPoints, nPoly, hPolyPartLenNew,CurrentDesc,ShowBorder,TRUE,0);
								else
									DoBorder = TRUE; 
								if ((DoBorder || nPoly > 1) && (hBorderPen || hDeletePen))
								{   
									if (hDeletePen)
										SelectObject(hDC,hDeletePen); 
									else
										SelectObject(hDC,hBorderPen); 
									pNumPoints = (LPINT)GlobalLock (hPolyPartLenNew);
					        		for (i=0;i<nPoly;i++)
					        		{   
										GWPolylineD (hDC,lpDCurPoints,*pNumPoints,0); 
										lpDCurPoints+=*pNumPoints++;
										if (i)
											lpDCurPoints++;
					        		}   
					        		GlobalUnlock (hPolyPartLenNew); 
					        	}
					        }
						}
					}*/
					lpDCurPoints = pPoints;
					nPnts = nCurPoints = NumPoints;
					if (PolyInMaskAreaFileCoord (GF_AREA,&NumPoints,0,0,&pPoints,TRUE))
					{
						int	SDCrtn = SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI);
							
						if (SDCrtn > 0)
							ProcessPolygon (hDC,ShowBorder,PltType,0,hTempPen,0);
						else if (SDCrtn < 0)
							HaveTXLoc = TRUE;
					}

				}
			}
			GSSiGlobFree (&hPolyPartLen);
			GSSiGlobFree (&hPolyPartLenNew);
			GSSiGlobUlFree (&hPoints);
		break;
		
		case geometryMultipoint:
		break;
	}
	if (hOldBrush)
		SelectObject (hDC,hOldBrush);
	if (hOldPen)
		SelectObject (hDC,hOldPen); 
   	if (hDeletePen != h0Pen)
   		GSSiDeleteObject (&hDeletePen);
   	GSSiDeleteObject (&hTempBrush);
   	GSSiDeleteObject (&hTempPen);
	ShowValue (hDC,FALSE);
	InGraphicsProcessor = FALSE; 
	if (hElement)
		GlobalUnlock (hElement); 
	GlobalUnlock (hRec);
{
#if ENABLETRACE
GSSiExitProg (1375);
#endif
	return TRUE;
}
RtnFalse:
	if (hElement)
		GlobalUnlock (hElement); 
	GlobalUnlock (hRec);
	InGraphicsProcessor = FALSE;
{
#if ENABLETRACE
GSSiExitProg (1375);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}
BOOL ProcessPGDBRecord (HDC hDC,long RecordNumber,int IsFGDB)
#if ENABLETRACE
{GSSiEnterProg (1375);
#endif
{ 
    HPDPOINT    pPoints, pFirstPoint;
    HPLONG      pPartIndex;   
    ULONG		i;
    short		Type, ltag;
    HANDLE		hPoints, hPartIndex;  
    LPINT		pNumPoints, pNumPointsNew;
    long		NumPoints; 
    HPEN		hOldPen=0, hDeletePen=0, hTempPen=0;
	HBRUSH		hOldBrush=0, hTempBrush=0;  
	DPOINT		DPoint,DynSegEndPoint;
	double		size;
    static		short	dbugid=6;
    short		ii;
    LPBYTE		pRec,pElement;
    HANDLE		hRec,hElement=0;
    char		str[512];  
    long		recloc=sizeof(SHPPOLYHEADER); 
    long		BinSizeR,BinSizeE;  
    static	long	DebugRecNum=21291;
	static	int	nCurveText=0;
	BOOL	save=FALSE;
	static	BOOL	db_dopoly=TRUE;
	int		opt;
	HANDLE	hPolyPartLenNew=0;
	char	FileID[2][6]={"PGDB","FGDB"};
	LPFILEGDBRECHEADER pFGDBRecHeader;
    
    if (CurView->ID == dbugid)
    	ii=1; 
    //if (RecordNumber == DebugRecNum)
    //	save = TRUE;
    SHPHeader.ShapeType = SHPType;//SHPT_POLYGONZ;
    sprintf (str,"[%s.%s]",FileID[IsFGDB],ShapeFieldName);
    ExpandText (str);
    hRec = (HANDLE) atol (str); 
    BinSizeR = GlobalSize (hRec);
    pRec = GlobalLock (hRec);  
	if (IsFGDB)
	{
		LPCURVAL pCurVal = (LPCURVAL)pRec;

		pRec = (LPBYTE)&pCurVal->Value;
		pFGDBRecHeader = (LPFILEGDBRECHEADER)pRec;
		pRec += sizeof (FILEGDBRECHEADER);
		BinSizeR -= (sizeof (FILEGDBRECHEADER) + sizeof(int));
		if (pFGDBRecHeader->hasCurves)
			ii=1;
	}
    sprintf (str,"[%s.ELEMENT]",FileID[IsFGDB]);
    ExpandText (str);
	hElement = (HANDLE) atol (str);      
	if (hElement)
	{
		BinSizeE = GlobalSize (hElement);
		pElement = GlobalLock (hElement);
		if (save)
		{
			HFILE FidDB=GSSiOpenFile ("c:\\temp.bin",0,OF_CREATE);
			BigWrite (FidDB,pElement,BinSizeE,-1);
			GSSiClose2 (&FidDB);
		}
	}
	ShowValue (hDC,FALSE);  
	InGraphicsProcessor = TRUE;
	ItemIsDeleted = FALSE;
	InitRecord (hDC); 
    SetSHPParms (RecordNumber);  
	ItemSeg = RecordNumber;  
	if (CurView->PassID && CurView->PassID < 4)
	{
		if ((SHPType == SHPT_POLYGON || SHPType == SHPT_POLYGON_PGDB || SHPType == SHPT_PGDB_POLYGONZ) && CurView->PassID != 2)
			goto RtnFalse;  
		if ((SHPType == SHPT_POINT || SHPType == SHPT_POINTZ || SHPType == SHPT_ARC || SHPType == SHPT_ARCZ || SHPType == SHPT_ARCM) && CurView->PassID == 2)
			goto RtnFalse;
		if (SHPType == SHPT_TEXT && CurView->PassID != 3)
			goto RtnFalse;  
	}
    if (!CurrentDesc)
    	goto RtnFalse;
	if (GRStartTime > TimeRangeEnd || GREndTime < TimeRangeBeg)
		goto RtnFalse;
    ltag = _fstrlen (SHPTag);
	SetSymNum (CurrentDesc);
	if (!ProcessRefAndTAG (GetVisibility (CurrentDesc),SHPTag,ltag))
		goto RtnFalse;
	if (!Pick && hDC && Display)
	{
		SetROP2(hDC,DisplayRasterOpt);
		if (HaveVarFillColor)
			SetTextColor (hDC,ConvertColor(GlobalColors[0],CurrentDesc));
		else
			SetTextColor (hDC,ConvertColor(DefaultTextColor,CurrentDesc));
		SelectObject (hDC,GetStockObject(BLACK_PEN));
		GSSiDeleteObject (&hBlackPen);
		hBlackPen = CreatePen (PS_SOLID,0,ConvertColor(0,CurrentDesc));		
		SelectObject (hDC,hBlackPen);
	}
	HiPrecis = TRUE; 
	CurrentPen = 0;
	switch (SHPType)
	{
		case SHPT_POINTZ:
			if (!SHPPointZRec.Type)
				goto RtnFalse;
			DPoint = SHPPointZRec.Point; 
		case SHPT_POINT:
		{
			int	SDCrtn = 0;

			if (SHPType == SHPT_POINT)
			{
				if (!SHPPointRec.Type)
					goto RtnFalse;
				DPoint = SHPPointRec.Point; 
			}
			if (ConvertCoord(&DPoint,0,1)) 
				goto RtnFalse;
	        lpDCurPoints = &DPoint;  
			CurrentPoint = CurPointLocD = DPoint;
	        LastElementBeginPoint = LastElementEndPoint = DPoint;
	        nPnts = 1; 
			CurPointLoc = BasePtToWinPt(lpDCurPoints); 
    		if (!PointInMaskAreaWinCoord (CurPointLoc))
    			goto RtnFalse;
    		if (PointIsBlocked (&CurPointLocD,CurrentDesc))
    			goto RtnFalse;   
       		HaveTXLoc = TRUE;
    		CurrentType = GF_POINT; 
			if (CurPointSize < 0)
				CurPointSize = -CurPointSize * DeviceToScreenFactor();
			else
				CurPointSize /= CurView->BaseUnitsPerPixel;
			if ((Pick||PickingByRefno) && GetTypeVisibility(TYPE_POINT))
			{
				PickPointItemD(lpDCurPoints, (CurPointSize*ThemeWidthFactor*GraphicsPointFactor)*CurView->BaseUnitsPerPixel, PTRot, CurrentDesc);
			} 
			else if (GetTypeVisibility(TYPE_POINT))
			{   
				short	iDesc=CurrentDesc;

				if (CopyRec)
				{   
				    AddPointToBuffer (CurPointLocD,CurrentRefno,0,CurrentDesc,10, PTRot,0,0,0,
									  CurrentPrefix,CurrentUDI,-1,-1,-1,TRUE,&hUpdateBuf,&lUpdateBuf,0);
				}
				else
				{
	 				if (CurrentDesc > 0 && CurrentDesc < 3201)   
	 				{
						if (TSize)
							CurView->CurVisType[CurrentDesc]=5;
						else
							CurView->CurVisType[CurrentDesc]=4;
					}
					HighlightPointSym=FALSE;
					if (SHPColor >= 0)
					{  
						GlobalColors[0]=SHPColor;  
						HaveVarFillColor = TRUE;
					}
					if (!GetTypeVisibility(6) && SymbolIsVisible (iDesc)) 
					{
						CurPointSize = 10*DeviceToScreenFactor();
						iDesc = InvisiblePointSymbol;
					}
					if ((SDCrtn = SetDisplayChar (hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI)) > 0)
					{   
						if (ThemePointSym)
						{
							iDesc = ThemePointSym;
							if (ThemePointSize < 0)
								size = -ThemePointSize *DeviceToScreenFactor();
							else
								size = ThemePointSize / CurView->BaseUnitsPerPixel; 
							size *= ThemeWidthFactor;
							size = min(max (size,1),MaxPointSize);
						}
						else
							size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor; 
						{
							long	DisplayedWidth=0;

							DisplayPointItem (hDC,CurPointLoc,size,PTRot,iDesc,&DisplayedWidth);
							CurView->MaxSymbolWidth = max (CurView->MaxSymbolWidth,DisplayedWidth);
							CurView->MaxFileDisplayedPointWidth[FileNum] = max (CurView->MaxFileDisplayedPointWidth[FileNum],(DisplayedWidth/ FileDistToWinDist)-(((long)CurrentItemMinMax.xmx)-CurrentItemMinMax.xmn));
						}
						HaveTXLoc = 1;
					} 
				}
			}
   			TXLoc = CurPointLocD; 
       		if (SDCrtn < 0)
				HaveTXLoc = 1;
		}
		break;
		
       	case SHPT_TEXT: 
//       		GetValFromOpenFiles ("pgdb.TEXTSTRING",str); 
//       		GetValFromOpenFiles ("pgdb.FONTNAME",str); 
//       		GetValFromOpenFiles ("pgdb.ANGLE",str);    
			goto DoPoly;
		break;
		case SHPT_ARC:
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=2; 
    		CurrentType = GF_POLYLINE; 
    		if (!GetTypeVisibility(TYPE_LINECURVE))
    			break;
			goto DoPoly; 
		case 4:
		case SHPT_POLYGON:
		case SHPT_POLYGONZ: 
		case SHPT_POLYGONM:  
    		if (!GetTypeVisibility(TYPE_AREA))
    			break;
    		CurrentType = GF_AREA; 
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=3; 
DoPoly:
	        if (SHPPolyHeader.NumPoints > USHRT_MAX)
	        	ii=1;
	        	//goto RtnFalse;
	        if (!SHPPolyHeader.NumPoints)
	        	goto RtnFalse;
	        if (!SHPPolyHeader.Type)
	        	goto RtnFalse;
	        nPoly = SHPPolyHeader.NumParts;  
	        NumPoints = SHPPolyHeader.NumPoints; 
			if (SHPPolyHeader.Type == -1610612685)
				SHPPolyHeader.Type = SHPT_POLYGON;	//added for runway ares for gmavl
	        if (SHPType != SHPT_TEXT && (SHPPolyHeader.Type == SHPT_POLYGON || SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES || SHPPolyHeader.Type == SHPT_POLYGONZ || SHPPolyHeader.Type == SHPT_POLYGONM || SHPPolyHeader.Type == SHPT_PGDB_POLYGONZ))
	        	NumPoints = NumPoints+nPoly-1;
	        hPartIndex = GSSiGlobAlloc (1418,GMEM_MOVEABLE,sizeof(long)*(nPoly+1));
	        hPolyPartLen = GSSiGlobAlloc (1787,GMEM_MOVEABLE,sizeof(int)*(nPoly+1));
	        hPolyPartLenNew = GSSiGlobAlloc (1788,GMEM_MOVEABLE,sizeof(int)*(nPoly+1));
			hPoints = GSSiGlobAlloc (1420,GMEM_MOVEABLE,sizeof(DPOINT)*NumPoints); 
	        pPartIndex = (HPLONG)GlobalLock (hPartIndex); 
	        hmemmove ((HPSTR)pPartIndex,&pRec[recloc],nPoly*sizeof(long));   //pPartIndex[5]
	        recloc += nPoly*sizeof(long);
	        pPartIndex[SHPPolyHeader.NumParts]= SHPPolyHeader.NumPoints;
	                    
            pPoints = pFirstPoint = (LPDPOINT)GlobalLock (hPoints); 
            pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
            pNumPointsNew = (LPINT)GlobalLock (hPolyPartLenNew);
	        for (i=0;i<nPoly;i++)
	        {   
	            long    numpoints, startpoint,ii; 
	                        
	            startpoint = *pPartIndex++;
	            numpoints = *pPartIndex-startpoint; 
	            //if (numpoints > (long)USHRT_MAX)
	            //	MessageBox (0,"Shape file record contains too many points",0,MB_ICONEXCLAMATION);
	            *pNumPoints++ = numpoints; 
				*pNumPointsNew++ = numpoints;
	            hmemmove ((HPSTR)pPoints,&pRec[recloc],numpoints * sizeof(DPOINT));     //pPoints[1]
	            recloc += numpoints * sizeof(DPOINT);
	            pPoints += numpoints;   
	            if (i && SHPType != SHPT_TEXT && ((SHPPolyHeader.Type == SHPT_POLYGON || SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES || SHPPolyHeader.Type == SHPT_POLYGONZ || SHPPolyHeader.Type == SHPT_POLYGONM ||SHPPolyHeader.Type == SHPT_PGDB_POLYGONZ)))
	            	*pPoints++ = *pFirstPoint; 
	        } 
	        ExtraBytes = BinSizeR - recloc; 
/*	        if (ExtraBytes>1)
	        {
	        	HPDPOINT pPoints = (HPDPOINT)&pRec[recloc];//pPoints[3]
	        	HPLONG pPointsl = (HPLONG)&pRec[recloc];//pPointsl[3]
	        	HPFLOAT pPointsf = (HPFLOAT)&pRec[recloc];//pPointsf[2]  
	        	HPSHORT	pPointss = (HPSHORT)&pRec[recloc];//pPointss[7]
				HFILE	Fid;
				char	BinFileName[128];

				sprintf (BinFileName,"c:\\curvedecode\\%i_%i-%i.bin",ExtraBytes,NumPoints,CurrentRefno);
				Fid = GSSiOpenFile (BinFileName,0,OF_CREATE);
				BigWrite (Fid,&pRec[recloc],ExtraBytes,-1);
				GSSiClose2 (&Fid);
				if (ExtraBytes != 33)
					ii=1;
				else
					db_dopoly=TRUE;
	        	 ii = 1;

	        }*/
			NumPOC = 0;
	        if (ExtraBytes>1)
	        {
				int	reclocSave = recloc;

				NumPOC = *(LPLONG)&pRec[recloc];
				recloc += 4;
				if (NumPOC > 0 && NumPOC < 0)
				for (i=0;i<NumPOC;i++)
				{
					POCPos[i] = *(LPLONG)&pRec[recloc];
					recloc += 4;
					POCFlag[i] = *(LPLONG)&pRec[recloc];
					recloc += 4;
					POCCoord[i].x = *(LPDOUBLE)&pRec[recloc];
					recloc += 8;
					POCCoord[i].y = *(LPDOUBLE)&pRec[recloc];
					recloc += 8;
					recloc += 4;
					ConvertCoord(&POCCoord[i],0,1);
				}
				recloc = reclocSave;
			}
	        GlobalUnlock (hPoints); 
	        GlobalUnlock (hPolyPartLen);
	        GlobalUnlock (hPolyPartLenNew);
	        GSSiGlobUlFree (&hPartIndex);  
			if (nPoly > 1)
				NumPOC = 0;
			pPoints = (HPDPOINT)GlobalLock (hPoints);
			for (i=0;i<NumPoints;i++)
				ConvertCoord(&pPoints[i],0,1);  
			if (SHPPolyHeader.Type == SHPT_POLYLINE_WITHCURVES || SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES)
			{   
				DPOINT	RP, BP, EP, POC;  //pPoints[3]
				//LPDPOINT RP = (LPDPOINT)(&pRec[recloc]+12);		//(LPDPOINT)(&pRec[recloc]+28)
				double	Radius, AZ, BackAZ;
				LPDPOINT	pPoints2;
				HANDLE		hPoints2;
				long		NumPoints2, NumPointsOrig, NumPointsAdded, nAddedPoints=0;
				UINT		ipoc;
				LPINT		pNumPointsChk = (LPINT)GlobalLock (hPolyPartLen);
				int			totpChk = 0;

				db_dopoly=TRUE;
//DisplayMarkers = TRUE;  
//SHPPolyHeader.Type == SHPT_POLYLINE_WITHCURVES;
	
				hPoints2 = GSSiGlobAlloc (0,GMEM_MOVEABLE,(1024*NumPOC+NumPoints*2)*sizeof(DPOINT));
				pPoints2 = (HPDPOINT)GlobalLock (hPoints2); 
				NumPoints2 = 0;
				for (i=0;i<NumPoints;i++)
				{
//char	str[32];

//itoa (i,str,10);
//DisplayMarker (pPoints[i],2,str,0.16,0,0,FALSE,FALSE,NULL,NULL,0);
					/*if (i >= totpChk + *pNumPointsChk)
					{
						totpChk += *pNumPointsChk;
						pNumPointsChk++;
						if (nAddedPoints)
							totpChk++;
						nAddedPoints++;
					}*/
					for (ipoc = 0;ipoc < NumPOC;ipoc++)
					{
						if (POCPos[ipoc] == i-nAddedPoints)
						{
							BP = pPoints[i];
							EP = pPoints[i+1];
							switch (POCFlag[ipoc])
							{
							case 1:
								POC = POCCoord[ipoc];
								break;
							case 5:
								{
									DPOINT	MidPoint = MidPointD (BP,EP);

									RP = POCCoord[ipoc];
									Radius = ldistpp (&RP,&BP); 
									AZ = getazd (&RP,&MidPoint);
									POC = dnewpt (RP,AZ, Radius);  
								}
								break;
							default:
								ii=1;
							}
							NumPointsOrig = NumPoints2;
							if (!Display || FastMapCopy)   
								CurvePointsD(&BP,&POC,&EP, &NumPoints2, &pPoints2,&BackAZ,1020,CurveChordDist,1);
							else
                				CurvePointsD(&BP,&POC,&EP, &NumPoints2, &pPoints2,&BackAZ,1020,DisplayCurveFactor,1); 
							NumPointsAdded = NumPoints2 - NumPointsOrig - 1;
							if (NumPointsAdded > 0)
							{
								UINT	j;
								int		totp=0;

								pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
								pNumPointsNew = (LPINT)GlobalLock (hPolyPartLenNew);
								for (j=0;j<nPoly;j++,pNumPoints++,pNumPointsNew++)
								{
									if (i >= totp && i<totp + *pNumPoints)
									{
										*pNumPointsNew  += NumPointsAdded;
										break;
									}
									totp += *pNumPoints;
								}
								GlobalUnlock (hPolyPartLen);
								GlobalUnlock (hPolyPartLenNew);
							}
							goto NextPt;
 						}
					}
					*pPoints2++ = pPoints[i];
					NumPoints2++;
NextPt:;
				}
			//	Radius = ldistpp (RP,&BP); 
			//	AZ = getazd (RP,&MidPoint);
			//	POC = dnewpt (*RP,AZ, Radius);  
			//	POC = *RP;
				GSSiGlobUlFree (&hPoints);
                GlobalUnlock (hPoints2);
				GlobalUnlock (hPolyPartLen);
				hPoints = hPoints2;
				NumPoints = NumPoints2;
				pPoints = (HPDPOINT)GlobalLock (hPoints);
//				if (Pick)
//					PickCurve (lpDCurPoints,nPnts,&BP,&POC,&EP);
//				if (nPoly != 1)
//					nPoly = 1;
				if (SHPPolyHeader.Type == SHPT_POLYLINE_WITHCURVES)
					SHPPolyHeader.Type = SHPT_ARC;
				else if (SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES)
					SHPPolyHeader.Type = SHPT_POLYGON;	

			}
			else if (SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES)
			{   
				LPLONG	pNumRP = (LPLONG)(&pRec[recloc]);
				short	irp;   
				typedef struct	{long	InsertAfter,RP2;
								 DPOINT	RP;
								 long	RP3;} RPBLOCK;
				typedef	RPBLOCK	FAR	*LPRPBLOCK;
				
				LPRPBLOCK	pRPBlock;
				
				recloc += 4;   
				pRPBlock = (LPRPBLOCK)(&pRec[recloc]);
				for (irp = 0;irp < *pNumRP; irp++)
				{
					pRPBlock++;
				}
			/*	DPOINT	MidPoint = MidPointD (BP,EP);
				double	Radius, AZ, BackAZ;
				
				ConvertCoord(RP,0,1);
				Radius = ldistpp (RP,&BP); 
				AZ = getazd (RP,&MidPoint);
				POC = dnewpt (*RP,AZ, Radius);  
				GSSiGlobUlFree (&hPoints);
				hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,4090*sizeof(DPOINT));
				pPoints = (HPDPOINT)GlobalLock (hPoints); 
				NumPoints = 0;
				if (!Display)   
                    CurvePointsD(&BP,&POC,&EP, &NumPoints, &pPoints,&BackAZ,4090,CurveChordDist,1);
                else
                	CurvePointsD(&BP,&POC,&EP, &NumPoints, &pPoints,&BackAZ,4090,DisplayCurveFactor,1); 
                GlobalUnlock (hPoints);
				pPoints = (HPDPOINT)GlobalLock (hPoints);
				pNumPoints = (LPWORD)GlobalLock (hPolyPartLen);
				*pNumPoints = NumPoints;
		        GlobalUnlock (hPolyPartLen);
				nPoly = 1;*/
				SHPPolyHeader.Type = SHPT_POLYGON;	
			}
			else if (SHPPolyHeader.Type == -1610612685)
			{
				double	Radius = ldistpp (&pPoints[0],&pPoints[1]);
				LPDPOINT	pUSPoints;
		        
				pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
				if (Radius)
				{
					HANDLE hCoords = CreateCirclePoly (pPoints[0],Radius,pNumPoints,0);
					GSSiGlobUlFree (&hPoints);
					pPoints = (HPDPOINT)GlobalLock (hCoords);
					hPoints = hCoords;
				}
				GlobalUnlock (hPolyPartLen);
				SHPPolyHeader.Type = SHPT_POLYGONM;	
			}

			if (SHPType == SHPT_TEXT)
			{
				GRTEXTHEADER	GRTextHeader;  
	            DPOINT	MidPoint;// = MinMaxMidPointD ((LPMNMXCORD)&SHPPolyHeader.Xmin);  
	            double	MidPointAZ; 
	            char	txt[512]="[pgdb.TEXTSTRING]";
	            char	word[256];
	            short	nchar; 
	            double	THeight, AZ;
				int		MDPointID;
	            char	THeightC[32]="[pgdb.FONTSIZE]";   
	            char	AZC[32]="[pgdb.ANGLE]";
				int		type;
       			int		nWords, nWords2, code, lProjection, Curloc;
				DPOINT	Points2[4];
				BOOL	CurveText = FALSE;
				DPOINT	PC,POC,PT;
				short	vjus=1, hjus=0;
	            
				sprintf (txt,"[%s.TEXTSTRING]",FileID[IsFGDB]);
				sprintf (AZC,"[%s.ANGLE]",FileID[IsFGDB]);
				sprintf (THeightC,"[%s.FONTSIZE]",FileID[IsFGDB]);
		    	ExpandText (THeightC);
		    	THeight = atof (THeightC)*NonPltFileDistToBaseDist * 1.55165; 
				pNumPoints = (LPINT)GlobalLock (hPolyPartLen);    
				lpDCurPoints = pPoints;
	    		ExpandText (txt);
	    		ExpandText (AZC);
				if (hElement)
				{
					type = *(LPSHORT)(pElement + 4);
					if (type && type != 2)
						ii=1;
					code = *(LPSHORT)(pElement + 24);
					if (BinSizeE > 26 && code == 124)
						CurveText = TRUE;
				}
				else
					type = 1;
				if (CurveText || type)
					nWords = 1;
				else
					nWords = *(LPLONG)(pElement + 24);
				if (nPoly > 1 && type)
					debugvalue++;
				else if (type == 2 && *pNumPoints != 5)
					ii = 1;
        		for (i=0;i<nPoly;i++)
        		{   
        			int	np=*pNumPoints;
					GWPolylineD (hDC,lpDCurPoints,np,0); 
					lpDCurPoints+=*pNumPoints++;
				}
				GlobalUnlock (hPolyPartLen);
 				pNumPoints = (LPINT)GlobalLock (hPolyPartLen);    
				lpDCurPoints = pPoints;
	       		for (i=0;i<nWords;i++)
        		{   
        			int	np=*pNumPoints;
        			int	nchr = (np-1)/4,j;
        			
        			TLSet = FALSE;
        			if (type)
					{
       					_fstrcpy (word,txt);
					}
        			else
        			{
						vjus = 0;
						lpDCurPoints = Points2;
	        			if (!i)
						{
							if (CurveText)
							{
								char	NewName[64];
								LPDPOINT	pDPoint, pDPointBeg;
								HANDLE	hCurve;
								double	BackAZ;
								DPOINT	P[2], MP[2];
								double	d, az;

								hjus = vjus = 1;
								strcpy (word,txt);
							/*	nCurveText++;
								sprintf (NewName,"c:\\curvtext%i.bin",nCurveText);
								GSSiRename ("c:\\temp.bin",NewName);*/
								Curloc = 32;
								P[0].x = *(LPDOUBLE)(pElement + Curloc) * PGDBCnvFac;
								P[0].y = *(LPDOUBLE)(pElement + Curloc + 8) * PGDBCnvFac;
								P[1].x = *(LPDOUBLE)(pElement + Curloc + 16) * PGDBCnvFac;
								P[1].y = *(LPDOUBLE)(pElement + Curloc + 24) * PGDBCnvFac;
					//GWPolylineD (hDC,P,2,0); 
								Curloc = 76;
								PC.x = *(LPDOUBLE)(pElement + Curloc) * PGDBCnvFac;
								PC.y = *(LPDOUBLE)(pElement + Curloc + 8) * PGDBCnvFac;
								PT.x = *(LPDOUBLE)(pElement + Curloc + 16) * PGDBCnvFac;
								PT.y = *(LPDOUBLE)(pElement + Curloc + 24) * PGDBCnvFac;
								P[0] = PC;
								P[1] = PT;
								MP[0] = MidPointD (P[0],P[1]);
					//GWPolylineD (hDC,P,2,0); 
								Curloc = 120;
								P[0].x = *(LPDOUBLE)(pElement + Curloc) * PGDBCnvFac;
								P[0].y = *(LPDOUBLE)(pElement + Curloc + 8) * PGDBCnvFac;
								P[1].x = *(LPDOUBLE)(pElement + Curloc + 16) * PGDBCnvFac;
								P[1].y = *(LPDOUBLE)(pElement + Curloc + 24) * PGDBCnvFac;
								MP[1] = MidPointD (P[0],P[1]);
					//GWPolylineD (hDC,P,2,0); 
								d = ldistp (MP[0],MP[1]);
								az = getazd (&MP[1],&MP[0]);
								POC = dnewpt (MP[1],az,d*0.27);
								hCurve = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(DPOINT)*4096);
								pDPoint = pDPointBeg = GlobalLock (hCurve);
								np = 0;
								CurvePointsD(&PC,&POC,&PT, &np, &pDPointBeg,&BackAZ,4090,DisplayCurveFactor,1);
					//GWPolylineD (hDC,pDPoint,np,0); 
								GSSiGlobUlFree (&hCurve);
								CurrentType = GF_CURVE;
	    						PTRot = getazd (&PC,&PT); 
								BP = PC;
								EP = PT;
								CurPOCW = POC;
								goto ExitText;
							}
							else
							{
			   					nWords2 = GetWords (txt);
								code = *(LPSHORT)(pElement + 44);
								if (!CheckCode (RecordNumber,pElement,BinSizeE,code,3)) goto badcode;
								code = *(LPSHORT)(pElement + 92);
								if (!CheckCode (RecordNumber,pElement,BinSizeE,code,8)) goto badcode;
								lProjection = *(LPLONG)(pElement + 100);
								if (!CheckCode (RecordNumber,pElement,BinSizeE,lProjection,544)) goto badcode;
								Curloc = 104;
								Curloc += lProjection;
								Curloc += 42;
								code = *(LPLONG)(pElement + Curloc);Curloc += 4;
								if (!CheckCode (RecordNumber,pElement,BinSizeE,code,64)) goto badcode;
								Curloc += 12;
								code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						// can be 63 or 65		if (!CheckCode (RecordNumber,pElement,BinSizeE,code,65)) goto badcode;
								Curloc += 53;
							}
						}
						strcpy (word,Words[i]);
			   		/*	for (j=0;j<nWords;j++)
	        			{ 
	        				if (WordLen[j] == nchr)
	        					goto HaveWord;	
	        			}
	        			j = 0;
	       	HaveWord:   _fstrcpy (word,Words[j]);
	   					nWords--;
	       				if (j < nWords)
	       				{
	       					_fmemmove (&WordLen[j],&WordLen[j+1],(nWords-j)*2);
	       					_fmemmove (&Words[j],&Words[j+1],(nWords-j)*128);
	       				}*/
						code = *(LPSHORT)(pElement + Curloc);Curloc += 2;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,2)) goto badcode;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,80)) goto badcode;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,3)) goto badcode;
						lpDCurPoints[0].x = *(LPDOUBLE)(pElement + Curloc) * PGDBCnvFac;
						lpDCurPoints[0].y = *(LPDOUBLE)(pElement + Curloc + 8) * PGDBCnvFac;
						lpDCurPoints[1].x = *(LPDOUBLE)(pElement + Curloc + 16) * PGDBCnvFac;
						lpDCurPoints[1].y = *(LPDOUBLE)(pElement + Curloc + 24) * PGDBCnvFac;
					//GWPolylineD (hDC,lpDCurPoints,2,0); 
						Curloc += 32;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,1)) goto badcode;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,2)) goto badcode;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,0)) goto badcode;
						lpDCurPoints[0].x = *(LPDOUBLE)(pElement + Curloc) * PGDBCnvFac;
						lpDCurPoints[0].y = *(LPDOUBLE)(pElement + Curloc + 8) * PGDBCnvFac;
						lpDCurPoints[1].x = *(LPDOUBLE)(pElement + Curloc + 16) * PGDBCnvFac;
						lpDCurPoints[1].y = *(LPDOUBLE)(pElement + Curloc + 24) * PGDBCnvFac;
					//GWPolylineD (hDC,lpDCurPoints,2,0); 
						Curloc += 32;
						Curloc += 18;
						lProjection = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,lProjection,544)) goto badcode;
						Curloc += lProjection;
						Curloc += 42;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
						if (!CheckCode (RecordNumber,pElement,BinSizeE,code,64)) goto badcode;
						Curloc += 12;
						code = *(LPLONG)(pElement + Curloc);Curloc += 4;
					// can be 63 or 65	if (!CheckCode (RecordNumber,pElement,BinSizeE,code,65)) goto badcode;
						Curloc += 53;
	       			}	
        			MidPoint =  lpDCurPoints[0];//MidPointD (lpDCurPoints[0],lpDCurPoints[nchr*2]);   
				//	RemoveDupPolyPoints (&np,lpDCurPoints,THeight/10);
					if (!type)
					{
						AZ = getazd (&lpDCurPoints[0],&lpDCurPoints[1]);
					}
					else
					{
	        			// removed for MSP data THeight = ldistpp (&lpDCurPoints[0],&lpDCurPoints[1]); 
						AZ = getazd (&lpDCurPoints[1],&lpDCurPoints[nchr*2]);
					}
					//DisplayPoint (hDC,BasePtToWinPt (&MidPoint));
					//if (ConvertCoord(&MidPoint,0,1)) 
					//	goto RtnFalse;
					CurPointLocD.x = MidPoint.x; 
					CurPointLocD.y = MidPoint.y;
					TXLoc = CurPointLocD; 
		    		LastElementBeginPoint = CurPointLocD;     
		    		//MidPointAZ = RADDEG * atof (AZC);
		    		MidPointAZ = AZ; 
		    		PTRot = MidPointAZ; 

ExitText:
					HaveTXLoc = TRUE;
		    		nchar = _fstrlen (word);
		    		_fmemset (&GRTextHeader,0,sizeof(GRTextHeader));
		    		GRTextHeader.lText = nchar;
					GRTextHeader.FontNum = 8;  
				 
					GRTextHeader.vJust=vjus;//0=above,1=baseline,2=center,3=below
					GRTextHeader.hJust=hjus;
					SetTextHeadSize (&GRTextHeader,THeight);
				    SetTextLocVars (&TLSet); 
				    if (CopyRec)
				    {   
				    	HANDLE hGRText=0;
				                
						if (UpdateItem == 20)
		        			CurPointLocD = NewPointD;
						if (UpdateItem == 201)
		        			AdjustPointRotation (&PTRot);
						if (UpdateItem == 19)
						{   
							LPGRTEXTHEADER pGRTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);	 
							LPSTR	pText = (LPSTR) (pGRTextHeader+1);
									
				    		nchar = pGRTextHeader->lText; 
							pGRTextHeader->UltiMapStyle = FALSE;
							pGRTextHeader->hJust += 2;
		            		hGRText = GRTextFromTextHeader (pGRTextHeader,pText);
							GlobalUnlock (hPickedTextHeader);	
		            	}
		            	else
						{
							GRTextHeader.UltiMapStyle = FALSE;
							GRTextHeader.hJust += 2;
		            		hGRText = GRTextFromTextHeader (&GRTextHeader,word);
						}
						if (CurveText)
						{
							int	npnts = 3;
							HANDLE	hPoints=GSSiGlobAlloc (0,GMEM_MOVEABLE,3*sizeof(DPOINT));
							LPDPOINT Points = GlobalLock (hPoints);

							Points[0] = BP;
							Points[1] =	CurPOCW;
							Points[2] = EP;

							AddPolyToBuffer (1,&npnts,&hPoints,3,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
                                     		 -1,-1,0,hGRText,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
							GSSiGlobUlFree (&hPoints);
						}
						else
						{
								if (nWords == 1)
									opt = 0;
								else if (!i)
									opt = 1;
								else if (i+1 == nWords)
									opt = 3;
								else
									opt = 2;
				    			AddPointToBuffer (CurPointLocD,CurrentRefno,0,CurrentDesc,10, PTRot,0,hGRText,0,
												  CurrentPrefix,CurrentUDI,-1,-1,-1,TRUE,&hUpdateBuf,&lUpdateBuf,opt);
						}
						GSSiGlobFree (&hGRText);
					}
		        	else if (CurVis->WantType[2] && HaveTXLoc && (CurView->PassID || Pick)) 
		        	{   
		        		COLORREF	OldColor = -1;
				        		
		        		if (hPickedTextHeader)
					    {
					    	LPGRTEXTHEADER pPickedTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);
					    	_fmemmove (pPickedTextHeader,&GRTextHeader,sizeof(GRTEXTHEADER)); 
					    	pPickedTextHeader++;
					    	_fstrcpy ((LPSTR)pPickedTextHeader,txt);
					    	GlobalUnlock (hPickedTextHeader);
					    }
				
						ProcessTextObject (hDC,&GRTextHeader,word,nchar,0,0,0,0,0);
						if (OldColor >= 0)
							SetTextColor (hDC,OldColor); 
						if (CurVis->WantType[8])
							DisplayPointItem (hDC,BasePtToWinPt(&CurPointLocD),10,PTRot,InvisiblePointSymbol,0); 
	        		}
					lpDCurPoints+=*pNumPoints++;
					/*					if (i)
											lpDCurPoints++;*/

				}
badcode:
       			GlobalUnlock (hPolyPartLen); 
			} 
			else if (SHPPolyHeader.Type == SHPT_ARC || SHPPolyHeader.Type == SHPT_ARCZ) 
			{
				if (Pick)
				{   
					PickPolylineD (pPoints,NumPoints,0,2,0,0,0);
				}
				else if (CopyRec)  
				{
					pNumPoints = (LPINT)GlobalLock (hPolyPartLenNew);//pNumPoints[1]
					if (nPoly > 1)
					{
						HANDLE	hhPoly = GSSiGlobAlloc ( 418,GMEM_MOVEABLE,sizeof(HANDLE)*nPoly); 
						LPHANDLE phPoly = (LPHANDLE)GlobalLock (hhPoly);
                        HPDPOINT	pPoints1=(HPDPOINT)GlobalLock (hPoints), pPoints2;
                        
	                    for (i=0;i<nPoly;i++)
	                    {   
	                   		phPoly[i] = GSSiGlobAlloc ( 420,GMEM_MOVEABLE,sizeof(DPOINT)*(long)pNumPoints[i]);
		                    pPoints2= (HPDPOINT)GlobalLock (phPoly[i]);  
		                    hmemmove ((HPSTR)pPoints2,(HPSTR)pPoints1,sizeof(DPOINT)*(long)pNumPoints[i]);
	                        GlobalUnlock (phPoly[i]); 
	                        pPoints1 += pNumPoints[i];
	                        if (i)
	                        	pPoints1++;
	                    } 
	                    GlobalUnlock (hPoints);
						AddPolyToBuffer (nPoly,pNumPoints,phPoly,TYPE_POLYLINE,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
	                                     -1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
		                for (i=0;i<nPoly;i++)
		                	GSSiGlobFree (&phPoly[i]);
	                    GSSiGlobUlFree (&hhPoly);
					}
					else
						AddPolyToBuffer (nPoly,pNumPoints,&hPoints,TYPE_POLYLINE,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
                                     	-1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
                    GlobalUnlock (hPolyPartLenNew);
				}
				else
				{    
					lpDCurPoints = pPoints;
					nPnts = nCurPoints = NumPoints;

					if (PolyInMaskAreaFileCoord (GF_LINE,&NumPoints,0,0,&pPoints,TRUE))
					{   
						HPEN hNewPen=0;
						
						hOldPen  = SelectObject(hDC,GetStockObject(BLACK_PEN));
						TempLineColor = SHPColor; 
						TempLineWidth = SHPWidth; 
						if (TempLineColor >= 0 || TempLineWidth != 0)
						{
							hNewPen = CreatePen (PS_SOLID,(short)IDNINT(TempLineWidth*DeviceToScreenFactor()),TempLineColor);
							SelectObject(hDC,hNewPen);
						}
//						if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
//						{   
							pNumPoints = (LPINT)GlobalLock (hPolyPartLenNew);
			        		for (i=0;i<nPoly;i++)
			        		{   
//								GWPolylineD (hDC,lpDCurPoints,*pNumPoints,CurrentDesc); 
//								lpDCurPoints+=*pNumPoints++;
			        			nPnts = *pNumPoints;  
			        			InDynamicSegmentation = FALSE;  
			        			DoDynamicFixedSegmentation (-1,0,0);
								while (nPnts > 1)
								{ 
									if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0)
									{   
										if (db_dopoly)
											GWPolylineD (hDC,lpDCurPoints,NumDynSegPoints,CurrentDesc); 
									}
									DynSegEndPoint = lpDCurPoints[NumDynSegPoints-1];  
									lpDCurPoints += (NumDynSegPoints - 2);
									nPnts = NumDynSegPointsRemaining;
									if (nPnts > 1)
									{   
										lpDCurPoints[0] = DynSegEndPoint;
										lpDCurPoints[1] = DynSegSavePoint;  
										InDynamicSegmentation = TRUE;
									}	
								}
			        			InDynamicSegmentation = FALSE;
								lpDCurPoints+=*pNumPoints++; 
								if (i)
									lpDCurPoints++;
			        		}   
			        		GlobalUnlock (hPolyPartLenNew); 
//						} 
						if (hNewPen)
						{
							SelectObject(hDC,GetStockObject(BLACK_PEN)); 
							GSSiDeleteObject (&hNewPen);
						}
					}
				}
			}
			else
			{   
				BOOL	ShowBorder = GetBit (5,(LPSTR)&CurVis->WantType[7]); 
				HPEN	hBorderPen=0;
				
				if (Pick)
				{   
					PickPolygonD (pPoints,NumPoints,nPoly,hPolyPartLenNew,9999999,0);
				}
				else if (CopyRec)  
				{
					pNumPoints = (LPINT)GlobalLock (hPolyPartLenNew);//pNumPoints[1]
					if (nPoly > 1)
					{
						HANDLE	hhPoly = GSSiGlobAlloc ( 418,GMEM_MOVEABLE,sizeof(HANDLE)*nPoly); 
						LPHANDLE phPoly = (LPHANDLE)GlobalLock (hhPoly);
                        HPDPOINT	pPoints1=(HPDPOINT)GlobalLock (hPoints), pPoints2;
                        
	                    for (i=0;i<nPoly;i++)
	                    {   
	                   		phPoly[i] = GSSiGlobAlloc ( 420,GMEM_MOVEABLE,sizeof(DPOINT)*(long)pNumPoints[i]);
		                    pPoints2= (HPDPOINT)GlobalLock (phPoly[i]);  
		                    hmemmove ((HPSTR)pPoints2,(HPSTR)pPoints1,sizeof(DPOINT)*(long)pNumPoints[i]);
	                        GlobalUnlock (phPoly[i]); 
	                        pPoints1 += pNumPoints[i];
	                        if (i)
	                        	pPoints1++;
	                    } 
	                    GlobalUnlock (hPoints);
						AddPolyToBuffer (nPoly,pNumPoints,phPoly,TYPE_AREA,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
	                                     -1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
		                for (i=0;i<nPoly;i++)
		                	GSSiGlobFree (&phPoly[i]);
	                    GSSiGlobUlFree (&hhPoly);
					}
					else
						AddPolyToBuffer (nPoly,pNumPoints,&hPoints,TYPE_AREA,CurrentRefno,0,-1,CurrentDesc,0,CurrentPrefix,CurrentUDI,
                                     	-1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
                    GlobalUnlock (hPolyPartLenNew);
                }
				else
				{    
					if (Display)
					{
						hOldPen  = SelectObject(hDC,h0Pen);
						if (ShowBorder)
		    				hBorderPen = hAreaBorderPen[TRUE];
			        	if (GetInVisibility(CurrentDesc))
			        	{
			        		if (CurView->HaveLayerColor[FileNum])
			        		{
			        			hTempBrush = CreateSolidBrush(CurView->LayerColor[FileNum]);
								hOldBrush = SelectObject (hDC,hTempBrush);
								hTempPen = CreatePen (PS_SOLID,0,CurView->LayerColor[FileNum]);
                                SelectObject (hDC,hTempPen);
                            }
			        		else
								hOldBrush = SelectRandomBrush (hDC,CurrentRefno,&hBorderPen,CurrentDesc); 
						}
					//	if (hBorderPen)
					//		SelectObject(hDC,hBorderPen); 
					}
					/*lpDCurPoints = pPoints;
					nPnts = nCurPoints = NumPoints;

					if (PolyInMaskAreaFileCoord (GF_AREA,&NumPoints,0,0,&pPoints,TRUE))
					{
						if (SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0)
						{   
							BOOL DoBorder = FALSE;
							
							if (Display)
							{
								if (nPoly > 1 && !ShowLinkLines)
									SelectObject(hDC,h0Pen); 
								if (FillAreas && (GetBit (7,(LPSTR)&CurVis->WantType[7]) ||
												  GetBit (6,(LPSTR)&CurVis->WantType[7])))
									hDeletePen = GWPolygonD (hDC,pPoints, NumPoints, nPoly, hPolyPartLenNew,CurrentDesc,ShowBorder,TRUE,0);
								else
									DoBorder = TRUE; 
								if ((DoBorder || nPoly > 1) && (hBorderPen || hDeletePen))
								{   
									if (hDeletePen)
										SelectObject(hDC,hDeletePen); 
									else
										SelectObject(hDC,hBorderPen); 
									pNumPoints = (LPINT)GlobalLock (hPolyPartLenNew);
					        		for (i=0;i<nPoly;i++)
					        		{   
										GWPolylineD (hDC,lpDCurPoints,*pNumPoints,0); 
										lpDCurPoints+=*pNumPoints++;
										if (i)
											lpDCurPoints++;
					        		}   
					        		GlobalUnlock (hPolyPartLenNew); 
					        	}
					        }
						}
					}*/
					lpDCurPoints = pPoints;
					nPnts = nCurPoints = NumPoints;
					if (PolyInMaskAreaFileCoord (GF_AREA,&NumPoints,0,0,&pPoints,TRUE))
					{
						int	SDCrtn = SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI);
							
						if (SDCrtn > 0)
							ProcessPolygon (hDC,ShowBorder,PltType,0,hTempPen,0);
						else if (SDCrtn < 0)
							HaveTXLoc = TRUE;
					}

				}
			}
			GSSiGlobFree (&hPolyPartLen);
			GSSiGlobFree (&hPolyPartLenNew);
			GSSiGlobUlFree (&hPoints);
		break;
		
		case SHPT_MULTIPOINT:
		break;
	}
	if (hOldBrush)
		SelectObject (hDC,hOldBrush);
	if (hOldPen)
		SelectObject (hDC,hOldPen); 
   	if (hDeletePen != h0Pen)
   		GSSiDeleteObject (&hDeletePen);
   	GSSiDeleteObject (&hTempBrush);
   	GSSiDeleteObject (&hTempPen);
	ShowValue (hDC,FALSE);
	InGraphicsProcessor = FALSE; 
	if (hElement)
		GlobalUnlock (hElement); 
	GlobalUnlock (hRec);
{
#if ENABLETRACE
GSSiExitProg (1375);
#endif
	return TRUE;
}
RtnFalse:
	if (hElement)
		GlobalUnlock (hElement); 
	GlobalUnlock (hRec);
	InGraphicsProcessor = FALSE;
{
#if ENABLETRACE
GSSiExitProg (1375);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}



