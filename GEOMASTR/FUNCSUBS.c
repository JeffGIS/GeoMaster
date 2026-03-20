#include "graphint.h"   
#include "extrndb.h"   
#include "dibapi.h"
#include <wininet.h>

#define MAXCORNERS	4096 
#define MAXCORNERPOINTS	72
#define	MAXOPENDIB	8
#define MAXPOINTSPERCORNER 6
#define BGUPDATEWINDOWCLASS	"BGUpdateWindowClass"
#define IMAGEZOOMWINDOWCLASS	"ImageZoomWindowClass"

static int imageZoomFrom;

typedef struct	{ 
					DPOINT	AvePoint,
							Points[MAXPOINTSPERCORNER],  
							BMPPoints[MAXPOINTSPERCORNER];
					short	nPoints, 
					 		PointIDs[MAXPOINTSPERCORNER], 
							FileID[MAXPOINTSPERCORNER];
					char	filler[256-234];
				}CORNERINFO;
typedef	CORNERINFO	HUGE	*LPCORNERINFO;  
typedef struct	{ 
					MNMXCORD	Bounds;
					short	nBoundPoints; 
					DPOINT	BoundPoints[MAXCORNERPOINTS];
					short	nTranPoints; 
					double	WorldX[MAXCORNERPOINTS],WorldY[MAXCORNERPOINTS],BMPX[MAXCORNERPOINTS],BMPY[MAXCORNERPOINTS];
					HDIB	hDIB; 
					HANDLE	hTran;
					char	Name[64];
					long	LastUsed;
					char	filler[4096-3564];
				}PRIMFILEINFO;
typedef	PRIMFILEINFO	HUGE	*LPPRIMFILEINFO;  

#include "gmextern.h"


static	LPSTR	pDEBUG;
static	LPCORNERINFO	pCorners;
static	LPPRIMFILEINFO	pPRIMFiles;
static	short	nCorners=0, nPRIMFiles, LastiFile, nOpenDIB, PRIMTranType=1;
static	HANDLE	hCorners=0, hPRIMFiles; 
static	long	NextUse; 
static	double	PRIMTol=500; 
static	HWND	MrSidWnd;
static	char	UserTimerMacro[10][1024];
static	int		UserTimerInterval[10];
static	int		UserTimerID[10];
static	time_t	LastTime[10]={0};
static	DWORD	CacheRenameListNum;
static	BOOL	BackgroundCacheStarted=FALSE;
static	char	NetTransferDir[MAX_PATH];
static	char	BUMessage[2][128]={0};
static	HWND	hWndImageZoom=0;
static	int		ImageZoomVP;
static	HDC		hDCImageZoom=0;
static	HBITMAP	hBMImageZoom=0,hOldBMImageZoom;
static	int		ImageZoomSize=300,ImageZoomSizeSmall=150,ImageZoomSizeMedium=300,ImageZoomSizeLarge=450;
static	int		ImageZoomHeight=300,ImageZoomWidth=300;
static	HDIB32	hDibImageZoom=0;
static	int		ImageZoomShape=1;
static	int		ImageZoomBorder=1;
static	int		ImageZoomOffset=1;
static	int		ImageZoomXoff=0,ImageZoomYoff=0;
static	HBITMAP	ImageZoomSavedScreen=0;

static	double gTileWidth[MAXGZOOMS+1];
static	double gTileScale[MAXGZOOMS + 1];
static	UINT   gTileNum[MAXGZOOMS + 1];

#define MAXRAWLINES	1024

typedef struct {DPOINT pt; 
				int nLines;
				int streetRef[2];
				float streetOffset[2];
				int numStreets;
}FIXNODE;
typedef struct {
				int refno;
				int bpNode, epNode;
				double trueDist, curDist, diffDist;
}FIXLINE;
static BOOL useRStreet, useUStreet, useBStreet;


//sample code to create full screen window
/*hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
	CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
	if (dwStyle & WS_OVERLAPPEDWINDOW) {
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(hwnd, &g_wpPrev) &&
			GetMonitorInfo(MonitorFromWindow(hwnd,
			MONITOR_DEFAULTTOPRIMARY), &mi)) {
			SetWindowLong(hwnd, GWL_STYLE,
				dwStyle & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(hwnd, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else {
		SetWindowLong(hwnd, GWL_STYLE,
			dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(hwnd, &g_wpPrev);
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}
*/
HWND GetTopParent (HWND	hWnd)
{
	HWND hWndPar;
	hWndPar = GetParent(hWnd);
	while (hWndPar)
	{
		hWnd = hWndPar;
		hWndPar = GetParent(hWnd);
	}
	return hWnd;
}

void AddStreetRefToNode(int inode, int streetRef, float streetOffset, FIXNODE *nodes)
{
	if (streetRef && nodes[inode].numStreets < 2)
	{
		if (nodes[inode].numStreets)
		{
			if (streetRef != nodes[inode].streetRef[0])
			{
				nodes[inode].numStreets = 2;
				nodes[inode].streetOffset[1] = streetOffset;
				nodes[inode].streetRef[1] = streetRef;
			}
		}
		else
		{
			nodes[inode].numStreets = 1;
			nodes[inode].streetOffset[0] = streetOffset;
			nodes[inode].streetRef[0] = streetRef;
		}
	}
	return;
}

static BOOL GetStreetPoly(int streetRef, int *pnStreetPoints, LPDPOINT *pStreetPoints)
{
	static DPOINT street1[2] = { 160725.78, 49553.61, 160724.87, 49355.31 };
	static DPOINT street2[2] = { 160585.26, 49355.73, 160724.87, 49355.31 };
	static DPOINT street3[2] = { 160647.79, 49368.10, 160648.66, 49452.66 };
	static DPOINT street4[2] = { 160725.78, 49553.61, 160600.48, 49553.97 };

	if (streetRef == 10024 && useRStreet)
	{
		*pnStreetPoints = 2;
		*pStreetPoints = street1;
		return TRUE;
	}
	if (streetRef == 7592 && useBStreet)
	{
		*pnStreetPoints = 2;
		*pStreetPoints = street2;
		return TRUE;
	}
	if (streetRef == 11)
	{
		*pnStreetPoints = 2;
		*pStreetPoints = street3;
		return TRUE;
	}
	if (streetRef == 5067 && useUStreet)
	{
		*pnStreetPoints = 2;
		*pStreetPoints = street4;
		return TRUE;
	}
	return FALSE;
}

void SetLineToTrueDist(int lineno, FIXLINE *lines, FIXNODE *nodes)
{
	double az = getazd(&nodes[lines[lineno].bpNode].pt, &nodes[lines[lineno].epNode].pt);
	DPOINT midPt = MidPointD(nodes[lines[lineno].bpNode].pt,nodes[lines[lineno].epNode].pt);
	double d = FTM * lines[lineno].trueDist / 2;
	int nStreetPoints;
	HPDPOINT pStreetPoints;
	DPOINT IntPoint;
	double OffDist, PolyDist;
	int i;
	
	nodes[lines[lineno].bpNode].pt = dnewpt(midPt, az, -d);
	nodes[lines[lineno].epNode].pt = dnewpt(midPt, az, d);
	for (i = 0; i < 2; i++)
	{
		if (GetStreetPoly(nodes[lines[lineno].bpNode].streetRef[i], &nStreetPoints, &pStreetPoints))
		{
			if (GetPerpendicularOffsetToPoly(&nodes[lines[lineno].bpNode].pt, nStreetPoints, pStreetPoints, &IntPoint, &OffDist, &PolyDist, 0))
			{
				az = getazd(&IntPoint, &nodes[lines[lineno].bpNode].pt);
				nodes[lines[lineno].bpNode].pt = dnewpt(IntPoint, az, nodes[lines[lineno].bpNode].streetOffset[i] * FTM);
			}
		}
		if (GetStreetPoly(nodes[lines[lineno].epNode].streetRef[i], &nStreetPoints, &pStreetPoints))
		{
			if (GetPerpendicularOffsetToPoly(&nodes[lines[lineno].epNode].pt, nStreetPoints, pStreetPoints, &IntPoint, &OffDist, &PolyDist, 0))
			{
				az = getazd(&IntPoint, &nodes[lines[lineno].epNode].pt);
				nodes[lines[lineno].epNode].pt = dnewpt(IntPoint, az, nodes[lines[lineno].epNode].streetOffset[i] * FTM);
			}
		}
	}
	return;
}

int ComputeDistDiff(int numLines, FIXLINE *lines, FIXNODE *nodes, double *totDiff)
{
	int i, iMax = 0;
	double diff, maxDiff = -1, tDiff = 0;

	for (i = 0; i < numLines; i++)
	{
		double d = ldistpp(&nodes[lines[i].bpNode].pt, &nodes[lines[i].epNode].pt) * MFT;
		diff = fabs (d - lines[i].trueDist);
		tDiff += diff;
		if (diff > maxDiff)
		{
			maxDiff = diff;
			iMax = i;
		}
	}
	*totDiff = tDiff;
	return iMax;
}

int addFixNode(double x, double y, int *numNodes, FIXNODE *nodes)
{
	int i, iNode = *numNodes;
	DPOINT pt;

	pt.x = x;
	pt.y = y;
	if (!*numNodes)
	{
		nodes[0].nLines = 1;
		nodes[0].pt = pt;
		(*numNodes) = 1;
		return 0;
	}
	for (i = 0; i < *numNodes; i++)
	{
		if (ldistpp(&nodes[i].pt, &pt) < 0.0001)
		{
			nodes[i].nLines++;
			return i;
		}
	}
	nodes[iNode].nLines = 1;
	nodes[iNode].pt = pt;
	(*numNodes)++;
	return iNode;
}

BOOL FixAreaToOutfile(LPSTR OutFile, LPMNMXCORD pBounds,int numLines, FIXLINE *lines, FIXNODE *nodes)
{
	HIGHLIGHTDATA	HighlightData;
	short			ipos = BT_FIRST;
	long			Refno, nPnts;
	HANDLE			hPoints;
	HPDPOINT3D		lpPoints;
	DPOINT3D		Point;
	double			LineLength, DistInc, AtDist;
	HFILE			Fid;
	char			str[256];
	char			cref[32];
	long			nNodePoints, i;
	short	SymNum;
	short	NumSyms = 0;
	HANDLE	hSymDesc = 0;
	int		nPoints = 0;
	int		np;
	BOOL	rtn;

	SymNum = GetOrCreateSym(0, "PEN1", &NumSyms, &hSymDesc, FALSE, 0);
	if (!SymNum)
		return FALSE;
	_fstrcpy(PltName, OutFile);
	PltType = 2;
	EditBounds = *pBounds;
	if (!OpenMap(CurView->hWnd, 0))
		return FALSE;
	AddToSymList(SymNum, &NumSyms, &hSymDesc);
	hPoints = GSSiGlobAlloc(GAIDNO 1913, GMEM_MOVEABLE, 2 * sizeof(DPOINT));
	nPoints = 2;
	for (i = 0; i < numLines; i++)
	{
		LPDPOINT points = GlobalLock(hPoints);

		points[0] = nodes[lines[i].bpNode].pt;
		points[1] = nodes[lines[i].epNode].pt;
		GlobalUnlock(hPoints);
		_fstrcpy(PltName, OutFile);
		Refno = lines[i].refno;
		itoa(Refno, cref, 10);
		rtn = AddPolyToMap(1, &nPoints, &hPoints, 1, Refno, 0, -1, SymNum, 0, "REFNO", cref, -1, -1, -1, 0, 0, 0, 0, TRUE, 0);
	}
	GSSiGlobFree(&hPoints);
	AddSymToMap(NumSyms, hSymDesc, 0, 0);
	CloseMap(TRUE);
	DestroySymList(&NumSyms, &hSymDesc);
	return rtn;
}

double fixMapFit(double fixTo, int maxLoops, int numLines, FIXLINE *lines, FIXNODE *nodes)
{
	double totDiff=99999,lastTotDiff;
	int maxDiffLine=-1;
	int nloops = 0;
	do
	{
		nloops++;
		lastTotDiff = totDiff;
		if (maxDiffLine >= 0)
		{
			SetLineToTrueDist(maxDiffLine, lines, nodes);
		}
		maxDiffLine = ComputeDistDiff(numLines, lines, nodes, &totDiff);
	} while (totDiff > fixTo && nloops < maxLoops);// && lastTotDiff > totDiff);
	return totDiff;
}

int FixMapCmd(LPSTR inGMDFile, LPSTR outPltFile,double fixTo,int marker)
{
	BOOL rtn = 2;
	int numLines = 0, maxDiffLine=-1, Tot;
	typedef struct {
		int ref;
		double trueLength, currentLength, fromX, fromY, toX, toY;
		int fromStreetRef, toStreetRef;
		float fromStreetOffset, toStreetOffset;
		int marker;
	}LINEREC;
	LINEREC *pLineRec;
	FIXNODE *nodes, *savenodes;
	FIXLINE *lines;
	int numNodes = 0;
	int maxLoops = 10000;
	double fit;
	HANDLE	hDB;
	LPGWDHEADER lpGWDHead;
	int pos = BT_FIRST, Offset, iNode, nloops=0;
	MNMXCORD bounds;

	hDB = OpenGWDatabase(inGMDFile, BT_READ);
	if (!hDB)
		return 0;
	lpGWDHead = (LPGWDHEADER)GlobalLock(hDB);
	pLineRec = (LINEREC *)lpGWDHead->GWDData;
	Tot = BT_NUM_IN_INDEX(lpGWDHead->BTHandle[0]);
	if (Tot > 0)
	{
		nodes = calloc(Tot * 2,sizeof(FIXNODE));
		lines = calloc(Tot,sizeof(FIXLINE));
		while (!BT_FIND(lpGWDHead->BTHandle[0], lpGWDHead->pKeys[0], pos, BT_ANY, (LPSTR)&Offset))
		{
			pos = BT_NEXT;
			FillGWDData(lpGWDHead, Offset);
			if (pLineRec->currentLength > 0 && pLineRec->marker == marker)
			{
				lines[numLines].refno = pLineRec->ref;
				lines[numLines].trueDist = pLineRec->trueLength;
				lines[numLines].bpNode = addFixNode(pLineRec->fromX, pLineRec->fromY, &numNodes, nodes);
				lines[numLines].epNode = addFixNode(pLineRec->toX, pLineRec->toY, &numNodes, nodes);
				AddStreetRefToNode(lines[numLines].bpNode, pLineRec->fromStreetRef, pLineRec->fromStreetOffset, nodes);
				AddStreetRefToNode(lines[numLines].epNode, pLineRec->toStreetRef, pLineRec->toStreetOffset, nodes);
				numLines++;
			}
		}
	}
	savenodes = calloc(numNodes, sizeof(FIXNODE));
	GlobalUnlock(hDB);
	CloseGWDatabase(hDB);
	if (!numLines)
		return 0;
	for (iNode = 0; iNode < numNodes; iNode++)
	{
		if (nodes[iNode].nLines < 2)
		{
			rtn = -(iNode + 1);
			goto Exit;
		}
	}
	useBStreet = useUStreet = FALSE;
	useRStreet = TRUE;
	fit = fixMapFit(0.1, 10000, numLines, lines, nodes);
	useBStreet = FALSE;
	fit = fixMapFit(0.1, 10000, numLines, lines, nodes);
	bounds = CurView->WBounds;
	//if (totDiff <= fixTo)
	{
		rtn = 0;
		if (CreateNewMap(outPltFile, &bounds, 0, 0, 0, 0, 0, 0, FALSE))
		{
			FixAreaToOutfile(outPltFile,&bounds, numLines, lines, nodes);
			rtn = 1+fit;
		}
	}

Exit:
	free(nodes);
	free(lines);
	free(savenodes);
	return rtn;
}

BOOL CompressedFileCmd(int nArgs, LPSTR *Arg)
{
	BOOL rtn = FALSE;
	HANDLE FidTF;
	HFILE fidFiles, fidIndex = HFILE_ERROR;
	char filePath[MAX_PATH + 2];
	char indexRec[MAX_PATH + 32];
	BOOL useGZIP = FALSE;

	if (nArgs < 3)
		goto Exit;
	if (!stricmp(Arg[1], "CREATE"))//$COMPRESSEDFILE(CREATE,path,filelistfile,sourcedir,outindexfile,useGZIP)
	{
		long	NextFileLoc = 0, loc = 0, len;
		long	TotLen;
		long	MaxLength = 8L * (long)USHRT_MAX;
		short	Version = 101;

		if (nArgs > 5 && atob(Arg[6]))
			useGZIP = TRUE;
		fidFiles = GSSiOpenFile(Arg[3], 0, OF_READ);
		if (fidFiles == HFILE_ERROR)
			goto Exit;
		if (nArgs > 4 && *Arg[5])
		{
			fidIndex = GSSiOpenFile(Arg[5], 0, OF_CREATE);
			if (fidIndex == HFILE_ERROR)
			{
				GSSiClose2 (&fidFiles);
				goto Exit;
			}
			sprintf(indexRec, "FILE\tFILELOC");
			fputstring(indexRec, fidIndex);
		}
		FidTF = OpenFileGM(Arg[2], 0, OF_CREATE);
		if (FidTF == INVALID_HANDLE_VALUE)
		{
			GSSiClose2 (&fidFiles);
			goto Exit;
		}

		BigWrite64(FidTF, (HPSTR)&Version, 2, -1);
		BigWrite64(FidTF, (HPSTR)&MaxLength, 4, -1);
		while (fgetstring(filePath, MAX_PATH, fidFiles))
		{
			len = _fstrlen(filePath) + 1;
			if (fidIndex != HFILE_ERROR)
			{
				LONGLONG loc = GSSillseek64(FidTF, 0, 1);
				sprintf(indexRec, "%s\t%lli", filePath, loc);
				fputstring(indexRec, fidIndex);
			}
			BigWrite64(FidTF, (HPSTR)&len, 4, -1);
			BigWrite64(FidTF, (HPSTR)filePath, len, -1);
			AddFileToTransferFile(0, FidTF, filePath, MaxLength, Arg[4],useGZIP);
			loc = -1;
			BigWrite64(FidTF, (HPSTR)&loc, 4, -1);
		}
		loc = -1;
		BigWrite64(FidTF, (HPSTR)&loc, 4, -1);
		loc = 32349;
		if (useGZIP)
			loc = 32449;
		BigWrite64(FidTF, (HPSTR)&loc, 4, -1);
		GSSiClose2 (&fidFiles);
		GSSiClose64 (&FidTF);
		GSSiClose2 (&fidIndex);
		rtn = TRUE;
	}
	else if (!stricmp(Arg[1], "EXPAND"))//$COMPRESSEDFILE(EXPAND,path,outputdir,whichfile(ALL or blank for all files))
	{
		rtn = DecompressGMZipFile(Arg[2], Arg[3], Arg[4], atob(Arg[5]));
	}
	Exit:
	return rtn;
}
static int GetFileFromTransferFile_del(HFILE FidTF,LPSTR FileToGet,long MaxLength,_int64 totlen)
{
			int lRec, rtn = FALSE;
			int	CompressedLength, LenRead = 0;
			double curLoc;
			HFILE Fid = GSSiOpenFile(FileToGet, 0, OF_CREATE);

			if (Fid == HFILE_ERROR)
				goto Exit;

			BigRead(FidTF, (HPSTR)&CompressedLength, 4);
			rtn = TRUE;
			while (CompressedLength > 0)
			{
				LPBYTE	pCompressedRec = (LPBYTE)malloc(CompressedLength + 32);
				LPBYTE	pRec = (LPBYTE)malloc(MaxLength * 2);
				LenRead += CompressedLength + 4;
				BigRead(FidTF, pCompressedRec, CompressedLength);
				lRec = DecompressBinaryRecordUnsafe(pRec, pCompressedRec, CompressedLength);
				if (BigWrite(Fid, pRec, lRec, -1) != lRec)
					rtn = FALSE;
				BigRead(FidTF, (HPSTR)&CompressedLength, 4);
				free(pCompressedRec);
				free(pRec);
				curLoc = GSSillseek(FidTF,0,1);
				/*if (_popover)
				{

					dispatch_async(dispatch_get_main_queue(),
						^{
						[_popover updateProgress : curLoc / _totLen];
					});

				}*/
			}

			GSSiClose2 (&Fid);

		Exit:
			return rtn;
		}

BOOL DecompressGMZipFile(LPSTR TransferFileName, LPSTR toDirectory, LPSTR whichFile,BOOL showStatus)
{
#define PATH_MAX MAX_PATH
			long len, MaxLength;
			char File[PATH_MAX], OutFile[PATH_MAX];
			short Version, endMarker;
			HANDLE FidTF;
			BOOL rtn = TRUE;
			BOOL useZIP = FALSE;
			LPSTR pBS;
			FidTF = OpenFileGM(TransferFileName, 0,OF_READ);
			LONGLONG fileLen;
			long nFilesRead = 0;
			LONGLONG totLen = 0;

			if (FidTF == INVALID_HANDLE_VALUE)
				return FALSE;

			fileLen = GSSifilelength64(FidTF);
			if (fileLen < 14)
			{
				rtn = FALSE;
				goto Exit;
			}
			totLen = fileLen;

			GSSillseek64(FidTF, -4, SEEK_END);
			BigRead64(FidTF,&endMarker, sizeof(short));

			if (endMarker == 32449)
			{
				useZIP = TRUE;
			}
			else if (endMarker != 32349)
			{
				rtn = FALSE;
				goto Exit;
			}

			GSSillseek64(FidTF, 0, SEEK_SET);
			BigRead64(FidTF, &Version, sizeof(short));
			BigRead64(FidTF, &MaxLength, sizeof(long));
			BigRead64(FidTF, &len, sizeof(long));

			while (rtn && len > 0)
			{
				BigRead64(FidTF, File, len);

				if (!(pBS = strrchr(File, '\\')))
				{
					pBS = File;
				}
				else
				{
					pBS++;
				}
				sprintf(OutFile, "%s\\%s", toDirectory, pBS);
				rtn = GetFileFromTransferFile(0,FidTF,OutFile,MaxLength,totLen,useZIP);
				nFilesRead++;
				if (!rtn)
					goto Exit;
				BigRead64(FidTF, &len, 4);
			}

		Exit:
			GSSiClose64 (&FidTF);
			return rtn;
}

HANDLE GetDistinctValues (HWND hWnd,LPSTR valueIn,int ln,HANDLE hDB,int nStatus)
{
	HANDLE hBT=0;
	char tempFile[MAX_PATH], value[130]={0},str[256];
	BTVARDESC	BTVar[1]; 
	int			count, numRows = 0, curLoc = 0, rc=1;
	short		pos;
	
	if (hDB && ln > 0 && ln < 128)
	{
		GSSiGetTempFileName (0,"gmd",0,tempFile); 
			
		BTVar[0].BT_VARTYP=BT_CHAR;
		BTVar[0].BT_VARLEN=ln;
		BTVar[0].BT_VAROFF=0;
		BT_CREATE (tempFile, sizeof(int), FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
		hBT = BT_OPEN (tempFile,0, BT_WRITE, 0);
		if (nStatus)
		{
			numRows = NumSQLRows (hDB); 
			if (numRows > nStatus)
			{
				sprintf (str,"Finding distinct values of %s",valueIn);
				CreateStatusWind (hWnd,1,str);
			}
		}
		ResetFileChangeTime (hDB);
		WaitCursor (1);
		while (rc && FetchDBRec (hDB))
		{   
			strcpy (str,valueIn);
    		ExpandText (str);
			strncpy0 (value,str,ln);
			if (!BT_FIND (hBT,value,BT_FIRST,BT_EQ,(LPSTR)&count))
				count++;
			else
				count = 1;
			BT_PUT (hBT,value,(LPSTR)&count);
			curLoc++;
			if (nStatus && numRows > nStatus && !(curLoc % nStatus))
			{
				int nfound = BT_NUM_IN_INDEX (hBT);
				sprintf (strchr (str,0),"\r%i found",nfound);
				rc = StatusWindowUpdate (0,str,numRows,curLoc);
			}
		}
		WaitCursor (-1);
		if (nStatus && numRows > nStatus)
			DestroyStatusWindow(0);  
	}
	return hBT;
}

BOOL RunForAll(int nArgs, LPSTR *Arg, LPSTR OutLoc, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen)
{
//$FORALL(RECORDS,file,sql,initalize,return,executable statements)
//$FORALL(TABLES,file,wildcard,initalize,return,executable statements)
//$FORALL(FIELDS,file,type(def all),initalize,return,executable statements)
//$FORALL(DISTINCT,file,sql,initalize,return,value,executable statements,varprefix(opt))
  	LPSTR	pEnd, pStatusText, pMaxRecs, pLoopText=0, pLineNo, pFile, pFileName, pVarName=0;
	long	ProcessLine = -1, AtLine=0, CurLoc, nlong;
	static	int istatus=0;
	BOOL	rtn=FALSE;
	HANDLE	hDB=0;
	short	itype;
	long	maxRecords = INT_MAX;
	long	nRecords = 0;

	
	ExpandText (Arg[1]);
	ExpandText (Arg[2]);
	pFile = Arg[2];
	if ((pMaxRecs = strrchr(pFile, '#')))
	{
		*pMaxRecs++ = 0;
		maxRecords = atol(pMaxRecs);
	}
	if ((pLoopText = strrchr(pFile, '!')))
		*pLoopText++ = 0;
	if ((pStatusText = strrchr (pFile,'!')))
		*pStatusText++ = 0;   
	else
	{
		pStatusText = pLoopText;
		pLoopText = 0;
	}
	ExpandText (pFile);
	ExpandText (Arg[3]);
	itype = OpenDataFile (pFile,Arg[3],OF_READ,&hDB);
	if (!hDB)
   		goto Exit;
	if (pStatusText)
	{   
		nlong = NumSQLRows (hDB); 
		nlong = min(nlong, maxRecords);
		if (istatus < 2)
			CreateStatusWind (hWndMain,1,pStatusText);
		istatus++;
	} 
	if (!stricmp (Arg[1],"RECORDS"))
	{
		IgnoreSelectVP = TRUE;    
		CurLoc = 0;
		ProcessTextDB(Arg[4], pBrkPt, bpOffset, bpLen);
		while (ContinueProcessing  && FetchDBRec (hDB) && nRecords++ < maxRecords)
		{   
			if (pStatusText)
			{
				int	rc=1;

				switch (istatus)
				{
				case 2:
				case 1:
					rc = StatusWindowUpdate (0,pLoopText,nlong,CurLoc);
					break;
				default:
					break;
				}
				if (!rc)
					break;
			}
			ProcessTextDB(Arg[6], pBrkPt, bpOffset, bpLen);
			if (ProcessLine >= 0)
				break; 
			CurLoc++;  
		}
		ProcessTextDB(Arg[5], pBrkPt, bpOffset, bpLen);
	}
	else if (!stricmp(Arg[1], "TABLES"))//???
	{
		long	n = 0;
		LPSTR	lpSTRING;
		short	NS;
		HANDLE	DBHandle;
		char	TableName[128];
		char	TablePartialName[128] = { 0 };

		strupr(TablePartialName);
		DBHandle = GetDBHandleFromSQL(hDB);
		lpSTRING = GetTableName(DBHandle, TRUE,itype);
		while (lpSTRING && *lpSTRING)
		{
			strupr(lpSTRING);
			if (strstr(lpSTRING, TablePartialName))
			{
				n++;
			}
			lpSTRING = GetTableName(DBHandle, FALSE,itype);
		}
	}
	else if (!stricmp (Arg[1],"DISTINCT"))
	{
		char tempFile[MAX_PATH], value[130]={0};
		BTVARDESC	BTVar[1]; 
		HANDLE		hBT; 
		HANDLE		hSTR = GSSiGlobAlloc(GAIDNO 1914,GMEM_MOVEABLE,4096);
		LPSTR		str = GlobalLock (hSTR);
		BOOL		rtn=FALSE;   
		int			count, lenValue=128;
		short		pos;
	
		GSSiGetTempFileName (0,"gmd",0,tempFile); 
			
		BTVar[0].BT_VARTYP=BT_CHAR;
		BTVar[0].BT_VARLEN=128;
		BTVar[0].BT_VAROFF=0;
		BT_CREATE (tempFile, sizeof(int), FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
		hBT = BT_OPEN (tempFile,0, BT_WRITE, 0);
		CurLoc = 0;
		while (ContinueProcessing  && FetchDBRec (hDB))
		{   
			if (pStatusText)
			{
				int	rc=1;

				switch (istatus)
				{
				case 2:
				case 1:
					rc = StatusWindowUpdate (0,pLoopText,nlong,CurLoc);
					break;
				default:
					break;
				}
				if (!rc)
					break;
			}
			strcpy (str,Arg[6]);
    		ExpandText (str);
			strncpy0 (value,str,lenValue);
			if (!BT_FIND (hBT,value,BT_FIRST,BT_EQ,(LPSTR)&count))
				count++;
			else
				count = 1;
			BT_PUT (hBT,value,(LPSTR)&count);
			CurLoc++;  
		}
		pos = BT_FIRST;
		while (ContinueProcessing  && !BT_FIND (hBT,value,pos,BT_ANY,(LPSTR)&count))
		{
			pos = BT_NEXT;
			sprintf (str,"%%%s_VALUE",Arg[8]);
			SetGlobalValue (str,value);
			sprintf (str,"%%%s_COUNT",Arg[8]);
			SetGlobalValueLong (str,count);
			ProcessText (Arg[7]);
		}
		GSSiGlobUlFree (&hSTR);
		BT_CLOSEANDDELETE(&hBT);
	}
	CloseDataFile (TRUE, &hDB);   
	IgnoreSelectVP = FALSE;
	rtn = ContinueProcessing;
	SetContinueProcessing ( TRUE);
	if (pStatusText)
		switch (istatus)
		{
			case 0:
				break;
			case 1:
			case 2:
				DestroyStatusWindow(0);  
			default:
				istatus--;
				break;
		}
Exit:		
	return rtn;
}

void GMDFieldTypeToSQL (LPSTR gmd,LPSTR sql)
{
	int nc;

	*sql = 0;
	switch (*gmd)
	{
	case 'B':
		strcpy (sql,"INTEGER");
		break;
	case 'C':
		nc = atoi (gmd+1);
		sprintf (sql,"CHAR(%i)",nc);
		break;
	case 'R':
		strcpy (sql,"NUMBER");
		break;
	}
	return;
}

void SQLFieldTypeToGMD (LPSTR sql,LPSTR gmd)
{
	return;
}

BOOL LinkLinesFunction (LPSTR Arg1,LPSTR Arg2,LPSTR OutLoc)
{
	static	HANDLE	hRawLines=0, hConnectedLines=0;
	static	int		nRawLines=0, nConnectedLines=0, totPoints=0;
	LPDPOINT	pPoints, pRawPoints, pConnectedPoints;
	int		i;
	typedef struct {int nPoints;
					HANDLE hPoints;
					}LINKEDLINES;
	typedef LINKEDLINES	*LPLINKEDLINES;

	LPLINKEDLINES	pRawLines, pConnectedLines;

	strcpy (OutLoc,"0");
	if (!stricmp (Arg1,"CLEAR"))
	{
		if (nConnectedLines)
		{
			pConnectedLines = GlobalLock (hConnectedLines);
			while (nConnectedLines--)
			{
				GSSiGlobFree (&pConnectedLines->hPoints);
				pConnectedLines++;
			}
			GlobalUnlock (hConnectedLines);
		}
		GSSiGlobFree (&hRawLines);
		GSSiGlobFree (&hConnectedLines);
		nRawLines = 0;
		nConnectedLines = 0;
		totPoints = 0;
		strcpy (OutLoc,"1");
	}
	else if (!stricmp (Arg1,"Add"))
	{
		if (!hRawLines)
		{
			nRawLines = 0;
			hRawLines = GSSiGlobAlloc(GAIDNO 1760,GMEM_MOVEABLE,MAXRAWLINES*sizeof(LINKEDLINES));
		}
		pRawLines = (LPLINKEDLINES)GlobalLock (hRawLines);
		pRawLines += nRawLines++;
		pRawLines->nPoints = GetPointsFromList (Arg2,&pRawLines->hPoints);
		totPoints += pRawLines->nPoints;
		GlobalUnlock (hRawLines);
		strcpy (OutLoc,"1");
	}
	else if (!stricmp (Arg1,"COMPUTE"))
	{
		double	tol = atof (Arg2);
		double	wantaz, az, daz;
		int		havei;
		BOOL	reverse=FALSE;
		
		pRawLines = (LPLINKEDLINES)GlobalLock (hRawLines);
		nConnectedLines = 0;
		while (nRawLines)
		{
			double	minx = DBL_MAX;
			double	mindaz = DBL_MAX;
			int		mini=0;

			//find leftmost endpoint
			for (i=0;i<nRawLines;i++)
			{
				pPoints = GlobalLock (pRawLines[i].hPoints);
				if (pPoints[0].x < minx)
				{
					minx = pPoints[0].x;
					mini = i;
					reverse = FALSE;
				}
				if (pPoints[pRawLines[i].nPoints-1].x < minx)
				{
					minx = pPoints[0].x;
					mini = i;
					reverse = TRUE;
				}
				GlobalUnlock (pRawLines[i].hPoints);
			}
			if (!nConnectedLines)
				hConnectedLines = GSSiGlobAlloc(GAIDNO 1761,GMEM_MOVEABLE,nRawLines*sizeof(LINKEDLINES));
			pConnectedLines = GlobalLock (hConnectedLines);
			if (reverse)
				pRawLines[mini].hPoints = ReversePoints (pRawLines[mini].nPoints,pRawLines[mini].hPoints);
			pConnectedLines[nConnectedLines].nPoints = pRawLines[mini].nPoints;
			pConnectedLines[nConnectedLines].hPoints = GSSiGlobAlloc(GAIDNO 1762,GMEM_MOVEABLE,totPoints*sizeof(DPOINT));
			pConnectedPoints = GlobalLock (pConnectedLines[nConnectedLines].hPoints);
			pRawPoints = GlobalLock (pRawLines[mini].hPoints);
			memmove (pConnectedPoints,pRawPoints,pRawLines[mini].nPoints*sizeof(DPOINT));
			totPoints -= pRawLines[mini].nPoints;
			GSSiGlobUlFree (&pRawLines[mini].hPoints);
			if (mini < nRawLines-1)
				memmove (&pRawLines[mini],&pRawLines[mini+1],(nRawLines - mini -1)*sizeof(LINKEDLINES));
NextLine:
			nRawLines--;
/*			if (nRawLines)
			{
				pRawPoints = GlobalLock (pRawLines[nRawLines-1].hPoints);
				GlobalUnlock (pRawLines[nRawLines-1].hPoints);
			}*/
			havei = -1;
			mindaz = DBL_MAX;
			wantaz = getazd (&pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-2],&pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-1]);
			for (i=0;i<nRawLines;i++)
			{
				pRawPoints = GlobalLock (pRawLines[i].hPoints);
				if (ldistp (pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-1],pRawPoints[0]) <= tol)
				{
					az = getazd (&pRawPoints[0],&pRawPoints[1]);
					daz = fabs (DeltaAZ (wantaz,az));
					if (daz < mindaz)
					{
						mindaz = daz;
						havei = i;
					}
				}
				else if (ldistp (pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-1],pRawPoints[pRawLines[i].nPoints-1]) <= tol)
				{
					ReversePoints2 (pRawLines[i].nPoints,pRawPoints);
					az = getazd (&pRawPoints[0],&pRawPoints[1]);
					daz = fabs (DeltaAZ (wantaz,az));
					if (daz < mindaz)
					{
						mindaz = daz;
						havei = i;
					}
				}
				GlobalUnlock (pRawLines[i].hPoints);
			}
			if (havei >= 0)
			{
				i = havei;
				pRawPoints = GlobalLock (pRawLines[i].hPoints);
				pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-1] = 
						MidPointD (pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-1],pRawPoints[0]);
				memmove (&pConnectedPoints[pConnectedLines[nConnectedLines].nPoints],&pRawPoints[1],(pRawLines[i].nPoints-1)*sizeof(DPOINT));
				pConnectedLines[nConnectedLines].nPoints += pRawLines[i].nPoints-1;
				GSSiGlobUlFree (&pRawLines[i].hPoints);
				if (i < nRawLines-1)
					memmove (&pRawLines[i],&pRawLines[i+1],(nRawLines - i -1)*sizeof(LINKEDLINES));
				goto NextLine;
			}
			GlobalUnlock (pConnectedLines[nConnectedLines].hPoints);
			nConnectedLines++;
			GlobalUnlock (hConnectedLines);
		}
		GSSiGlobUlFree (&hRawLines);
		itoa (nConnectedLines,OutLoc,10);
	}
	else if (!stricmp (Arg1,"GET"))
	{
		int id = atoi (Arg2);

		if (id > nConnectedLines)
			return FALSE;
		pConnectedLines = GlobalLock (hConnectedLines);
		pConnectedLines += id - 1;
		pointstoa (OutLoc,pConnectedLines->nPoints,pConnectedLines->hPoints,0);
		GlobalUnlock (hConnectedLines);
	}
	return  TRUE;
}

int LinkPipesFunction (int iOpt,int id,double tol,LPINT pnPnts,LPDPOINT3D pPoints3D,LPINT pPipeSize,LPSTR pMaterial)
{
	//iOpt 0=Clear,1=Add,2=Compute,3=GetNext
	static	HANDLE	hRawLines=0, hConnectedLines=0;
	static	int		nRawLines=0, nConnectedLines=0, totPoints=0;
	LPDPOINT3D	pPoints, pRawPoints, pConnectedPoints;
	int		i, rtn=0;
	typedef struct {int nPoints;
					HANDLE hPoints;
					int	Size;
					char Material[32];
					}LINKEDPIPES;
	typedef LINKEDPIPES	*LPLINKEDPIPES;

	LPLINKEDPIPES	pRawLines, pConnectedLines;

	switch (iOpt)
	{
		case 0: // clear
		{
			if (nConnectedLines)
			{
				pConnectedLines = GlobalLock (hConnectedLines);
				while (nConnectedLines--)
				{
					GSSiGlobFree (&pConnectedLines->hPoints);
					pConnectedLines++;
				}
				GlobalUnlock (hConnectedLines);
			}
			GSSiGlobFree (&hRawLines);
			GSSiGlobFree (&hConnectedLines);
			nRawLines = 0;
			nConnectedLines = 0;
			totPoints = 0;
		}
		break;
		case 1: //add
		{
			HPDPOINT3D pPnts3D;

			if (!hRawLines)
			{
				nRawLines = 0;
				hRawLines = GSSiGlobAlloc(GAIDNO 1760,GMEM_MOVEABLE,MAXRAWLINES*sizeof(LINKEDPIPES));
			}
			pRawLines = (LPLINKEDPIPES)GlobalLock (hRawLines);
			pRawLines += nRawLines++;
			pRawLines->Size = *pPipeSize;
			strncpy0 (pRawLines->Material,pMaterial,sizeof(pRawLines->Material-1));
			pRawLines->nPoints = *pnPnts;
			pRawLines->hPoints = GSSiGlobAlloc(GAIDNO 1764,GMEM_MOVEABLE,pRawLines->nPoints*sizeof(DPOINT3D));
			pPnts3D = GlobalLock (pRawLines->hPoints);
			memmove (pPnts3D,pPoints3D,pRawLines->nPoints*sizeof(DPOINT3D));
			GlobalUnlock (pRawLines->hPoints);
			totPoints += pRawLines->nPoints;
			GlobalUnlock (hRawLines);
		}
		break;
		case 2: //compute
		{
			double	wantaz, az, daz;
			int		havei;
			BOOL	reverse=FALSE;
			
			nConnectedLines = 0;
			if (hRawLines)
			{
				pRawLines = (LPLINKEDPIPES)GlobalLock (hRawLines);
				while (nRawLines)
				{
					double	minx = DBL_MAX;
					double	mindaz = DBL_MAX;
					int		mini;

					//find leftmost endpoint
					for (i=0;i<nRawLines;i++)
					{
						pPoints = GlobalLock (pRawLines[i].hPoints);
						if (pPoints[0].x < minx)
						{
							minx = pPoints[0].x;
							mini = i;
							reverse = FALSE;
						}
						if (pPoints[pRawLines[i].nPoints-1].x < minx)
						{
							minx = pPoints[0].x;
							mini = i;
							reverse = TRUE;
						}
						GlobalUnlock (pRawLines[i].hPoints);
					}
					if (!nConnectedLines)
						hConnectedLines = GSSiGlobAlloc(GAIDNO 1761,GMEM_MOVEABLE,nRawLines*sizeof(LINKEDPIPES));
					pConnectedLines = GlobalLock (hConnectedLines);
					if (reverse)
						pRawLines[mini].hPoints = ReversePoints (pRawLines[mini].nPoints,pRawLines[mini].hPoints);
					pConnectedLines[nConnectedLines].nPoints = pRawLines[mini].nPoints;
					pConnectedLines[nConnectedLines].Size = pRawLines[mini].Size;
					strcpy (pConnectedLines[nConnectedLines].Material,pRawLines[mini].Material);
					pConnectedLines[nConnectedLines].hPoints = GSSiGlobAlloc(GAIDNO 1762,GMEM_MOVEABLE,totPoints*sizeof(DPOINT3D));
					pConnectedPoints = GlobalLock (pConnectedLines[nConnectedLines].hPoints);
					pRawPoints = GlobalLock (pRawLines[mini].hPoints);
					memmove (pConnectedPoints,pRawPoints,pRawLines[mini].nPoints*sizeof(DPOINT3D));
					totPoints -= pRawLines[mini].nPoints;
					GSSiGlobUlFree (&pRawLines[mini].hPoints);
					if (mini < nRawLines-1)
						memmove (&pRawLines[mini],&pRawLines[mini+1],(nRawLines - mini -1)*sizeof(LINKEDPIPES));
		NextLine:
					nRawLines--;
		/*			if (nRawLines)
					{
						pRawPoints = GlobalLock (pRawLines[nRawLines-1].hPoints);
						GlobalUnlock (pRawLines[nRawLines-1].hPoints);
					}*/
					havei = -1;
					mindaz = DBL_MAX;
					wantaz = get2dazfrom3d (&pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-2],&pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-1]);
					for (i=0;i<nRawLines;i++)
					{
						pRawPoints = GlobalLock (pRawLines[i].hPoints);
						if (pConnectedLines[nConnectedLines].Size == pRawLines[i].Size &&
							!stricmp (pConnectedLines[nConnectedLines].Material,pRawLines[i].Material) &&
							l2ddistfrom3d (pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-1],pRawPoints[0]) <= tol)
						{
							az = get2dazfrom3d (&pRawPoints[0],&pRawPoints[1]);
							daz = fabs (DeltaAZ (wantaz,az));
							if (daz < mindaz)
							{
								mindaz = daz;
								havei = i;
							}
						}
						else if (pConnectedLines[nConnectedLines].Size == pRawLines[i].Size &&
								 !stricmp (pConnectedLines[nConnectedLines].Material,pRawLines[i].Material) &&
								 l2ddistfrom3d (pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-1],pRawPoints[pRawLines[i].nPoints-1]) <= tol)
						{
							ReversePoints3D2 (pRawLines[i].nPoints,pRawPoints);
							az = get2dazfrom3d (&pRawPoints[0],&pRawPoints[1]);
							daz = fabs (DeltaAZ (wantaz,az));
							if (daz < mindaz)
							{
								mindaz = daz;
								havei = i;
							}
						}
						GlobalUnlock (pRawLines[i].hPoints);
					}
					if (havei >= 0)
					{
						i = havei;
						pRawPoints = GlobalLock (pRawLines[i].hPoints);
						pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-1] = 
								MidPoint3D (pConnectedPoints[pConnectedLines[nConnectedLines].nPoints-1],pRawPoints[0]);
						memmove (&pConnectedPoints[pConnectedLines[nConnectedLines].nPoints],&pRawPoints[1],(pRawLines[i].nPoints-1)*sizeof(DPOINT3D));
						pConnectedLines[nConnectedLines].nPoints += pRawLines[i].nPoints-1;
						GSSiGlobUlFree (&pRawLines[i].hPoints);
						if (i < nRawLines-1)
							memmove (&pRawLines[i],&pRawLines[i+1],(nRawLines - i -1)*sizeof(LINKEDPIPES));
						goto NextLine;
					}
					GlobalUnlock (pConnectedLines[nConnectedLines].hPoints);
					nConnectedLines++;
					GlobalUnlock (hConnectedLines);
				}
				GSSiGlobUlFree (&hRawLines);
			}
			rtn = nConnectedLines;
		}
		break;
		case 3: //get
		{
			if (id > nConnectedLines)
				return FALSE;
			pConnectedLines = GlobalLock (hConnectedLines);
			pConnectedLines += id;
			pPoints = GlobalLock (pConnectedLines->hPoints);
			memmove (pPoints3D,pPoints,pConnectedLines->nPoints*sizeof(DPOINT3D));
			*pnPnts = pConnectedLines->nPoints;
			*pPipeSize = pConnectedLines->Size;
			strcpy (pMaterial,pConnectedLines->Material);

			//pointstoa (OutLoc,pConnectedLines->nPoints,pConnectedLines->hPoints,0);
			GlobalUnlock (pConnectedLines->hPoints);
			GlobalUnlock (hConnectedLines);
			rtn = 1;
		}
		break;
	}
	return  rtn;
}

void GoogleTilesInit (void)
{
	static	BOOL	first = TRUE;
	int		zoom;

	if (!first)
		return;
	first = FALSE;
	gTileWidth[0] = 20037508.342789244 * 2.0;
	gTileScale[0] = gTileWidth[0] / 256;
	gTileNum[0] = 1;
	for ( zoom=1; zoom<=MAXGZOOMS; zoom++)
	{
		gTileWidth[zoom] = gTileWidth[zoom - 1] / 2.0;
		gTileNum[zoom] = gTileNum[zoom - 1] * 2;
		gTileScale[zoom] = gTileWidth[zoom] / 256;
	}
	return;
}


BOOL GetGoogleTileBounds (int GZoom,int GRow,int GCol,LPMNMXCORD pBounds)
{
	BOOL rtn = TRUE;

	GoogleTilesInit ();
	pBounds->xmn = GCol * gTileWidth[GZoom] - 20037508.342789244; 
	pBounds->ymn = 20037508.342789244 - (GRow+1) * gTileWidth[GZoom]; 
	pBounds->xmx = pBounds->xmn + gTileWidth[GZoom];
	pBounds->ymx = pBounds->ymn + gTileWidth[GZoom];
	if (pBounds->xmn < -20037508.342789244 ||
		pBounds->xmx > 20037508.342789244  ||
		pBounds->ymn < -20037508.342789244 ||
		pBounds->ymx > 20037508.342789244)
		rtn = FALSE;
	return rtn;
}

void AddjustSphericalMercatorBounds (LPMNMXCORD pBounds)
{
	pBounds->xmn = max (pBounds->xmn,-20037508.342789244);
	pBounds->xmx = min (pBounds->xmx, 20037508.342789244);
	pBounds->ymn = max (pBounds->ymn,-20037508.342789244);
	pBounds->ymx = min (pBounds->ymx, 20037508.342789244);
	return;
}

double GetGoogleTileBoundsFromPointAndZoom (int GZoom,DPOINT GPoint,LPMNMXCORD pBounds,LPINT pgRow,LPINT pgCol,LPINT pGoogleRow)
{
	int		gRow, gCol, zoom;

	GoogleTilesInit ();		
	gRow = (20037508.342789244 + GPoint.y)/gTileWidth[GZoom];
	gCol = (GPoint.x + 20037508.342789244) / gTileWidth[GZoom];
	if (gRow == 167890 && gCol == 63121)
		ii = 1;
	pBounds->xmn = gCol * gTileWidth[GZoom] - 20037508.342789244;
	//pBounds->ymn = 20037508.342789244 - (gRow+1) * gTileWidth[GZoom]; 
	pBounds->ymn = gRow * gTileWidth[GZoom] - 20037508.342789244; 
	pBounds->xmx = pBounds->xmn + gTileWidth[GZoom];
	pBounds->ymx = pBounds->ymn + gTileWidth[GZoom];
	*pGoogleRow = gTileNum[GZoom] - gRow - 1;
//	*pGoogleRow = (20037508.342789244 - GPoint.y) / gTileWidth[GZoom];
	*pgRow = gRow;
	*pgCol = gCol;
	return gTileScale[GZoom];
}

double GetGoogleScaleForZoom (int iZoom)
{
	if (iZoom < 0 || iZoom > MAXGZOOMS)
		return -1;
	return gTileScale[iZoom];
}

int GetGoogleZoomForSCale (double scale)
{
	int iZoom;
	int nearZoom;
	double minDiff, diff;
	double nearScale, testScale;
	nearScale = gTileScale[1];
	minDiff = fabs(scale - nearScale);
	for (int izoom = 1; izoom <= MAXGZOOMS; izoom++)
	{
		testScale = gTileScale[izoom];
		diff = fabs(scale - testScale);
		if (diff < minDiff)
		{
			nearScale = testScale;
			minDiff = diff;
			nearZoom = izoom;
		}
	}
	return nearZoom;
}

int GetTranID(LPSTR cid)
{
	int	id;
	int ib = 0;

	if (*cid)
		ib = FIRST_USER_PROJ;
	for (id = ib; id < MAX_PROJ; id++)
		if (!stricmp(projid[id], cid))
			return id;

	return -1;
}
int GetNextUserTranID(void)
{
	int	id;
	int ib = 0;

	ib = FIRST_USER_PROJ;
	for (id = ib; id < MAX_PROJ; id++)
		if (!*projid[id])
			return id;

	return -1;
}

BOOL GetGoogleZoomAndTileFromBounds (LPMNMXCORD pBoundsInBaseProjection,int StartZoom,LPINT pZoom,LPINT pTileX,LPINT pTileY,LPDOUBLE pScale,LPMNMXCORD pTileBounds)
{
	BOOL	rtn = FALSE;
	int	zoom = -1;
	int googleRow;
	MNMXCORD	BoundsInGoogleProjection, gTileBounds;
	double	scale;
	
	*pZoom = 0;
	*pTileX = *pTileY = 0;
	*pScale = 0;
	if (ConvertRectCoord (&BoundsInGoogleProjection,pBoundsInBaseProjection, 1,GOOGLEMAPSPROJECTION))
	{
		DPOINT	Point = MinMaxMidPointD (&BoundsInGoogleProjection);

		for (zoom = MAXGZOOMS;zoom > -1;zoom--)
		{
			scale = GetGoogleTileBoundsFromPointAndZoom (zoom,Point,&gTileBounds,pTileY,pTileX,&googleRow);
			if (BoundsInBounds (&BoundsInGoogleProjection,&gTileBounds,0))
			{
				rtn = TRUE;
				*pZoom = zoom;
				*pScale = scale;
				if (pTileBounds)
					*pTileBounds = gTileBounds;
				break;
			}
		}
	}
	return rtn;
}

void ProjectionFunction (LPSTR Arg1,LPSTR Arg2,LPSTR Arg3,LPSTR Arg4,LPSTR Arg5, LPSTR Arg6,LPSTR OutLoc)
{
#define MAXUSERPROJ	8
	DPOINT	wpoint, newpoint1, newpoint2;
	double	dist, scale, az;
	BOOL	err;
	int		rc, id;
	char	str[512];
	static	projPJ	projdef[MAXUSERPROJ] = { 0 };
	static  double  projFactor[MAXUSERPROJ];

	strcpy (OutLoc,"0");
	if (!stricmp(Arg1, "NVPFROMPRJ"))
	{
		HFILE fid = GSSiOpenFile(Arg2, 0, OF_READ);
		if (fid != HFILE_ERROR)
		{
			int l = GSSifilelength(fid);
			HANDLE hMem = GSSiGlobAlloc(GAIDNO 1915, GMEM_MOVEABLE, l + 4096 + 4);
			LPSTR pMem = GlobalLock(hMem);
			LPSTR pMemOut = &pMem[l + 1];
			BigRead(fid, pMem, l);
			GSSiClose2 (&fid);
			pMem[l] = 0;
			if (!ConvertPRJtoProj4(pMem, pMemOut))
			{
				l = strlen(pMemOut);
				fid = GSSiOpenFile(Arg3, 0, OF_CREATE);
				BigWrite(fid, pMemOut, l, -1);
				GSSiClose2 (&fid);
				strcpy(OutLoc, "1");
			}
		}
	}
	else if (!stricmp(Arg1, "DEFINE"))
	{
		LPSTR def = Arg3;
		BOOL doFree = FALSE;
		double factor=1.0;
		if ((id = GetTranID(Arg2)) < 0)
		{
			if ((id = GetNextUserTranID()) < 0)
				return;
		}
		if (IsProjectionFile(Arg3))
		{
			def = SHPGetNVP(Arg3, &factor);
			doFree = TRUE;
		}

		projdef[id] = pj_init_plus(def);
		projFactor[id] = factor;
		if (doFree) free(def);
		if (!projdef[id])
			return;
		strcpy(projid[id], Arg2);
		strcpy(OutLoc, "1");
	}
	else if (!stricmp(Arg1, "DELETE"))
	{
		if ((id = GetTranID (Arg2)) < 0)
			return;
		if (id < FIRST_USER_PROJ)
			return;
		id -= 8;
		pj_free(projdef[id]);
		projdef[id] = 0;
		*projid[id] = 0;
		strcpy (OutLoc,"1");
	}
	else if (!stricmp (Arg1,"ROTATION"))
	{
		if ((id = GetTranID (Arg2)) < 0)
			return;
		wpoint = atopt (Arg3,&err);
		if (err)
			return;
		dist = atof (Arg4);
		newpoint1 = dnewpt (wpoint,0,-dist/2);
		newpoint2 = dnewpt (wpoint,0,dist/2);
		ConvertCoord (&newpoint1, 1,2);
		ConvertCoord (&newpoint2, 1,2);
		newpoint1.x *= DEG_TO_RAD;
		newpoint1.y *= DEG_TO_RAD;
		newpoint2.x *= DEG_TO_RAD;
		newpoint2.y *= DEG_TO_RAD;
		rc = pj_transform(projdef[0],projdef[id], 1, 1, &newpoint1.x, &newpoint1.y, NULL, &projFactor[id] );
		rc = pj_transform(projdef[0], projdef[id], 1, 1, &newpoint2.x, &newpoint2.y, NULL, &projFactor[id]);
		az = getazd (&newpoint1,&newpoint2);
		ftoa (OutLoc,az*RAD_TO_DEG);
	}
	else if (!stricmp (Arg1,"SCALE"))
	{
		if ((id = GetTranID (Arg2)) < 0)
			return;
		wpoint = atopt (Arg3,&err);
		if (err)
			return;
		dist = atof (Arg4);
		newpoint1 = dnewpt (wpoint,0,-dist/2);
		newpoint2 = dnewpt (wpoint,0,dist/2);
		ConvertCoord (&newpoint1, 1,2);
		ConvertCoord (&newpoint2, 1,2);
		newpoint1.x *= DEG_TO_RAD;
		newpoint1.y *= DEG_TO_RAD;
		newpoint2.x *= DEG_TO_RAD;
		newpoint2.y *= DEG_TO_RAD;
		rc = pj_transform(projdef[0], projdef[id], 1, 1, &newpoint1.x, &newpoint1.y, NULL,&projFactor[id]);
		rc = pj_transform(projdef[0], projdef[id], 1, 1, &newpoint2.x, &newpoint2.y, NULL, &projFactor[id]);
		scale = ldistp (newpoint1,newpoint2)/dist;
		ftoa (OutLoc,scale);
	}
	else if (!stricmp (Arg1,"CONVERT"))
	{
		int	id1, id2;

		wpoint = atopt (Arg2,&err);
		if (err)
			return;
		if ((id1 = GetTranID (Arg3)) < 0)
			return;
		if ((id2 = GetTranID (Arg4)) < 0)
			return;
		rc = pj_transform(projdef[id1], projdef[id2], 1, 1, &wpoint.x, &wpoint.y, NULL, &projFactor[id2]);
		if (!rc)
			dpointtoa (OutLoc,&wpoint);
	}
	else if (!stricmp(Arg1, "GUESS"))
	{
		BOOL err;
		MNMXCORD bounds = atobounds(Arg3,&err);
		char projection[256];
		int num = GuessProjection2(Arg2, &bounds,-1,projection);
		strcpy(OutLoc, projection);
	}
	else if (!stricmp (Arg1,"CONVERSIONGRID"))
	{
		MNMXCORD	bounds;
		double		accuracy;
		HANDLE		hTran;
		BOOL		err;
		double	XFROM[5],YFROM[5],XTO[5],YTO[5];  
		float	RSQMIN;
		int	nrow=1, ncol=1;
		int	iGrid, j, k, nTest=16, irow, icol, nGrid=4096,maxStrLen;
		double	gridWidth, gridHeight, xinc, yinc, diff, maxDiff;
		DPOINT	truePoint, tranPoint;
		HFILE	Fid, FidOut;
		char	tempFile[MAX_PATH];
		int		idFrom=1, idTo=2;

		bounds = atobounds (Arg2,&err);
		if (err)
			return;
		if (*Arg5)
		{
			if ((idFrom = GetTranID (Arg5)) < 0)
				return;
			if ((idTo = GetTranID (Arg6)) < 0)
				return;
		}
		GSSiGetTempFileName (0,"gm",0,(LPSTR)tempFile); 
		accuracy = atof (Arg3);
		for (iGrid=0;iGrid<nGrid;iGrid++)
		{
			if (iGrid == nGrid-1)
				ii=1;
			gridWidth = BoundsWidth (&bounds)/(iGrid+1);
			gridHeight = BoundsHeight (&bounds)/(iGrid+1);
			Fid = GSSiOpenFile (tempFile,0,OF_CREATE);
			maxStrLen = 0;
			for (irow=0;irow<nrow;irow++)
			{
				for (icol=0;icol<ncol;icol++)
				{
					XFROM[0] = bounds.xmn + icol * gridWidth;
					YFROM[0] = bounds.ymn + irow * gridHeight;
					XFROM[1] = bounds.xmn + icol * gridWidth;
					YFROM[1] = bounds.ymn + (irow+1) * gridHeight;
					XFROM[2] = bounds.xmn + (icol+1) * gridWidth;
					YFROM[2] = bounds.ymn + (irow+1) * gridHeight;
					XFROM[3] = bounds.xmn + (icol+1) * gridWidth;
					YFROM[3] = bounds.ymn + irow * gridHeight;
					XFROM[4] = (XFROM[0] + XFROM[1] + XFROM[2] + XFROM[3]) / 4;
					YFROM[4] = (YFROM[0] + YFROM[1] + YFROM[2] + YFROM[3]) / 4;
					for (j=0;j<5;j++)
					{
						DPOINT point;

						point.x = XFROM[j];
						point.y = YFROM[j];
						ConvertCoord (&point,idFrom,idTo);
						XTO[j] = point.x;
						YTO[j] = point.y;
					}
	
					hTran = STRAN2 (1664,XFROM,YFROM,XTO,YTO,5,(LPFLOAT)&RSQMIN,1,NULL);
					xinc = gridWidth / nTest;
					yinc = gridHeight / nTest;
					maxDiff = 0;
					for (j=0;j<nTest;j++)
					{
						for (k=0;k<nTest;k++)
						{
							truePoint.x = XFROM[0] + k * xinc;
							truePoint.y = YFROM[0] + j * yinc;
							tranPoint = TranPoint (&truePoint,hTran);
							ConvertCoord (&tranPoint,idTo,idFrom);
							diff = ldistp (truePoint,tranPoint);
							if (diff > accuracy)
							{
								CloseTRANS2 (&hTran);
								goto NextGrids;
							}
							maxDiff = max (maxDiff,diff);
						}
					}
					if (hTran)
					{
						LPTRANDATA	TranPtr = (LPTRANDATA)GlobalLock(hTran);

						sprintf (str,"B=%.16lg %.16lg %.16lg %.16lg %.16lg %.16lg %.16lg %.16lg %.16lg %.16lg %.16lg %.16lg %.4lg",XFROM[0],YFROM[0],XFROM[2],YFROM[2],
										TranPtr->BASX,TranPtr->BASY,TranPtr->A1,TranPtr->B1,TranPtr->C1,TranPtr->A2,TranPtr->B2,TranPtr->C2,maxDiff);
						fputstring (str,Fid);
						maxStrLen = max (maxStrLen,strlen (str));
						GlobalUnlock (hTran);
					}
					CloseTRANS2 (&hTran);
				}
			}
			itoa (iGrid+1,OutLoc,10);
			GSSillseek (Fid,0,0);
			FidOut = GSSiOpenFile (Arg4,0,OF_CREATE);
			sprintf (str,"%i %i %.16lg %.16lg %.16lg %.16lg",iGrid+1,maxStrLen+2,gridWidth,gridHeight,bounds.xmn,bounds.ymn);
			PadString (str,' ',98);
			fputstring (str,FidOut);
			while (fgetstring (str,510,Fid))
			{
				PadString (str,' ',maxStrLen);
				fputstring (str,FidOut);
			}
			GSSiClose2 (&Fid);
			GSSiRemove (tempFile);
			GSSiClose2 (&FidOut);
			return;
NextGrids:
			nrow++;
			ncol++;
			GSSiClose2 (&Fid);
		}
		ii = 1;
	}
	return;
}

BOOL GetSymAttrFile2 (LPSTR SymName,LPSTR NewDir,BOOL AddRefno,LPSTR RefFile,LPSTR OverrideAppendVal,LPSTR AttrFile)
{
	static	BOOL First=TRUE;
	static	HANDLE	hSymAttrFiles=0;
	static	char	SymAppendVar[70] = { 0 };
	int		len;
	LPSTR	pSymAttrFiles;
	LPSTR	pSC;
	BOOL	rtn=FALSE;
	OFSTRUCTGM OFStruct;

	if (!SymName)
	{
		GSSiGlobFree (&hSymAttrFiles);
		First = TRUE;
		return TRUE;
	}
	*AttrFile = 0;
	if (First)
	{
		HFILE	Fid;
		int		l=0;

		First = FALSE;
		Fid = GSSiOpenFile ("[%DL]symattrfiles.txt",&OFStruct,OF_READ);
		if (Fid != HFILE_ERROR)
		{
			char	str[302];

			len = GSSifilelength (Fid);
			hSymAttrFiles = GSSiGlobAlloc(GAIDNO 1560,GMEM_MOVEABLE,len*2);
			pSymAttrFiles = GlobalLock (hSymAttrFiles);
			fgetstring (SymAppendVar,66,Fid);
			while (fgetstring (str,300,Fid))
			{
				LPSTR	pTab = strchr (str,'\t');
				char	NameAndNum[128];
				int		SymNum;
				char	SymAppend[32];

				if (pTab)
				{
					*pTab++ = 0;
					if ((pSC = strrchr (str,':')))
					{
						*pSC++ = 0;
						strcpy (SymAppend,pSC);
					}
					else
						*SymAppend = 0;
					SymNum = GetDictSymbolNumber (str);
					sprintf (NameAndNum,"%s|%i:%s",str,SymNum,SymAppend);
					strcpy (&pSymAttrFiles[l],NameAndNum);
					l += strlen (NameAndNum) + 1;
					strcpy (&pSymAttrFiles[l],pTab);
					l += strlen (pTab) + 1;
				}
			}
			pSymAttrFiles[l] = 0;
			GSSiClose2 (&Fid);
			GlobalUnlock (hSymAttrFiles);
		}
	}
	if (hSymAttrFiles)
	{
		LPSTR	pEnd, pVB;
		BOOL	WantNum = FALSE;
		char	AppendVal[64];

		if (OverrideAppendVal && *OverrideAppendVal)
			strncpy0 (AppendVal,OverrideAppendVal,sizeof(AppendVal)-1);
		else
			strcpy (AppendVal,SymAppendVar);
		ExpandText (AppendVal);
		if (*SymName == '#')
		{
			WantNum = TRUE;
			SymName++;
		}

		pSymAttrFiles = GlobalLock (hSymAttrFiles);
		while (*pSymAttrFiles)
		{
			pSC = strrchr (pSymAttrFiles,':');
			*pSC++ = 0;
			pVB = strchr (pSymAttrFiles,'|');
			if (WantNum)
				pSymAttrFiles = pVB + 1;
			else
				*pVB = 0;
			if (!stricmp (AppendVal,pSC))
			{
				if (!stricmp (pSymAttrFiles,SymName))
				{
					*pVB = '|';
					*(pSC-1) = ':';
					pEnd = strchr (pSC,0);
					pEnd++;
					strcpy (AttrFile,pEnd);
					rtn = TRUE;
					break;
				}
			}
			*pVB = '|';
			*(pSC-1) = ':';
			pEnd = strchr (pSymAttrFiles,0);
			pEnd++;
			pSymAttrFiles = strchr (pEnd,0) + 1;
		}
		GlobalUnlock (hSymAttrFiles);
		if (*NewDir)
		{
			LPSTR pBS = strrchr (AttrFile,'\\');

			if (pBS)
			{
				char NewName[MAX_PATH];

				*pBS = 0;
				strcpy (NewName,AttrFile);
				sprintf (strchr (NewName,0),"\\%s\\%s",NewDir,pBS+1);
				*pBS = '\\';
				if (!FileType (NewName))
				{
					LPSTR pRefFile=RefFile;

					if (!*pRefFile)
						pRefFile = AttrFile;
					rtn = CreateGWDDatabase (NewName,1,FALSE,0,0,pRefFile);
					if (rtn && AddRefno)
					{
						GWFLDINFO	FieldInfo;
			 
	 					FieldInfo.Type = BT_INTEGER;
						FieldInfo.Len  = 4;
						strcpy (FieldInfo.Name,"INT_REFNO");
						rtn = GWDAddField (NewName,&FieldInfo);
					}
				}
				strcpy (AttrFile,NewName);
			}
		}
	}
	return rtn;
}
BOOL GetSymAttrFile(LPSTR SymName, LPSTR NewDir, BOOL AddRefno, LPSTR RefFile, LPSTR OverrideAppendVal, LPSTR AttrFile)
{
	int SymNum, iParent;
	char	symnumC[8];
	LPSTR pTab;
	BOOL rtn = GetSymAttrFile2(SymName, NewDir, AddRefno, RefFile, OverrideAppendVal, AttrFile);

	if (!rtn)
	{
		if (*SymName == '#')
			SymNum = atoi(&SymName[1]);
		else
			SymNum = GetDictSymbolNumber(SymName);
		iParent = GetDictSymParent(SymNum);
		while (iParent && !rtn)
		{
			sprintf(symnumC, "#%i", iParent);
			rtn = GetSymAttrFile2(symnumC, NewDir, AddRefno, RefFile, OverrideAppendVal, AttrFile);
			if (!rtn)
				iParent = GetDictSymParent(iParent);
		}
	}
	if (rtn && AttrFile)
	{
		pTab = strrchr(AttrFile, '\t');
		if (pTab)
			*pTab = 0;
	}
	return rtn;
}

BOOL GetSymAttrKey(LPSTR SymName, LPSTR NewDir, BOOL AddRefno, LPSTR RefFile, LPSTR OverrideAppendVal, LPSTR Key)
{
	int		SymNum, iParent;
	char	symnumC[8];
	LPSTR	pTab;
	char	AttrFile[MAX_PATH];
	BOOL	rtn = GetSymAttrFile2(SymName, NewDir, AddRefno, RefFile, OverrideAppendVal, AttrFile);
	
	*Key = 0;
	if (!rtn)
	{
		if (*SymName == '#')
			SymNum = atoi(&SymName[1]);
		else
			SymNum = GetDictSymbolNumber(SymName);
		iParent = GetDictSymParent(SymNum);
		while (iParent && !rtn)
		{
			sprintf(symnumC, "#%i", iParent);
			rtn = GetSymAttrFile2(symnumC, NewDir, AddRefno, RefFile, OverrideAppendVal, AttrFile);
			if (!rtn)
				iParent = GetDictSymParent(iParent);
		}
	}
	if (rtn)
	{
		pTab = strrchr(AttrFile, '\t');
		if (pTab)
		{
			pTab++;
			strcpy(Key, pTab);
		}
		else
			rtn = FALSE;
	}
	return rtn;
}

TIMERPROC UserTimerProc(HWND hwnd, UINT uMsg, UINT dEvent, DWORD dwTime)
{
	int id = dEvent - USERTIMER -1;
	time_t	ThisTime = GetTickCount ();

   	if (id >= 0 && id < 10 && (ThisTime - LastTime[id] >= UserTimerInterval[id])) 
	{
 		LastTime[id] = ThisTime;
  		ProcessText (UserTimerMacro[id]);
	}
	return 0;
}

BOOL SetUserTimer (int id,int milliseconds,LPSTR Macro)
{
	int	st;

	if (id < 1 || id > 10)
		return FALSE;
	st = SetTimer (hWndMain,id+USERTIMER,milliseconds,(TIMERPROC)UserTimerProc);
	id--;
	UserTimerID[id] = st;
	LastTime[id] = GetTickCount ();
	UserTimerInterval[id] = milliseconds;
	strcpy (UserTimerMacro[id],Macro);
	return TRUE;
}

BOOL KillUserTimer (int id)
{
	BOOL	st;

	if (id < 1 || id > 10)
		return FALSE;
	st = KillTimer (hWndMain,UserTimerID[id-1]);
	LastTime[id-1] = LONG_MAX;
	return TRUE;
}
BOOL ResetUserTimer (int id, int milliseconds)
{
	int	st;

	if (id < 1 || id > 10)
		return FALSE;
	UserTimerInterval[id-1] = milliseconds;
	st = KillTimer (hWndMain,UserTimerID[id-1]);
	st = SetTimer (hWndMain,id+USERTIMER,milliseconds,(TIMERPROC)UserTimerProc);
	UserTimerID[id-1] = st;
	LastTime[id-1] = GetTickCount ();
	return TRUE;
}

BOOL XMLToGMD1 (LPSTR XMLFile,LPSTR GMDFile)
{
	HFILE	Fid = GSSiOpenFile (XMLFile,0,OF_READ);
	int		l;
	HANDLE	hMem=0;
	LPSTR	pFile,pEnd,pRec,pLoc;
	char	DefStr[]="StationID(B4),Volume(R8),Occupancy(R8),Flow(B4),Speed(B4)";
	short	NumFields=5, NumIndexFields=1;
	BOOL	st,rtn=FALSE; 
	HANDLE	hDB;
	LPGWDHEADER lpGWDHead; 
	int		StationID;

	if (Fid == HFILE_ERROR)
		return FALSE;
	if (!CreateGWDDatabase (GMDFile,1,FALSE,NumFields,NumIndexFields,DefStr))
	{
		GSSiClose2 (&Fid);
		goto Exit;
	}
	l = GSSifilelength (Fid);
	hMem = GSSiGlobAlloc(GAIDNO 1561,GMEM_MOVEABLE,l);
	pFile = GlobalLock (hMem);
	pEnd = pFile + l;
	BigRead (Fid,pFile,l);
	GSSiClose2 (&Fid);
				
    hDB = OpenGWDatabase (GMDFile,BT_WRITE);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
//	CreateStatusWindow (hWndMain,1,"Store Blocked Refs"); 
	while (pFile && pFile < pEnd)
	{ 
		pRec = pFile;
		if (!(pRec = strstr (pRec,"<station id='")))
			break;
		pRec += 13;
		pFile = strstr (pRec,"<station id='");
		StationID = atoi (pRec);
		if (!(pLoc = strstr (pRec,"status='")))
			break;
		pLoc += 8;
		if (!strnicmp (pLoc,"ok",2))
		{
			int Flow=-1, Speed=-1;
			double	Volume=-1, Occupancy=-1;
			
			if ((pLoc = strstr (pRec,"<volume>")))
				Volume = atof (pLoc+8);
			if ((pLoc = strstr (pRec,"<occupancy>")))
				Occupancy = atof (pLoc+11);
			if ((pLoc = strstr (pRec,"<flow>")))
				Flow = atoi (pLoc+6);
			if ((pLoc = strstr (pRec,"<speed>")))
				Speed = atoi (pLoc+7);
			SetFieldValFromCharAndName(lpGWDHead, "StationID", (LPSTR)&StationID, TRUE, TRUE);
			SetFieldValFromCharAndName(lpGWDHead, "Volume", (LPSTR)&Volume, TRUE, TRUE);
			SetFieldValFromCharAndName(lpGWDHead, "Occupancy", (LPSTR)&Occupancy, TRUE, TRUE);
			SetFieldValFromCharAndName(lpGWDHead, "Flow", (LPSTR)&Flow, TRUE, TRUE);
			SetFieldValFromCharAndName(lpGWDHead, "Speed", (LPSTR)&Speed, TRUE, TRUE);
			st = GWDReplaceRecord (lpGWDHead,0,0,-1); 
		}
		//StatusWindowUpdate (0,0, (int)pEnd,(int)pFile);
	}
    GlobalUnlock (hDB); 
    CloseGWDatabase (hDB); 
	rtn = TRUE;
	//DestroyStatusWindow(0);
Exit:
	GSSiGlobUlFree (&hMem);
	return rtn;
}

BOOL XMLToGMD2 (LPSTR XMLFile,LPSTR GMDFile)
{
	HFILE	Fid = GSSiOpenFile (XMLFile,0,OF_READ);
	int		l,tot=0,havecord=0;
	HANDLE	hMem=0;
	LPSTR	pFile,pEnd,pRec,pLoc,pEndRec,pLocEnd,pWeb;
	char	DefStr[]="EventIdentifier(C32),EventMessageTimeStamp(C32),KeyPhrase(C255),EventType(C255),EventDescription(C255),EventDescription2(C255),WEB(C255),X(R8),Y(R8)";
	short	NumFields=05, NumIndexFields=1;
	BOOL	st,rtn=FALSE; 
	HANDLE	hDB;
	LPGWDHEADER lpGWDHead; 
	double	Lat,Lon;
	DPOINT	WPoint;
	char	str[1024],SubKey[256],nulval[]="";

	if (Fid == HFILE_ERROR)
		return FALSE;
	if (!CreateGWDDatabase (GMDFile,1,FALSE,NumFields,NumIndexFields,DefStr))
	{
		GSSiClose2 (&Fid);
		goto Exit;
	}
	l = GSSifilelength (Fid);
	hMem = GSSiGlobAlloc(GAIDNO 1562,GMEM_MOVEABLE,l);
	pFile = GlobalLock (hMem);
	pEnd = pFile + l;
	BigRead (Fid,pFile,l);
	GSSiClose2 (&Fid);
				
    hDB = OpenGWDatabase (GMDFile,BT_WRITE);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
//	CreateStatusWindow (hWndMain,1,"Store Blocked Refs"); 
	while (pFile && pFile < pEnd)
	{ 
		pRec = pFile;
		if (!(pRec = strstr (pRec,"<event-report-message>")))
			break;
		if (!(pEndRec = strstr (pRec,"</event-report-message>")))
			break;
		tot++;
		*pEndRec++ = 0;
		pFile = pEndRec;
		pLoc = strstr (pRec,"<event-location-coordinates-latitude>");
		if (!pLoc)
			continue;
		havecord++;
		pLoc += strlen ("<event-location-coordinates-latitude>");
		Lat = ((double)atoi (pLoc)) / 1000000;
		pLoc = strstr (pRec,"<event-location-coordinates-longitude>");
		if (!pLoc)
			continue;
		pLoc += strlen ("<event-location-coordinates-longitude>");
		Lon = ((double)atoi (pLoc)) / 1000000;
		WPoint.x = Lon;
		WPoint.y = Lat;
		ConvertCoord(&WPoint,2,1);
		SetFieldValFromCharAndName(lpGWDHead, "X", (LPSTR)&WPoint.x, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "Y", (LPSTR)&WPoint.y, TRUE, TRUE);

		pLoc = strstr (pRec,"<event-identifier>");
		pLoc += strlen ("<event-identifier>");
		pLocEnd = strstr (pLoc,"</event-identifier>");
		*pLocEnd = 0;
		SetFieldValFromCharAndName(lpGWDHead, "EventIdentifier", (LPSTR)pLoc, FALSE, TRUE);
		*pLocEnd = '<';

		pLoc = strstr (pRec,"<event-message-time-stamp>");
		pLoc += strlen ("<event-message-time-stamp>");
		pLocEnd = strstr (pLoc,"</event-message-time-stamp>");
		*pLocEnd = 0;
		SetFieldValFromCharAndName(lpGWDHead, "EventMessageTimeStamp", (LPSTR)pLoc, FALSE, TRUE);
		*pLocEnd = '<';

		pLoc = strstr (pRec,"<key-phrase>");
		pLoc += strlen ("<key-phrase>");
		pLocEnd = strstr (pLoc,"</key-phrase>");
		*pLocEnd = 0;
		strcpy (str,pLoc);
		*pLocEnd = '<';
		pLoc = str + 1;
		pLocEnd = strchr (pLoc,'>');
		*pLocEnd = 0;
		sprintf (SubKey,"</%s>",pLoc);
		*pLocEnd = ':';
		pLocEnd = strstr (pLoc,SubKey);
		*pLocEnd = 0;
		SetFieldValFromCharAndName(lpGWDHead, "KeyPhrase", (LPSTR)pLoc, FALSE, TRUE);

		pLoc = strstr (pRec,"<eventType>");
		pLoc += strlen ("<eventType>");
		pLocEnd = strstr (pLoc,"</eventType>");
		*pLocEnd = 0;
		strcpy (str,pLoc);
		*pLocEnd = '<';
		pLoc = str + 1;
		pLocEnd = strchr (pLoc,'>');
		*pLocEnd = 0;
		sprintf (SubKey,"</%s>",pLoc);
		*pLocEnd = ':';
		pLocEnd = strstr (pLoc,SubKey);
		*pLocEnd = 0;
		SetFieldValFromCharAndName(lpGWDHead, "EventType", (LPSTR)pLoc, FALSE, TRUE);

		if ((pLoc = strstr (pRec,"<event-description>")))
		{
			pLoc += strlen ("<event-description>");
			pLocEnd = strstr (pLoc,"</event-description>");
			*pLocEnd = 0;
			strcpy (str,pLoc);
			*pLocEnd = '<';
			pLoc = str;
			pWeb = strstr (pLoc,"&lt;a");
			if ((pLocEnd = strstr (pLoc,"For more inf")))
				*pLocEnd = 0;
			else if ((pLocEnd = strstr (pLoc,"please visit")))
				*pLocEnd = 0;
			else if ((pLocEnd = strstr (pLoc,"Please visit")))
				*pLocEnd = 0;
			else if ((pLocEnd = strstr (pLoc,"Please Visit")))
				*pLocEnd = 0;
			if (pWeb)
			{
				LPSTR	pQuote;

				*pWeb++ = 0;
				if ((pQuote = strchr (pWeb,'"')))
				{
					pWeb = pQuote+1;
					if ((pQuote = strchr (pWeb,'"')))
						*pQuote = 0;
				}
				SetFieldValFromCharAndName(lpGWDHead, "WEB", (LPSTR)pWeb, FALSE, TRUE);
			}
			else
				SetFieldValFromCharAndName(lpGWDHead, "WEB", "", FALSE, TRUE);
		}
		else
			pLoc = nulval;
		if (strlen (pLoc) > 255)
		{
			SetFieldValFromCharAndName(lpGWDHead, "EventDescription2", (LPSTR)&pLoc[255], FALSE, TRUE);
			pLoc[255] = 0;
		}
		else
			SetFieldValFromCharAndName(lpGWDHead, "EventDescription2", "", FALSE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "EventDescription", (LPSTR)pLoc, FALSE, TRUE);

		pLoc = strstr (pRec,"<event-identifier>");
		if (pLoc)
		{
			pLoc += strlen("<event-identifier>");
			pLocEnd = strstr(pLoc, "</event-identifier>");
			*pLocEnd = 0;
			SetFieldValFromCharAndName(lpGWDHead, "EventIdentifier", (LPSTR)pLoc, FALSE, TRUE);
			*pLocEnd = '<';

			st = GWDReplaceRecord(lpGWDHead, 0, 0, -1);
		}
		//StatusWindowUpdate (0,0, (int)pEnd,(int)pFile);
	}
    GlobalUnlock (hDB); 
    CloseGWDatabase (hDB); 
	rtn = TRUE;
	//DestroyStatusWindow(0);
Exit:
	GSSiGlobUlFree (&hMem);
	return rtn;
}

BOOL TxtToHtm (LPSTR TxtFile,LPSTR HTMFile)
{
	HFILE	FidIn = GSSiOpenFile (TxtFile,0,OF_READ);
	HFILE	FidOut;
	HANDLE	hStr;
	LPSTR	str;

	if (FidIn == HFILE_ERROR)
		return FALSE;
	FidOut = GSSiOpenFile (HTMFile,0,OF_CREATE);
	if (FidOut == HFILE_ERROR)
	{
		GSSiClose2 (&FidIn);
		return FALSE;
	}
	hStr = GSSiGlobAlloc(GAIDNO 1563,GMEM_MOVEABLE,4096);
	str = GlobalLock (hStr);
	while (fgetstring (str,1020,FidIn))
	{
		if (*str == '%')
		{
			HANDLE	hCmd=GSSiGlobAlloc(GAIDNO 1564,GMEM_MOVEABLE,USHRT_MAX);
			LPSTR	cmd = GlobalLock (hCmd);

			strcpy (cmd,&str[1]);
			while (fgetstring (str,4090,FidIn))
				if (*str == '%')
				{
					strcat (cmd,&str[1]);
					break;
				}
				else
					strcat (cmd,str);
			FidMacroOutput = FidOut;
			ExpandText (cmd);
			FidMacroOutput = HFILE_ERROR;
			GSSiGlobUlFree (&hCmd);
		}
		else
		{
			ExpandText (str);
			fputstring (str,FidOut);
		}
	}
	GSSiClose2 (&FidIn);
	FlushFile (FidOut);
	GSSiClose2 (&FidOut);
	GSSiGlobUlFree (&hStr);
	return TRUE;
}

BOOL MultiZoomBegin (int UpLevels,int DownLevels,double OverlapFactor) 
{   
	MNMXCORD	Bounds;    
	RECT		Rect; 
	int			i;
	BOOL		SaveMemMap = MemMap;
	HDC			SaveDC[MAX_VIEWPORTS];
	
	for (i=0;i<NumMultiZoomLevels;i++)
		GSSiDeleteObject (&hbmpMultiLevel[i]);    
	NumMultiZoomLevels = MultiZoomLevel = UpLevels + DownLevels;    
	MultiZoomVP = CurView->ID;
	CurView->Scale *= pow(2,UpLevels);
//	Bounds = FactorBounds (&CurView->WBounds,pow(2,UpLevels));  
	//MemMap = TRUE;
	hdcMemMap = CreateCompatibleDC(CurView->hDC);  
	GetClientRect(hWndMain,&Rect);
	MemMapWidth = OverlapFactor * (Rect.right - Rect.left + 1);
	MemMapHeight = OverlapFactor * (Rect.bottom - Rect.top + 1);  
	curProgID = 10003;
	hMemBitmap = CreateCompatibleBitmap (CurView->hDC,(int)MemMapWidth,(int)MemMapHeight);
	hbmpOld = SelectObject(hdcMemMap, hMemBitmap);  
	curProgID = -1;
	MemMap = TRUE;
	for (i=0;i<*pNumViewports;i++) 
	{
		SaveDC[i] = pViewports[i]->hDC;
		pViewports[i]->hDC = hdcMemMap;
	}
	ZoomToPointAndScale (CurView->MidPointW,CurView->Scale,FALSE);
//	ZoomToRect(Bounds,TRUE); 
	MemMap = SaveMemMap;
	GSSiDeleteObject (&hMemBitmap);
	DeleteDC (hdcMemMap);
	hdcMemMap = 0;
	for (i=0;i<*pNumViewports;i++) 
		pViewports[i]->hDC = SaveDC[i];
	return TRUE;
}

BOOL CopyTextFile (LPSTR ToFile,LPSTR FromFile,LPSTR Opt,LPSTR Value)
{
	long	nRows, WantRows, irow=0,StartRow;
	HFILE	Fid = GSSiOpenFile (FromFile,0,OF_READ); 
	HFILE	FidOut;
	HANDLE	hStr=GSSiGlobAlloc(GAIDNO 1565,GMEM_MOVEABLE,4096);
	LPSTR	str;
	
	if (Fid == HFILE_ERROR)
		return FALSE; 
	str = GlobalLock (hStr);
	FidOut = GSSiOpenFile (ToFile,0,OF_CREATE);  
	if (!_fstricmp (Opt,"LAST"))
	{
		nRows = NumRowsInTxtFile (Fid);
		WantRows = atol (Value);
		StartRow = max (0,nRows-WantRows);
	   	while (fgetstring (str,4094,Fid))  
	   	{
	   		if (irow++ >= StartRow)
	   			fputstring (str,FidOut);
	   	} 
	}
	else
	{
		WantRows = atol (Value);
	   	while (irow++ < WantRows && fgetstring (str,4094,Fid))  
	   	{
   			fputstring (str,FidOut);
	   	} 
	}
	GSSiClose2 (&Fid); 
	GSSiClose2 (&FidOut);
	GSSiGlobUlFree (&hStr);
	return TRUE;
}

BOOL FixBlockedRefs (LPSTR FileName)
{
	LPGWDHEADER lpGWDHead; 
    HANDLE		hDB;   
    long		Offset; 
    short		PickFile; 
    typedef struct		{
    				long	Refno;
    				short	BlockedType,BlockedSym,BlockingType,BlockingSym,BlockingRefDeleted;
    				char	BlockedTAG[42], BlockingTAG[42];
    				char	BlockingFile[128];
    			}FBRD;
    typedef	FBRD	FAR	*	LPFBRD;
    LPFBRD	pFBRD;
    short	pos=BT_FIRST;
    long	Tot,Num=0, nFixed=0, nNotFound=0;	
	char	mess[128] = { 0 };
    
    hDB = OpenGWDatabase (FileName,BT_READ);
    if (!hDB)
    	return FALSE;
	CreateStatusWind (hWndMain,1,"Fix Blocked Refs");
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
    pFBRD = (LPFBRD)lpGWDHead->GWDData;  
    Tot = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]); 
   	while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],pos,BT_ANY,(LPSTR)&Offset)) 
   	{   
   		pos = BT_NEXT;
	   	FillGWDData (lpGWDHead,Offset);  
	   	if (pFBRD->BlockingType &&
	   		((pFBRD->BlockedType != pFBRD->BlockingType) || 
	   		 (pFBRD->BlockedSym  != pFBRD->BlockingSym)  ||
	   		 _fstricmp (pFBRD->BlockedTAG,pFBRD->BlockingTAG)))
	   	{
			PickFile = GetPickFile (1);   
			nFixed++;
			if (PickByRefno(pFBRD->Refno,0,0,PickFile))
				ChangePickedItemRefno (0,LONG_MAX);
			else
				nNotFound++;
		} 
		sprintf (mess,"%ld Fix Attempts: %ld not found",nFixed,nNotFound);
		StatusWindowUpdate (0,mess, Tot,Num++);
	}
    GlobalUnlock (hDB); 
    CloseGWDatabase (hDB); 
	DestroyStatusWindow(0);
	MessageBox (0,mess,"",MB_OK);  
	return TRUE;
}

BOOL ProcessBlockedRefsFile (LPSTR OutFileName) 
{
	HFILE	Fid2 = FidBlockedRefs;   
	short	BlockingType, BlockingDesc, BlockingLayer, BlockingRefDeleted; 
	short	RefType, RefDesc;
	MNMXCORD	BlockingRefMNMX;
	char	DefStr[]="BlockedRef(B4),BlockedType(B2),BlockedSymNum(B2),BlockingType(B2),BlockingSymNum(B2),BlockingRefDeleted(B2),BlockedTAG(C42),BlockingTAG(C42),BlockingRefFile(C128)";
	short	NumFields=9, NumIndexFields=1;
	BOOL	st; 
	HANDLE	hDB;
	LPGWDHEADER lpGWDHead; 
	long	Refno; 
	short	SymNum;
	REFINDEXDATA	RefIdxData1, RefIdxData2;
	char	BlockingFile[128], BlockedTAG[50],BlockingTAG[50]; 
	long	Tot;
	
	FidBlockedRefs = HFILE_ERROR;
	if (!CreateGWDDatabase (OutFileName,1,FALSE,NumFields,NumIndexFields,DefStr))
		return FALSE;
				
    hDB = OpenGWDatabase (OutFileName,BT_WRITE);
    if (!hDB)
    	return FALSE;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
	GSSillseek (Fid2,0,0); 
	CreateStatusWind (hWndMain,1,"Store Blocked Refs"); 
	Tot = GSSillseek (Fid2,0,2);
	GSSillseek (Fid2,0,0);
	while (ContinueProcessing && BigRead (Fid2,(HPSTR)&Refno,4) == 4)
	{ 
		BigRead (Fid2,(HPSTR)&SymNum,2);
		BigRead (Fid2,(HPSTR)&PickList[0].FileNum,2); 
		BlockingLayer = PickList[0].FileNum;
		BigRead (Fid2,(HPSTR)&RefIdxData2,sizeof(REFINDEXDATA)); 
		PickList[0].Segment = RefIdxData2.Segment;
		PickList[0].Offset = RefIdxData2.Offset;
		ProcessPickedItem (0,FALSE);  
		_fstrcpy (BlockingFile,PltName);
		BlockingType = CurrentType;
		BlockingDesc = CurrentDesc; 
		if (CurlTAG)
			sprintf (BlockingTAG,"%s:%s",CurrentPrefix,CurrentUDI);
		else
			*BlockingTAG = 0;
		BlockingRefDeleted = ItemIsDeleted + ItemIsRemoved * 4 + ItemIsBlocked * 128;
		BigRead (Fid2,(HPSTR)&PickList[0].FileNum,2);
		BigRead (Fid2,(HPSTR)&RefIdxData1,sizeof(REFINDEXDATA));  
		PickList[0].Segment = RefIdxData1.Segment;
		PickList[0].Offset = RefIdxData1.Offset;
		ProcessPickedItem (0,FALSE);        		
		RefType = CurrentType;
		RefDesc = CurrentDesc; 
		if (CurlTAG)
			sprintf (BlockedTAG,"%s:%s",CurrentPrefix,CurrentUDI);
		else
			*BlockedTAG = 0;
		SetFieldValFromCharAndName(lpGWDHead, "BlockedRef", (LPSTR)&Refno, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "BlockedType", (LPSTR)&RefType, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "BlockedSymNum", (LPSTR)&RefDesc, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "BlockedTAG", (LPSTR)BlockedTAG, FALSE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "BlockingLayer", (LPSTR)&BlockingLayer, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "BlockingType", (LPSTR)&BlockingType, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "BlockingSymNum", (LPSTR)&BlockingDesc, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "BlockingRefDeleted", (LPSTR)&BlockingRefDeleted, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "BlockingFile", (LPSTR)BlockingFile, FALSE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "BlockingTAG", (LPSTR)BlockingTAG, FALSE, TRUE);
		st = GWDReplaceRecord (lpGWDHead,0,0,-1); 
		StatusWindowUpdate (0,0, Tot,GSSillseek (Fid2,0,1));
	}
	GSSiClose2 (&Fid2);
    GlobalUnlock (hDB); 
    CloseGWDatabase (hDB); 
	DestroyStatusWindow(0);  
	return TRUE;
}  

BOOL PointInAreaFunctions (int nArgs,LPSTR *Args,LPSTR OutLoc)
{
	BOOL rtn=FALSE;
	int	nLoops, nPnts;
	HANDLE	hPoly;
	LPLONG	pPolyPartLen;
	BOOL	Err;
	HPDPOINT	pPoints,pPoints2;
	typedef struct	{
					 MNMXCORD Bounds;
					 long	  nPoly;
					 long	  nPoints;
					 double	  Offset;
					 HANDLE	  hPoints;
					 HANDLE	  hPolyPartLen;
					 int	  Type;
					 HANDLE	  hAccel;
					} POINTINAREASTRUCT; 
	typedef	POINTINAREASTRUCT	*LPPOINTINAREASTRUCT; 
		
	*OutLoc = 0;
	if (!stricmp (Args[1],"LOAD")) //$POINTINAREA(LOAD,offset)
	{
		switch (PickList[0].Type)
		{
		default:
			break;
		case 1:
		case 2:
		break;
		case 3:
		
			nLoops = GetPolyPointsWithParts ((LPPICKDATAHEADER)&PickList[0],&nPnts,&hPoly,&hPolyPartLen);
			if (nLoops)
			{   
				LPMNMXCORD	pBounds = (LPMNMXCORD)GlobalLock (hPoly);
				HANDLE		hPIAStruct = GSSiGlobAlloc(GAIDNO 1597,GHND,sizeof(POINTINAREASTRUCT));
				LPPOINTINAREASTRUCT	PIAStruct = (LPPOINTINAREASTRUCT)GlobalLock (hPIAStruct);
				
				pPoints = (HPDPOINT)(pBounds+1);
				PIAStruct->hPolyPartLen = hPolyPartLen;
				PIAStruct->Offset = atobasedist (Args[2],&Err);
				ExpandBounds (pBounds,PIAStruct->Offset);
				PIAStruct->hAccel = PointInAreaAcceleratorSetupMono (nPnts,pPoints,PIAStruct->nPoly,PIAStruct->hPolyPartLen,PIAStruct->Offset,pBounds); 
				PIAStruct->nPoints = nPnts;
				PIAStruct->nPoly = nLoops;
				PIAStruct->Bounds = *pBounds;
				PIAStruct->hPoints = GSSiGlobAlloc(GAIDNO 1598, GMEM_MOVEABLE, sizeof(DPOINT)*nPnts);
				pPoints2 = (HPDPOINT)GlobalLock (PIAStruct->hPoints);
				memmove (pPoints2,pPoints,nPnts*sizeof(DPOINT));
				GSSiGlobUlFree (&hPoly);
				GlobalUnlock (PIAStruct->hPoints);
				GlobalUnlock (hPIAStruct);
				ltoa ((long)hPIAStruct,OutLoc,10);
				rtn = TRUE;
			}
		}
	}
	else if (!stricmp (Args[1],"SAVE"))//$POINTINAREA(SAVE,hpia)
	{
		HANDLE	hPIAStruct = (HANDLE)atol (Args[2]);
		if (hPIAStruct)
		{
			LPPOINTINAREASTRUCT	PIAStruct = (LPPOINTINAREASTRUCT)GlobalLock (hPIAStruct);
			HFILE	Fid = GSSiOpenFile (Args[3],0,OF_CREATE);

			if (Fid != HFILE_ERROR)
			{
				HPDPOINT	pPoints = (HPDPOINT)GlobalLock (PIAStruct->hPoints);
				LPPIAAStruct pPIAA=(LPPIAAStruct)GlobalLock (PIAStruct->hAccel);

				BigWrite (Fid,PIAStruct,sizeof(POINTINAREASTRUCT),-1);
				BigWrite (Fid,pPoints,PIAStruct->nPoints*sizeof(DPOINT),-1);
				GlobalUnlock (PIAStruct->hPoints);
				if (PIAStruct->hPolyPartLen)
				{
					pPolyPartLen = GlobalLock (PIAStruct->hPolyPartLen);
					BigWrite (Fid,pPolyPartLen,(PIAStruct->nPoly+1)*sizeof(long),-1);
					GlobalUnlock (PIAStruct->hPolyPartLen);
				}
				BigWrite (Fid,pPIAA,sizeof(PIAAStruct)+pPIAA->arraylen*2,-1);
				GlobalUnlock (PIAStruct->hAccel);
				GSSiClose2 (&Fid);
				rtn = TRUE;
			}
			GlobalUnlock (hPIAStruct);
		}
	}
	else if (!stricmp (Args[1],"RECALL"))//$POINTINAREA(RECALL,path)
	{
		HFILE	Fid = GSSiOpenFile (Args[2],0,OF_READ);

		if (Fid != HFILE_ERROR)
		{
			HANDLE		hPIAStruct = GSSiGlobAlloc(GAIDNO 1597,GHND,sizeof(POINTINAREASTRUCT));
			LPPOINTINAREASTRUCT	PIAStruct = (LPPOINTINAREASTRUCT)GlobalLock (hPIAStruct);
			LPPIAAStruct pPIAA;
			PIAAStruct PIAA;

			BigRead (Fid,PIAStruct,sizeof(POINTINAREASTRUCT));
			PIAStruct->hPoints = GSSiGlobAlloc(GAIDNO 1598,GMEM_MOVEABLE,sizeof(DPOINT)*PIAStruct->nPoints);
			pPoints = (HPDPOINT)GlobalLock (PIAStruct->hPoints);
			BigRead (Fid,pPoints,sizeof(DPOINT)*PIAStruct->nPoints);
			GlobalUnlock (PIAStruct->hPoints);
			if (PIAStruct->nPoly > 1)//changed back to 1 to get police update process to work
			{
				PIAStruct->hPolyPartLen = GSSiGlobAlloc(GAIDNO 1598,GMEM_MOVEABLE,(PIAStruct->nPoly+1)*sizeof(long));
				pPolyPartLen = GlobalLock (PIAStruct->hPolyPartLen);

				BigRead (Fid,pPolyPartLen,(PIAStruct->nPoly+1)*sizeof(long));
				GlobalUnlock (PIAStruct->hPolyPartLen);
			}
			BigRead (Fid,&PIAA,sizeof(PIAAStruct));
			PIAStruct->hAccel = GSSiGlobAlloc(GAIDNO 1598,GMEM_MOVEABLE,sizeof(PIAAStruct)+PIAA.arraylen*2);
			pPIAA=(LPPIAAStruct)GlobalLock (PIAStruct->hAccel);
			*pPIAA = PIAA;
			BigRead (Fid,&pPIAA->barray[0],pPIAA->arraylen*2);
			GlobalUnlock (PIAStruct->hAccel);
			GSSiClose2 (&Fid);
			GlobalUnlock (hPIAStruct);
			ltoa ((long)hPIAStruct,OutLoc,10);
			rtn = TRUE;
		}
	}
	else if (!stricmp (Args[1],"DESTROY"))//$POINTINAREA(DESTROY,hpia)
	{
		HANDLE	hPIAStruct = (HANDLE)atol (Args[2]);
		if (hPIAStruct)
		{
			LPPOINTINAREASTRUCT	PIAStruct = (LPPOINTINAREASTRUCT)GlobalLock (hPIAStruct);

			GSSiGlobFree (&PIAStruct->hPoints);
			GSSiGlobFree (&PIAStruct->hAccel);
			GSSiGlobFree (&PIAStruct->hPolyPartLen);
			GSSiGlobUlFree (&hPIAStruct);
			strcpy(OutLoc, "1");
			rtn = TRUE;
		}
	}
	else if (!stricmp (Args[1],"TEST"))//$POINTINAREA(TEST,hpia,point)
	{
		HANDLE	hPIAStruct = (HANDLE)atol (Args[2]);
		LPPOINTINAREASTRUCT	PIAStruct = (LPPOINTINAREASTRUCT)GlobalLock (hPIAStruct);
		DPOINT	WPoint;

		WPoint = atopt (Args[3],&Err);
		if (!Err)
		{
			HPDPOINT	Points = GlobalLock (PIAStruct->hPoints);

			rtn = POINT_IN_AREAD (WPoint, PIAStruct->nPoints,Points,1,0,0,&PIAStruct->hAccel);
			if (rtn)
				strcpy(OutLoc, "1");

			GlobalUnlock (PIAStruct->hPoints);
		}
		GlobalUnlock (hPIAStruct);
	}
	return rtn;
}

void PCTInAreaFunction (int iopt,LPSTR Arg2,LPSTR Arg3, LPSTR Arg4, LPSTR OutLoc)
{
	BOOL	err;
	int		nPoly, nPnts, item;
	HANDLE	hPoly, hPolyPartLen, hPIA;
	double	Pct;

	strcpy (OutLoc,"0");
	switch (iopt)
	{
	case 1://init
		{
			MNMXCORD Bounds = atobounds (Arg2,&err);

			if (err)
				return;
			hPIA = PCTInAreasInit (&Bounds,atoi (Arg3),atob(Arg4));
			itoa ((int)hPIA,OutLoc,10);
		}
		break;
	case 2://add item to be tested
	case 3://add area
		{
			item = atoi (Arg2);
			hPIA = (HANDLE)atoi (Arg3);
			if (!hPIA)
				break;
			if (item == 0)
			{
				int	AreaNum = 1;
				int	Type;
				double	Offset;

				while (hPoly = GetNextHighlightArea (AreaNum++,0,&Type,&nPnts,&nPoly,&hPolyPartLen,&Offset,0))
				{
					LPMNMXCORD	pBounds = (LPMNMXCORD)GlobalLock (hPoly);
					LPDPOINT	pPoints = (LPDPOINT)(pBounds + 1);

					if (PCTInAreasLoad (hPIA,iopt-2,Type,nPnts,pPoints,nPoly,hPolyPartLen,0,0))
						strcpy (OutLoc,"1");
					GSSiGlobFree (&hPolyPartLen);
					GSSiGlobUlFree (&hPoly);
				}
			}
			else if (item >0 && item<=MAXPICKITEMS)
			{
				if (PickList[item-1].Type == 2 || PickList[item-1].Type == 3)
				{
					nPoly = GetPolyPointsWithParts ((LPPICKDATAHEADER)&PickList[item-1],&nPnts,&hPoly,&hPolyPartLen);
					if (nPoly)
					{
						LPMNMXCORD	pBounds = (LPMNMXCORD)GlobalLock (hPoly);
						LPDPOINT	pPoints = (LPDPOINT)(pBounds + 1);

						if (PCTInAreasLoad (hPIA,iopt-2,PickList[item-1].Type,nPnts,pPoints,nPoly,hPolyPartLen,0, pBounds))
							strcpy (OutLoc,"1");
						GSSiGlobFree (&hPolyPartLen);
						GSSiGlobUlFree (&hPoly);
					}
				}
			}
		}
		break;
	case 4:
		hPIA = (HANDLE)atoi (Arg2);
		Pct = PCTInAreas (hPIA);
		ftoa (OutLoc,Pct);
		break;
	case 5:
		hPIA = (HANDLE)atoi (Arg2);
		strcpy (OutLoc,"1");
		PCTInAreasDestroy (hPIA);
		break;
	case 6:
	{
		hPIA = (HANDLE)atoi(Arg2);
		DPOINT pt = PCTInAreasCreatePoint(hPIA);
		dpointtoa(OutLoc, &pt);
	}
		break;
	}
	return;
}

BOOL AddIslandsToPoly (BOOL Reverse,BOOL Delete)
{  
	long	Refno, Sequence, nPoly=0; 
	short	pos=BT_FIRST;
	HIGHLIGHTDATA	HighlightData, HighlightData2;
	LPTHEME	pTheme, SaveTheme = CurTheme; 
	LPVIEWPORT	SaveView=CurView;
	LPINT	pMultiPolygon;  
	HPDPOINT	pNewPolyPoints; 
	DPOINT	FirstPoint; 
	USHORT	i;

	if (!CurView->UpdateFile)
	{
 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
        return FALSE;
	} 
	
	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))
	{   
		pos = BT_NEXT;
		if (HighlightData.PD.Type != 3)
		{
			GSSiMessageBox (0,"Only polygons should be highlighted",0,MB_ICONEXCLAMATION,0);
			return FALSE;
		}
		nPoly++;
	}
	if (nPoly < 2) 
	{  
		GSSiMessageBox (0,"More than one polygon must be highlighted",0,MB_ICONEXCLAMATION,0);
		return FALSE;
	}
	BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_FIRST,BT_ANY,(LPSTR)&Refno);
   	BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData);  

	GSSiGlobUlFree (&hUpdateMultiPolygon);
	hUpdateMultiPolygon = GSSiGlobAlloc(GAIDNO 1124,GMEM_MOVEABLE,USHRT_MAX);
	pMultiPolygon = (LPINT)GlobalLock (hUpdateMultiPolygon);
    nUpdatePolyPoints = 0;
	GSSiGlobUlFree (&hUpdatePoly);
	hUpdatePoly = GSSiGlobAlloc(GAIDNO 1125,GMEM_MOVEABLE,MAXREORGBUF);
	pNewPolyPoints = (HPDPOINT)GlobalLock (hUpdatePoly);
    nUpdateMultiPolygon = 0;
    pos=BT_FIRST;
	while (!BT_FIND (hHighlight2,(LPSTR)&Sequence,pos,BT_ANY,(LPSTR)&Refno))
	{
   		BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData2);  
        PickList[0]=HighlightData2.PD;
		SetConfig (PickList[0].ConfigID);
	    SetViewport (PickList[0].ViewID);
		pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
		CurView->PassID = 4;
		ProcessSelectedTheme = CurView->NumThemes;
		ProcessPickedItem (0,FALSE);        		
		ProcessSelectedTheme = 0;
		DeleteTheme (pTheme);  
		GetSavedPolys ();  
					
   		if (hSavePoly)
		{   LPMNMXCORD lpRect;
			HPDPOINT	lpDpoint; 
			BOOL	First=TRUE;  
			LPINT	pPolyParts; 
			int		nPParts;
			short	nareas, inc=0; 
			long	nPnts;
			DPOINT	TiePoint;
						
            if (hSavePolyParts)
            {
            	pPolyParts = (LPINT)GlobalLock (hSavePolyParts);
            	nareas = *pPolyParts++; 
            }
            else
            { 
                nareas=1;
                nPParts = nSavePoly;  
                pPolyParts = &nPParts;
            }
            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
            lpRect++;
            lpDpoint = (HPDPOINT) lpRect; 
            for (i=0;i<nareas;i++)
            {   
            	nSavePoly = pPolyParts[i];
            	if (Reverse)
            		ReversePoints2 (nSavePoly,lpDpoint);
	            if (!SameDPoint (lpDpoint,&lpDpoint[nSavePoly-1]))
	            	inc = 1;
	           	FirstPoint = *lpDpoint;  
	           	if (!nUpdatePolyPoints)
	           		TiePoint = FirstPoint; 
	           	else
	           		First = FALSE;
	           	*pMultiPolygon++ = nSavePoly+inc; 
	           	nUpdateMultiPolygon++;
	            while (nSavePoly--)
	            {   
	            	*pNewPolyPoints++ = *lpDpoint++; 
	            	nUpdatePolyPoints++;
		        } 
		        if (inc)
	            {   
	            	*pNewPolyPoints++ = FirstPoint; 
	            	nUpdatePolyPoints++;
		        } 
		        if (!First)
	            {   
	            	*pNewPolyPoints++ = TiePoint; 
	            	nUpdatePolyPoints++; 
		        } 
		        if (i)
   	            	lpDpoint++;
	        }
            GlobalUnlock (hSavePoly);
			GlobalUnlock(hSavePolyParts);
        } 
        DestroySavedPolys (); 
        if (Delete && pos != BT_FIRST)
        	DeletePickedItem2 (0);  
		pos = BT_NEXT;
	}

	CurTheme = SaveTheme;
	SetCurView (SaveView);
    ClearHighlightList (FALSE); 
    GlobalUnlock (hUpdatePoly);   
    GlobalUnlock (hUpdateMultiPolygon);
	PickList[0] = HighlightData.PD;
	UpdateItem=51;   
	UpdateRecord (0,PickList[0].Desc,0,0,0,0,1,-1);
	GSSiGlobFree (&hUpdatePoly);
	GSSiGlobFree (&hUpdateMultiPolygon);  
    return (TRUE);
} 

BOOL SaveMaskAreas(LPSTR Directory)
{
	short	pos = BT_FIRST;
	long	Refno;
	HIGHLIGHTDATA	HighlightData;
	long	nPnts;
	HANDLE	hPnts;
	HPDPOINT	Points;
	HFILE	Fid;
	char	FileName[MAX_PATH];

	while (!BT_FIND(hHighlight, (LPSTR)&Refno, pos, BT_ANY, (LPSTR)&HighlightData))
	{
		pos = BT_NEXT;
		if (HighlightData.PD.Type == 3)
		{
			if (GetPolyPoints((LPPICKDATAHEADER)&HighlightData.PD, FALSE, &nPnts, &hPnts, 0))
			{
				Points = (HPDPOINT)GlobalLock(hPnts);
				sprintf(FileName, "%s\\%s.bin", Directory, HighlightData.PD.UDI);
				Fid = GSSiOpenFile(FileName, 0, OF_CREATE);
				if (Fid != HFILE_ERROR)
				{
					BigWrite(Fid, (HPSTR)&nPnts, 4, -1);
					BigWrite(Fid, (HPSTR)Points, nPnts*sizeof(DPOINT), -1);
					GSSiClose2 (&Fid);
				}
				GSSiGlobUlFree(&hPnts);
			}
		}
		nPoly++;
	}
	return TRUE;
}

POINT PIAACenter(LPPIAAStruct pPIAA, LPLONG piCPDist, LPLONG pMaxn, BOOL UsePCTBox, LPBOOL pHaveCP)
#if ENABLETRACE
{GSSiEnterProg (1380);
#endif
{   
	DWORD	loc, n,maxn=0, stopn;
	int		jx, jy, midx, midy, yinc=0, xinc, up, back, rowlen, endx, endy;
	int		ix, iy, mnx=0, mny=0, mxx, mxy, maxx, maxy;
    int		x,y;
	POINT	PIAAPoint;
	DPOINT	DPoint;
	long	iCPDist=0;
	HPBYTE	barray;   
	POINT	CenterPoint={0,0};
	
	*pHaveCP = FALSE;
	
	barray = (HPBYTE)&pPIAA->barray[0];
	
	mxx = maxx = pPIAA->Width-1;
	mxy = maxy = pPIAA->Height-1;  
	midx = pPIAA->Width / 2;
	midy = pPIAA->Height / 2; 
	stopn = min (maxx * maxy * 0.33,min(maxx,maxy)*min(maxx,maxy)*0.67);
	for (jy=0;jy<pPIAA->Height;jy++) 
	{   
		if (jy % 2)
			y = midy - yinc;
		else
			y = midy + yinc++;   
		if (y < mny || y > mxy)
			continue;
		if (!ContinueProcessing)
			break;
		xinc = 0;
		for (jx=0;jx<pPIAA->Width;jx++)
		{
			if (jx % 2)
				x = midx - xinc;
			else
				x = midx + xinc++;   
			if (x < mnx || x > mxx)
				continue;
			n=0; 
			up   = 1;
			back = 0;
			rowlen = 2;
			
			loc = x + y * pPIAA->Width;
			if (barray[loc] != 1)
				goto HaveSize; 
								
NextLoop:
			iy = y + up;
			ix = x - back;
	
			endx = ix + rowlen;
			while (ix < endx)
			{
				if (ix < 0 || ix > maxx || iy < 0 || iy > maxy)
					goto HaveSize;
				loc = ix + iy * pPIAA->Width;
				if (barray[loc] != 1)
					goto HaveSize; 
				n++;
				ix++;
			} 
			ix--; 
			iy--;
	
			endy = iy - rowlen;
			while (iy > endy)
			{
				if (ix < 0 || ix > maxx || iy < 0 || iy > maxy)
					goto HaveSize;
								
				loc = ix + iy * pPIAA->Width;
				if (barray[loc] != 1)
					goto HaveSize; 
				n++;
				iy--;
			} 
			iy++;  
			ix--;
	
			endx = ix - rowlen;
			while (ix > endx)
			{
				if (ix < 0 || ix > maxx || iy < 0 || iy > maxy)
					goto HaveSize;
								
				loc = ix + iy * pPIAA->Width;
				if (barray[loc] != 1)
					goto HaveSize; 
				n++;
				ix--;
			} 
			ix++; 
			iy++;
	
			endy = iy + rowlen;
			while (iy < endy)
			{
				if (ix < 0 || ix > maxx || iy < 0 || iy > maxy)
					goto HaveSize;
								
				loc = ix + iy * pPIAA->Width;
				if (barray[loc] != 1)
					goto HaveSize; 
				n++;
				iy++;
			} 
	
			up++;
			back++;
			rowlen+=2; 
			goto NextLoop;
							
	HaveSize:
			if (n > maxn)
			{
				maxn = n;
				CenterPoint.x = x;
				CenterPoint.y = y; 
				*pHaveCP = TRUE;
				iCPDist = max (abs(ix-x),abs(iy-y));
				if (n > stopn)
					goto Exit;
				mnx = max (mnx,back);
				mny = max (mny,up);
				mxx = min (mxx,pPIAA->Width-back-1);
				mxy = min (mxy,pPIAA->Height-up-1);
			}
								
		}
		if (UsePCTBox)
			SetContinueProcessing(StatusWindowUpdate2(0, pPIAA->Height, jy));
	} 
Exit:
	if (piCPDist)
		*piCPDist = iCPDist; 
	if (pMaxn)
		*pMaxn = maxn;
{
#if ENABLETRACE
GSSiExitProg (1380);
#endif
	return CenterPoint;
}
#if ENABLETRACE
}
#endif
}  

BOOL AreaInArea2 (LPSTR TAGOrRef,LPSTR AinAGMD,int SpeedFactor,double MinPCT,BOOL IncludeBoundaries)
#if ENABLETRACE
{GSSiEnterProg (1149);
#endif
{   
	char	DefStr[]="BoundaryRef(B4),AreaRef(B4),FractionOfBoundary(R8),FractionOfAreaInBoundary(R8),SizeOfBoundary(R8),SizeOfArea(R8),BoundaryTAG(C42),AreaTAG(C42),NumInBoundary(B4)";
	short	NumIndexFields=2, pos=BT_FIRST;
	LPSTR	pColon=_fstrchr (TAGOrRef,':');
	long	AreaRefno, BoundaryAreaRefno, Num, InRefno,nBoundaryPoints,BoundaryAreaRef,nRecs=1,nRecs2,Done=0,Done2;
	BOOL	st; 
	HANDLE	hDB;
	LPGWDHEADER lpGWDHead;
	HIGHLIGHTDATA	HighlightData;   
	char	BoundaryAreaTAG[44], AreaTAG[44]; 
	HANDLE	BoundaryAccelerator, AreaAccelerator, hBoundaryPoints;    
	double	BoundaryAreaSize, SizeInBounds,TotSizeInBounds=0,FractionInBounds,FractionOfBounds, AreaSize;
	HPDPOINT	BoundaryPoints;
	double	SavePIASizeFactor = PIASizeFactor; 
	short	SavePIAWidthFactor = PIAWidthFactor;
	char	SavedHltList[MAX_PATH];
	HANDLE	hTempHlt=0;
	BOOL	rtn=FALSE;
	
	if (!ExistFile (AinAGMD))
	{
		if (!CreateGWDDatabase (AinAGMD,1,FALSE,0,NumIndexFields,DefStr))
			goto Exit;
	} 
	ClearPolyOff(FALSE);
	if (!stricmp (TAGOrRef,"HLT"))
	{
		GSSiGetTempFileName (0,"gma",0,SavedHltList); 
		CopyHighlightList (SavedHltList);
		ClearHighlightList(FALSE);
		hTempHlt = BT_OPEN (SavedHltList,0,BT_READ,0);
		if (BT_FIND (hTempHlt,(LPSTR)&BoundaryAreaRef,BT_FIRST,BT_ANY,(LPSTR)&HighlightData))
			st = 0;
		else
		{
			st = 1;
	        PickList[0]=HighlightData.PD;
			nRecs = BT_NUM_IN_INDEX (hTempHlt);  
			CreateStatusWind (hWndMain,2,"Locate Areas in Boundaries");
		}
	}
	else if (pColon)
	{  
		*pColon++ = 0;
		st = PickByRefno (0,TAGOrRef,pColon,-1); 
		pColon--;
		*pColon = ':';
		BoundaryAreaRef = PickList[0].Refno;
	}
	else 
	{
		BoundaryAreaRef = atol (TAGOrRef);
		st = PickByRefno (BoundaryAreaRef,0,0,-100);
	}
NextArea:
	if (!st || !SelectAreaToOffsetFile(0, 0, 0))
		goto Exit;
	if (!GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nBoundaryPoints,&hBoundaryPoints, 0))
		goto Exit;
	sprintf (BoundaryAreaTAG,"%s:%s",PickList[0].Prefix,PickList[0].UDI);
	ClearHighlightList(FALSE);
	Num = HighlightInArea (CurView->hWnd,0,TRUE,FALSE,0);
    hDB = OpenGWDatabase (AinAGMD,BT_WRITE);
    if (!hDB)
		goto Exit;
	if (SpeedFactor)
		PIASizeFactor=SpeedFactor;  
	PIAWidthFactor = 1;
	TotSizeInBounds = 0;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);  
    BoundaryPoints = (HPDPOINT)GlobalLock (hBoundaryPoints);
	BoundaryAreaSize = fabs (ComputeAreaAreaD (BoundaryPoints,nBoundaryPoints,0));
	BoundaryAccelerator = PointInAreaAcceleratorSetup (nBoundaryPoints,BoundaryPoints,1,0,0);

	nRecs2 = BT_NUM_IN_INDEX (hHighlight);  
	StatusWindowUpdate2 (PickList[0].UDI,nRecs, Done);
	pos=BT_FIRST;
	Done2 = 0;
	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&AreaRefno,pos,BT_ANY,(LPSTR)&HighlightData))
	{
		pos = BT_NEXT;     
		if (HighlightData.PD.Type == 3)
		{   
			HANDLE	hPnts;
			
			sprintf (AreaTAG,"%s:%s",HighlightData.PD.Prefix,HighlightData.PD.UDI);
			if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&nPnts,&hPnts,0))
			{  
				HPDPOINT	Points = (HPDPOINT)GlobalLock (hPnts);
	      		HANDLE		hAccelerator = PointInAreaAcceleratorSetup (nPnts,Points,1,0,0); 
				HPBYTE		barray;
				long		TotGridPoints = 0;
				long		GridPointsInBounds = 0; 
				long		x,y,loc;
				LPPIAAStruct pPIAA = (LPPIAAStruct)GlobalLock (hAccelerator);
	
 				AreaSize = fabs (ComputeAreaAreaD (Points,nPnts,0));
   				barray = (HPBYTE)&pPIAA->barray[0];
				for (y=0;y<pPIAA->Height;y++) 
				{   
					for (x=0;x<pPIAA->Width;x++)
					{  
						loc = (long)x + (long)y * pPIAA->Width;
						if (barray[loc]) 
						{   
							int	n = 4 / barray[loc], ina;
							POINT	PIAAPoint; 
							DPOINT	DPoint;
							
					      	TotGridPoints += n;
							PIAAPoint.x = x;
							PIAAPoint.y = y;
							DPoint = PIAAPointToDPoint (PIAAPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
					      	ina = PointInAreaAccelerator (&DPoint,BoundaryAccelerator); 
					      	if (ina)
								GridPointsInBounds += n/ina; 
						}
					} 
				} 
				GSSiGlobUlFree (&hPnts);
				FractionInBounds = min(max (0,(double)GridPointsInBounds / (double)TotGridPoints),1.0);
				SizeInBounds = FractionInBounds * AreaSize; 
				FractionOfBounds = SizeInBounds / BoundaryAreaSize;
				TotSizeInBounds += SizeInBounds;
	      		GSSiGlobUlFree (&hAccelerator);
				GSSiGlobUlFree (&hPnts);
				SetFieldValFromCharAndName(lpGWDHead, "BoundaryRef", (LPSTR)&BoundaryAreaRef, TRUE, TRUE);
				SetFieldValFromCharAndName(lpGWDHead, "AreaRef", (LPSTR)&AreaRefno, TRUE, TRUE);
				SetFieldValFromCharAndName(lpGWDHead, "AreaTAG", (LPSTR)AreaTAG, FALSE, TRUE);
				SetFieldValFromCharAndName(lpGWDHead, "BoundaryTAG", (LPSTR)BoundaryAreaTAG, FALSE, TRUE);
				SetFieldValFromCharAndName(lpGWDHead, "NumInBoundary", (LPSTR)&Num, TRUE, TRUE);
				SetFieldValFromCharAndName(lpGWDHead, "FractionOfBoundary", (LPSTR)&FractionOfBounds, TRUE, TRUE);
				SetFieldValFromCharAndName(lpGWDHead, "FractionOfAreaInBoundary", (LPSTR)&FractionInBounds, TRUE, TRUE);
				SetFieldValFromCharAndName(lpGWDHead, "SizeOfBoundary", (LPSTR)&BoundaryAreaSize, TRUE, TRUE);
				SetFieldValFromCharAndName(lpGWDHead, "SizeOfArea", (LPSTR)&AreaSize, TRUE, TRUE);
			   	if (FractionInBounds * 100 >= MinPCT)
					st = GWDReplaceRecord (lpGWDHead,0,0,-1); 
			} 
		}
		StatusWindowUpdate2 (AreaTAG,nRecs2, ++Done2);
//		StatusWindowUpdate (0,0, nRecs, NumRecs++);  
	}
	SetFieldValFromCharAndName(lpGWDHead, "BoundaryRef", (LPSTR)&BoundaryAreaRef, TRUE, TRUE);
   	AreaRefno = 0;
	SetFieldValFromCharAndName(lpGWDHead, "AreaRef", (LPSTR)&AreaRefno, TRUE, TRUE);
   	*AreaTAG = 0;
	SetFieldValFromCharAndName(lpGWDHead, "AreaTAG", (LPSTR)AreaTAG, FALSE, TRUE);
	SetFieldValFromCharAndName(lpGWDHead, "BoundaryTAG", (LPSTR)BoundaryAreaTAG, FALSE, TRUE);
	SetFieldValFromCharAndName(lpGWDHead, "NumInBoundary", (LPSTR)&Num, TRUE, TRUE);
    FractionOfBounds = 1.0 - (TotSizeInBounds/BoundaryAreaSize);
	SetFieldValFromCharAndName(lpGWDHead, "FractionOfBoundary", (LPSTR)&FractionOfBounds, TRUE, TRUE);
   	FractionInBounds = 0;
	SetFieldValFromCharAndName(lpGWDHead, "FractionOfAreaInBoundary", (LPSTR)&FractionInBounds, TRUE, TRUE);
	SetFieldValFromCharAndName(lpGWDHead, "SizeOfBoundary", (LPSTR)&BoundaryAreaSize, TRUE, TRUE);
   	AreaSize = BoundaryAreaSize - TotSizeInBounds;
	SetFieldValFromCharAndName(lpGWDHead, "SizeOfArea", (LPSTR)&AreaSize, TRUE, TRUE);
	if (IncludeBoundaries)
		st = GWDReplaceRecord (lpGWDHead,0,0,-1); 
    GlobalUnlock (hDB); 
    CloseGWDatabase (hDB); 
	ClearPolyOff(FALSE);
	ClearHighlightList(FALSE); 
	GSSiGlobFree (&BoundaryAccelerator);
	GSSiGlobUlFree (&hBoundaryPoints); 
	if (ContinueProcessing && hTempHlt && !BT_FIND (hTempHlt,(LPSTR)&BoundaryAreaRef,BT_NEXT,BT_ANY,(LPSTR)&HighlightData))
	{
        PickList[0]=HighlightData.PD;
		StatusWindowUpdate (0,HighlightData.PD.UDI, nRecs, ++Done);
		goto NextArea;
	}
Exit:
	DestroyStatusWindow(0);  
	SetContinueProcessing ( TRUE);
	PIASizeFactor = SavePIASizeFactor;
	PIAWidthFactor = SavePIAWidthFactor;
	if (hTempHlt)
	{
		RecallHighlightList (SavedHltList);
		BT_CLOSEANDDELETE (&hTempHlt);
	}
{
#if ENABLETRACE
GSSiExitProg (1149);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}  

BOOL CreateSF3File (LPSTR InFile,LPSTR OutFile)
{
 	int		i;
	BTVARDESC BTVar[2];
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	long	TotFileLen;
	GWDHEADER16 GWDHead; 
	GWDHEADER	GWDHead32;
	LPGWDHEADER	lpGWDHead;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo, lpCTField, lpMCDField, lpMAField;
	HANDLE hBT, hVars, hDB, hBlock, hFldInLen,hBTOld, hDBma, hDBct;
	int	FidOld;
	time_t ltime;
	int		FidData;
	int		ibeg;
	OFSTRUCTGM	OFStruct;
	LPVOID	lpVal;
	LPSTR	pName;
	GWFLDINFO FldInfo;
	long	SaveFrame, TLID, Offsetct, Offsetsa;
	int		st, lenct, lenma; 
	char	FileIn1[128], Name[128], TxtFile[128], str[2048]; 
   	char	File[128]; 
	LPSTR	lpDot;    
	HFILE	FidIn, FidTxt;  
	long	CurOff;
 	HFILE	Fid;
 	LPSTR	pBS;   
 	long	TotLen;  

	 _fstrcpy (FileIn1,OutFile);
	 lpDot = _fstrrchr (FileIn1,'.');
	 if (lpDot)
	 	*lpDot = 0; 
//	 _fstrcpy (TxtFile,FileIn1);
	 _fstrcat (FileIn1,".in1");
//	 _fstrcat (TxtFile,".ctf");	
	 lpGWDHead = &GWDHead32; 
     _fmemset (&GWDHead,0,sizeof(GWDHEADER16));

	 FidData = GSSiOpenFile (OutFile,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=2;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 GWDHead.NumIndexFields[1]=1;
	 GWDHead.IndexFields[1][0]=1; 
	 GWDHead.lKeys[1] = SHRT_MAX;
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	 ibeg = 0;

	FldInfo.Len = 12;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"BLKGRPID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
    
	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"LOGRECNO");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

    for (i=0;i<76;i++)
    {
    	sprintf (Name,"OFFSET%2.2i",i+1);
		FldInfo.Len = 4;
		FldInfo.Beg = ibeg;
		ibeg += FldInfo.Len;
		FldInfo.Type = BT_INTEGER;
		_fstrcpy (FldInfo.Name,Name);
		BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
		GWDHead.NumFields++; 
	}
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = time(0);
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
 	 GSSillseek (FidData,0,2);
			     
			
	 BTVar[0].BT_VARLEN=12;
	 BTVar[0].BT_VARTYP=BT_CHAR;
	 BTVar[0].BT_VAROFF=0;
	 BT_CREATE (FileIn1, 4, FALSE, 1, 1,BTVar,FALSE, 0, GWDHead.TimeStamp, FALSE);
 	 GSSiClose2 (&FidData);

     hDB = OpenGWDatabase (OutFile,BT_WRITE);   
     lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
     for (i=0;i<76;i++)
     { 
     	_fstrcpy (File,InFile);
     	pBS = _fstrrchr (File,'\\');
     	pBS += 3;
     	sprintf (pBS,"%5.5i.UF3",i+1);  
     	Fid = GSSiOpenFile (File,0,OF_READ);
     	if (Fid == HFILE_ERROR)
	    {
			MessageBox (0,"Unable to open file",InFile,MB_ICONEXCLAMATION);     
	     	return FALSE; 
	    }  
	    GSSiClose2 (&Fid);
	 }
     FidIn = GSSiOpenFile (InFile,0,OF_READ);
     if (FidIn == HFILE_ERROR)
	 {
	 	MessageBox (0,"Unable to open file",InFile,MB_ICONEXCLAMATION);     
	 	return FALSE; 
	 }  
	 CreateStatusWind (hWndMain,2,"Create SF3 File");
	 StatusWindowUpdate (0,InFile, 77,1); 
   	 TotLen = GSSifilelength (FidIn); 
     while (ContinueProcessing && fgetstring (str,512,FidIn))
     {  
     	long	LOGRECNO = ldread (&str[18],7);
     	long	STATE=ldread (&str[29],2);
     	long	COUNTY=ldread (&str[31],3);
     	char	TRACT[8];
     	char	BLKGRP=str[61];
     	char	BlkGrpKey[16]; 
     	long	M1=-1;
     	long	SUMLEV = ldread (&str[8],3);
     	long	GEOCOMP = ldread (&str[11],2);
     	
     	switch (SUMLEV)
     	{
     		case 50:
     		case 140:
     		case 150: 
     		if (GEOCOMP)
     			break;
	     	strncpy0 (TRACT,&str[55],6);
	     	Truncate (TRACT);
	     	PadString (TRACT,'0',6);
	     	sprintf (BlkGrpKey,"%2.2ld%3.3ld%s%c",STATE,COUNTY,TRACT,BLKGRP);  
			SetFieldValFromCharAndName(lpGWDHead, "LOGRECNO", (LPSTR)&LOGRECNO, TRUE, TRUE);
			SetFieldValFromCharAndName(lpGWDHead, "BLKGRPID", (LPSTR)BlkGrpKey, TRUE, TRUE);
			for (i=0;i<76;i++)
			{
		    	sprintf (Name,"OFFSET%2.2i",i+1);
				SetFieldValFromCharAndName(lpGWDHead, Name, (LPSTR)&M1, TRUE, TRUE);
			}
			GWDAddRecord (lpGWDHead,0,0); 
			default:
			break;
		}
		StatusWindowUpdate2 (0,TotLen,GSSillseek (FidIn,0,1));  
     }  
     GSSiClose2 (&FidIn); 
//     FidTxt = GSSiOpenFile (TxtFile,0,OF_CREATE);
     for (i=0;i<76;i++)
     { 
     	short	lentxt;
     	
	    sprintf (Name,"OFFSET%2.2i",i+1);
     	_fstrcpy (File,InFile);
     	pBS = _fstrrchr (File,'\\');
     	pBS += 3;
     	sprintf (pBS,"%5.5i.UF3",i+1);  
     	Fid = GSSiOpenFile (File,0,OF_READ);
     	if (Fid == HFILE_ERROR)
	    {
			MessageBox (0,"Unable to open file",InFile,MB_ICONEXCLAMATION);     
	     	return FALSE; 
	    }
     	TotLen = GSSifilelength (Fid);  
		StatusWindowUpdate (0,File, 77,i+2);
     	while (ContinueProcessing && fgetstring (str,2040,Fid))
     	{
     		LPSTR	comma=str, pLastComma;
     		short	j; 
     		long	LOGRECNO;
     		
     		for (j=0;j<4;j++)
     			comma = _fstrchr (comma+1,',');
     		comma++;
     		while (*comma == '0')
     			comma++;
     		LOGRECNO = atol (comma);
     		if (!BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&LOGRECNO,BT_FIRST,BT_EQ,(LPSTR)&Offset))
     		{
				len=FillGWDData (lpGWDHead,Offset);
				CurOff = GSSillseek (lpGWDHead->Fid,0,2); 
				while ((pLastComma = _fstrrchr (comma,',')))
				{
					if (_fstrcmp (pLastComma,",0"))
						break;
					*pLastComma = 0;
				} 
				lentxt = CompressCensusString (comma);  
				
				BigWrite (lpGWDHead->Fid,(HPSTR)&lentxt,2,-1);
				if (lentxt)
					BigWrite (lpGWDHead->Fid,(HPSTR)comma,lentxt,-1); 
				SetFieldValFromCharAndName(lpGWDHead, Name, (LPSTR)&CurOff, TRUE, TRUE);
				GWDReplaceRecord (lpGWDHead,len,0,Offset);  
     		}
			StatusWindowUpdate2 (0,TotLen,GSSillseek (Fid,0,1));  
     	}
     	GSSiClose2 (&Fid);
		if (!ContinueProcessing)
			break;
     } 
//     GSSiClose2 (&FidTxt);
     SetContinueProcessing ( TRUE); 
     BT_CLOSEANDDELETE (&lpGWDHead->BTHandle[1]);
	 lpGWDHead->NumIndex=1;
     GlobalUnlock (hDB);
     CloseGWDatabase (hDB); 
	 DestroyStatusWindow(0);  
	 return TRUE;
}

BOOL CreateSF1File (LPSTR InFile,LPSTR OutFile)
{
 	int		i;
	BTVARDESC BTVar[2];
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	long	TotFileLen;
	short	maxlen = 0;
	GWDHEADER16 GWDHead;
	GWDHEADER	GWDHead32;
	LPGWDHEADER	lpGWDHead;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo, lpCTField, lpMCDField, lpMAField;
	HANDLE hBT, hVars, hDB, hBlock, hFldInLen,hBTOld, hDBma, hDBct;
	int	FidOld;
	time_t ltime;
	int		FidData;
	int		ibeg;
	OFSTRUCTGM	OFStruct;
	LPVOID	lpVal;
	LPSTR	pName;
	GWFLDINFO FldInfo;
	long	SaveFrame, TLID, Offsetct, Offsetsa,ii=0,iii=0;
	int		st, lenct, lenma; 
	char	FileIn1[128], Name[128], TxtFile[128], str[4096]; 
	LPSTR	lpDot;    
	HFILE	FidIn, FidTxt;  
	long	CurOff;
 	char	File[128]; 
 	HFILE	Fid;
 	LPSTR	pBS;   
   	long	TotLen;  

	 _fstrcpy (FileIn1,OutFile);
	 lpDot = _fstrrchr (FileIn1,'.');
	 if (lpDot)
	 	*lpDot = 0; 
//	 _fstrcpy (TxtFile,FileIn1);
	 _fstrcat (FileIn1,".in1");
//	 _fstrcat (TxtFile,".ctf");	
	 lpGWDHead = &GWDHead32; 
     _fmemset (&GWDHead,0,sizeof(GWDHEADER16));

	 FidData = GSSiOpenFile (OutFile,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=2;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 GWDHead.NumIndexFields[1]=1;
	 GWDHead.IndexFields[1][0]=1; 
	 GWDHead.lKeys[1] = SHRT_MAX;
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	 ibeg = 0;

	FldInfo.Len = 15;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"BLOCKID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
    
	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"LOGRECNO");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

    for (i=0;i<39;i++)
    {
    	sprintf (Name,"OFFSET%2.2i",i+1);
		FldInfo.Len = 4;
		FldInfo.Beg = ibeg;
		ibeg += FldInfo.Len;
		FldInfo.Type = BT_INTEGER;
		_fstrcpy (FldInfo.Name,Name);
		BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
		GWDHead.NumFields++; 
	}
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = time(0);
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
 	 GSSillseek (FidData,0,2);
			     
			
	 BTVar[0].BT_VARLEN=15;
	 BTVar[0].BT_VARTYP=BT_CHAR;
	 BTVar[0].BT_VAROFF=0;
	 BT_CREATE (FileIn1, 4, FALSE, 1, 1,BTVar,FALSE, 0, GWDHead.TimeStamp, FALSE);
 	 GSSiClose2 (&FidData);

     hDB = OpenGWDatabase (OutFile,BT_WRITE);   
     lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
     for (i=0;i<39;i++)
     { 
     	_fstrcpy (File,InFile);
     	pBS = _fstrrchr (File,'\\');
     	pBS += 3;
     	sprintf (pBS,"%5.5i.UF1",i+1);  
     	Fid = GSSiOpenFile (File,0,OF_READ);
     	if (Fid == HFILE_ERROR)
	    {
			MessageBox (0,"Unable to open file",InFile,MB_ICONEXCLAMATION);     
	     	return FALSE; 
	    }  
	    GSSiClose2 (&Fid);
	 }
     FidIn = GSSiOpenFile (InFile,0,OF_READ);
     if (FidIn == HFILE_ERROR)
     {
		MessageBox (0,"Unable to open file",InFile,MB_ICONEXCLAMATION);     
     	return FALSE; 
     }
	 CreateStatusWind (hWndMain,2,"Create SF1 File");
	 StatusWindowUpdate (0,InFile, 40,1); 
   	 TotLen = GSSifilelength (FidIn); 
     while (ContinueProcessing && fgetstring (str,512,FidIn))
     {  
     	long	LOGRECNO = ldread (&str[18],7);
     	long	STATE=ldread (&str[29],2);
     	long	COUNTY=ldread (&str[31],3);
     	char	TRACT[8];
     	char	BLOCK[5];
     	char	BlkGrpKey[16]; 
     	long	M1=-1;
     	long	SUMLEV = ldread (&str[8],3);
     	long	GEOCOMP = ldread (&str[11],2);
     	
     	switch (SUMLEV)
     	{
     		case 101: 
     			iii++;
     		case 50:  
     		case 140:
     		case 150: 
     		if (GEOCOMP)
     			break; 
     		ii++;
	     	strncpy0 (TRACT,&str[55],6);
	     	Truncate (TRACT);
	     	PadString (TRACT,'0',6); 
	     	strncpy0 (BLOCK,&str[62],4); 
	     	Truncate (BLOCK);
	     	PadString (BLOCK,'0',4); 
	     	sprintf (BlkGrpKey,"%2.2ld%3.3ld%s%s",STATE,COUNTY,TRACT,BLOCK);  
			SetFieldValFromCharAndName(lpGWDHead, "LOGRECNO", (LPSTR)&LOGRECNO, TRUE, TRUE);
			SetFieldValFromCharAndName(lpGWDHead, "BLOCKID", (LPSTR)BlkGrpKey, TRUE, TRUE);
			for (i=0;i<39;i++)
			{
		    	sprintf (Name,"OFFSET%2.2i",i+1);
				SetFieldValFromCharAndName(lpGWDHead, Name, (LPSTR)&M1, TRUE, TRUE);
			}
			GWDAddRecord (lpGWDHead,0,0); 
			default:
			break;
		}
		StatusWindowUpdate2 (0,TotLen,GSSillseek (FidIn,0,1));  
     }  
     GSSiClose2 (&FidIn); 
//     FidTxt = GSSiOpenFile (TxtFile,0,OF_CREATE);
     for (i=0;i<39;i++)
     { 
     	short	lentxt;
     	
	    sprintf (Name,"OFFSET%2.2i",i+1);
     	_fstrcpy (File,InFile);
     	pBS = _fstrrchr (File,'\\');
     	pBS += 3;
     	sprintf (pBS,"%5.5i.UF1",i+1);  
     	Fid = GSSiOpenFile (File,0,OF_READ);
     	if (Fid == HFILE_ERROR)
	    {
			MessageBox (0,"Unable to open file",InFile,MB_ICONEXCLAMATION);     
	     	return FALSE; 
	    }
     	TotLen = GSSifilelength (Fid); 
     	sprintf (str,"File = %s, Maxlen = %i",File,maxlen); 
		StatusWindowUpdate (0,str, 40,i+2); 
     	while (ContinueProcessing && fgetstring (str,4090,Fid))
     	{
     		LPSTR	comma=str, pLastComma;
     		short	j; 
     		long	LOGRECNO;
     		
			maxlen = max (maxlen,_fstrlen(str));
     		for (j=0;j<4;j++)
     			comma = _fstrchr (comma+1,',');
     		comma++;
     		while (*comma == '0')
     			comma++;
     		LOGRECNO = atol (comma);
     		if (!BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&LOGRECNO,BT_FIRST,BT_EQ,(LPSTR)&Offset))
     		{
				len=FillGWDData (lpGWDHead,Offset);
				CurOff = GSSillseek (lpGWDHead->Fid,0,2); 
				while ((pLastComma = _fstrrchr (comma,',')))
				{
					if (_fstrcmp (pLastComma,",0"))
						break;
					*pLastComma = 0;
				} 
				lentxt = CompressCensusString (comma);  
				BigWrite (lpGWDHead->Fid,(HPSTR)&lentxt,2,-1);
				if (lentxt)
					BigWrite (lpGWDHead->Fid,(HPSTR)comma,lentxt,-1);
				SetFieldValFromCharAndName(lpGWDHead, Name, (LPSTR)&CurOff, TRUE, TRUE);
				GWDReplaceRecord (lpGWDHead,len,0,Offset);  
     		}
			StatusWindowUpdate2 (0,TotLen,GSSillseek (Fid,0,1));  
     	}
     	GSSiClose2 (&Fid); 
     	if (!ContinueProcessing)
     		break;
     } 
//     GSSiClose2 (&FidTxt);
     SetContinueProcessing ( TRUE); 
     BT_CLOSEANDDELETE (&lpGWDHead->BTHandle[1]);
	 lpGWDHead->NumIndex=1;
     GlobalUnlock (hDB);
     CloseGWDatabase (hDB); 
	 DestroyStatusWindow(0);  
	 return TRUE;
}

BOOL TABToComma (LPSTR InFile,LPSTR OutFile)
{   
	HFILE	FidIn, FidOut; 
	HANDLE	hStr=GSSiGlobAlloc(GAIDNO 1126,GMEM_MOVEABLE,4096);
	LPSTR	pStr=GlobalLock (hStr);
	
	FidIn = GSSiOpenFile (InFile,0,OF_READ);
	FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);  
	while (fgetstring (pStr,4090,FidIn))
	{   
		ReplaceChar (pStr,'\t',',');
		fputstring (pStr,FidOut);
	}
	GSSiGlobUlFree (&hStr);
	GSSiClose2 (&FidIn);
	GSSiClose2 (&FidOut);
	return TRUE;
}

BOOL CrossMatch (LPSTR File1, LPSTR File2, LPSTR Cmd1, LPSTR Cmd2, LPSTR OutFile1, LPSTR OutFile2, short CompareLength)
{   
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;
	BTVARDESC	BTVar[2]; 
	char		Name1[144], Name2[144], Value[256];      
	HANDLE		hDLT,hBT1, hBT2, hSTR=GSSiGlobAlloc(GAIDNO 1127,GMEM_MOVEABLE,4096); 
	LPSTR		str = GlobalLock (hSTR);
	BOOL		rtn=FALSE;   
	short		Dummy=0;
	
	if (!CompareLength)
		return FALSE;
	GSSiGetTempFileName (0,"gma",0,Name1); 
	GSSiGetTempFileName (0,"gmb",0,Name2); 
			
	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=CompareLength;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (Name1, 2, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	hBT1 = BT_OPEN (Name1,0, BT_WRITE, 0);
	BT_CREATE (Name2, 2, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	hBT2 = BT_OPEN (Name2,0, BT_WRITE, 0);
	
	Fid = GSSiOpenFile (File1,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
		goto Exit;
		
	fgetstring (str,1024,Fid);
	if (!ProcessDelimTextHeader(str,0,Fid,&hDLT,0,0))
	{
		GSSiClose2 (&Fid);
		goto Exit;
	}
	while (fgetstring (str,1024,Fid))
	{
		GetDelimTextData(str,hDLT,1024);
		_fstrcpy (Value,Cmd1);
		ExpandText (Value);
		PadString (Value,0,CompareLength);
		BT_PUT (hBT1,Value,(LPSTR)&Dummy);
	}
	GSSiClose2 (&Fid); 
	GSSiGlobFree (&hDLT);
	Fid = GSSiOpenFile (File2,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
		goto Exit;
		
	fgetstring (str,1024,Fid);
	if (!ProcessDelimTextHeader(str,0,Fid,&hDLT,0,0))
	{
		GSSiClose2 (&Fid);
		goto Exit;
	}
	while (fgetstring (str,1024,Fid))
	{
		GetDelimTextData(str,hDLT,1024);
		_fstrcpy (Value,Cmd2);
		ExpandText (Value);
		PadString (Value,0,CompareLength);
		BT_PUT (hBT2,Value,(LPSTR)&Dummy);
	}
	GSSiClose2 (&Fid); 
	GSSiGlobFree (&hDLT);

	Fid = GSSiOpenFile (OutFile1,&OFStruct,OF_CREATE);
    if (!BT_FIND (hBT1,Value,BT_FIRST,BT_ANY,(LPSTR)&Dummy))
    	do
    	{
    		if (BT_FIND (hBT2,Value,BT_FIRST,BT_EQ,(LPSTR)&Dummy))
    			fputstring (Value,Fid);
    	}
    	while (!BT_FIND (hBT1,Value,BT_NEXT,BT_ANY,(LPSTR)&Dummy));
	GSSiClose2 (&Fid); 

	Fid = GSSiOpenFile (OutFile2,&OFStruct,OF_CREATE);
    if (!BT_FIND (hBT2,Value,BT_FIRST,BT_ANY,(LPSTR)&Dummy))
    	do
    	{
    		if (BT_FIND (hBT1,Value,BT_FIRST,BT_EQ,(LPSTR)&Dummy))
    			fputstring (Value,Fid);
    	}
    	while (!BT_FIND (hBT2,Value,BT_NEXT,BT_ANY,(LPSTR)&Dummy));
	GSSiClose2 (&Fid); 
    
	rtn = TRUE;
Exit:
	GSSiGlobUlFree (&hSTR);
	BT_CLOSEANDDELETE (&hBT1);
	BT_CLOSEANDDELETE (&hBT2);  
	return rtn;
}

BOOL CreateSportMapCMD (LPSTR FromPath,LPSTR CmdFile)
{ 
	char	Name[144], Name2[144],str2[150], CDName[32], OrigCDName[8],str[255]; 
	short	DType, Res=0, AreaNum;
	HFILE	Fid,Fid2;  
	long	TotFiles=0;   
	OFSTRUCTGM	OFStruct;
	LPSTR	pFile=str2, pLoc;    
    
    pLoc = _fstrrchr (FromPath,'\\');
    _fstrcpy (CDName,++pLoc);   
    _fstrupr (CDName);
	GSSiGetTempFileName(0,"gma",0,Name);
	GSSiGetTempFileName(0,"gmb",0,Name2);
	Fid =	GSSiOpenFile (Name,(LPOFSTRUCTGM)&OFStruct,OF_CREATE);
	SearchDirectoriesInDir (FromPath,Fid,&TotFiles,"wetlands",1); 
	GSSillseek (Fid,0,0);
	while (fgetstring (str2,144,Fid))
	{  
		Fid2 =	GSSiOpenFile (Name2,(LPOFSTRUCTGM)&OFStruct,OF_CREATE);
		SearchFilesInDir (str2, ".plt", Fid2,&TotFiles,"*.plt",1,TRUE,TRUE);     
		GSSiClose2 (&Fid2);
		CreateMapIndex (str2,Name2,1,FALSE,"","",FALSE,1,0,0,0,FALSE);
		GSSiRemove (Name2);
	}
	GSSiClose2 (&Fid);
	Fid =	GSSiOpenFile (Name,(LPOFSTRUCTGM)&OFStruct,OF_CREATE);
	SearchFilesInDir (FromPath, "", Fid,&TotFiles,"index",1,TRUE,TRUE); 
	GSSillseek (Fid,0,0);
	while (fgetstring (str2,144,Fid))
	{   
		_fstrupr (str2);
		pLoc = _fstrstr (str2,"\\AREA");
		pLoc += 5;
		AreaNum = atoi (pLoc);
		if ((pLoc = _fstrstr (str2,"MN100"))) 
		{
			_fstrncpy (OrigCDName,pLoc,6);
			OrigCDName[6] = 0;
		}
		else
			*OrigCDName = 0;
		if (_fstrstr (str2,"PRIM\\INDEX"))
			DType = 'P';
		else if (_fstrstr (str2,"WETLANDS\\INDEX"))
			DType = 'W';
		else
		{
			DType = 'O';
			if (!*OrigCDName)
			{
				if (_fstrstr (str2,"ORTHOS4"))
					Res = 4;
				else if (_fstrstr (str2,"ORTHOS2"))  
					Res = 2;
				else if (_fstrstr (str2,"ORTHOS1"))  
					Res = 1;
			}
		}
		switch (DType)
		{
			case 'O': 
				if (*OrigCDName)
				{
					sprintf (str,"$INSERTLINE([%%DL]ORTHOS\\FILELIST.TXT,- Subset Data -,@$SUBSET(%s)@[%%DL]CDDATA\\%s\\AREA%i\\%s\\ORTHOS8\\INDEX)",
								  OrigCDName,CDName,AreaNum,OrigCDName);
				}
				else
				{
					sprintf (str,"$INSERTLINE([%%DL]ORTHOS\\FILELIST.TXT,- %i Meter Data -,@$SCALERNG(8,0)@[%%DL]CDDATA\\%s\\AREA%i\\ORTHOS%i\\INDEX)",
				    			  Res,CDName,AreaNum,Res);
				}
			break;
			
			case 'P':
				sprintf (str,"$APPEND([%%DL]PRIM\\FILELIST.TXT,@$SUBSET(%s)@[%%DL]CDDATA\\%s\\AREA%i\\%s\\prim\\index)",
                         	 OrigCDName,CDName,AreaNum,OrigCDName);
			break;
			
			case 'W':
				sprintf (str,"$APPEND([%%DL]WETLANDS\\FILELIST.TXT,@$SUBSET(%s)@[%%DL]CDDATA\\%s\\AREA%i\\%s\\wetlands\\index)",
                         	 OrigCDName,CDName,AreaNum,OrigCDName);
			break;
		}
		AppendFile (CmdFile,str);   
	}  
	GSSiClose2 (&Fid);
	GSSiRemove (Name);
	return TRUE;
} 

BOOL CreateSportMapCD (LPSTR OrderFile,LPSTR OutDir,short AreaNum)
{   
	OFSTRUCTGM	OFStruct;
	HFILE		FidOrder;  
	char		str[260]; 
	MNMXCORD	Bounds;   
	short		i;     
	LPSTR		pTAB, pVal;   
	long		MB, SQMiles;
	short		Type, n=0;
	char		Name[128], DType[64],CmdFile[128]; 
	long		Width; 
	DPOINT		Center; 
	BOOL		Found = FALSE;
	
	sprintf (CmdFile,"%s\\gmcmdfil.txt",OutDir); 
	FidOrder = GSSiOpenFile (OrderFile,&OFStruct,OF_READ);
	if (FidOrder == HFILE_ERROR)
		return FALSE;
	for (i=0;i<11;i++)
		fgetstring (str,256,FidOrder); 
	while (fgetstring (str,256,FidOrder))
	{   
		Truncate (str);
		if (*str)
		{
			if (n == AreaNum)
			{   
				Found = TRUE;
				pTAB = _fstrchr (str,'\t');
				pTAB++;
				Type = atoi (str);
				pVal = pTAB;
				pTAB = _fstrchr (pVal,'\t');
				*pTAB++ = 0;
				_fstrcpy (Name,pVal);
				pVal = pTAB;
				pTAB = _fstrchr (pVal,'\t');
				*pTAB++ = 0;
				_fstrcpy (DType,pVal);
				pVal = DType;
				*++pVal = 0;
				SetGlobalValue ("%RES",DType);
				sscanf (pTAB,"%ld\t%ld\t%lf %lf %lf %lf",&SQMiles,&MB,&Bounds.xmn,&Bounds.ymn,&Bounds.xmx,&Bounds.ymx);
				Width = sqrt (SQMiles);  
				Width *= (5280 * FTM); 
				Width *= GetGlobalDVal2 ("[SPMINC]",1.0);
				Center = MinMaxMidPointD (&Bounds);
				Bounds.xmn = Center.x - Width/2;
				Bounds.ymn = Center.y - Width/2;
				Bounds.xmx = Center.x + Width/2;
				Bounds.ymx = Center.y + Width/2;     
				sprintf (str,"$APPEND([%%DL]zoomlist\\hires.txt,%s|(%.1f,%.1f,%.1f,%.1f))",Name,Bounds.xmn,Bounds.ymn,Bounds.xmx,Bounds.ymx); 
				makedirectories (CmdFile,FALSE,FALSE);
				AppendFile (CmdFile,str); 
				sprintf (str,"%s\\area%i",OutDir,n+1);
				SetGlobalValue ("%COPYMAPTO",str);
			    CurView->CurZoomAreaRef = 0;
				ZoomToRect(Bounds,FALSE);  
			}  
			n++;
		}
	}
	GSSiClose2 (&FidOrder);  
	if (!Found)  
	{   
		CreateSportMapCMD (OutDir,CmdFile);
		GSSiMessageBox (0,"End of Order","",MB_OK,0);
	}
	return TRUE;
}

BOOL FontDisplay (LPSTR FontName)
{
	LOGFONT	LogFont;    
   	HFONT	OldFont,hFont;
	char	txt[16];
	short	x,y, chr;
	RECT	Rect;   
	HDC		hDC = GetDC (hWndMain);
	HRGN	hRgn;
	HBRUSH	FillBrush; 

	GetClientRect(hWndMain,&Rect); 
	hRgn = CreateRectRgn(Rect.left,Rect.top,Rect.right,Rect.bottom);
  	SelectClipRgn (hDC,hRgn);
  	DeleteObject(hRgn);
	FillBrush = GetStockObject(WHITE_BRUSH);
	FillRect (hDC,&Rect,FillBrush);   
	_fmemset (&LogFont,0,sizeof(LOGFONT));
	LogFont.lfHeight = -30; 
	LogFont.lfEscapement = 0;   
	LogFont.lfWeight = FW_NORMAL;    
	LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	LogFont.lfQuality = PROOF_QUALITY; 
	_fstrcpy (LogFont.lfFaceName,FontName);  
	hFont = CreateFontIndirect((LPLOGFONT)&LogFont);
	OldFont = SelectObject (hDC,hFont);
	for (chr=0;chr<256;chr++)
	{   
		SIZE	txSize;

		x = (chr % 16) * 60;
		y = (chr / 16) * 40;
		txt[0] = chr;
		txt[1] = 0;  
		SelectObject (hDC,hFont); 
		GetTextExtentPoint32 (hDC,txt,1,&txSize);
		if (txSize.cx || txSize.cy)
		{
			TextOut (hDC,x,y,txt,1);
			GdiFlush ();
			SelectObject (hDC,OldFont);   
			sprintf (txt,"(%i)",chr);
			x+=25;
			TextOut (hDC,x,y,txt,_fstrlen(txt));
		}	
	}
	
	DeleteObject (hFont);
	ReleaseDC (hWndMain,hDC);
	return TRUE;
}
	

HDIB GetPRIMBMP (LPDPOINT pWorldPoint,LPHANDLE phTran)
{
	HDIB	hDib=0; 
	short	ifile;
	float	RSQMIN;
	

	if (!DPointInBounds (pWorldPoint,&pPRIMFiles[0].Bounds) && LastiFile > -1) 
	{
		if (DPointInBounds (pWorldPoint,&pPRIMFiles[LastiFile].Bounds))
		{   
			if (POINT_IN_AREAD (*pWorldPoint, (DWORD)pPRIMFiles[LastiFile].nBoundPoints,(HPDPOINT)&pPRIMFiles[LastiFile].BoundPoints,1,0,0,0))
			{   
				*phTran = pPRIMFiles[LastiFile].hTran;
				return pPRIMFiles[LastiFile].hDIB;
			}
		}
	}
	for (ifile =0;ifile<nPRIMFiles;ifile++)
	{
		if (DPointInBounds (pWorldPoint,&pPRIMFiles[ifile].Bounds))
		{   
			if (POINT_IN_AREAD (*pWorldPoint, (DWORD)pPRIMFiles[ifile].nBoundPoints,(HPDPOINT)&pPRIMFiles[ifile].BoundPoints,1,0,0,0))
			{   
				LastiFile = ifile;
				if (!pPRIMFiles[ifile].hDIB)
				{   
					if (nOpenDIB == MAXOPENDIB)
					{   
						short	jfile, minfile=0;
						long	MinUsed=pPRIMFiles[0].LastUsed;
						for (jfile =1;jfile<nPRIMFiles;jfile++)
						{
							if (pPRIMFiles[jfile].LastUsed < MinUsed)
							{
								MinUsed = pPRIMFiles[jfile].LastUsed;
								minfile = jfile;
							}
						}
						if (pPRIMFiles[minfile].hDIB > (HDIB)1)
				    		DestroyDIB (pPRIMFiles[minfile].hDIB); 
				    	pPRIMFiles[minfile].hDIB = 0;
						CloseTRANS2 (&pPRIMFiles[minfile].hTran); 
						pPRIMFiles[minfile].LastUsed = LONG_MAX;
					}
					else
						nOpenDIB++;
					pPRIMFiles[ifile].hDIB=LoadDIB (pPRIMFiles[ifile].Name);   
					pPRIMFiles[ifile].LastUsed = NextUse++;
					if (!pPRIMFiles[ifile].hDIB)
					{
						pPRIMFiles[ifile].hDIB = (HDIB)1;   
						pPRIMFiles[ifile].LastUsed = LONG_MAX;
						GSSiMessageBox (0,"Unable to load bitmap",pPRIMFiles[ifile].Name,MB_ICONEXCLAMATION,0);
					}
					else
						pPRIMFiles[ifile].hTran = STRAN2 (1606,pPRIMFiles[ifile].WorldX,pPRIMFiles[ifile].WorldY,
														  pPRIMFiles[ifile].BMPX,pPRIMFiles[ifile].BMPY,
														  pPRIMFiles[ifile].nTranPoints,&RSQMIN,PRIMTranType,0);  
				} 
				*phTran = pPRIMFiles[ifile].hTran;
				return pPRIMFiles[ifile].hDIB;
			}
		}
	}
	return hDib;
}

RGBTRIPLE GetPRIMRGBTriple (LPDPOINT pWorldPoint)
{
	RGBTRIPLE rgb={254,249,248}; 
	short	j;
	static	long	i=0;   
	DPOINT	BMPPoint;   
	HDIB	hDIB;
	HANDLE	hTran;
	long	BMPRow, BMPCol;
	
	if ((hDIB = GetPRIMBMP (pWorldPoint,&hTran))>(HDIB)1)
	{ 
	    LPBITMAPINFOHEADER pDIB = (LPBITMAPINFOHEADER)GlobalLock (hDIB);    
		HPSTR	pBits = (HPSTR)FindDIBBits ((LPSTR)pDIB);
        long	RowLen = pDIB->biSizeImage / pDIB->biHeight;   
typedef RGBTRIPLE HUGE *HPRGBTRIPLE;
        HPRGBTRIPLE	pRGB;
        
		BMPPoint = TranPoint (pWorldPoint,hTran);
		BMPRow = IDNINT (BMPPoint.y);
		BMPCol = IDNINT (BMPPoint.x); 
		if (BMPRow >= 0 && BMPRow < pDIB->biHeight  &&
			BMPCol >= 0 && BMPCol < pDIB->biWidth) 
		{
			pBits += BMPRow * RowLen + BMPCol*3;
			pRGB = (HPRGBTRIPLE)pBits;
			rgb = *pRGB;
		}
	    GlobalUnlock (hDIB);
	}
	return rgb;
}

void AddPointsToCornerList (HPDPOINT pPoints,HPDPOINT pPointsBMP,short NumPoints,short FileID)
{   
	short	i,j,k,ii;  

	
	if (!pPoints)
	{
		nCorners = 0;
		GSSiGlobUlFree (&hCorners);
		return;
	}  
	if (!hCorners)
	{
		hCorners = GSSiGlobAlloc(GAIDNO 1128,GHND,MAXCORNERS * (long)sizeof(CORNERINFO)); 
		pCorners = (LPCORNERINFO)GlobalLock (hCorners);
	}
	for (i=0;i<NumPoints;i++) 
	{
		for (j=0;j<nCorners;j++)
		{   
			for (k=0;k<pCorners[j].nPoints;k++)
			{
				if (FileID == pCorners[j].FileID[k])
					goto SkipCorner;
			}
			if (ldistp (pPoints[i],pCorners[j].AvePoint) < PRIMTol)
			{   
				short	n=pCorners[j].nPoints;
				
				if (n<MAXPOINTSPERCORNER)
				{
					pCorners[j].Points[n] = pPoints[i];
					pCorners[j].BMPPoints[n] = pPointsBMP[i];
					pCorners[j].PointIDs[n] = i;
					pCorners[j].FileID[n] = FileID;
					pCorners[j].nPoints++;
					pCorners[j].AvePoint = AverageDPoints (pCorners[j].Points,pCorners[j].nPoints); 
				}
				else
					ii=1;
				goto NextPoint;
			} 
SkipCorner:;
		}
		pCorners[nCorners].AvePoint = pCorners[nCorners].Points[0] = pPoints[i];   
		pCorners[nCorners].BMPPoints[0] = pPointsBMP[i];
		pCorners[nCorners].nPoints = 1;
		pCorners[nCorners].PointIDs[0] = i;
		pCorners[nCorners++].FileID[0] = FileID;
NextPoint:;
	}
	return;
}

BOOL GetPRIMCornerPoint (short FileID,short PointID,HPDPOINT pPoints,LPDPOINT pBMPPoint)
{   
	short	i,j;
	
	for (i=0;i<nCorners;i++)
	{
		for (j=0;j<pCorners[i].nPoints;j++)
		{
			if (pCorners[i].FileID[j] == FileID && pCorners[i].PointIDs[j] == PointID)
			{
				pPoints[PointID]=pCorners[i].AvePoint;   
				*pBMPPoint = pCorners[i].BMPPoints[j];
				return TRUE;
			}
		}
	} 
	return FALSE;
}

BOOL LoadPRIMBounds (short idum)
{
	HFILE Fid, Fid2, FidRun, FidAD, FidMore4;
	OFSTRUCTGM OFStruct; 
	char	str[260], File[132], leaf[16],dir[128];
	short	i=0,n, ifile, Maxp=0; 
	float	RSQMIN;
	HANDLE	hTran;
	double	XFROM[32],YFROM[32],XTO[32],YTO[32];
	MNMXCORD	Bounds;
	HANDLE	hSymDesc=0, hPoints=GSSiGlobAlloc(GAIDNO 1129,GMEM_MOVEABLE,4096), hDLT=0, hBT=0;
   	LPDPOINT	pPoints=(LPDPOINT)GlobalLock (hPoints);
	short	NumSyms=0;  
    short	NewAreaSymbol = GetDictSymbolNumber ("PRIMBNDS");
   	long	NewRefno=1, NumPoints,ii,NumFiles=0; 
    double	metersperpix=16,Res=metersperpix,d1,d2, AveX, AveY;   
    DPOINT	BMPPoint, pPointsBMP[MAXCORNERPOINTS], MidPointWorld, MidPointBMP;
    char	Prefix[10]="PRIMMAP", UDI[32],drive[8], AreaDumpFile[128];   
	short	Dummy=0; 
	LPSTR	pNam;
	BTVARDESC	BTVar[2];   
	BOOL	UseCorner = GetGlobalBVal2 ("[%USECORNERS]",TRUE);
    
    PRIMTol = GetGlobalDVal2 ("[%PRIMTOL]",500);
    PRIMTranType = GetGlobalLVal2 ("[%PRIMTRANTYPE]",1);
/*    HDIB	hDIB=LoadDIB ("c:\\prims\\vla1.bmp");
    LPBITMAPINFOHEADER pDIB = GlobalLock (hDIB);    
     GlobalUnlock (hDIB);
     DestroyDIB (hDIB); */ 
	nPRIMFiles = 0;   
	LastiFile = -1;  
	NextUse = 1;
	nOpenDIB = 0;
    _fstrcpy (PltName,"[%DL]maplib\\primbnds.plt");   
    CreateNewMap (PltName,&CurView->WBounds,0,0,0,0,0,0,FALSE);
	EditBounds = CurView->FileMNMX;
		        
	AddToSymList (NewAreaSymbol,&NumSyms,&hSymDesc); 
	Fid2 = GSSiOpenFile ("[%DL]PRIMS\\FILES.TXT",&OFStruct,OF_READ);   
	FidMore4 = GSSiOpenFile ("c:\\morefour.txt",&OFStruct,OF_CREATE);
	while (fgetstring (File,128,Fid2))
	{   
		_splitpath (File,drive,dir,0,0);  
		sprintf (AreaDumpFile,"%s%sareadump.txt",drive,dir);
		pNam = _fstrrchr (dir,'\\');
		*pNam = 0;
		pNam = _fstrrchr (dir,'\\');
		pNam++;
		SetWindowText (hWndMain,File);
		if (_fstrstr (File,"WILLMAR1"))
			ii=1;
		Fid = GSSiOpenFile (File,&OFStruct,OF_READ); 
/*		fgetstring (str,256,Fid);
		fgetstring (str,256,Fid);
		fgetstring (str,256,Fid);
		sscanf (str,"%f %f",&Bounds.xmn,&Bounds.ymn);
		fgetstring (str,256,Fid);
		sscanf (str,"%f %f",&Bounds.xmx,&Bounds.ymx);
		fgetstring (str,256,Fid); 
		n = atol (str); 
		i = 0;
		while (n--)
		{
			if (fgetstring (str,256,Fid))
			{   
				if (*str != '#')
				{
					if (sscanf (str,"%f %f %f %f",&XFROM[i],&YFROM[i],&XTO[i],&YTO[i]) == 4)
						i++; 
				}
			}
		} */
		i = 0;
		while (fgetstring (str,256,Fid))
		{   
			if (*str != '#')
			{
				if (sscanf (str,"%lf %lf %lf %lf",&XFROM[i],&YFROM[i],&XTO[i],&YTO[i]) == 4)
					i++; 
			}
		} 
		hTran = STRAN2 (1607,XFROM,YFROM,XTO,YTO,i,&RSQMIN,1,0); 
		if (RSQMIN < 0.9999)
		{
			sprintf (str,"Bad RSQ: %f",RSQMIN);
			GSSiMessageBox (0,str,File,MB_OK,0); 
		}
		NumPoints=0;
		FidAD = GSSiOpenFile (AreaDumpFile,&OFStruct,OF_READ);
		if (FidAD == HFILE_ERROR)
			GSSiMessageBox (0,"No areadump",AreaDumpFile,MB_ICONEXCLAMATION,0);
		else
		{   
			
			fgetstring (str,256,FidAD);
			fgetstring (str,256,FidAD);
			while (*str && *str != 'E')
			{   
				
				ii = sscanf (str,"%lf %lf",&pPointsBMP[NumPoints].x,&pPointsBMP[NumPoints].y);
				NumPoints++;
				fgetstring (str,256,FidAD);
			}
			fgetstring (str,256,FidAD); 
			if (*str != 'E')
				GSSiMessageBox (0,"Mult Areas",AreaDumpFile,MB_ICONEXCLAMATION,0); 
			else
				Maxp = max (Maxp,NumPoints);
			GSSiClose2 (&FidAD);
			if (NumPoints > 6)
				fputstring (pNam,FidMore4);
		}  
		d1 = ldistp (pPointsBMP[0],pPointsBMP[2]);
		for (i=0;i<NumPoints;i++)  
			pPoints[i] = TranPoint (&pPointsBMP[i],hTran); 
		d2 = ldistp (pPoints[0],pPoints[2]);  
//		metersperpix = d2/d1;
		AddPointsToCornerList (pPoints,pPointsBMP,(short)NumPoints,nPRIMFiles); 
		AddPolyToMap (1,&NumPoints, &hPoints,0,NewRefno++,0,2,NewAreaSymbol,0,Prefix,pNam,-1,-1,-1,0,0,0,0,TRUE,0);
		GSSiClose2 (&Fid);   
		CloseTRANS2 (&hTran);
		nPRIMFiles++;
	} 
	GSSiClose2 (&FidMore4);
	NewAreaSymbol = GetDictSymbolNumber ("PRIMBNDSL");
	AddToSymList (NewAreaSymbol,&NumSyms,&hSymDesc); 
	hPRIMFiles = GSSiGlobAlloc(GAIDNO 1130,GHND,nPRIMFiles * (long)sizeof(PRIMFILEINFO));    
	pPRIMFiles = (LPPRIMFILEINFO)GlobalLock (hPRIMFiles);
	GSSillseek (Fid2,0,0);
	ifile = 0;
	while (fgetstring (File,128,Fid2))
	{   
		HANDLE	hPoints2=GSSiGlobAlloc(GAIDNO 1131,GMEM_MOVEABLE,2*sizeof(DPOINT));
		HPDPOINT	pPoints2=(HPDPOINT)GlobalLock (hPoints2);  
		int		NumPoints2=2; 
		LPSTR	pBS; 
		
		SetWindowText (hWndMain,File);
		_fstrcpy (pPRIMFiles[ifile].Name,File);  
		pPRIMFiles[ifile].LastUsed = LONG_MAX;
        pBS = _fstrrchr (pPRIMFiles[ifile].Name,'\\');  
        _fstrcpy (pBS,".bmp");
		Fid = GSSiOpenFile (File,&OFStruct,OF_READ); 
/*		fgetstring (str,256,Fid);
		fgetstring (str,256,Fid);
		fgetstring (str,256,Fid);
		fgetstring (str,256,Fid);
		fgetstring (str,256,Fid);
		n = atol (str); */
		pPRIMFiles[ifile].nTranPoints = 0;   
		AveX = AveY = 0;
/*		while (n--)
		{*/
		while (fgetstring (str,256,Fid))
		{   
			if (*str != '#')
			{
				if (sscanf (str,"%lf %lf %lf %lf",&pPRIMFiles[ifile].BMPX[pPRIMFiles[ifile].nTranPoints],
				                                      &pPRIMFiles[ifile].BMPY[pPRIMFiles[ifile].nTranPoints],
				                                      &pPRIMFiles[ifile].WorldX[pPRIMFiles[ifile].nTranPoints],
				                                      &pPRIMFiles[ifile].WorldY[pPRIMFiles[ifile].nTranPoints]) == 4)
				{
				    AveX += pPRIMFiles[ifile].WorldX[pPRIMFiles[ifile].nTranPoints];
				    AveY += pPRIMFiles[ifile].WorldY[pPRIMFiles[ifile].nTranPoints]; 
				    if (UseCorner)
						pPRIMFiles[ifile].nTranPoints++;  
				}
			}
		}
        GSSiClose2 (&Fid); 
/*        MidPointWorld.x = AveX / pPRIMFiles[ifile].nTranPoints;
        MidPointWorld.y = AveY / pPRIMFiles[ifile].nTranPoints;
		hTran = STRAN2 (pPRIMFiles[ifile].WorldX,pPRIMFiles[ifile].WorldY,
					    pPRIMFiles[ifile].BMPX,pPRIMFiles[ifile].BMPY,
					    pPRIMFiles[ifile].nTranPoints,&RSQMIN,1,0);   
		MidPointBMP = TranPoint (&MidPointWorld,hTran);
		CloseTRANS2 (&hTran);  
		pPRIMFiles[ifile].nTranPoints = 1;
		pPRIMFiles[ifile].WorldX[0] = MidPointWorld.x;
		pPRIMFiles[ifile].WorldY[0] = MidPointWorld.y;
		pPRIMFiles[ifile].BMPX[0] = MidPointBMP.x;
		pPRIMFiles[ifile].BMPY[0] = MidPointBMP.y; */
		NumPoints = 0;   
		DBoundsInit (&pPRIMFiles[ifile].Bounds);
		while (GetPRIMCornerPoint (ifile,(short)NumPoints,pPoints,&BMPPoint)) 
		{
			pPRIMFiles[ifile].WorldX[pPRIMFiles[ifile].nTranPoints] = pPoints[NumPoints].x;
			pPRIMFiles[ifile].WorldY[pPRIMFiles[ifile].nTranPoints] = pPoints[NumPoints].y; 
			pPRIMFiles[ifile].BMPX[pPRIMFiles[ifile].nTranPoints] = BMPPoint.x;
			pPRIMFiles[ifile].BMPY[pPRIMFiles[ifile].nTranPoints] = BMPPoint.y; 
			pPRIMFiles[ifile].nTranPoints++;
			NumPoints++; 
		}
		_splitpath (File,0,dir,0,0);  
		pNam = _fstrrchr (dir,'\\');
		*pNam = 0;
		pNam = _fstrrchr (dir,'\\');
		pNam++;
		for (i=0;i<NumPoints;i++)
		{   
			pPRIMFiles[ifile].BoundPoints[i] = pPoints[i];
			AddDPointToMinMax (&pPoints[i],&pPRIMFiles[ifile].Bounds);
			pPoints2[0] = pPoints[i];
			if (i < NumPoints-1)
				pPoints2[1] = pPoints[i+1];
			else 
				pPoints2[1] = pPoints[0];  
			sprintf (UDI,"%s-%0.2i",pNam,i+1);
			AddPolyToMap (1,&NumPoints2, &hPoints2,1,NewRefno++,0,2,NewAreaSymbol,0,Prefix,UDI,-1,-1,-1,0,0,0,0,TRUE,0);
		}
		GSSiGlobUlFree (&hPoints2);      
		pPRIMFiles[ifile++].nBoundPoints = NumPoints;
	}
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,0); 
    DestroySymList (&NumSyms,&hSymDesc); 

	UseUserPickAp =FALSE;
	SystemPickAp = -2000;	
	MaxPick=16;
	SetPickAp(0); 
	for (i=0;i<nCorners;i++)
	{
		PickItems2 (CurView->hWnd,pCorners[i].AvePoint,FALSE,TRUE,TRUE); 
		while (NumPicked--)
		{   
			if (PickList[NumPicked].Desc == NewAreaSymbol &&
				PickList[NumPicked].PCT > 0.01 &&
				PickList[NumPicked].PCT < 0.99)
				{   
					pCorners[i].AvePoint = PickList[NumPicked].PickedPoint;
					goto NextCorner;
				}
		}
NextCorner:; 
	} 

	NewAreaSymbol = GetDictSymbolNumber ("PRIMBNDSC");
	AddToSymList (NewAreaSymbol,&NumSyms,&hSymDesc); 
	GSSillseek (Fid2,0,0);  
	ifile = 0;
	while (fgetstring (File,128,Fid2))
	{   
		LPSTR	UDI;
		SetWindowText (hWndMain,File);
		_splitpath (File,0,dir,0,0);  
		UDI = _fstrrchr (dir,'\\');
		*UDI = 0;
		UDI = _fstrrchr (dir,'\\');
		UDI++;
		NumPoints = 0;
		while (GetPRIMCornerPoint (ifile,(short)NumPoints,pPoints,&BMPPoint))
			NumPoints++;  
		AddPolyToMap (1,&NumPoints, &hPoints,0,NewRefno++,0,2,NewAreaSymbol,0,Prefix,UDI,-1,-1,-1,0,0,0,0,TRUE,0);
		ifile++;
	}
	GSSiClose2 (&Fid2);
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,0); 
    DestroySymList (&NumSyms,&hSymDesc); 
	AddPointsToCornerList (0,0,0,0);
	UseUserPickAp =TRUE;
	
	LoadGlobalInit ("c:\\primbnds.txt",FALSE); 
	FidRun = GSSiOpenFile ("c:\\primrun.txt",&OFStruct,OF_READ);
	if (FidRun == HFILE_ERROR)
		goto Exit;
	fgetstring (str,128,FidRun);
	ProcessDelimTextHeader(str,0,FidRun,&hDLT,0,0);

	GSSiGetTempFileName (0,"gmp",0,(LPSTR)str);
	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=6;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (str, 2, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hBT= BT_OPEN (str, 0, BT_WRITE, 0);
	while (fgetstring (str,128,FidRun))
	{   
		
    	GetDelimTextData(str,hDLT,128);  
    	_fstrcpy (str,"[UDI]");
    	ExpandText (str);
    	BT_PUT (hBT,str,(LPSTR)&Dummy);   
    	NumFiles++;
	} 
	GSSiClose2 (&FidRun);
	GSSiGlobFree (&hDLT);
	
	{
		long	FileWidth=1280,FileHeight=960; 
		double	width = UserBounds[1].x - UserBounds[0].x, height = UserBounds[1].y - UserBounds[0].y;   
		long	TotHeight = (height / metersperpix), TotWidth = (width / metersperpix);
		long	nFileRows = TotHeight / FileHeight + 1;
		long	nFileCols = TotWidth / FileWidth + 1;
		long	iFileCol, iFileRow;
	    BITMAPFILEHEADER bmfHead;
	    LPBITMAPINFO    pDibInfo, pDibInfoOut;   
	    char	fNameBM[128];  
	    double	FileX, FileY;
	    HFILE	FidBPW;
	    	    
/*	    Bounds.xmn = UserBounds[0].x;
	    Bounds.ymn = UserBounds[0].y;
	    Bounds.xmx = UserBounds[1].x;
	    Bounds.ymx = UserBounds[1].y;
	    NewAreaSymbol = GetDictSymbolNumber ("PRIMBNDS");
	    _fstrcpy (PltName,"[%DL]maplib\\primindx.plt");   
	    CreateNewMap (PltName,&Bounds,0,0,0,0,0,0);
		EditBounds = CurView->FileMNMX;
		AddToSymList (NewAreaSymbol,&NumSyms,&hSymDesc);*/ 
		        
		CreateStatusWind (hWndMain,2,0); 
		ifile = 0;
		for (iFileRow=0;iFileRow<nFileRows;iFileRow++)
		{   
			long	RowHeight=FileHeight;
			
			if (iFileRow == nFileRows-1)
				RowHeight = TotHeight - (nFileRows-1) * FileHeight;
			FileY = UserBounds[0].y + iFileRow * FileHeight * metersperpix;
			for (iFileCol=0;iFileCol<nFileCols;iFileCol++)
			{   
				long	ColWidth = FileWidth, scanlinewidth, HeadLen, irow, icol;      
				HFILE	FidBM;
				OFSTRUCTGM	OFStruct;
				HANDLE	hDibInfoOut; 
				LPRGBTRIPLE	pScanLine, pScanLineBeg, pFirstVoid; 
				RGBTRIPLE	FirstPix, black={0,0,0};
				HANDLE	hScanLine;  
				DPOINT	WorldPoint;
				
				FileX = UserBounds[0].x + iFileCol * FileWidth * metersperpix;
				if (iFileCol == nFileCols-1)
					ColWidth = TotWidth - (nFileCols-1) * FileWidth;   
					
				sprintf (UDI,"%0.3ld%0.3ld",iFileRow,iFileCol);
/*				NumPoints=4;
				pPoints[0].x = FileX;
				pPoints[0].y = FileY;
				pPoints[1].x = FileX;
				pPoints[1].y = FileY + FileHeight * metersperpix;
				pPoints[2].x = FileX + FileWidth * metersperpix;
				pPoints[2].y = FileY + FileHeight * metersperpix;
				pPoints[3].x = FileX + FileWidth * metersperpix;
				pPoints[3].y = FileY; 
				AddPolyToMap (1,&NumPoints, &hPoints,0,NewRefno++,2,NewAreaSymbol,0,Prefix,UDI,-1,-1,-1,0,0,0,0,TRUE);
*/					
				if (!BT_FIND (hBT,UDI,BT_FIRST,BT_EQ,(LPSTR)&Dummy))
				{	
					HeadLen = sizeof(BITMAPINFOHEADER);
					hDibInfoOut = GSSiGlobAlloc(GAIDNO 1132,GHND,HeadLen);
					pDibInfoOut = (LPBITMAPINFO)GlobalLock (hDibInfoOut); 
					pDibInfoOut->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					pDibInfoOut->bmiHeader.biPlanes = 1;
					pDibInfoOut->bmiHeader.biBitCount = 24;
					pDibInfoOut->bmiHeader.biCompression = BI_RGB;
					pDibInfoOut->bmiHeader.biHeight = RowHeight;
					pDibInfoOut->bmiHeader.biHeight = RowHeight;
					pDibInfoOut->bmiHeader.biWidth = ColWidth;
					scanlinewidth = pDibInfoOut->bmiHeader.biWidth * 3;
					if (scanlinewidth % 4)
						scanlinewidth += 4 - scanlinewidth % 4;
					                  
					pDibInfoOut->bmiHeader.biSizeImage = (long)pDibInfoOut->bmiHeader.biHeight * scanlinewidth;
														 
					sprintf (fNameBM,"c:\\primout\\%0.3ld%0.3ld.bmp",iFileRow,iFileCol);
					FidBM = GSSiOpenFile (fNameBM,&OFStruct,OF_CREATE);
					bmfHead.bfType = 19778;
					bmfHead.bfSize = sizeof(BITMAPFILEHEADER) + HeadLen + pDibInfoOut->bmiHeader.biSizeImage;
					bmfHead.bfReserved1 = 0;
					bmfHead.bfReserved2 = 0;
					bmfHead.bfOffBits = HeadLen + sizeof(BITMAPFILEHEADER);
					BigWrite (FidBM,(HPSTR)&bmfHead,sizeof(BITMAPFILEHEADER),-1);
					BigWrite (FidBM,(HPSTR)pDibInfoOut,(UINT)HeadLen,-1);     
					hScanLine = GSSiGlobAlloc(GAIDNO 1133,GHND,scanlinewidth);
					pScanLine = pScanLineBeg = (LPRGBTRIPLE)GlobalLock (hScanLine);  
					WorldPoint.y = FileY;
					for (irow=0;irow<pDibInfoOut->bmiHeader.biHeight;irow++,WorldPoint.y+=metersperpix) 
					{   
						short nVoid = 0;
						WorldPoint.x = FileX; 
						pScanLine = pScanLineBeg;
						for (icol=0;icol<pDibInfoOut->bmiHeader.biWidth;icol++,WorldPoint.x+=metersperpix,pScanLine++) 
						{   
							*pScanLine = GetPRIMRGBTriple (&WorldPoint); 
/*							if (pScanLine->rgbtRed == 248 && pScanLine->rgbtGreen == 249 && pScanLine->rgbtBlue == 254)
							{   
								if (!nVoid)
									pFirstVoid = pScanLine;
								nVoid++;
							} 
							else
							{    
								if (nVoid && nVoid < 30)
								{    
									short redinc = ((short)pScanLine->rgbtRed - (short)FirstPix.rgbtRed) / nVoid;
									short greeninc = ((short)pScanLine->rgbtGreen - (short)FirstPix.rgbtGreen) / nVoid;
									short blueinc = ((short)pScanLine->rgbtBlue - (short)FirstPix.rgbtBlue) / nVoid;
									while (nVoid--) 
									{
									 	pFirstVoid->rgbtRed = FirstPix.rgbtRed;
									 	FirstPix.rgbtRed+=redinc;
									 	pFirstVoid->rgbtGreen = FirstPix.rgbtGreen;
									 	FirstPix.rgbtGreen+=greeninc;
									 	pFirstVoid++->rgbtBlue = FirstPix.rgbtBlue;
									 	FirstPix.rgbtBlue+=blueinc;
									} 
								}
								_fmemmove (&FirstPix,pScanLine,3);
								nVoid = 0;
							} */
						}
						BigWrite (FidBM,(HPSTR)pScanLineBeg,(UINT)scanlinewidth,-1);
						StatusWindowUpdate2 ("",pDibInfoOut->bmiHeader.biHeight,irow+1);  
						if (!ContinueProcessing)
							break;
					}
					GSSiClose2 (&FidBM); 
					sprintf (fNameBM,"c:\\primout\\%0.3ld%0.3ld.bpw",iFileRow,iFileCol);
					FidBPW = GSSiOpenFile (fNameBM,&OFStruct,OF_CREATE);
					sprintf (str,"%f",Res);
					fputstring (str,FidBPW);
					fputstring ("0",FidBPW);
					fputstring ("0",FidBPW);
					sprintf (str,"%f",-Res);
					fputstring (str,FidBPW);
					sprintf (str,"%f",FileX);
					fputstring (str,FidBPW);
					sprintf (str,"%f",WorldPoint.y);
					fputstring (str,FidBPW);
					GSSiClose2 (&FidBPW);
					GSSiGlobUlFree (&hScanLine);  
					GSSiGlobUlFree (&hDibInfoOut);  
					ifile++; 
				}
				StatusWindowUpdate (fNameBM,"",NumFiles,ifile);
				if (!ContinueProcessing)
					break;
			}
		}
	}
/*    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,0); 
    DestroySymList (&NumSyms,&hSymDesc);   */
Exit:
	BT_CLOSEANDDELETE (&hBT); 
	SetContinueProcessing ( TRUE);
	for (ifile =0;ifile<nPRIMFiles;ifile++)
	{
		if (pPRIMFiles[ifile].hDIB>(HDIB)1);
	    	DestroyDIB (pPRIMFiles[ifile].hDIB);
		CloseTRANS2 (&pPRIMFiles[ifile].hTran); 
    }
    GSSiGlobUlFree (&hPoints);
	GSSiGlobUlFree (&hPRIMFiles);    
	DestroyStatusWindow(0); 
	return TRUE;
}

BOOL BuildZoomList (LPSTR Arg1,LPSTR Heading,LPSTR Separator,LPSTR Arg4)
{   BTVARDESC   BTVar[2];
	LPGWDHEADER	lpGWDHead;  
	HIGHLIGHTDATA	HighlightData;
	HCURSOR	hcurSave; 
	long	Refno;    
	short	pos=BT_FIRST; 
	char	UDI[66], TempFile[144], LastUDI[66]="", str[260];  
	OFSTRUCTGM	OFStruct;
	HANDLE	hBT;
	HFILE	FidOut; 
	double	MinDist = atof (Arg4);
	MNMXCORD CombinedMNMX;  
	LPSTR	pSep; 
	BOOL	rtn=FALSE;

	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
    BTVar[0].BT_VARTYP=BT_CHAR;
    BTVar[0].BT_VARLEN=66;
    BTVar[0].BT_VAROFF=0;
	GSSiGetTempFileName (0,"gm",0,TempFile);
    BT_CREATE (TempFile, sizeof(HIGHLIGHTDATA), FALSE, 1, 1,(LPBTVARDESC) BTVar,FALSE, 0, 0, FALSE);      
    hBT = BT_OPEN (TempFile, 0, BT_WRITE, 0);
    FidOut = GSSiOpenFile (Arg1,&OFStruct,OF_CREATE);
    fputstring (Heading,FidOut);
	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
    {   
    	pos = BT_NEXT;
		BT_PUT (hBT,(LPSTR)HighlightData.PD.UDI,(LPSTR)&HighlightData);
    } 
    if (pos == BT_FIRST)
    	goto Exit; 
    rtn = TRUE;
	BT_FIND (hBT,(LPSTR)UDI,BT_FIRST,BT_ANY,(LPSTR)&HighlightData); 
    do
    {   
    	if ((pSep = _fstrstr (UDI,Separator)))
    		*pSep = 0;
    	if (!_fstrcmp (UDI,LastUDI))
    	{
    		AddMinMaxD (&CombinedMNMX,&HighlightData.PD.Rect);
    	}
    	else
    	{   
    		if (*LastUDI)
    		{   
    			if ((CombinedMNMX.xmx - CombinedMNMX.xmn) < MinDist)
    			{
    				CombinedMNMX.xmn = (CombinedMNMX.xmx + CombinedMNMX.xmn)/2 - MinDist/2; 
    				CombinedMNMX.xmx = CombinedMNMX.xmn + MinDist;
    			}
    			if ((CombinedMNMX.ymx - CombinedMNMX.ymn) < MinDist)
    			{
    				CombinedMNMX.ymn = (CombinedMNMX.ymx + CombinedMNMX.ymn)/2 - MinDist/2; 
    				CombinedMNMX.ymx = CombinedMNMX.ymn + MinDist;
    			}
	    		sprintf (str,"%s|(%f %f %f %f)",LastUDI,CombinedMNMX.xmn,CombinedMNMX.ymn,CombinedMNMX.xmx,CombinedMNMX.ymx);
	    		fputstring (str,FidOut);
	    	} 
    		CombinedMNMX = HighlightData.PD.Rect; 
    		_fstrcpy (LastUDI,UDI);
    	}
    }
	while (!BT_FIND (hBT,(LPSTR)UDI,BT_NEXT,BT_ANY,(LPSTR)&HighlightData)); 
	sprintf (str,"%s|(%f %f %f %f)",LastUDI,CombinedMNMX.xmn,CombinedMNMX.ymn,CombinedMNMX.xmx,CombinedMNMX.ymx);
	fputstring (str,FidOut);
Exit:
    BT_CLOSEANDDELETE (&hBT);  
    GSSiClose2 (&FidOut);
    GSSiSetCursor (hcurSave); 
    return rtn;
}
			
BOOL SplitXFERFile (LPSTR Name,LPSTR Dir,short nPieces)
{ 
	OFSTRUCTGM	OFStruct;
	HFILE		Fid, Fid2=HFILE_ERROR;
	char		NewName[128], line[1030];
	long		TotLen, PieceEnd=-1, loc=0;
	short		PieceNum=0;
	
	if ((Fid = GSSiOpenFile (Name,&OFStruct,OF_READ)) == HFILE_ERROR)
		return FALSE;
	TotLen = GSSillseek (Fid,0,2);
	GSSillseek (Fid,0,0);
	CreateStatusWind (hWndMain,1,0); 
	while (fgetstring (line,1024,Fid) && ContinueProcessing)
	{   
		if (*line == 'E')
		{
			loc = GSSillseek (Fid,0,1);
			if (loc > PieceEnd) 
			{   
				if (PieceEnd > 0)
					GSSiClose2 (&Fid2);
				PieceNum++;
				PieceEnd = PieceNum * (TotLen / nPieces); 
				if (PieceNum == nPieces)
					PieceEnd = LONG_MAX; 
				sprintf (NewName,"%s\\part%i.txt",Dir,PieceNum);
				makedirectories (NewName,FALSE,FALSE);
				Fid2 = GSSiOpenFile (NewName,&OFStruct,OF_CREATE);
			}
		} 
		fputstring (line,Fid2);
		StatusWindowUpdate2 ("",TotLen,loc);  
	}
	SetContinueProcessing ( TRUE);
	GSSiClose2 (&Fid);
	GSSiClose2 (&Fid2);
	DestroyStatusWindow(0);  
	return TRUE;
}  

BOOL URTCommands (short nArgs,LPSTR *Arg)
{   
	long	hNum, nlong, nl, StartRefno, FromRef, ToRef;
	HANDLE	hStr=GSSiGlobAlloc(GAIDNO 1134,GMEM_MOVEABLE,256+sizeof(HIGHLIGHTDATA));
	LPSTR	str=GlobalLock (hStr);  
	LPHIGHLIGHTDATA	pHighlightData = (LPHIGHLIGHTDATA)(str+256);
	short	pos;
	BOOL	Opened; 
	long	Refno;
	
	if (!_fstricmp (Arg[1],"CREATE"))
	{
		if (CreateUsedRefTable (atob (Arg[2])))
			goto RtnTrue;
	}
	else if (!_fstricmp (Arg[1],"DUMP"))
	{
		if (DumpUsedRefTable (Arg[2]))
			goto RtnTrue;
	}   
	else if (!_fstricmp (Arg[1],"LOAD"))
	{   
	URTLoad:				
		if (!OpenUsedRefTable (TRUE,&Opened)) 
			goto RtnFalse;
		hNum = BT_NUM_IN_INDEX (hHighlight); 
		nlong = 0;
		CreateStatusWind (CurView->hWnd,1,0); 
		sprintf (str,"%ld items highlighted",hNum);
		StatusWindowUpdate ("Load Used Ref Table",str,hNum,nlong);
		{
						
		    pos = BT_FIRST;
			while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)pHighlightData)) 
			{   
				pos = BT_NEXT; 
				AddRefToUsedRefTable (Refno);
				StatusWindowUpdate (0,0,hNum,nlong++); 
			} 
		}
		CloseUsedRefTable (Opened);
		DestroyStatusWindow(0);  
		goto RtnTrue;
	}
	else if (!_fstricmp (Arg[1],"ADD"))
	{   
		FromRef = atol (Arg[2]);
		ToRef = atol (Arg[3]);
		 
		if (!AddRangeToUsedRefTable (FromRef,ToRef))
			goto RtnFalse;
	}
	else if (!_fstricmp (Arg[1],"CHECK"))
	{   
		if (!OpenUsedRefTable (FALSE,&Opened)) 
			goto RtnFalse;
		hNum = BT_NUM_IN_INDEX (hHighlight); 
		nlong = nl = 0;
		CreateStatusWind (CurView->hWnd,1,0); 
		sprintf (str,"%ld items highlighted",hNum);
		StatusWindowUpdate ("Check for Unique Reference Numbers",str,hNum,nlong);
		{
						
		    pos = BT_FIRST;
			while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)pHighlightData)) 
			{   
				pos = BT_NEXT; 
				if (RefInUsedRefTable (Refno))
					nl++;	
				StatusWindowUpdate (0,0,hNum,nlong++); 
			} 
		}
		CloseUsedRefTable (Opened);
		DestroyStatusWindow(0); 
		if (nl)
		{   
			sprintf (str,"%ld of %ld reference numbers are in use\r\nDo you wish to fix them?",nl,hNum);
			if (GSSiMessageBox (0,str,"Check Complete",MB_YESNO,0) == IDYES) 
			{   
				long	StartRefno=GetGlobalLVal2 ("[%STARTREFNO]",1000000);
                
                ltoa (StartRefno,str,10);
			  	if (!GetTextString (hWndMain,str,16,"Enter start reference number",0,0,0,TRUE,TRUE))
			  		goto RtnFalse;   
			  	StartRefno = atol (str);
				if (!OpenUsedRefTable (TRUE,&Opened)) 
					goto RtnFalse;
				hNum = BT_NUM_IN_INDEX (hHighlight); 
				nlong = 0;
				CreateStatusWind (CurView->hWnd,1,0); 
				sprintf (str,"%ld items highlighted",hNum);
				StatusWindowUpdate ("Making Reference Numbers Unique",str,hNum,nlong);
				{
				    pos = BT_FIRST;
					while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)pHighlightData)) 
					{   
						pos = BT_NEXT; 
						if (RefInUsedRefTable (Refno))
						{
							Refno = GetNextRefno (&StartRefno,TRUE,TRUE);
							PickList[0] = pHighlightData->PD;  
							ChangePickedItemRefno (0,Refno);
						}
						AddRefToUsedRefTable (Refno);
						StatusWindowUpdate (0,0,hNum,nlong++); 
					} 
				}
				CloseUsedRefTable (Opened);
				DestroyStatusWindow(0);  
			}
			goto RtnTrue;
		}
		else
		{
			if (GSSiMessageBox (0,"All references are unique.\r\nDo you wish to add them to the UsedRef Table?",
								"Check Complete",MB_YESNO,0) == IDYES)
				goto URTLoad;
		}
		goto RtnTrue;
	}
RtnFalse:
	GSSiGlobUlFree (&hStr); 
	return FALSE;
RtnTrue:
	GSSiGlobUlFree (&hStr); 
	return TRUE;
} 





double ConnectHighlightEnds (LPHIGHLIGHTDATA pHighlightData1,LPHIGHLIGHTDATA pHighlightData2,LPSHORT NearEnds)
{
	double d, mind;
	
	mind = ldistp (pHighlightData1->PD.BeginPoint,pHighlightData2->PD.BeginPoint);
	NearEnds[0] = NearEnds[1] = 1;
	if ((d = ldistp (pHighlightData1->PD.BeginPoint,pHighlightData2->PD.EndPoint)) < mind)
	{
		mind = d;
		NearEnds[1] = 2;
	}
	if ((d = ldistp (pHighlightData1->PD.EndPoint,pHighlightData2->PD.BeginPoint)) < mind)
	{
		mind = d;
		NearEnds[0] = 2;
		NearEnds[1] = 1;
	}
	if ((d = ldistp (pHighlightData1->PD.EndPoint,pHighlightData2->PD.EndPoint)) < mind)
	{
		mind = d;
		NearEnds[0] = 2;
		NearEnds[1] = 2;
	}
	return mind;
} 

BOOL ConvertTextPointers (HWND hWnd)
{
	long	TotLen = BT_NUM_IN_INDEX (hHighlight); 
	long	Processed = 0;
	HIGHLIGHTDATA	HighlightData1,HighlightData2;
	short	pos,i;     
	double	Mindist, MaxArrowHeadDist=5;
	double	d, LongLength;
	long	Refno=LONG_MIN, LongRef; 
	short	NearEnds[2], LongEnd, Pass;
	
	if (!TotLen)
		return FALSE;
// removes every third element from hightlight list		
	CreateStatusWind (hWnd,1,"Convert Text Pointers"); 
	while (!BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_GT,(LPSTR)&HighlightData1)) 
	{
        BT_DELETE (hHighlight,(LPSTR)&Refno,(LPSTR)&HighlightData1,FALSE); 
        Processed+=3;
		BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_GT,(LPSTR)&HighlightData2); 
		BT_FIND (hHighlight,(LPSTR)&Refno,BT_NEXT,BT_ANY,(LPSTR)&HighlightData2); 
		StatusWindowUpdate (0,0,TotLen,Processed); 
	} 
	DestroyStatusWindow(0);  
	return TRUE;	
} 

long FindAdjoiningArea (DPOINT Point,long nAreas,LPMNMXCORD pAreas)
{
	long	i;
	
	for (i=0;i<nAreas;i++)
	{
		if (DPointInBounds (&Point,&pAreas[i]))
			return (i+2);
	}
	return 0;
}



BOOL PickNearestCPTFile (DPOINT Point,LPSTR Filelist,LPSTR IgnoreName,double MaxDist,LPSTR OutLoc)
{   
	HFILE	FidFL, Fid;    
	HANDLE	hStr = GSSiGlobAlloc(GAIDNO 1566,GMEM_MOVEABLE,1024);
	LPSTR	str = GlobalLock (hStr);
	LPSTR	PathName = str + 256;
	LPSTR	FullPath = PathName + 256;
	LPSTR	Name = FullPath + 256;
	DPOINT	From, To;
	
	*OutLoc = 0;
	FidFL = GSSiOpenFile (Filelist,0,OF_READ);
	if (FidFL == HFILE_ERROR)
		return FALSE;
	while (fgetstring (PathName,256,FidFL))
	{
		_fullpath (FullPath,PathName,256);
		_splitpath (FullPath,0,0,Name,0);
		if (_fstricmp (Name,IgnoreName))
		{
			Fid = GSSiOpenFile (FullPath,0,OF_READ);
			if (Fid != HFILE_ERROR) 
			{
				while (fgetstring (str,256,Fid))
				{
					Truncate (str); 
					if (!*str || *str == '#')
						goto Next;
					ExpandText (str);
					if (sscanf (str,"%lf %lf %lf %lf",&From.x,&From.y,&To.x,&To.y) == 4)
					{
						if (ldistp (Point,To) < MaxDist)
						{
							MaxDist = ldistp (Point,To);
							_fstrcpy (OutLoc,Name);
						}
					} 
				Next:;
				}
				GSSiClose2 (&Fid);
			}
		}
	}
	GSSiClose2 (&FidFL);   
	GSSiGlobUlFree (&hStr);
	return TRUE;
}
 
BOOL CreateAdjoiningAreasFile (LPSTR OutFileName) 
{
	char	DefStr[]="AreaUDI(C32),LeftAreaUDI(C32),UpperLeftAreaUDI(C32),UpperAreaUDI(C32),UpperRightAreaUDI(C32),RightAreaUDI(C32),LowerRightAreaUDI(C32),LowerAreaUDI(C32),LowerLeftAreaUDI(C32)";
	short	NumFields=0, NumIndexFields=1;
	BOOL	st; 
	HANDLE	hDB;
	LPGWDHEADER lpGWDHead; 
	long	Refno;   
	long	nAreas=0, iArea, jArea;
   	HPMNMXCORD	pAreas; 
	HANDLE	hAreas, hAreaUDIs; 
	HPSTR	pAreaUDIs;
	long	TotLen = BT_NUM_IN_INDEX (hHighlight); 
	HIGHLIGHTDATA	HighlightData;      
	short	pos=BT_FIRST;
	
	if (!TotLen)
		return FALSE;
	if (!CreateGWDDatabase (OutFileName,1,FALSE,NumFields,NumIndexFields,DefStr))
		return FALSE;
				
    hDB = OpenGWDatabase (OutFileName,BT_WRITE);
    if (!hDB)
    	return FALSE;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
	hAreas = GSSiGlobAlloc(GAIDNO 1567,GMEM_MOVEABLE,TotLen * sizeof (MNMXCORD));
	pAreas = (HPMNMXCORD)GlobalLock (hAreas);
	hAreaUDIs = GSSiGlobAlloc(GAIDNO 1568,GMEM_MOVEABLE,TotLen * 32);  
	pAreaUDIs = (HPSTR)GlobalLock (hAreaUDIs);
	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
	{
		pos = BT_NEXT; 
		pAreas[nAreas] = HighlightData.PD.Rect;
		_fstrcpy (&pAreaUDIs[32L*nAreas++],HighlightData.PD.UDI);
	}
	for (iArea = 0;iArea<nAreas;iArea++)
	{   
		double	Width=pAreas[iArea].xmx - pAreas[iArea].xmn;
		double	Height=pAreas[iArea].ymx - pAreas[iArea].ymn;
		DPOINT	Point; 
		
		SetFieldValFromCharAndName(lpGWDHead, "AreaUDI", (LPSTR)&pAreaUDIs[iArea * 32], FALSE, TRUE);
		Point = MinMaxMidPointD (&pAreas[iArea]);
		Point.x -= Width;
		jArea = FindAdjoiningArea (Point,nAreas,pAreas);
		if (jArea)
			SetFieldValFromCharAndName(lpGWDHead, "LeftAreaUDI", (LPSTR)&pAreaUDIs[(jArea - 2) * 32], FALSE, TRUE);
	   	else
			SetFieldValFromCharAndName(lpGWDHead, "LeftAreaUDI", (LPSTR)"", FALSE, TRUE);
		Point.y += Height;
		jArea = FindAdjoiningArea (Point,nAreas,pAreas);  
		if (jArea)
			SetFieldValFromCharAndName(lpGWDHead, "UpperLeftAreaUDI", (LPSTR)&pAreaUDIs[(jArea - 2) * 32], FALSE, TRUE);
		else
			SetFieldValFromCharAndName(lpGWDHead, "UpperLeftAreaUDI", (LPSTR)"", FALSE, TRUE);
		Point.x += Width;
		jArea = FindAdjoiningArea (Point,nAreas,pAreas);   
		if (jArea)
			SetFieldValFromCharAndName(lpGWDHead, "UpperAreaUDI", (LPSTR)&pAreaUDIs[(jArea - 2) * 32], FALSE, TRUE);
		else
			SetFieldValFromCharAndName(lpGWDHead, "UpperAreaUDI", (LPSTR)"", FALSE, TRUE);
		Point.x += Width;
		jArea = FindAdjoiningArea (Point,nAreas,pAreas);
		if (jArea)
			SetFieldValFromCharAndName(lpGWDHead, "UpperRightAreaUDI", (LPSTR)&pAreaUDIs[(jArea - 2) * 32], FALSE, TRUE);
		else
			SetFieldValFromCharAndName(lpGWDHead, "UpperRightAreaUDI", (LPSTR)"", FALSE, TRUE);
		Point.y -= Height;
		jArea = FindAdjoiningArea (Point,nAreas,pAreas);
		if (jArea)
			SetFieldValFromCharAndName(lpGWDHead, "RightAreaUDI", (LPSTR)&pAreaUDIs[(jArea - 2) * 32], FALSE, TRUE);
	   	else
			SetFieldValFromCharAndName(lpGWDHead, "RightAreaUDI", (LPSTR)"", FALSE, TRUE);
		Point.y -= Height;
		jArea = FindAdjoiningArea (Point,nAreas,pAreas);
		if (jArea)
			SetFieldValFromCharAndName(lpGWDHead, "LowerRightAreaUDI", (LPSTR)&pAreaUDIs[(jArea - 2) * 32], FALSE, TRUE);
	   	else
			SetFieldValFromCharAndName(lpGWDHead, "LowerRightAreaUDI", (LPSTR)"", FALSE, TRUE);
		Point.x -= Width;
		jArea = FindAdjoiningArea (Point,nAreas,pAreas); 
		if (jArea)
			SetFieldValFromCharAndName(lpGWDHead, "LowerAreaUDI", (LPSTR)&pAreaUDIs[(jArea - 2) * 32], FALSE, TRUE);
	   	else
			SetFieldValFromCharAndName(lpGWDHead, "LowerAreaUDI", (LPSTR)"", FALSE, TRUE);
		Point.x -= Width;
		jArea = FindAdjoiningArea (Point,nAreas,pAreas);
		if (jArea)
			SetFieldValFromCharAndName(lpGWDHead, "LowerLeftAreaUDI", (LPSTR)&pAreaUDIs[(jArea - 2) * 32], FALSE, TRUE);
	   	else
			SetFieldValFromCharAndName(lpGWDHead, "LowerLeftAreaUDI", (LPSTR)"", FALSE, TRUE);
		st = GWDReplaceRecord (lpGWDHead,0,0,-1); 
	}
    GlobalUnlock (hDB); 
    CloseGWDatabase (hDB); 
	GSSiGlobUlFree (&hAreas);
	GSSiGlobUlFree (&hAreaUDIs);  
	return TRUE;
}  

long MaxFileRefno (LPSTR FileListFile)
{
	HFILE Fid = GSSiOpenFile (FileListFile,0,OF_READ);
	long	MaxRef = LONG_MIN;
	char	str[260];
	
	if (Fid == HFILE_ERROR)
		return 0;
	
	while (fgetstring (str,256,Fid))
	{ 
		if (OpenDGNFile (str,0)) 
		{
			MaxRef = max (MaxRef,DGNNumElements);
			CloseDGNFile ();
        }
	}
	GSSiClose2 (&Fid);
	return MaxRef;
}

BOOL CALLBACK EnumChildProc(HWND hCtrl,LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (427);
#endif
    {
    char        str[256];
    long	lUserData;
    POINT	Loc; 
    RECT	Rect;
    
    GetWindowText (hCtrl,str,200);  
    GetWindowRect(hCtrl,&Rect);
    if (!_fstricmp (str,"OK"))
    {   
    	SetFocus (hCtrl);  
    	GetWindowRect(hCtrl,&Rect);
    	Loc = RectMid (&Rect);   
    	SetCursorPos (Loc.x,Loc.y); 
    	Loc.x = (Rect.right-Rect.left)/2;
    	Loc.y = (Rect.bottom-Rect.top)/2;
	    PostMessage (hCtrl,WM_LBUTTONDOWN,1,MAKELONG(Loc.x,Loc.y)); 
	    PostMessage (hCtrl,WM_LBUTTONUP,0,MAKELONG(Loc.x,Loc.y)); 
{
#if ENABLETRACE
GSSiExitProg (427);
#endif
    	return FALSE;
}
    }
{
#if ENABLETRACE
GSSiExitProg (427);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
    }
BOOL CALLBACK EnumWndProc(HWND hCtrl,LONG lParam)
{
	char        str[130];
	long	lUserData; 
	HWND	hPar;
	    
	GetWindowText (hCtrl,str,128); 
	hPar = GetParent (hCtrl); 
	if (!_fstrnicmp (str,"MrSid Viewer",12))
		MrSidWnd = hCtrl; 
	return TRUE;
}

BOOL SplitMrSidFile (LPSTR File,LPSTR MrSidEx)
{   
	char	str[256], mess[128];
	DWORD	WVer;
	int		WinVer, DosVer;
	UINT	ierr;
	BOOL	rtn = TRUE;
    DLGPROC lpfnEnumWndProc;
    
    //GetShortPathName (MrSidEx,256);
    if (ExistFile(MrSidEx))
    {							      	 
		sprintf (str,"%s %s",MrSidEx,File); 
		if ((ierr = WinExec (str,SW_SHOW)) < 32)
		{  
			sprintf (mess,"Error loading MrSid: %i",(int) ierr);
			MessageBox (GetFocus(),mess,0,0);  
			return FALSE;
		}  
		MrSidWnd = 0;
		Wait (2000);
		lpfnEnumWndProc = MakeProcInstance((DLGPROC)EnumWndProc, hInst);
		EnumWindows ((WNDENUMPROC)lpfnEnumWndProc,0);   
		FreeProcInstance(lpfnEnumWndProc); 
		PostMessage (MrSidWnd, WM_CLOSE, 0, 0L);
    }
	return TRUE;
} 

BOOL GetDuplicateRecords (LPSTR OutFile)
{   
/*	int	st, BPConnect,EPConnect,SaveMaxPick;
	LPGWDHEADER lpGWDHead;
	BOOL	HaveMidHit;
	LPREFCONNECT	pRC;
	short	pos, pos2, len;  
	HCURSOR	hcurSave;
	POINT	Point; 
	char	mess[128];  
	struct {
		POINT	Point; 
		long	Ref;
		} ConnectKey;
   
	LPLINEINT	hpLineInt, hpLineIntStart, hpLineInt1, hpLineInt2; 
	HANDLE		hLineInt=0;
	long		maxlines, nHighlight;  
	long		nRecs, nLoaded=0;
	DWORD		il, jl, NumLines=0; 
	POINT		LastPoint;  
	LPTHEME		pTheme; 
	MNMXCORD	IntBounds;
	double		FixTol=GetGlobalDVal2("[%FIXTOL]",1); 
	static		long	debugref=1000006;
	short		ii;
	short		nBlocks=1; */  	
    char		File[256],str[128];
    BTVARDESC	Vars[5];
	long	Refno, Offset, NumDups=0, dupref, nRecs,nLoaded=0, debugref=0,ii;  
	HIGHLIGHTDATA	HighlightData;         
    short		pos=BT_FIRST;
    HFILE		FidOut; 
    ULONG		MSLink;
    
    FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
    if (FidOut == HFILE_ERROR)
    	return FALSE;
	GSSiGetTempFileName (0,"gmd",0,File);
	Vars[0].BT_VARLEN=4;
	Vars[0].BT_VARTYP=BT_INTEGER;
	Vars[0].BT_VAROFF=0;
	Vars[1].BT_VARLEN=4;
	Vars[1].BT_VARTYP=BT_INTEGER;
	Vars[1].BT_VAROFF=4;
	Vars[2].BT_VARLEN=4;
	Vars[2].BT_VARTYP=BT_INTEGER;
	Vars[2].BT_VAROFF=8;
	Vars[3].BT_VARLEN=4;
	Vars[3].BT_VARTYP=BT_INTEGER;
	Vars[3].BT_VAROFF=12;
	BT_CREATE (File, 8, FALSE, 4, 1,Vars,FALSE, 0, 0, FALSE);
	hBTDups = BT_OPEN (File, 0, BT_WRITE, 0);  
		
	//GSSiRemove ("[%CONSTATFILE].gmd");  

	nRecs = BT_NUM_IN_INDEX (hHighlight);
	CreateStatusWind (CurView->hWnd,1,"Locate Duplicate Records");
	fputstring ("REFNO,DUPREFNO,MSLNK,DUPMSLNK",FidOut);	
	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
	{   
		pos = BT_NEXT;
		if (Refno == debugref)
			ii=1; 
		if (!HighlightData.PD.Length)
			goto NextRec;  
		MSLink = HighlightData.PD.MSLink;
		if ((dupref=CheckForDupLines (Refno,PointToLPoint(HighlightData.PD.BeginPointFile),PointToLPoint(HighlightData.PD.EndPointFile),&MSLink)))
		{
			NumDups ++; 
			sprintf (str,"%ld,%ld,%ld,%ld",Refno,dupref,HighlightData.PD.MSLink,MSLink);
			fputstring (str,FidOut);	
		}
			
NextRec:
		StatusWindowUpdate (0,0, nRecs, ++nLoaded);

	} 
	DestroyStatusWindow(0); 
	GSSiClose2 (&FidOut);
	BT_CLOSEANDDELETE (&hBTDups);
	return NumDups;
}

BOOL GetDuplicatePoints (LPSTR OutFile,double Tol)
{   
    char		File[256],str[128],SymName1[64],SymName2[64];
    BTVARDESC	Vars[5];
	long	Refno, Offset, NumDups=0, dupref, nRecs,nLoaded=0, debugref=0,ii;  
	HIGHLIGHTDATA	HighlightData;         
    short		pos=BT_FIRST, PointPos, PointLoc;
    HFILE		FidOut; 
    long		iX, iY;
	DPOINT		MidPoint = MinMaxMidPointD (&HLTBounds);
	double		fac = Tol*2;
	DUPHEADER	Header, CurHeader;
	DUPDATA	DupData, CurDupData;
	BOOL	GetNext;

    FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
    if (FidOut == HFILE_ERROR)
    	return FALSE;
	GSSiGetTempFileName (0,"gmd",0,File);
	Vars[0].BT_VARLEN=4;
	Vars[0].BT_VARTYP=BT_INTEGER;
	Vars[0].BT_VAROFF=0;
	Vars[1].BT_VARLEN=4;
	Vars[1].BT_VARTYP=BT_INTEGER;
	Vars[1].BT_VAROFF=4;
	Vars[2].BT_VARLEN=4;
	Vars[2].BT_VARTYP=BT_INTEGER;
	Vars[2].BT_VAROFF=8;
	BT_CREATE (File, sizeof(DUPDATA), FALSE, 3, 1,Vars,FALSE, 0, 0, FALSE);
	hBTDups = BT_OPEN (File, 0, BT_WRITE, 0);  
		
	nRecs = BT_NUM_IN_INDEX (hHighlight);
	CreateStatusWind (CurView->hWnd,1,"Locate Duplicate Points");
	fputstring ("REFNO,DUPREFNO,SYM1,SYM2,PREFIX1,UDI1,PREFIX2,UDI2",FidOut);	
	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
	{   
		pos = BT_NEXT;
		if (Refno == debugref)
			ii=1; 
		if (SysTypeFromPickType (HighlightData.PD.Type) != GF_POINT)
			goto NextRec;  
		Header.iX = IDNINT((HighlightData.PD.BeginPoint.x - MidPoint.x) / fac);
		Header.iY = IDNINT((HighlightData.PD.BeginPoint.y - MidPoint.y) / fac);
		Header.Ref = Refno;
		CurHeader = Header;
		Header.Ref = LONG_MIN;
		GetNext = TRUE;
		PointPos = BT_FIRST;
		PointLoc = BT_GE;
		
		CurDupData.Point = HighlightData.PD.BeginPoint;
		CurDupData.iDesc = HighlightData.PD.Desc;
		strcpy (CurDupData.Prefix,HighlightData.PD.Prefix);
		strcpy (CurDupData.UDI,HighlightData.PD.UDI);
		while (GetNext && !BT_FIND (hBTDups,(LPSTR)&Header,PointPos,PointLoc,(LPSTR)&DupData)) 
		{
			PointPos = BT_NEXT;
			PointLoc = BT_ANY;
			if (CurHeader.iX != Header.iX)
				GetNext = FALSE;
			else if (CurHeader.iY == Header.iY)
			{
				if (ldistp (DupData.Point,HighlightData.PD.BeginPoint) <= Tol)
				{
					NumDups ++;
					GetDictSymName (HighlightData.PD.Desc,SymName1);
					GetDictSymName (DupData.iDesc,SymName2);
					sprintf (str,"%ld,%ld,%s,%s,%s,%s,%s,%s",Refno,Header.Ref,SymName1,SymName2,HighlightData.PD.Prefix,HighlightData.PD.UDI,DupData.Prefix,DupData.UDI);
					fputstring (str,FidOut);
				}
			}
		}
		BT_PUT (hBTDups,(LPSTR)&CurHeader,(LPSTR)&CurDupData);
			
NextRec:
		StatusWindowUpdate (0,0, nRecs, ++nLoaded);
	} 
	DestroyStatusWindow(0); 
	GSSiClose2 (&FidOut);
	BT_CLOSEANDDELETE (&hBTDups);
	return NumDups;
}

BOOL CreateNearestPointTable (LPSTR NPTable,LPSTR FromDB,LPSTR FromSQL,LPSTR FromPoint,LPSTR FromRef,LPSTR FromDesc,LPSTR FromPrefix,LPSTR FromUDI)
{
    BTVARDESC	Vars[2];
	HANDLE		hBT;
	HANDLE		hDB=0;
	NPHEADER	Header;
	DUPDATA		DupData;
	char		str[256];
	DPOINT		MidPoint, Point;
	BOOL		Err;
	double		fac;

    if (!OpenDataFile (FromDB,FromSQL,BT_READ,&hDB))
        return FALSE;
	Vars[0].BT_VARLEN=8;
	Vars[0].BT_VARTYP=BT_REAL;
	Vars[0].BT_VAROFF=0;
	Vars[1].BT_VARLEN=4;
	Vars[1].BT_VARTYP=BT_INTEGER;
	Vars[1].BT_VAROFF=8;
	BT_CREATE (NPTable, sizeof(DUPDATA), FALSE, 2, 1,Vars,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (NPTable, 0, BT_WRITE, 0);  
	while (FetchDBRec (hDB))
	{
		strcpy (str,FromPoint);
		ExpandText (str);
		Point = atopt (str,&Err);
		Header.XorY = Point.x;
		DupData.Point = Point;
		strcpy (str,FromRef);
		ExpandText (str);
		Header.Ref = atoi (str);
		strcpy (str,FromDesc);
		ExpandText (str);
		DupData.iDesc = atoi (str);
		strcpy (str,FromPrefix);
		ExpandText (str);
		strcpy (DupData.Prefix,str);
		strcpy (str,FromUDI);
		ExpandText (str);
		strcpy (DupData.UDI,str);
		BT_PUT (hBT,(LPSTR)&Header,(LPSTR)&DupData);
	}
    CloseDataFile (TRUE, &hDB);  
	BT_CLOSE (hBT);
	return TRUE;
}

BOOL GetNearestPointFromTable (LPSTR NPTable,LPDPOINT pPoint,int idesc,LPDPOINT pFoundPoint,LPLONG pFoundRef,LPSTR FoundPrefix,LPSTR FoundUDI)
{   
	NPHEADER	Header, FirstHeader;
	DUPDATA		DupData, NearestDup;
	double		dist, NearestDist = DBL_MAX;
	int			st, stfirst, NearestRef=0;
	HANDLE		hBT;

	hBT = BT_OPEN (NPTable, 0, BT_READ, 0);  
	if (!hBT)
		return FALSE;
	if (!BT_NUM_IN_INDEX (hBT))
	{
		BT_CLOSE (hBT);
		return FALSE;
	}
	Header.Ref = LONG_MIN;
	Header.XorY = pPoint->x;
	stfirst = BT_FIND (hBT,(LPSTR)&FirstHeader,BT_FIRST,BT_GE,(LPSTR)&NearestDup);
	if (!stfirst)
	{
		NearestRef = FirstHeader.Ref;
		NearestDist = ldistp (*pPoint,NearestDup.Point);
		do
		{
			if (!(st = BT_FIND (hBT,(LPSTR)&Header,BT_NEXT,BT_ANY,(LPSTR)&DupData)))
			{
				dist = ldistp (*pPoint,DupData.Point);
				if (dist < NearestDist)
				{
					NearestDist = dist;
					NearestDup = DupData;
					NearestRef = Header.Ref;
				}
			}
		}
		while (!st && (Header.XorY -  pPoint->x) < NearestDist);
	}
	else
		NearestDist = DBL_MAX;
	if (!stfirst)
	{
		st = BT_FIND (hBT,(LPSTR)&FirstHeader,BT_FIRST,BT_EQ,(LPSTR)&DupData);
		st = BT_FIND (hBT,(LPSTR)&Header,BT_PRIOR,BT_ANY,(LPSTR)&DupData);
	}
	else
		st = BT_FIND (hBT,(LPSTR)&Header,BT_LAST,BT_ANY,(LPSTR)&DupData);
	if (!st && (pPoint->x - Header.XorY) < NearestDist)
	{
		do
		{
			dist = ldistp (*pPoint,DupData.Point);
			if (dist < NearestDist)
			{
				NearestDist = dist;
				NearestDup = DupData;
				NearestRef = Header.Ref;
			}
			st = BT_FIND (hBT,(LPSTR)&Header,BT_PRIOR,BT_ANY,(LPSTR)&DupData);
		}
		while (!st && (pPoint->x - Header.XorY) < NearestDist);
	}
	BT_CLOSE (hBT);
	if (pFoundPoint)
		*pFoundPoint = NearestDup.Point;
	if (pFoundRef)
		*pFoundRef = NearestRef;
	if (FoundPrefix)
		strcpy (FoundPrefix,NearestDup.Prefix);
	if (FoundUDI)
		strcpy (FoundUDI,NearestDup.UDI);
	return TRUE;
}

BOOL GetDuplicateLines (LPSTR OutFile,double Tol)
{   
    char		File[256],str[128],SymName1[64],SymName2[64];
    BTVARDESC	Vars[5];
	long	Refno, Offset, NumDups=0, dupref, nRecs,nLoaded=0, debugref=0,ii;  
	HIGHLIGHTDATA	HighlightData;         
    short		pos=BT_FIRST, PointPos, PointLoc;
    HFILE		FidOut; 
    long		iX, iY;
	DPOINT		MidPoint = MinMaxMidPointD (&HLTBounds), Point;
	double		fac = Tol*2;
	typedef struct	{long	iX,iY,Ref;} DUPHEADER;
	DUPHEADER	Header, CurHeader;
	typedef	struct	{DPOINT Point, OpPoint;
					 int	nPoints;
					 short	iDesc;
					 char	Prefix[8],UDI[64];
					} DUPDATA;
	DUPDATA	DupData, CurDupData;
	BOOL	GetNext;

    FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
    if (FidOut == HFILE_ERROR)
    	return FALSE;
	GSSiGetTempFileName (0,"gmd",0,File);
	Vars[0].BT_VARLEN=4;
	Vars[0].BT_VARTYP=BT_INTEGER;
	Vars[0].BT_VAROFF=0;
	Vars[1].BT_VARLEN=4;
	Vars[1].BT_VARTYP=BT_INTEGER;
	Vars[1].BT_VAROFF=4;
	Vars[2].BT_VARLEN=4;
	Vars[2].BT_VARTYP=BT_INTEGER;
	Vars[2].BT_VAROFF=4;
	BT_CREATE (File, sizeof(DUPDATA), FALSE, 3, 1,Vars,FALSE, 0, 0, FALSE);
	hBTDups = BT_OPEN (File, 0, BT_WRITE, 0);  
		
	nRecs = BT_NUM_IN_INDEX (hHighlight);
	CreateStatusWind (CurView->hWnd,1,"Locate Duplicate Lines");
	fputstring ("REFNO,DUPREFNO,SYM1,SYM2,PREFIX1,UDI1,PREFIX2,UDI2",FidOut);	
	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
	{   
		pos = BT_NEXT;
		if (Refno == debugref)
			ii=1; 
		if (SysTypeFromPickType (HighlightData.PD.Type) != GF_POLYLINE)
			goto NextRec;  
		Header.iX = IDNINT((HighlightData.PD.BeginPoint.x - MidPoint.x) / fac);
		Header.iY = IDNINT((HighlightData.PD.BeginPoint.y - MidPoint.y) / fac);
		Header.Ref = Refno;
		CurHeader = Header;
		Header.Ref = LONG_MIN;
		GetNext = TRUE;
		PointPos = BT_FIRST;
		PointLoc = BT_GE;
		
		CurDupData.Point = HighlightData.PD.BeginPoint;
		CurDupData.OpPoint = HighlightData.PD.EndPoint;
		CurDupData.nPoints = HighlightData.PD.NumPoints;
		CurDupData.iDesc = HighlightData.PD.Desc;
		strcpy (CurDupData.Prefix,HighlightData.PD.Prefix);
		strcpy (CurDupData.UDI,HighlightData.PD.UDI);
		while (GetNext && !BT_FIND (hBTDups,(LPSTR)&Header,PointPos,PointLoc,(LPSTR)&DupData)) 
		{
			PointPos = BT_NEXT;
			PointLoc = BT_ANY;
			if (CurHeader.iX != Header.iX)
				GetNext = FALSE;
			else if (CurHeader.iY == Header.iY)
			{
				if (ldistp (DupData.Point,HighlightData.PD.BeginPoint) <= Tol && ldistp (DupData.OpPoint,HighlightData.PD.EndPoint) <= Tol)
				{
					NumDups ++;
					GetDictSymName (HighlightData.PD.Desc,SymName1);
					GetDictSymName (DupData.iDesc,SymName2);
					sprintf (str,"%ld,%ld,%s,%s,%s,%s,%s,%s",Refno,Header.Ref,SymName1,SymName2,HighlightData.PD.Prefix,HighlightData.PD.UDI,DupData.Prefix,DupData.UDI);
					fputstring (str,FidOut);
				}
			}
		}
		BT_PUT (hBTDups,(LPSTR)&CurHeader,(LPSTR)&CurDupData);
		Header.iX = IDNINT((CurDupData.OpPoint.x - MidPoint.x) / fac);
		Header.iY = IDNINT((CurDupData.OpPoint.y - MidPoint.y) / fac);
		Point = CurDupData.OpPoint;
		CurDupData.OpPoint = CurDupData.Point;
		CurDupData.Point = Point;
		BT_PUT (hBTDups,(LPSTR)&Header,(LPSTR)&CurDupData);
NextRec:
		StatusWindowUpdate (0,0, nRecs, ++nLoaded);
	} 
	DestroyStatusWindow(0); 
	GSSiClose2 (&FidOut);
	BT_CLOSEANDDELETE (&hBTDups);
	return NumDups;
}
BOOL LinesToPoints (LPSTR OutFile,double SampleDist)
{ 
    HIGHLIGHTDATA	HighlightData;   
    short			ipos=BT_FIRST;    
    long			Refno, nPnts; 
    HANDLE			hPoints;
    HPDPOINT3D		lpPoints; 
    DPOINT3D		Point; 
    double			LineLength, DistInc, AtDist;
	HFILE			Fid;   
	char			str[256];
	long			nNodePoints,i;
	
	Fid = GSSiOpenFile (OutFile,0,OF_CREATE);
	if (Fid == HFILE_ERROR)
		return FALSE;
	fputstring ("X,Y,Z",Fid);
   	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,ipos,BT_ANY,(LPSTR)&HighlightData)) 
   	{   
   		ipos = BT_NEXT;
        PickList[0]=HighlightData.PD;
		if (GetPolyPoints3D ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPoints))
		{   
			lpPoints = (HPDPOINT3D)GlobalLock (hPoints);
			LineLength = GetPoly3DLength2D (lpPoints,nPnts); 
			sprintf (str,"%f,%f,%f",lpPoints->x*MFT,lpPoints->y*MFT,lpPoints->z);
			fputstring (str,Fid); 
			if (LineLength > P_TOL)
			{    
				nNodePoints = (LineLength-P_TOL)/SampleDist; 
				AtDist = DistInc = LineLength / (nNodePoints+1); 
				for (i=0;i<nNodePoints;i++)
				{
					Point = PointAtDistOnPoly3D (lpPoints,nPnts,AtDist,0,0);   
					sprintf (str,"%f,%f,%f",Point.x*MFT,Point.y*MFT,Point.z);
					fputstring (str,Fid);  
					AtDist += DistInc;
				}
				sprintf (str,"%f,%f,%f",lpPoints[nPnts-1].x*MFT,lpPoints[nPnts-1].y*MFT,lpPoints[nPnts-1].z);
				fputstring (str,Fid);   
			}
			GSSiGlobUlFree (&hPoints);
		}
	}
	GSSiClose2 (&Fid);
	return TRUE;
}

BOOL AreasToLines (HWND hWnd,LPSTR OutFile,LPSTR LineSym,LPSTR Prefix)
{ 
    HIGHLIGHTDATA	HighlightData;   
    short			ipos=BT_FIRST;    
    long			Refno, nPnts; 
    HANDLE			hPoints;
    HPDPOINT3D		lpPoints; 
    DPOINT3D		Point; 
    double			LineLength, DistInc, AtDist;
	HFILE			Fid;   
	char			str[256];
	long			nNodePoints,i;
	short	SymNum;
	short	NumSyms=0; 
	HANDLE	hSymDesc=0; 
	int		nPoints=0;  
	BOOL	rtn=FALSE;
	int		np;
	
    SymNum = GetOrCreateSym (hWnd,LineSym,&NumSyms,&hSymDesc,FALSE,0); 
    if (!SymNum)
    	return FALSE;
	_fstrcpy (PltName,OutFile);
	PltType = 2;
	if (!OpenMap (CurView->hWnd,0))
		return FALSE;
	EditBounds = CurView->FileMNMX; 
 	CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,0); 
  	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,ipos,BT_ANY,(LPSTR)&HighlightData)) 
   	{   
   		ipos = BT_NEXT;
        PickList[0]=HighlightData.PD;
		if (PickList[0].Type != 3)
			continue;
		GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&np,&hPoints, 0);
		nPoints = np;
		_fstrcpy (PltName,OutFile);
		PltType = 2;
		if (OpenMap (CurView->hWnd,0))
		{
			Refno = GetNewRefno (PltName,0,0,0,0);
			rtn = AddPolyToMap (1,&nPoints, &hPoints,1,Refno,0,-1,SymNum,0,Prefix,PickList[0].UDI,-1,-1,-1,0,0,0,0,TRUE,0);
		}
       	GSSiGlobFree (&hPoints);
	}
	DestroySymList (&NumSyms,&hSymDesc);
	return rtn;
}

BOOL FilterTextFile (LPSTR InFile,LPSTR OutFile,MNMXCORD Bounds)  
{
	HFILE	FidIn = GSSiOpenFile (InFile,0,OF_READ);
	HFILE	FidOut = GSSiOpenFile (OutFile,0,OF_CREATE); 
	char	str[256],strorig[256];
	HANDLE	TxtHandle;  
	double	X,Y,Z;     
	long	TotLen,CurLoc,lineno;  
	DPOINT	Point;
	 
	TotLen = GSSillseek (FidIn,0,2);
	GSSillseek (FidIn,0,0);
	ProcessDelimTextHeader(str, InFile, FidIn, &TxtHandle, 0, 0);
	fputstring (str,FidOut); 
	CreateStatusWind (hWndMain,1,"Filtering Points"); 
	lineno = 1;
	while (ContinueProcessing && fgetstring (str,64,FidIn))
	{   
		lineno++;   
		_fstrcpy (strorig,str);
		GetDelimTextData(str,TxtHandle,64);
		X = GetGlobalDVal ("[X]")*FTM;
		Y = GetGlobalDVal ("[Y]")*FTM;
		Z = GetGlobalDVal ("[Z]")*FTM;    
		Point.x = X;
		Point.y = Y;
		if (PointInBounds (Point,&Bounds))
			fputstring (strorig,FidOut);
		CurLoc = GSSillseek (FidIn,0,1);
		StatusWindowUpdate (0,0, TotLen, CurLoc); 
    }  
    SetContinueProcessing ( TRUE); 
    GSSiClose2 (&FidIn);   
    GSSiClose2 (&FidOut);   
    GSSiGlobFree (&TxtHandle);
	DestroyStatusWindow (0);  
	return TRUE;
}

BOOL RecoverPltFile (LPSTR Name,LPSTR OutName)
{
	BOOL	rtn=FALSE;
	
	HFILE Fid=GSSiOpenFile (Name,0,OF_READ);
	long	len=GSSifilelength (Fid);
	HANDLE	hMem = GSSiGlobAlloc(GAIDNO 1569,GMEM_MOVEABLE,len);
	LPSHORT	pFile=GlobalLock (hMem);
	int		lrec,i,nfound=0;

	BigRead (Fid,pFile,len);

	len /= 2;
	for (i=0;i<len;i++)
	{
		LPBYTE pCode = (LPBYTE)&pFile[i];
		LPSHORT	startpnt,ipnt=startpnt=(LPSHORT)pCode;

		if (*pCode == 12 || *pCode == 92 || *pCode == 93)
		{
			ipnt++;
			while (SkipSubRec (pCode,(HPSHORT*)&ipnt,TRUE))
			{
				 pCode = (LPBYTE)ipnt;
				 ipnt++;
				 if (*pCode == 13)
				 {
					 nfound++;
					 lrec = (long)ipnt - (long)startpnt;
					 memset (startpnt,0,lrec);
					 goto NextRec;
				 }
			}
		}
NextRec:;
	}
	GSSiGlobUlFree (&hMem);
	GSSiClose2 (&Fid);
	return rtn;
}

BOOL GetProfileAreaPoints (double x,LPSTR SymName, LPSTR VPName,LPDPOINT Points)
{
	BOOL	Err;
	VISLIST	SaveVis;
	short	idesc=0;
	long	Refno;
	int		nAreas;
	HIGHLIGHTDATA	HighlightData;
	short		pos=BT_FIRST;
	HANDLE	hPoly;
	long	npnts;
	short	type[2]; 
	DPOINT	PickPointBase, IntPoint, ProfPoints[2];
	BOOL	rtn=FALSE;
	MNMXCORD	WBounds;

	if (*SymName)
	{
		idesc = GetSymbolNum (SymName); 
		if (!idesc)
			return FALSE;
	}
	SetCurView (SetVPFromName (VPName,&Err));
	if (Err)
		return FALSE;

	GetVisBounds (&WBounds,CurView->hDC); 
	ProfPoints[0].x = x;
	ProfPoints[1].x = x;
	ProfPoints[0].y = WBounds.ymn;
	ProfPoints[1].y = WBounds.ymx;
	ExtendPoly (2,ProfPoints,10);
	SelectVisList (2);
	SaveVis = *CurVis;
	InitVis (); 
	if (idesc)
	{
		_fmemset (CurVis->VisBits,0,sizeof(CurVis->VisBits));
		ToggleVisibility (idesc);
	}
    ClearHighlightList (FALSE);  
	nAreas = HighlightInArea (CurView->hWnd,&WBounds,TRUE,FALSE,0);
   	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
   	{
   		pos=BT_NEXT;
		if ((!idesc || HighlightData.PD.Desc == idesc) && x >= HighlightData.PD.Rect.xmn && x<= HighlightData.PD.Rect.xmx)
		{
			if (GetPolyPnts ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts,&hPoly,TRUE))
			{
				HPDPOINT	pPolyPoints=GlobalLock (hPoly);
				double		D1, D2;
				int	na = IntersectPolys1 (GF_LINE,GF_AREA,2,ProfPoints,0,npnts,pPolyPoints,0,0,&ProfPoints[0],&IntPoint,&D1,&D2,0);
				
				if (na)
				{
					rtn = TRUE;
					Points[0] = IntPoint;
					IntersectPolys1 (GF_LINE,GF_AREA,2,ProfPoints,0,npnts,pPolyPoints,0,0,&ProfPoints[1],&IntPoint,&D1,&D2,0);
					Points[1] = IntPoint;
				}
				GSSiGlobUlFree (&hPoly);
			}
		}
	}
	*CurVis = SaveVis;
	return rtn;
}

BOOL MovePointItem (long Refno,DPOINT NewPoint)
{
	LPVIEWPORT	SaveVP = CurView;
	BOOL	rtn=FALSE;

	NewPointD = NewPoint;
	if (!PickByRefno(Refno,0,0,-101))
		return FALSE;
	SetConfig (PickList[0].ConfigID);
	SetViewport (PickList[0].ViewID);
	if (!CurView->UpdateFile)
	{
		CurView = SaveVP;
		return FALSE;
	}
	UpdateItem=20;
	rtn = UpdateRecord (0,PickList[0].Desc,0,0,0,0,1,0);
	CurView = SaveVP;
	return rtn;
}

int GetOpenFileChecksum2 (HFILE Fid,int frombyte,int tobyte)
{
	int		checksum=0;
	int		cl;
	LPBYTE	prec;
	HANDLE	hMem;

	if (Fid == HFILE_ERROR)
		return 0;

	cl = _llseek (Fid,0,2);
	if (!frombyte && !tobyte)
	{
		hMem = GSSiGlobAlloc(GAIDNO 1916,GMEM_MOVEABLE,cl);
		prec = GlobalLock (hMem);
		_lread (Fid,prec,cl);
		checksum = ComputeCheckSum (prec,cl);
		GSSiGlobUlFree (&hMem);
	}
	else if (tobyte)
	{
		if (frombyte < cl && frombyte <= tobyte)
		{
			cl = min (cl,tobyte-frombyte+1);
			hMem = GSSiGlobAlloc(GAIDNO 1917,GMEM_MOVEABLE,cl);
			prec = GlobalLock (hMem);
			_llseek (Fid,frombyte,0);
			_lread (Fid,prec,cl);
			checksum = ComputeCheckSum (prec,cl);
			GSSiGlobUlFree (&hMem);
		}
	}
	else if (frombyte < 0)
	{
		cl = min (cl,-frombyte);
		frombyte = _llseek (Fid,-cl,2);
		hMem = GSSiGlobAlloc(GAIDNO 1918,GMEM_MOVEABLE,cl);
		prec = GlobalLock (hMem);
		_lread (Fid,prec,cl);
		checksum = ComputeCheckSum (prec,cl);
		GSSiGlobUlFree (&hMem);
	}
	return checksum;
}

int GetOpenFileChecksum (HFILE Fid,int frombyte,int tobyte)
{
	int		checksum=0;
	int		cl;
	LPBYTE	prec;
	HANDLE	hMem;

	if (Fid == HFILE_ERROR)
		return 0;

	cl = GSSifilelength (Fid);
	if (!frombyte && !tobyte)
	{
		hMem = GSSiGlobAlloc(GAIDNO 1919,GMEM_MOVEABLE,cl);
		prec = GlobalLock (hMem);
		BigRead (Fid,prec,cl);
		checksum = ComputeCheckSum (prec,cl);
		GSSiGlobUlFree (&hMem);
	}
	else if (tobyte)
	{
		if (frombyte < cl && frombyte <= tobyte)
		{
			cl = min (cl,tobyte-frombyte+1);
			hMem = GSSiGlobAlloc(GAIDNO 1920,GMEM_MOVEABLE,cl);
			prec = GlobalLock (hMem);
			GSSillseek (Fid,frombyte,0);
			BigRead (Fid,prec,cl);
			checksum = ComputeCheckSum (prec,cl);
			GSSiGlobUlFree (&hMem);
		}
	}
	else if (frombyte < 0)
	{
		cl = min (cl,-frombyte);
		frombyte = GSSillseek (Fid,-cl,2);
		hMem = GSSiGlobAlloc(GAIDNO 1908,GMEM_MOVEABLE,cl);
		prec = GlobalLock (hMem);
		BigRead (Fid,prec,cl);
		checksum = ComputeCheckSum (prec,cl);
		GSSiGlobUlFree (&hMem);
	}
	return checksum;
}

int GetFileChecksum (LPSTR File,int frombyte,int tobyte)
{
	HFILE	Fid = GSSiOpenFile (File,0,OF_READ);
	int		checksum = GetOpenFileChecksum (Fid,frombyte,tobyte);

	GSSiClose2 (&Fid);

	return checksum;
}

int GetFileList (LPSTR OutFile,BOOL New,LPSTR SearchLoc,LPSTR WildCard,BOOL SearchSubdir,BOOL WantDirectories,int nameOnly)
{
     int	Num,i, st;
     char	str2[_MAX_PATH+80],TempName[_MAX_FNAME],Name[_MAX_FNAME], drive[_MAX_DRIVE], dir[_MAX_DIR], extension[_MAX_EXT];
	 char	GMDFile[MAX_PATH] = { 0 };
     LPSTR	lpBrack, lpDot; 
     HFILE	OutFileFID, Fid;  
     long	TotFiles=0;
     HCURSOR	hcurSave; 
	 HANDLE	hStr=GSSiGlobAlloc(GAIDNO 1908,GMEM_MOVEABLE,4096);
	 LPSTR	str = GlobalLock (hStr);
	 BOOL isSLTFile = FALSE;
	 BOOL dropSearchLoc = FALSE;
	 sqlite3 *db;

	 if (!OutFile || !*OutFile) //just return num hits
		 OutFileFID = HFILE_ERROR;
	 else
	 {
		 LPSTR  pDot = strrchr(OutFile, '.');
		 if (!stricmp(pDot, ".gmd"))
		 {
			 char	DefStr[] = "FULLNAME(C255)\tFILENAME(C128)\tDRIVE(C8)\tDIRECTORY(C255)\tLASTDIR(C255)\tDRIVEDIR(C255)\tEXTENSION(C16)\tCREATTIME(B4)\tLASTACCESS(B4)\tLASTWRITE(B4)\tFILELENGTH(B4)\tSTATUS(B4)";
			 strcpy(GMDFile, OutFile);
			 GSSiGetTempFileName(0, "gm", 0, OutFile);
			 pDot = strrchr(OutFile, '.');
			 strcpy(pDot, ".txt");
			 OutFileFID = GSSiOpenFile(OutFile, 0, OF_CREATE);
			 nameOnly = 0;
			 if (OutFileFID == HFILE_ERROR)
				 goto Exit;

			 fputstring(DefStr, OutFileFID);
		 }
		 else if (!stricmp(pDot, ".slt"))
		 {
			 char	DefStr[] = "CREATE TABLE FILELIST (FULLNAME CHAR(255) PRIMARY KEY,FILENAME CHAR(128),DRIVE CHAR(8),DIRECTORY CHAR(255),LASTDIR CHAR(255),DRIVEDIR CHAR(255),EXTENSION CHAR(16),CREATTIME INT,LASTACCESS INT,LASTWRITE INT,FILELENGTH INT,STATUS INT)";
			 GSSiRemove(OutFile);
			 isSLTFile = TRUE;
			 if (nameOnly == 3)
				 dropSearchLoc = TRUE;
			 nameOnly = 0;

			 st = sqlite3_open(OutFile, &db);

			 if (st)
				 goto Exit;

			 SLT_StartTrans(db);
			 SLT_Execute(DefStr, db);

		 }
		 else if (New || !ExistFile(OutFile))
		 {
			 OutFileFID = GSSiOpenFile(OutFile, 0, OF_CREATE);
			 if (OutFileFID == HFILE_ERROR)
				 goto Exit;
			 //fputstring ("FULLNAME\tFILENAME\tDRIVE\tDIRECTORY\tLASTDIR\tDRIVEDIR\tEXTENSION\tSTATUS\tLASTUPDATE\tSIZE",OutFileFID);
			 if (!nameOnly)
				 fputstring("FULLNAME(C255)\tFILENAME(C128)\tDRIVE(C8)\tDIRECTORY(C255)\tLASTDIR(C255)\tDRIVEDIR(C255)\tEXTENSION(C16)\tCREATTIME(B4)\tLASTACCESS(B4)\tLASTWRITE(B4)\tFILELENGTH(B4)\tSTATUS(B4)", OutFileFID);
		 }
		 else
		 {
			 OutFileFID = GSSiOpenFile(OutFile, 0, OF_READWRITE);
			 if (OutFileFID == HFILE_ERROR)
				 goto Exit;
			 GSSillseek(OutFileFID, 0, 2);
		 }
	 }
	 GSSiGetTempFileName(0,"gm",0,TempName);
 	 Fid =	GSSiOpenFile (TempName,0,OF_CREATE);
     SearchFilesInDir (SearchLoc, "", Fid,&TotFiles,WildCard,1,SearchSubdir,FALSE); 
	 
	 int lSearchLoc = strlen(SearchLoc);
	 GSSillseek (Fid,0,0);
	 while (fgetstring (str2,_MAX_PATH+80-2,Fid))
	 { 
		LPSTR	pLastDir;
		char	LastDir[_MAX_DIR];
		//LONGLONG	lfile=0;
		//DWORD	lastup=0;
	    struct _stati64    fstat; 
		HFILE	Fid2;
		char	timesAndLength[256]={0};
	    LPSTR pTAB = strchr (str2,'\t');
		LPSTR pFullName = str2;

		if (pTAB)
			*pTAB++ = 0;

		if (pTAB)
			strcpy (timesAndLength,pTAB);
		/*if ((Fid2 = GSSiOpenFile (str2,0,OF_READ)) != HFILE_ERROR)
		{
			GSSifstat (Fid2,&fstat);
			lastup = fstat.st_mtime;
			lfile = fstat.st_size;
			GSSiClose2 (&Fid2);
		}*/
		ReplaceQuoteWithTwoQuotes(str2, sizeof(str2));
		_splitpath (str2,drive,dir,Name,extension);
		strcpy (LastDir,dir);
		if (*LastChr (LastDir) == '\\')
			*LastChr (LastDir) = 0;
		pLastDir=_fstrrchr (LastDir,'\\');
		if (pLastDir)
			pLastDir++;
		else
			pLastDir = _fstrchr (LastDir,0);
		//sprintf (str,"%s\t%s\t%s\t%s\t%s\t%s\t%ld\t%ld",str2,Name,drive,dir,pLastDir,extension,lastup,lfile);
		if (dropSearchLoc)
			pFullName += lSearchLoc;
		if (nameOnly == 1)
		{
			sprintf(str, "%s%s", Name, extension);
			fputstring(str, OutFileFID);
		}
		else if (nameOnly == 2)
		{
			sprintf(str, "%s", str2);
			fputstring(str, OutFileFID);
		}
		else if (isSLTFile)
		{
			REPLAC(timesAndLength, "\t", ",", sizeof(timesAndLength) - 1);
			sprintf(str, "INSERT INTO FILELIST VALUES ('%s','%s','%s','%s','%s','%s%s','%s',%s,0)", pFullName, Name, drive, dir, pLastDir, drive, dir, extension, timesAndLength);
			strlwr(str);
			SLT_Execute(str, db);
		}
		else
		{
			sprintf(str, "%s\t%s\t%s\t%s\t%s\t%s%s\t%s\t%s\t", str2, Name, drive, dir, pLastDir, drive, dir, extension, timesAndLength);
			if (*GMDFile)
				strlwr(str);
			fputstring(str, OutFileFID);
		}
	 } 
	 GSSiClose2(&Fid);
	 GSSiRemove(TempName);
	 if (isSLTFile)
	 {
		 SLT_EndTrans(db);
		 sqlite3_close(db);
	 }
	 else
	 {
		 GSSiClose2(&OutFileFID);
		 if (*GMDFile)
		 {
			 char cmd[1024];
			 sprintf(cmd, "$GMDIMPORT(%s,N,%s,,FILENAME)", GMDFile, OutFile);
			 ExpandText(cmd);
			 GSSiRemove(OutFile);
		 }
	 }
Exit:
	 GSSiGlobUlFree (&hStr);
	 return TotFiles;
}

BOOL FAR PASCAL MESSAGE_INCOMINGMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{

 int	BRtn; 
 HANDLE	hResponse;
 RECT	rect2;
 static	RECT	rect1;
 LPSTR	pStr;
 POINT	pt;
 
  if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
   switch (Message)
      {
   case WM_INITDIALOG: 
	    GetWindowRect(hWndDlg,&rect1);
	    GetWindowRect(GetDlgItem (hWndDlg,IDC_MESSAGEREPLY),&rect2);
		pt.x = rect2.left;
		pt.y = rect2.top;
		ScreenToClient (hWndDlg,&pt);  
		SetWindowPos(hWndDlg, HWND_TOP,0, 0, RECTWIDTH(&rect1),abs(rect2.top-rect1.top), SWP_NOMOVE);
        cwCenter(hWndDlg,0);
	    GetWindowRect(GetDlgItem (hWndDlg,IDC_REPLY),&rect2);
     	SetCursorPos (rect2.left,rect2.top);

		break;
   case WM_DESTROY:
 		hResponse = (HANDLE)GetWindowLong (hWndDlg,GWL_USERDATA);
		GSSiGlobFree (&hResponse);
       break; 
   case WM_CLOSE:
	   PostMessage (hWndDlg, WM_COMMAND, IDCANCEL, 0L);
 	   break;
   case WM_COMMAND:
      switch (LOWORD(wParam))
         {
          case IDC_EXIT: 
	      case IDCANCEL:
		      DestroyWindow(hWndDlg); 
	          break;
		  case IDC_REPLY:
			  {
				  char	wintxt[32];

				  GetWindowText (GetDlgItem (hWndDlg,IDC_REPLY),wintxt,32);
				  if (!strcmp (wintxt,"Send Reply"))
				  {
						SetGlobalFromTextBox (hWndDlg,IDC_MESSAGEREPLY,"%REPLY",TRUE);
						hResponse = (HANDLE)GetWindowLong (hWndDlg,GWL_USERDATA);
						pStr = GlobalLock (hResponse);
						ProcessText (pStr);
						GlobalUnlock (hResponse);
						PostMessage (hWndDlg, WM_COMMAND, IDCANCEL, 0L);
				  }
				  else
				  {
						SetWindowPos(hWndDlg, HWND_TOP,0, 0,RECTWIDTH(&rect1),RECTHEIGHT(&rect1), SWP_NOMOVE);
						ShowWindow (GetDlgItem (hWndDlg,IDC_MESSAGEREPLY),SW_SHOW);
						SetWindowText (GetDlgItem (hWndDlg,IDC_REPLY),"Send Reply");
						SetFocus (GetDlgItem(hWndDlg,IDC_MESSAGEREPLY));

				  }
			  }
			  break;
	      default:
	         return FALSE;
	     } 
      break;

   default:
      return FALSE;
      }
   return TRUE;
}

BOOL FAR PASCAL MESSAGE_OUTGOINGMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{

 int	BRtn; 
 HANDLE	hResponse;
 RECT	rect2;
 LPSTR	pStr;
 POINT	pt;
 
  if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
   switch (Message)
      {
   case WM_INITDIALOG: 
        cwCenter(hWndDlg,0);
	    SetFocus (GetDlgItem(hWndDlg,IDC_MESSAGE));
	    GetWindowRect(GetDlgItem (hWndDlg,IDC_MESSAGE),&rect2);
     	SetCursorPos (rect2.left,rect2.top);
		return FALSE;
		break;
   case WM_DESTROY:
 		hResponse = (HANDLE)GetWindowLong (hWndDlg,GWL_USERDATA);
		GSSiGlobFree (&hResponse);
       break; 
   case WM_CLOSE:
	   PostMessage (hWndDlg, WM_COMMAND, IDCANCEL, 0L);
 	   break;
   case WM_COMMAND:
      switch (LOWORD(wParam))
         {
          case IDC_EXIT: 
	      case IDCANCEL:
		      DestroyWindow(hWndDlg); 
	          break;
		  case IDOK:
			  {
					SetGlobalFromTextBox (hWndDlg,IDC_MESSAGE,"%MESSAGE",TRUE);
					hResponse = (HANDLE)GetWindowLong (hWndDlg,GWL_USERDATA);
					pStr = GlobalLock (hResponse);
					ProcessText (pStr);
					GlobalUnlock (hResponse);
					PostMessage (hWndDlg, WM_COMMAND, IDCANCEL, 0L);
			  }
			  break;
	      default:
	         return FALSE;
	     } 
      break;

   default:
      return FALSE;
      }
   return TRUE;
}

BOOL ComposeMessage (HWND hWnd,LPSTR Title,LPSTR InMessage,LPSTR ResponseAction)
{
	HWND	hWndDlg;
	BOOL	rtn=FALSE;
	int		lResponse = strlen (ResponseAction)+1;
	HANDLE	hResponse;
	LPSTR	pstr;


	if (*InMessage)
	{
		if ((hWndDlg = CreateDialog(hInst, (LPSTR)"MESSAGE_INCOMING", hWnd,(DLGPROC) MESSAGE_INCOMINGMsgProc)))
		{
			SetWindowText (hWndDlg,Title);
			REPLAC (InMessage,"\\r\\n","\r\n",strlen (InMessage));
			SetWindowText (GetDlgItem(hWndDlg,IDC_INCOMINGMESSAGE),InMessage);
			hResponse = GSSiGlobAlloc(GAIDNO 1703,GMEM_MOVEABLE,lResponse);
			pstr = GlobalLock (hResponse);
			strcpy (pstr,ResponseAction);
			GlobalUnlock (hResponse);
			SetWindowLong (hWndDlg,GWL_USERDATA,(long)hResponse);
			rtn = TRUE;
		}
	}
	else
	{
		if ((hWndDlg = CreateDialog(hInst, (LPSTR)"MESSAGE_OUTGOING", hWnd, (DLGPROC)MESSAGE_OUTGOINGMsgProc)))
		{
			SetWindowText (hWndDlg,Title);
			hResponse = GSSiGlobAlloc(GAIDNO 1703,GMEM_MOVEABLE,lResponse);
			pstr = GlobalLock (hResponse);
			strcpy (pstr,ResponseAction);
			GlobalUnlock (hResponse);
			SetWindowLong (hWndDlg,GWL_USERDATA,(long)hResponse);
			rtn = TRUE;
		}
	}

	return rtn;
}


long SearchFilesInDirBC (LPSTR CurDirIN, LPSTR Ext, HANDLE OutFid,LPLONG TotFiles,LPSTR WildCardIn,int Lev,BOOL WantSub)
{   
	DWORD	hDir, Type;
	long	NumFilesIn=*TotFiles;
    char    setstr[256],FileName[256],FullName[256], TestExt[64], CurDir[256], str[256], WildCard[34];
    short       i, rtn,ii;
    int st;
    BOOL	FirstPass=TRUE, SubDirOnly; 
	LPSTR	lc;
	WIN32_FIND_DATA	FindFileData;
	
	_fstrcpy (WildCard,WildCardIn);
	if (WildCard && !_fstricmp (WildCard,"*.*"))
		*WildCard = 0;
	if (Lev < 0)
	{
		WantSub = FALSE;
		Lev = -Lev;
	}
	_fstrcpy (CurDir,CurDirIN); 
//	ExpandText (CurDir);  
//	ConvertToNewLocation (CurDir,FALSE);
	lc  = LastChr (CurDir);
    if (*lc == '\\')
    	*lc = 0;
Top: 
	SubDirOnly = FALSE;   
    if (*WildCard && FirstPass)
    {   
    	if (_fstrchr (WildCard,'.'))
    		sprintf (setstr,"%s\\%s",CurDir,WildCard);
    	else
    		sprintf (setstr,"%s\\%s",CurDir,WildCard);  
    	_fstrcpy (FileName,setstr);
		hDir = SearchDirectory32 (FileName,0,&Type,&FindFileData);
    }
    else 
    {   
    	sprintf (setstr,"%s\\*",CurDir);
    	if (*WildCard)
			SubDirOnly = TRUE;   
    	_fstrcpy (FileName,setstr);
		hDir = SearchDirectory32 (FileName,0,&Type,&FindFileData);
	   	FirstPass=FALSE;
    }
    while (hDir)
    {   
        if (FileName[0] != '.')
        {
            sprintf (str,"%s\\%s",CurDir,FileName);
			if (Type)
            {   
            	if (WantSub && !FirstPass)
                	SearchFilesInDirBC (str,Ext,OutFid,TotFiles,WildCard,Lev+1,WantSub);
            }
            else if (SubDirOnly)
            	goto SkipFile; 
            else
            {   
            	
                _fullpath (FullName,str,sizeof(FullName));
                _splitpath (FullName,0,0,0,TestExt);  
                (*TotFiles)++;
				strupr (FullName);
                if (OutFid != INVALID_HANDLE_VALUE)
                	fputstring2 (FullName,OutFid);
	     SkipFile:;
            }
        }
		hDir = SearchDirectory32 (FileName,hDir,&Type,&FindFileData);
    }
    if (FirstPass)
    {
    	FirstPass=FALSE;
    	goto Top;
    }
    return (*TotFiles - NumFilesIn);
} 

void __cdecl BackgroundCache (LPHANDLE phArgs)
{
    char    File[MAX_PATH];
	char	CacheDir[MAX_PATH];
	char	DataLocDir[MAX_PATH];
	char	CachingPidFile[MAX_PATH];
	char	cachFileList[MAX_PATH];
	char	tempFile[MAX_PATH];
	char	str[MAX_PATH+2];
	HANDLE	Fid, Fid2;
	LPSTR	arg1=GlobalLock (*phArgs);
	LPSTR	arg2 = arg1 + 4096, arg3 = arg2 + 4096, arg4 = arg3 + 4096;
	FILE	*FidFilelist;  
	OFSTRUCTGM	OFStruct;
	DWORD	Pid = _getpid ();
	PVOID	oldValue;
	int		pass=0;
	int		totFiles=0;
	int		pctdone, ifile=0;
	BOOL	skip=FALSE;

	//Wow64DisableWow64FsRedirection (&oldValue);
	strcpy (cachFileList,arg1);
	strcpy (CacheDir,arg2);
	strcpy (DataLocDir,arg3);
	strcpy (CachingPidFile,arg4);
	GlobalUnlock (*phArgs);
	GSSiGlobFree (phArgs);
	Fid = OpenFileGM (CachingPidFile,&OFStruct,OF_CREATE);
	BigWrite64 (Fid,(LPSTR)&Pid,sizeof(DWORD),-1);
	GSSiClose64 (&Fid);
	GSSiGetTempFileName(0,"gmb",0,tempFile);
	ContinueInteruptedCache (CacheDir);

	while (pass < 2)
	{
		if (ContinueBackgroundCache)
		{
			FidFilelist = fopen(cachFileList,"rt");
			if (FidFilelist != NULL)
			{
				while (ContinueBackgroundCache && fgetss(str,MAX_PATH,FidFilelist))
				{					
					REPLAC (str,"[%DL]",DataLocDir,MAX_PATH);
					if (strchr (str,'*'))
					{
						int		nFiles=0;
						LPSTR	pWild = strrchr (str,'\\');
						BOOL	wantSub=FALSE;

						if (pWild)
						{
							if (*(pWild-1) == '\\')
							{
								wantSub = TRUE;
								*(pWild-1) = 0;
							}
							*pWild++ = 0;
							Fid2 = OpenFileGM (tempFile,&OFStruct,OF_CREATE); 
							ii = SearchFilesInDirBC (str, 0, Fid2,&nFiles,pWild,1,wantSub);     
							llFileSeek(Fid2,0,0);
							if (pass)
							{
								while (ContinueBackgroundCache && fgetstring2 (str,MAX_PATH,Fid2))
								{
									REPLAC (str,"[%DL]",DataLocDir,MAX_PATH);
									pctdone = (100 * ifile++)/totFiles;
									CacheFileInBackground (str,CacheDir,DataLocDir,0);
								}
							}
							else
								totFiles += nFiles;
							GSSiClose64 (&Fid2);
							OpenFileGM (tempFile,&OFStruct,OF_DELETE);
						}
					}
					else if (pass)
					{
						pctdone = (100 * ifile++)/totFiles;
						CacheFileInBackground (str,CacheDir,DataLocDir,0);
					}
					else
						totFiles++;	
				}
				if (ContinueBackgroundCache && pass)
				{
					char	LastCacheCheckFile[MAX_PATH];
					HANDLE	FidLastCacheCheck;

					sprintf (LastCacheCheckFile,"%sLastCacheCheck.txt",CacheDir);
					FidLastCacheCheck = OpenFileGM(LastCacheCheckFile,&OFStruct,OF_CREATE);
					if (FidLastCacheCheck != INVALID_HANDLE_VALUE)
					{
						BigWrite64 (FidLastCacheCheck,"Cache check complete",20,-1);
						GSSiClose64 (&FidLastCacheCheck);
					}
					BackgroundUpdateMessage ("Cache is up to date");
				}
				fclose (FidFilelist);
			}
		}
		pass++;
	}
	OpenFileGM (CachingPidFile,&OFStruct,OF_DELETE);
	hCacheThread = 0;
	//Wow64RevertWow64FsRedirection (oldValue);
	return;
}

BOOL StartBackgroundCache (void)
{
	char	BackgroundCacheFilelist[MAX_PATH];
	char	str[MAX_PATH+2];
	static	HANDLE hArgs;
	LPSTR	arg1,arg2,arg3,arg4;//filelistpath,cachepath,datalocpath,cachingpidfile
	HANDLE	Fid, Fid2, FidList, FidLastCacheCheck;
	char	CachingPidFile[MAX_PATH];
	char	LastCacheCheckFile[MAX_PATH];
	BOOL	rtn = FALSE;
	OFSTRUCTGM	OFStruct;
	static	BOOL	firstCall=TRUE;

//	return TRUE;
	if (!firstCall)
		return TRUE;

	firstCall = FALSE;
	ProcessText ("[%RUNFROMCACHE]=F");
	//return TRUE;

	GetGlobalCVal ("[%BackgroundCacheFilelist]",BackgroundCacheFilelist,"[%DL]BackgroundCacheFilelist.txt");
	if (!*BackgroundCacheFilelist || !AllowCache || !ExistFile (BackgroundCacheFilelist))
		return FALSE;

	ExpandText(BackgroundCacheFilelist);
	sprintf (LastCacheCheckFile,"%sLastCacheCheck.txt",CachePathnameTo);
	FidLastCacheCheck = OpenFileGM(LastCacheCheckFile,&OFStruct,OF_READ);
	if (FidLastCacheCheck != INVALID_HANDLE_VALUE)
	{
		BY_HANDLE_FILE_INFORMATION fiList, fiLastCacheCheckFile;
		LONG	dtime;
		BOOL	st;
		SYSTEMTIME systim;

		FidList = OpenFileGM (BackgroundCacheFilelist,&OFStruct,OF_READ);
		st = GetFileInformationByHandle((HANDLE)FidList,&fiList);
		st = GetFileInformationByHandle((HANDLE)FidLastCacheCheck,&fiLastCacheCheckFile);
		FileTimeToSystemTime(&fiLastCacheCheckFile.ftLastWriteTime, &systim);
		dtime = CompareFileTime (&fiList.ftLastWriteTime,&fiLastCacheCheckFile.ftLastWriteTime); 
		GSSiClose64(&FidList);
		GSSiClose64(&FidLastCacheCheck);
		//if (dtime < 0)
		//	ProcessText ("[%RUNFROMCACHE]=T");
	}

	MaxFileSizeToCache = -1024 * 1024;
	hArgs = GSSiGlobAlloc(GAIDNO 9999,GMEM_MOVEABLE,4096*5);
	arg1=GlobalLock (hArgs);
	arg2=arg1+4096;
	arg3=arg2+4096;
	arg4=arg3+4096;

	strcpy (arg1,OFStruct.szPathName);
	strcpy (arg2,CachePathnameTo);
	ExpandText (arg2);
	strcpy (arg3,"[%DL]");
	ExpandText (arg3);
	sprintf (CachingPidFile,"%sCachingPid.bin",CachePathnameTo);
	strcpy (arg4,CachingPidFile);
	GlobalUnlock (hArgs);
	if (!AnotherProcessIsCaching (CachingPidFile))
	{
		BOOL	skip=FALSE;

		arg1=GlobalLock (hArgs);
		Fid = OpenFileGM (BackgroundCacheFilelist,&OFStruct,OF_READ);//caches file list so it can be updated
		GSSiGetTempFileName (0,"txt",0,arg1); 
		Fid2 = OpenFileGM (arg1,0,OF_CREATE);
		GlobalUnlock (hArgs);
		while (fgetstring2 (str,MAX_PATH,Fid))
		{
			if (!strncmp (str,"[%DL]",5))
			{
				int	l = strlen (str);

				memmove (&str[1],str,l+1);
				*str = '@';
			}
			if (!strnicmp (str,"IF(",3))
			{
				LPSTR pPar = strrchr (str,')');
				BOOL	rtn, irc;

				if (pPar)
					*pPar = 0;
				rtn = LogicP (&str[3],&irc);
				if (!irc)
					skip = !rtn;
			}
			else if (!stricmp (str,"ENDIF"))
				skip = FALSE;
			else if (!skip)
			{
				ExpandText (str);
				fputstring2 (str,Fid2);
			}
		}
		GSSiClose64 (&Fid);
		GSSiClose64 (&Fid2);
		GSSiRemove(arg1);
		BackgroundCacheStarted = TRUE;
		RenameCachedFiles ();
		ContinueBackgroundCache = TRUE;
		BackgroundUpdateMessage ("Checking for files to cache");
		{
			char cmd[512];
			arg1 = GlobalLock(hArgs);
			sprintf(cmd, "$TEXTTOCLIPBOARD(%s)", arg1);
			GlobalUnlock(hArgs);
			ExpandText (cmd);
		}
		hCacheThread = (HANDLE)_beginthread( BackgroundCache, 0, &hArgs);
		SetThreadPriority (hCacheThread,THREAD_PRIORITY_LOWEST);
		rtn = TRUE;
	}
	else
		GSSiGlobFree (&hArgs);
	return rtn;
}

void StopBackgroundCache (void)
{
	DWORD	rtn;

	if (hCacheThread)
	{
		ContinueBackgroundCache = FALSE;
		rtn = WaitForSingleObject (hCacheThread,5000);
	}
	RenameCachedFiles ();
	return;
}

void __cdecl BackgroundCacheViaServer (LPHANDLE phArgs)
{
    char    File[MAX_PATH];
	char	CacheDir[MAX_PATH];
	char	DataLocDir[MAX_PATH];
	char	CachingPidFile[MAX_PATH];
	char	FileListFile[MAX_PATH];
	char	remoteNetTransferFile[MAX_PATH];
	LPSTR	localNetTransferFile;
	char	ipAddress[32];
	char	str[MAX_PATH+2];
	HANDLE	Fid;
	LPSTR	arg1=GlobalLock (*phArgs);
	LPSTR	arg2 = arg1 + 4096, arg3 = arg2 + 4096, arg4 = arg3 + 4096, arg5 = arg4 + 4096;
	FILE	*FidFilelist;  
	OFSTRUCTGM	OFStruct;
	DWORD	Pid = _getpid ();
	SOCKET	sock;
	u_short	port = atoi (arg3);
	int		err, st, lnRecv;
	DWORD	timeOut=1000;
	char	compCommand[1024];
	int		lnCompCommand;

	strcpy (FileListFile,arg1);
	strcpy (ipAddress,arg2);
	strcpy (compCommand,arg4);
	lnCompCommand = strlen (compCommand);
	strcpy (CachingPidFile,arg5);
	GlobalUnlock (*phArgs);
	GSSiGlobFree (phArgs);

	BackgroundUpdateMessage ("Waiting for connection to server");

	do
	{
		sock = connectToServer (ipAddress,port,TRUE,&err);
		if (sock == INVALID_SOCKET)
			Sleep (2000);
	}while (sock == INVALID_SOCKET && ContinueBackgroundCache);
	if (sock == INVALID_SOCKET)
	{
		char	msg[128];

		sprintf (msg,"Fail to connect to update server at %s port %i",ipAddress,port);
		BackgroundUpdateMessage (msg);
		goto Exit;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeOut,  sizeof(timeOut)))
		err=1;
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeOut,  sizeof(timeOut)))
		err=1;
	if (!ReadNTBlock (sock,str,36))
		goto Exit;
	if (!strnicmp (str,"Connected to GeoMaster Update Server",36))
	{
		str[36] = 0;
		BackgroundUpdateMessage (str);
		Fid = OpenFileGM (CachingPidFile,&OFStruct,OF_CREATE);
		BigWrite64 (Fid,(LPSTR)&Pid,sizeof(DWORD),-1);
		GSSiClose64(&Fid);
	//	ContinueInteruptedCache (CacheDir);
		if (ContinueBackgroundCache)
		{
			long	ln;
			LPSTR	pBuf;

			Fid = OpenFileGM (FileListFile,&OFStruct,OF_READ);
			ln = llFileSeek(Fid,0,2);
			llFileSeek(Fid,0,0);
			pBuf = malloc (ln);
			BigRead64 (Fid,pBuf,ln);
			GSSiClose64 (&Fid);
			Fid = OpenFileGM (FileListFile,&OFStruct,OF_DELETE);
			st = SendNTBlock (sock,pBuf,ln);
			free (pBuf);
			if (st)
			{
				if (ReadNTBlock (sock,(LPSTR)&ln ,4))
				{
					pBuf = malloc (ln);
					if (ReadNTBlock (sock,pBuf,ln))
					{
						if (!strncmp (pBuf,"OK:",3))
						{
							LPSTR pLen = strrchr (pBuf,'|');
							long	lenTranFile, totRead=0;

							*pLen++ = 0;
							lenTranFile = atoi (pLen);
							strcpy (remoteNetTransferFile,pBuf+3);
							localNetTransferFile = strrchr (pBuf,'\\');
							sprintf (File,"%s%s",NetTransferDir,localNetTransferFile);
							free (pBuf);
							makedirectories (File,FALSE,FALSE);
							Fid = OpenFileGM (File,&OFStruct,OF_CREATE);
							if (Fid != INVALID_HANDLE_VALUE)
							{
								long	lnBlock;

								while (totRead < lenTranFile && ReadNTBlock (sock,(LPSTR)&lnBlock,4))
								{
									LPBYTE pBlock = malloc (lnBlock);

									if (ReadNTBlock (sock,pBlock,lnBlock))
									{
										BOOL	rtnFlush;
										int		lenExpandedBlock = *(LPINT)pBlock;
										LPBYTE	pExpandedBlock = malloc (lenExpandedBlock+1024);
										int		lRec = DecompressBinaryRecordUnsafe (pExpandedBlock,pBlock+sizeof(int),lnBlock-sizeof(int));

										free (pBlock);
											
										if (BigWrite64 (Fid,pExpandedBlock,lRec,-1) == lenExpandedBlock)
										{
											rtnFlush = FlushFileBuffers ((HANDLE)Fid);
											totRead += lenExpandedBlock;
											sprintf (str,"%i%% transfered",(100 * totRead)/lenTranFile);
											BackgroundUpdateMessage (str);
											SendNTBlock (sock,"GOT IT",6);
											free (pExpandedBlock);
										}
										else
										{
											free (pExpandedBlock);
											break;
										}
									}
									else
									{
										free (pBlock);
										break;
									}
								}
								GSSiClose64(&Fid);
								if (totRead == lenTranFile)
								{
									HANDLE	hMem = GSSiGlobAlloc(GAIDNO 1732,GHND,strlen(File)+1+lnCompCommand+1);
									LPSTR	pMem = GlobalLock (hMem);
									
									BackgroundUpdateMessage ("Transfer complete");
									strcpy (pMem,File);
									if (lnCompCommand)
										strcpy (strchr(pMem,0)+1,compCommand);
									GlobalUnlock (hMem);
									PostMessage (hWndMain,GF_UPDATE_FILE_RECEIVED,0,(LPARAM)hMem);
								}
								else
									BackgroundUpdateMessage ("Invalid file length");
							}
							else
							{
								BackgroundUpdateMessage ("Cannot create transfer file");
								MessageBox (0,OFStruct.szPathName,"bad name",MB_ICONEXCLAMATION);
								itoa (OFStruct.nErrCode,str,10);
								MessageBox (0,str,"err code",MB_ICONEXCLAMATION);
							}
						}
						else
						{
							char	msg[256];

							sprintf (msg,"Invalid response from update server:%s",pBuf);
							free (pBuf);
							BackgroundUpdateMessage (msg);
						}
					}
					else
					{
						free (pBuf);
						BackgroundUpdateMessage ("Invalid response from update server(2)");
					}
				}
				else
					BackgroundUpdateMessage ("Invalid response from update server(3)");
			}
			OpenFileGM (CachingPidFile,&OFStruct,OF_DELETE);
		}
	}
	else
		BackgroundUpdateMessage ("Invalid response from update server(4)");
Exit:
	if (sock != INVALID_SOCKET)
	{
		shutdown (sock,2);
		closesocket (sock);
	}
	hCacheThread = 0;
	return;
}


BOOL CacheCommands(int nArgs, LPSTR* Arg, LPSTR OutLoc)
{
	BOOL	rtn = FALSE;
	static	int	port = 0;
	static  char	ipAddress[32] = { 0 };
	static	char	tmpFile[MAX_PATH];
	int		err, ln;
	char	fileName[MAX_PATH + 12];
	char	cachingPidFile[MAX_PATH];
	static	HFILE	Fid = HFILE_ERROR;
	static	int		nFiles;

	*OutLoc = 0;
	ExpandText(Arg[1]);
	ExpandText(Arg[2]);
	if (!stricmp(Arg[1], "SERVER"))
	{
		strcpy(NetTransferDir, "[%CACHEDIRACTUAL]NetTransferFiles");
		ExpandText(NetTransferDir);
		makedirectories(NetTransferDir, TRUE, FALSE);
		if (!stricmp(Arg[2], "OPEN"))
		{
			int lastUpdate;
			long	totFileLen = 0;

			GSSiGetTempFileName(0, "gmc", 0, tmpFile);
			Fid = GSSiOpenFile(tmpFile, 0, OF_CREATE);
			BigWrite(Fid, &totFileLen, sizeof(long), -1);
			ExpandText(Arg[3]);
			strcpy(ipAddress, Arg[3]);
			ExpandText(Arg[4]);
			port = atoi(Arg[4]);
			ExpandText(Arg[5]);
			lastUpdate = atoi(Arg[5]);
			nFiles = 0;
			BigWrite(Fid, &nFiles, sizeof(int), -1);
			BigWrite(Fid, &lastUpdate, sizeof(int), -1);
			rtn = TRUE;
		}
		else if (!stricmp(Arg[2], "ADD"))
		{
			HANDLE hDB;
			BOOL	saveAllowCache = AllowCache;

			if (AllowCache)
				AllowCache = 2;
			if (strstr(Arg[3], ".gmd") || strstr(Arg[3], ".GMD"))
			{
				HANDLE hDB = OpenGWDatabase(Arg[3], BT_READ);
				int	   iCPID;
				if (hDB)
				{
					LPGWDHEADER lpGWDHead = GlobalLock(hDB);

					iCPID = lpGWDHead->CheckPointID;
					GlobalUnlock(hDB);
					CloseGWDatabase(hDB);
					if (iCPID > 0)
						sprintf(fileName, "%s(%i)", Arg[3], iCPID);
					else
						strcpy(fileName, Arg[3]);
					ln = strlen(fileName) + 1;
					BigWrite(Fid, &ln, sizeof(int), -1);
					BigWrite(Fid, fileName, ln, -1);
					nFiles++;
				}
			}
			else
			{
				strcpy(fileName, Arg[3]);
				ln = strlen(fileName) + 1;
				BigWrite(Fid, fileName, ln, -1);
				nFiles++;
				strcpy(OutLoc, "1");
			}
			AllowCache = saveAllowCache;
			rtn = TRUE;
		}
		else if (!stricmp(Arg[2], "CLOSE"))
		{
			static	HANDLE hArgs;
			LPSTR	arg1, arg2, arg3, arg4, arg5;//filelistpath,ipaddress,port,commandtorunoncompletion,cachingpidfile
			long	totFileLen = 0;

			hArgs = GSSiGlobAlloc(GAIDNO 9999, GMEM_MOVEABLE, 4096 * 5);
			arg1 = GlobalLock(hArgs);
			arg2 = arg1 + 4096;
			arg3 = arg2 + 4096;
			arg4 = arg3 + 4096;
			arg5 = arg4 + 4096;

			totFileLen = GSSillseek(Fid, 0, 2);
			GSSillseek(Fid, 0, 0);
			BigWrite(Fid, &totFileLen, sizeof(long), -1);
			BigWrite(Fid, &nFiles, sizeof(int), -1);
			GSSiClose2(&Fid);
			strcpy(arg1, tmpFile);
			strcpy(arg2, ipAddress);
			itoa(port, arg3, 10);
			strcpy(arg4, Arg[3]);
			sprintf(cachingPidFile, "%sCachingPid.bin", CachePathnameTo);
			strcpy(arg5, cachingPidFile);
			GlobalUnlock(hArgs);
			if (OpenWinSock() && !hCacheThread && !AnotherProcessIsCaching(cachingPidFile))
			{
				BackgroundCacheStarted = TRUE;
				ContinueBackgroundCache = TRUE;
				BackgroundUpdateMessage("Starting background cache");
				hCacheThread = (HANDLE)_beginthread(BackgroundCacheViaServer, 0, &hArgs);
				SetThreadPriority(hCacheThread, THREAD_PRIORITY_LOWEST);
				rtn = TRUE;
			}
			else
				GSSiGlobFree(&hArgs);
		}
		else if (!stricmp(Arg[2], "RESTART"))
		{
		}
	}
	else if (!stricmp(Arg[1], "NEWFILES"))
	{
		UpdateLastDataUpdate(Arg[2],atoi(Arg[3]));
		rtn = TRUE;
	}
	else if (!stricmp(Arg[1], "HOLD"))
	{
		char today[64];
		char cmd[64];
		sprintf(today, "[%%SYS_CLOCK]");
		ExpandText(today);
		int now = atol(today);
		sprintf(today, "$CAL([%%SYS_CLOCK],3)");
		ExpandText(today);
		LPSTR pSpace = strchr(today, ' ');
		if (pSpace)
		{
			*pSpace = 0;
			sprintf(cmd, "$CLK(%s %s)", today,Arg[2]);
			ExpandText(cmd);
			GMCacheOnHoldUntil = atol(cmd);
			int delay = GMCacheOnHoldUntil - now;
			if (delay < 0)
				delay = 10;
			if (isGMCache)
				SetTimer(hWndMain, GMCACHE_TIMER, delay * 1000, 0);
		}

	}
	return rtn;
}

LONG FAR PASCAL BGUpdateWndProc(HWND hWnd, UINT Message, WPARAM wParam, LONG lParam)
{
	RECT	rect,screenrect;


	switch (Message)
   {
		case WM_CREATE:
			hWndBGUpdateMsg = hWnd;
		break;

		case WM_DESTROY:
			hWndBGUpdateMsg = 0;
			return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
		
		case WM_COMMAND:
			switch(LOWORD(wParam))
            {
			case IDOK:
			
				if (!IsRectEmpty (&PromptRect))
				{
					rect = PromptRect;
					rect.left = rect.right - 400;
				//	InflateRect (&rect,1,1);
					ClientRectToScreenRect (hWndMain,&rect);
				}
				else
				{
					GetWindowRect(GetDesktopWindow(),&screenrect);
					GetWindowRect(hWndMain,&rect);
					rect.right = min (screenrect.right,rect.right);
					rect.bottom = min (screenrect.bottom,rect.bottom);
					rect.bottom -= 2;
					rect.right -= 2;
					rect.top = rect.bottom - 18;
				}
				rect.left = rect.right - 400;
				ScreenRectToClientRect (hWndMain,&rect);
				SetWindowPos (hWnd,hWndMain,rect.left,rect.top,RECTWIDTH(&rect),RECTHEIGHT(&rect),SWP_SHOWWINDOW|SWP_NOZORDER);
				InvalidateRect (hWnd,0,TRUE);
				break;
			case IDCANCEL:
				break;
			}
			
			return 0;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC	hDC = BeginPaint (hWnd,&ps);
				if (hDC)
				{
					HFONT	hOldFont, hFont;
					char	str[1024];
					int FontSize;

					GetClientRect(hWnd,&rect);
					FontSize =RECTHEIGHT (&rect);
  					SelectClipRgn (hDC,0);   
  					SetTextColor (hDC,0); 
					SetBkMode (hDC,TRANSPARENT);
					rect.left+=2;
					hFont = CreateFont(FontSize, 0, 0, 0, FW_BOLD, 
    									0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
					hOldFont = SelectObject (hDC,hFont);
					GetWindowText (hWnd,str,1023);
					//InflateRect (&rect,-1,-1);
					rect.left += 300;
					FillRect (hDC,&rect,GetStockObject (LTGRAY_BRUSH));
					GdiFlush ();
					SetBkColor (hDC,RGB(196,196,196));
					DrawText (hDC,BUMessage[1],strlen(BUMessage[1]),&rect,DT_RIGHT);
					rect.left -= 300;
					rect.right -= 100;
					FillRect(hDC, &rect, GetStockObject(WHITE_BRUSH));//(LTGRAY_BRUSH));
					DrawText (hDC,BUMessage[0],strlen(BUMessage[0]),&rect,DT_LEFT);
					SelectObject (hDC,hOldFont);
					GSSiDeleteObject (&hFont);
					EndPaint (hWnd,&ps);
				}
			}
			return 0;
		case WM_ERASEBKGND:
			{
				HDC hDC=GetDC (hWnd);

				GetClientRect(hWnd,&rect);
				FillRect(hDC, &rect, GetStockObject(WHITE_BRUSH));//(LTGRAY_BRUSH));
				ReleaseDC (hWnd,hDC);
			}
			return 1;

/*		case WM_EXITSIZEMOVE:
			return DefWindowProc(hWnd, Message, wParam, lParam);

		case WM_MOVING:
			return DefWindowProc(hWnd, Message, wParam, lParam);
		case WM_SIZE:
			return DefWindowProc(hWnd, Message, wParam, lParam);*/

	}

	return DefWindowProc(hWnd, Message, wParam, lParam);
}


BOOL RegisterBgUpdateClass(BOOL UnRegister)
{
    WNDCLASS  wc; 
    static	Called=FALSE;

	if (UnRegister)
	{
		 if (Called)
		 {
			 UnregisterClass(BGUPDATEWINDOWCLASS, hInst);
			 Called = FALSE;
		 }
		 return TRUE;
	}
    if (Called) return TRUE;
    Called = TRUE;
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)BGUpdateWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = 0;//LoadCursor(NULL, IDC_HAND);
    wc.hbrBackground = GetStockObject (LTGRAY_BRUSH);
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = BGUPDATEWINDOWCLASS;

    return (RegisterClass(&wc));
}

void BackgroundUpdateMessage (LPSTR mess)
{
	static	HWND hWnd=0;
	static	char	lastMess[1024]="";
	RECT	rect;

	if (hWndMain)
		SetWindowText(hWndMain, mess);
	return;
	if (!mess)
	{
		if (hWnd)
			DestroyWindow (hWnd);
		return;
	}
	if (!strcmp (mess,"!REDISPLAY!"))
	{
		if (hWnd)
		{
			SetWindowText (hWnd,lastMess);
			InvalidateRect (hWnd,0,FALSE);
		}
		return;
	}
	if (!hWnd)
	{
		int x=100,y=100,w=400,h=18;

		*lastMess = 0;
		GetWindowRect(hWndMain,&rect);
		y = rect.bottom - h*2;
		x = rect.right - w*2;
			RegisterBgUpdateClass (FALSE);
			hWnd = CreateWindowEx(0, BGUPDATEWINDOWCLASS, (LPSTR) NULL,
										WS_CHILD|WS_CLIPSIBLINGS,
										x, y, w, h,
										hWndMain, (HMENU) NULL, hInst, NULL); 
			PostMessage (hWnd,WM_COMMAND,IDOK,0);
	}
	if (*mess == ':')
		strcpy (BUMessage[1],&mess[1]);
	else
		strcpy (BUMessage[0],mess);
	if (strcmp (mess,lastMess))
	{
		strcpy (lastMess,mess);
		SetWindowText (hWnd,mess);
		InvalidateRect (hWnd,0,FALSE);
	}
	return;
}

BOOL ProcessUpdateFile (HANDLE hMem)
{
	LPSTR pFile = GlobalLock (hMem);
	LPSTR pCmd  = strchr (pFile,0)+1;
	BOOL	st=FALSE;
	HFILE	Fid, FidUp;

	BackgroundUpdateMessage ("Processing update file");
	HaltMapDisplay (FALSE,TRUE);
	CloseAllRequestedFiles (FALSE);
	Fid = GSSiOpenFile (pFile,0,OF_READ);
	if (Fid != HFILE_ERROR)
	{
		LONGLONG lenFile, lnFile = GSSillseek (Fid,0,2);

		GSSillseek (Fid,0,0);
		BigRead (Fid,&lenFile,sizeof(LONGLONG));
		if (lenFile == lnFile)
		{
			int	lnf, lnc, version;
			LPSTR	pLastUpdateVal;

			BigRead (Fid,&version,sizeof(int));
			BigRead (Fid,&lnc,sizeof(int));
			pLastUpdateVal = malloc (lnc);
			BigRead (Fid,pLastUpdateVal,lnc);
			BigRead (Fid,&lnf,sizeof(int));
			while (lnf > 0)
			{
				char	fileName[MAX_PATH+2];
				LONGLONG	blockLoc;

				BigRead (Fid,fileName,lnf);
				FidUp = GSSiOpenFile (fileName,0,OF_READWRITE);
				BigRead (Fid,(LPSTR)&blockLoc,sizeof(LONGLONG));
				while (blockLoc >= 0)
				{
					long	nBytes;
					LPBYTE	pBlockData;

					BigRead (Fid,(LPSTR)&nBytes,sizeof(long));
					pBlockData = malloc (nBytes);
					BigRead (Fid,pBlockData,nBytes);
					if (FidUp != HFILE_ERROR)
					{
						GSSillseek (FidUp,blockLoc,0);
						BigWrite (FidUp,pBlockData,nBytes,-1);
					}
					free (pBlockData);
					BigRead (Fid,(LPSTR)&blockLoc,sizeof(LONGLONG));
				}
				GSSiClose2 (&FidUp);
				BigRead (Fid,&lnf,sizeof(int));
			}
			st = TRUE;
			if (st)
				SetGlobalValue ("%LASTNETUPDATE",pLastUpdateVal);
			free (pLastUpdateVal);
		}
		GSSiClose2 (&Fid);
		GSSiRemove (pFile);
	}
	if (st)
		BackgroundUpdateMessage ("Update complete");
	else
		BackgroundUpdateMessage ("Update failed");
	ProcessText (pCmd);
	GSSiGlobUlFree (&hMem);
	RedisplayViewports (FALSE);
	return TRUE;
}

void SetImageZoomOffset (void)
{
	if (ImageZoomOffset)
	{
		RECT	wRect,mRect;
		POINT	cPt,midPt;

		GetWindowRect(hWndImageZoom,&wRect);
		GetWindowRect(hWndMain,&mRect);
		GetCursorPos (&cPt);
		midPt = RectMid (&mRect);
		if (ImageZoomShape == 1)
			ImageZoomXoff = RECTWIDTH (&wRect) / 2;
		else
			ImageZoomXoff = (1.2 * RECTWIDTH (&wRect)) / 2;
		if (cPt.x > midPt.x)
			ImageZoomXoff = -ImageZoomXoff;
		if (ImageZoomShape == 1)
			ImageZoomYoff = RECTHEIGHT (&wRect) / 2;
		else
			ImageZoomYoff = (1.2 * RECTHEIGHT (&wRect)) / 2;
		if (cPt.y > midPt.y)
			ImageZoomYoff = -ImageZoomYoff;
	}
	else
		ImageZoomXoff = ImageZoomYoff = 0;

	return;
}

void DisplayImageZoom (HWND hWnd,HDC hDC,int From)
{
	DPOINT	wPoint, fPoint;
	POINT	centerPoint;
	RECT	wRect, cRect;
	int		w,h;
	static	RECT	LastImageZoomRect={0};

	GetWindowRect(hWnd,&wRect);
	GetClientRect(hWnd,&cRect);
	if (From == 1 && ImageZoomSavedScreen && LastImageZoomRect.left != LastImageZoomRect.right)
	{
		DPOINT	wPoint, fPoint;
		POINT	centerPoint;
		int		w,h;
		HDC		hDC = GetDC (hWnd);

		ScreenRectToClientRect (GetDesktopWindow(),&LastImageZoomRect);
		BitBlt (hDC,0,0,RECTWIDTH(&LastImageZoomRect),RECTHEIGHT(&LastImageZoomRect),
				hDCImageZoom,LastImageZoomRect.left,LastImageZoomRect.top,SRCCOPY);
		ReleaseDC (hWnd,hDC);
		LastImageZoomRect.left = LastImageZoomRect.right;
	}
	else if (From == 2 || From == 3)
	{
		w = RECTWIDTH(&cRect);
		h = RECTHEIGHT(&cRect);
		centerPoint = RectMid (&wRect);
		centerPoint.x -= ImageZoomXoff;
		centerPoint.y -= ImageZoomYoff;
		//SetViewport (ImageZoomVP);
		ScreenToClient (CurView->hWnd,&centerPoint);
		if (From == 2)
		{
			wPoint = ScreenPtToBasePt(centerPoint);
			fPoint = BasePtToFilePtD(wPoint);
		}
		else
			fPoint = PointToDPoint (centerPoint);
		//BitBlt (hDC,0,0,w,h,
		//		hDCImageZoom,fPoint.x-w/2,ImageZoomHeight-(fPoint.y+h/2),SRCCOPY);
   		if (!StretchDIBitsFromHandle (hDC,0,0,w,h,fPoint.x-w/2,(fPoint.y-h/2),w,h,hDibImageZoom,(UINT)DIB_RGB_COLORS,SRCCOPY,1))
			FillRectPoly (hDC,&cRect,CurView->BackGroundColor);

		LastImageZoomRect = wRect;
		if (ImageZoomBorder)
		{
			HBRUSH hBrush = GetStockObject(NULL_BRUSH);
			HBRUSH	hOldBrush = SelectObject(hDC, hBrush);
			HPEN hPen = CreatePen(PS_SOLID, 2, 0);
			HPEN hOldPen = SelectObject(hDC, hPen);
				 
			RECT	rect = cRect;

			InflateRect (&rect,-1,-1);
			if (ImageZoomShape == 1)
				Ellipse(hDC, 1, 1, w - 1, h - 1);
			else
				Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
			SelectObject(hDC, hOldBrush);
			SelectObject(hDC, hOldPen);
			DeleteObject(hPen);
		}
	}
	return;
}

void SetImageZoomSize (HWND hWnd,int size)
{
	int		x,y,w,h;
	RECT	wRect;
	POINT	pt;

	ShowWindow(hWnd, SW_HIDE);
	if (size)
		ImageZoomSize = size;

	w = ImageZoomSize;
	h = ImageZoomSize;
	GetWindowRect(hWnd,&wRect);
	pt = RectMid (&wRect);
	x = pt.x - w/2;
	y = pt.y - h/2;
	if (ImageZoomShape==1)
	{
		HRGN hRgn = CreateEllipticRgn(0,0,w,h);

		SetWindowRgn(hWnd,hRgn,FALSE);
	}
	else
		SetWindowRgn(hWnd,0,FALSE);
//	MoveWindow (hWnd,x,y,w,h,TRUE);
	SetWindowPos (hWnd,0,x,y,w,h,SWP_NOZORDER|SWP_NOOWNERZORDER);
	SetImageZoomOffset ();
	if (!ImageZoomOffset)
		SetCursorPos (pt.x,pt.y);
	ShowWindow (hWnd,SW_SHOW);
	return;
}

LONG FAR PASCAL ImageZoomWndProc(HWND hWnd, UINT Message, WPARAM wParam, LONG lParam)
{
	RECT	rect,screenrect;
	static	BOOL	needSaveScreen=TRUE;
	static	RECT	oldRect,newRect;

	switch (Message)
   {
		case WM_CREATE:
		{
			OSVERSIONINFO osvi;
			
			hWndImageZoom = hWnd;
			SetImageZoomSize (hWnd,0);

/*			ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			GetVersionEx(&osvi);
			if (osvi.dwMajorVersion > 9)
				needSaveScreen = FALSE;
			if (needSaveScreen)
			{
				HDC hDC = GetDC (0);
				RECT	wRect;

				GetClientRect(GetDesktopWindow(),&wRect);
				ImageZoomSavedScreen = SaveScreen (hDC, wRect);
				hDCImageZoom = CreateCompatibleDC (CurView->hDC);
				hOldBMImageZoom = SelectObject (hDCImageZoom,ImageZoomSavedScreen);
			}*/

		}

		break;

		case WM_DESTROY:
			hWndImageZoom = 0;
			if (ImageZoomSavedScreen)
			{
				SelectObject (hDCImageZoom,hOldBMImageZoom);
				GSSiDeleteObject (&ImageZoomSavedScreen);
				DeleteDC (hDCImageZoom);
			}
			DestroyDIB32 (hDibImageZoom,FALSE); 
			//GSSiDeleteObject (&hBMImageZoom);
			return DefWindowProc(hWnd, Message, wParam, lParam);
		break;

		case WM_LBUTTONUP:
		{
			HMENU	hMenu=CreatePopupMenu();   
			POINT	position;

			AppendMenu (hMenu,MF_ENABLED|(ImageZoomSize==ImageZoomSizeSmall?MF_CHECKED:0)|MF_STRING,65001,"Small");
			AppendMenu (hMenu,MF_ENABLED|(ImageZoomSize==ImageZoomSizeMedium?MF_CHECKED:0)|MF_STRING,65002,"Medium");
			AppendMenu (hMenu,MF_ENABLED|(ImageZoomSize==ImageZoomSizeLarge?MF_CHECKED:0)|MF_STRING,65003,"Large");
			AppendMenu (hMenu, MF_SEPARATOR, 0,0);
			AppendMenu (hMenu,MF_ENABLED|(ImageZoomShape==0?MF_CHECKED:0)|MF_STRING,65011,"Square");
			AppendMenu (hMenu,MF_ENABLED|(ImageZoomShape==1?MF_CHECKED:0)|MF_STRING,65012,"Round");
			//AppendMenu (hMenu,MF_ENABLED|ImageZoomShape==2?MF_CHECKED:0|MF_STRING,65013,"Use Viewport Proportions");
			AppendMenu (hMenu, MF_SEPARATOR, 0,0);
			AppendMenu (hMenu,MF_ENABLED|(ImageZoomBorder?MF_CHECKED:0)|MF_STRING,65021,"Display Border");
			AppendMenu (hMenu, MF_SEPARATOR, 0,0);
			AppendMenu (hMenu,MF_ENABLED|(ImageZoomOffset?MF_CHECKED:0)|MF_STRING,65031,"Offset");
			AppendMenu (hMenu, MF_SEPARATOR, 0,0);
			AppendMenu (hMenu,MF_ENABLED|MF_STRING,65041,"Exit");
			AppendMenu (hMenu, MF_SEPARATOR, 0,0);
			AppendMenu (hMenu,MF_ENABLED|MF_STRING,65040,"Cancel");
			GetCursorPos (&position);
			TrackPopupMenu (hMenu,TPM_LEFTBUTTON|TPM_CENTERALIGN|TPM_VCENTERALIGN,position.x,position.y,0,hWnd,0);
			DestroyMenu (hMenu); 
			SetCursorPos (position.x,position.y);
			ScreenToClient (hWndMain,&position);
			SendMessage (hWndMain,WM_MOUSEMOVE,0,MAKELPARAM(position.x,position.y));
		}
			break;

		case WM_COMMAND:
			{
		        switch(LOWORD(wParam))
				{
				case 65001:
					SetImageZoomSize (hWnd,ImageZoomSizeSmall);
					break;
				case 65002:
					SetImageZoomSize (hWnd,ImageZoomSizeMedium);
					break;
				case 65003:
					SetImageZoomSize (hWnd,ImageZoomSizeLarge);
					break;
				case 65011:
					ImageZoomShape=0;
					SetImageZoomSize (hWnd,0);
					break;
				case 65012:
					ImageZoomShape=1;
					SetImageZoomSize (hWnd,0);
					break;
				case 65013:
					ImageZoomShape=2;
					SetImageZoomSize (hWnd,0);
					break;
				case 65021:
					ImageZoomBorder = !ImageZoomBorder;
					SetImageZoomSize (hWnd,0);
					break;
				case 65031:
					ImageZoomOffset = !ImageZoomOffset;
					SetImageZoomOffset ();
					SetImageZoomSize (hWnd,0);
					break;
				case 65041:
					DestroyWindow (hWnd);
					break;
				}
			}
			break;
		case WM_RBUTTONUP:
			DestroyWindow (hWnd);
			break;

		case WM_PAINT:
		{
			PAINTSTRUCT	ps;
			HDC	hDC;

			_fmemset(&ps, 0x00, sizeof(PAINTSTRUCT));
            hDC = BeginPaint(hWnd, &ps);
			DisplayImageZoom(hWnd, hDC, imageZoomFrom);
            EndPaint(hWnd, &ps);
		}
			return 0;

/*		case  WM_WINDOWPOSCHANGING:
			{
				HDC		hDC = GetDC (hWnd);

				GetWindowRect(hWnd,&oldRect);
				DisplayImageZoom (hWnd,hDC,1);
				ReleaseDC (hWnd,hDC);
			}
			break;*/

		case  WM_WINDOWPOSCHANGED:
			{
				HDC		hDC = GetDC (hWnd);
				static	n=0;

				GetWindowRect(hWnd,&newRect);
				if (n++ == 100)
					n = 0;
				DisplayImageZoom (hWnd,hDC,imageZoomFrom);
				ReleaseDC (hWnd,hDC);
			}

			break;
		case WM_MOUSEMOVE:
			{
				POINT mousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
				POINT screenPoint = mousePoint;
				POINT mainClientPoint;
				RECT	wRect;
				int		x,y,w,h;

				SetViewport (ImageZoomVP);
				ClientToScreen (hWnd,&screenPoint);
				mainClientPoint = screenPoint;
				ScreenToClient (hWndMain,&mainClientPoint);

				if (PtInRect (&CurView->ScreenRect,mainClientPoint))
				{
					ShowWindow (hWnd,SW_SHOW);
					GetWindowRect(hWnd,&wRect);
					w = RECTWIDTH (&wRect);
					h = RECTHEIGHT (&wRect);
					x = (screenPoint.x + ImageZoomXoff) - w/2;
					y = (screenPoint.y + ImageZoomYoff) - h/2;

					//MoveWindow (hWnd,x,y,w,h,FALSE);
					SetWindowPos (hWnd,0,x,y,w,h,SWP_NOZORDER|SWP_NOOWNERZORDER);


				}
				else
					ShowWindow (hWnd,SW_HIDE);
			}
			break;

		case WM_ERASEBKGND:
			{
				HDC hDC=GetDC (hWnd);

				GetClientRect(hWnd,&rect);
				SetViewport (ImageZoomVP);
				FillRectPoly (hDC,&rect,CurView->BackGroundColor);
				ReleaseDC (hWnd,hDC);
			}
			return 1;

	}

	return DefWindowProc(hWnd, Message, wParam, lParam);
}


BOOL RegisterImageZoomClass(BOOL UnRegister)
{
    WNDCLASS  wc; 
    static	Called=FALSE;

	if (UnRegister)
	{
		 if (Called)
		 {
			 UnregisterClass(IMAGEZOOMWINDOWCLASS, hInst);
			 Called = FALSE;
		 }
		 return TRUE;
	}
    if (Called) return TRUE;
    Called = TRUE;
    wc.style = CS_SAVEBITS|CS_DROPSHADOW;
    wc.lpfnWndProc = (WNDPROC)ImageZoomWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = 0;//LoadCursor(NULL, IDC_HAND);
    wc.hbrBackground = 0;
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = IMAGEZOOMWINDOWCLASS;

    return (RegisterClass(&wc));
}

BOOL CreateImageZoomWindow (int VPID)
{
	POINT pt;
	int		x,y;
	HWND	hWnd;
	int		i;
	BOOL	On=TRUE;
	MNMXCORD	BitmapBounds,WBounds;

	if (hWndImageZoom)
		DestroyWindow(hWndImageZoom);

	if (VPID < 0)
	{
		HBITMAP hBitmap;
		BITMAPINFOHEADER DibInfo = { 0 };
		RECT clientRect;

		GetClientRect(CurView->hWnd, &clientRect);
		ImageZoomVP = -VPID;
		SetViewport(ImageZoomVP);
		hBitmap = SaveScreen(CurView->hDC,clientRect);
		hDibImageZoom = BitmapToDIB32(hBitmap);
		if (!GetBitmapInfoFromHandle(&DibInfo, hDibImageZoom))
			return FALSE;
		BitmapBounds.xmn = BitmapBounds.ymn = 0;
		BitmapBounds.xmx = DibInfo.biWidth - 1;
		BitmapBounds.ymx = DibInfo.biHeight - 1;
		goto HaveImage;
	}
	ImageZoomVP = VPID;
	SetViewport (ImageZoomVP);
	for (i=0;i<CurView->NumFiles;i++)
	{
		if (CurView->FileType[i] == 3)
		{
			if (!(hDibImageZoom = LoadDIB32 (CurView->lpFiles[i],TRUE, 0)))
				return FALSE;

			if (!GetImageBounds (CurView->lpFiles[i],hDibImageZoom,&BitmapBounds,&WBounds))
			{
				DestroyDIB32 (hDibImageZoom,FALSE); 
				return FALSE;
			}
			goto HaveImage;
		}
	}
	return FALSE;

HaveImage:
	ImageZoomWidth = BitmapBounds.xmx;
	ImageZoomHeight = BitmapBounds.ymx;
//	hDCImageZoom = CreateCompatibleDC (CurView->hDC);
//	hBMImageZoom = CreateCompatibleBitmap (CurView->hDC,ImageZoomWidth,ImageZoomHeight);
//	hOldBMImageZoom = SelectObject (hDCImageZoom,hBMImageZoom);
//	StretchDIBitsFromHandle (hDCImageZoom,0,0,ImageZoomWidth,ImageZoomHeight,0,0,ImageZoomWidth,ImageZoomHeight,
//	   						 hDib, (UINT)DIB_RGB_COLORS,SRCCOPY,1);
//	DestroyDIB32 (hDib,FALSE); 
	GetCursorPos (&pt);
	x = pt.x - ImageZoomWidth/2;
	y = pt.y - ImageZoomHeight/2;
	RegisterImageZoomClass (FALSE);
	hWnd = CreateWindowEx(0, IMAGEZOOMWINDOWCLASS, (LPSTR) NULL,WS_POPUP,
 								x, y, ImageZoomWidth,ImageZoomHeight,
								hWndMain, (HMENU) NULL, hInst, NULL);
	SystemParametersInfo(SPI_SETDROPSHADOW,0,&On,0);
  	
	ShowWindow (hWnd,SW_SHOW);
	return (hWnd != NULL);
}

BOOL ImageZoom (HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		imageZoomFrom = 2;
		CreateImageZoomWindow (CurView->ID);
   		break;
    
	case WM_LBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
		if (hWndImageZoom)
			PostMessage (hWndImageZoom,Message,wParam,lParam);
		break;

	case WM_RBUTTONUP:
		if (wParam)
			return FALSE;
		if (hWndImageZoom)
			DestroyWindow (hWndImageZoom);
		else
			CreateImageZoomWindow (CurView->ID);
		break;
	
	case GF_EXIT_VIEWPORT:
		if (hWndImageZoom)
			DestroyWindow(hWndImageZoom);
		break;

    case WM_MOUSEMOVE:
    {
		POINT mousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		POINT screenPoint = mousePoint;

		if (hWndImageZoom && PtInRect (&CurView->ScreenRect,mousePoint))
		{
			RECT	wRect;
			int	x,y,w,h;

			ClientToScreen (hWnd,&screenPoint);
			GetWindowRect(hWndImageZoom,&wRect);
			w = RECTWIDTH (&wRect);
			h = RECTHEIGHT (&wRect);
			SetImageZoomOffset ();
			x = (screenPoint.x + ImageZoomXoff) - w/2;
			y = (screenPoint.y + ImageZoomYoff) - h/2;
		//	MoveWindow (hWndImageZoom,x,y,w,h,FALSE);
			SetWindowPos (hWndImageZoom,0,x,y,w,h,SWP_NOZORDER|SWP_NOOWNERZORDER);
			ShowWindow (hWndImageZoom,SW_SHOW);
		}
		else if (hWndImageZoom)
			ShowWindow (hWndImageZoom,SW_HIDE);

	}
   		break;
   		
    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL ScreenZoom(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch (Message)
	{
	case GF_INIT:
		AddLBUTTON = FALSE;
		imageZoomFrom = 3;
		CreateImageZoomWindow(-CurView->ID);
		break;

	case WM_LBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
		if (hWndImageZoom)
			PostMessage(hWndImageZoom, Message, wParam, lParam);
		break;

	case WM_RBUTTONUP:
		if (wParam)
			return FALSE;
		if (hWndImageZoom)
			DestroyWindow(hWndImageZoom);
		else
			CreateImageZoomWindow(CurView->ID);
		break;

	case WM_MOUSEMOVE:
	{
						 POINT mousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
						 POINT screenPoint = mousePoint;

						 if (hWndImageZoom && PtInRect(&CurView->ScreenRect, mousePoint))
						 {
							 RECT	wRect;
							 int	x, y, w, h;

							 ClientToScreen(hWnd, &screenPoint);
							 GetWindowRect(hWndImageZoom, &wRect);
							 w = RECTWIDTH(&wRect);
							 h = RECTHEIGHT(&wRect);
							 SetImageZoomOffset();
							 x = (screenPoint.x + ImageZoomXoff) - w / 2;
							 y = (screenPoint.y + ImageZoomYoff) - h / 2;
							 //	MoveWindow (hWndImageZoom,x,y,w,h,FALSE);
							 SetWindowPos(hWndImageZoom, 0, x, y, w, h, SWP_NOZORDER | SWP_NOOWNERZORDER);
							 ShowWindow(hWndImageZoom, SW_SHOW);
						 }
						 else if (hWndImageZoom)
							 ShowWindow(hWndImageZoom, SW_HIDE);

	}
		break;

	default:
		return (FALSE);
	}
	return (TRUE);
}

BOOL CreateCompressedFenceFromBitmap (LPSTR File,HDIB32 hDib,LPSTR cColors)
{
	BOOL rtn = FALSE;
	UINT	nrow = FreeImage_GetHeight (hDib);
	UINT	ncol = FreeImage_GetWidth (hDib);
	UINT	irow,icol, i;
	HFILE	Fid;
	COLORREF color, colors[64];
	int	ncolors=0, icolor, nUnknown=0;
	LPSTR	pSC = cColors - 1;
	short	rowRepeat = -1;
	HANDLE	hFile;
	LPBYTE	pFile;
	int		loc=0, lFile, n;
	char	line[4096]="\t";
	char	headerFile[MAX_PATH];
	char	str[256];
	LPSTR	pLoc;

//	n = sizeof (mspFence);

	do {
		colors[ncolors++] = atoi (++pSC);
		pSC = strchr (pSC,';');
	}while (pSC);
	
	if (nrow && ncol)
	{
		Fid = GSSiOpenFile (File,0,OF_CREATE);
		if (Fid != HFILE_ERROR)
		{
			HANDLE	hOutArray = GSSiGlobAlloc(GAIDNO 1910,GMEM_MOVEABLE,USHRT_MAX);
			LPBYTE	pOutArray = GlobalLock (hOutArray);
			HANDLE	hLastRow = GSSiGlobAlloc(GAIDNO 1911,GHND,USHRT_MAX);
			LPBYTE	pLastRow = GlobalLock (hLastRow);

			for (irow = 0;irow < nrow;irow++)
			{
				LPBYTE	pOutLoc = pOutArray;
				LPBYTE	pCurColor = pOutLoc;
				LPSHORT	pRepeat = (LPSHORT)(pCurColor+1);
				short	lOutArray = 3;
				RGBTRIPLE	*p24Bit = (RGBTRIPLE	*)FreeImage_GetScanLine (hDib,irow);

				*pCurColor = 0;
				*pRepeat = 0;
				for (icol = 0;icol < ncol;icol++,p24Bit++)
				{
					color = COLORREFFromRGBTRIPLE (*p24Bit);
					icolor = 0;
					for (i=0;i<ncolors;i++)
						if (color == colors[i])
							icolor = i+1;

					if (!icolor)
					{
						nUnknown++;
						icolor++;
					}
					if (!icol)
						*pCurColor = icolor;
					if (*pCurColor == icolor)
						(*pRepeat)++;
					else
					{
						pCurColor = (LPBYTE)(pRepeat+1);
						*pCurColor = icolor;
						pRepeat = (LPSHORT)(pCurColor+1);
						*pRepeat = 0;
						lOutArray += 3;
					}
				}
				if (memcmp (pLastRow,pOutArray,lOutArray))
				{
					if (rowRepeat >= 0)
						BigWrite (Fid,&rowRepeat,2,-1);
					BigWrite (Fid,&lOutArray,2,-1);
					BigWrite (Fid,pOutArray,lOutArray,-1);
					rowRepeat = 0;
				}
				else
					rowRepeat++;
				memcpy (pLastRow,pOutArray,lOutArray);
			}
			BigWrite (Fid,&rowRepeat,2,-1);
			rtn = TRUE;
			lFile = GSSillseek (Fid,0,2);
			GSSillseek (Fid,0,0);
			hFile = GSSiGlobAlloc(GAIDNO 1912,GMEM_MOVEABLE,lFile);
			pFile = GlobalLock (hFile);
			BigRead (Fid,pFile,lFile);
			GSSiClose2 (&Fid);
			strcpy (headerFile,File);
			if ((pLoc = strrchr (headerFile,'.')))
				strcpy (pLoc,"Data.h");
			Fid = GSSiOpenFile (headerFile,0,OF_CREATE);
			sprintf (str,"\tint minGridX = 0, maxGridX = %i;",ncol);
			fputstring (str,Fid);
			sprintf (str,"\tint minGridY = 0, maxGridY = %i;",nrow);
			fputstring (str,Fid);
			fputstring ("\tunsigned char mspFence[]={",Fid);
			while (lFile > 0)
			{
				n = min (64,lFile);
				lFile -= n;
				for (i=0;i<n;i++)
					sprintf (strchr (line,0),"%i,",pFile[loc++]);
				if (!lFile)
					*LastChr (line) = 0;
				fputstring (line,Fid);
				strcpy (line,"\t");
			}
			fputstring ("\t};",Fid);
			GSSiClose2 (&Fid);
			GSSiGlobUlFree (&hFile);
			GSSiGlobUlFree (&hOutArray);
			GSSiGlobUlFree (&hLastRow);
		}
	}

	return rtn;
}

int PersonalStuff(LPSTR Arg1, LPSTR Arg2, LPSTR Arg3, LPSTR OutLoc)
{
	int st = 0;
	double total = 0;
	double video = 0;
	double amt;
	int num = 0;
	if (!stricmp(Arg1, "AMAZONBILL"))
	{
		HFILE fid = GSSiOpenFile(Arg2, 0, OF_READ);
		if (fid != HFILE_ERROR)
		{
			char line[256];
			while (fgetstring(line, 250, fid))
			{
				LPSTR ploc = strstr(line, "-$");
				if (ploc)
				{
					ploc += 2;
					amt = atof(ploc);
					total += amt;
					num++;
					continue;
				}
				ploc = strstr(line, "+$");
				if (ploc)
				{
					continue;
				}
				if (strstr(line, "Video"))
					video += amt;
			}
			GSSiClose(fid);
		}
	}
	return st;
}