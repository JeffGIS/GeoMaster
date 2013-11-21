#include "CONFIG.h"
#include "graphint.h" 
#include "dict.h"
#define WIN31
#include "commdlg.h"
#include "dlgs.h"
#include "cddemo.h"
#include "GW.h"     
#include "MCI.H"
#include "resource.h"
#include <dyndlg.h>
#include <math.h> 
#include <float.h>   
#include <mmsystem.h>    
#include <time.h>
#include "dibapi.h"  

long	TimeRangeBeg=0, TimeRangeEnd=LONG_MAX;
BLOWUPREC BlowUpRec;
HANDLE	hSymdb=0;
mnmxCor	CurrentItemMinMax;
HWND	VideoWnd;   
MNMXCORD	EditBounds;  
BOOL	ShowBaseLine=FALSE;
struct {char PREFIX[8], UDI[32];} TAGKey;       
extern		double      OrthoAdjustX, OrthoAdjustY;
HANDLE	hIntData=0;  
HANDLE	hPolyBuffer=0,hPolyPartLen=0;
short	nPoly=0, iPoly=0, nPolyPoints=0; 
BOOL	MoveOnlyPickedPoint=TRUE, ItemIsHighlighted=FALSE, CopyRec=FALSE;  
long	lCoords=0, nCoords=0;
long	DoNotPickThisRefno=LONG_MAX;
HANDLE	hCoords=0; 
long	FastPickFileNum, FastPickFII; 
long	ParcelLoc=0;
BOOL	FastPick=FALSE;
long	TempLineColor, TempFillColor, TempBackColor; 
BOOL	HighlightPointSym=FALSE,WantOnlyShapePoints=FALSE; 
long	TypeTime[100];

extern	float   PixelsPerHInch; 
extern	char	FontNames[8][32];
extern	long	GRStartTime, GREndTime;
extern	long	GlobalColors[10];
extern	double	MapAdjustX, MapAdjustY;
extern	short	MAXORTHOBUFS;
extern	HPEN	hOldPen, hSpecialPen;
extern	HBRUSH	hOldBrush,hSpecialBrush;
extern	HANDLE	hIntRefno, hPrefix, hUDI, hTEXT;
extern	long	PenCOLOR[8],AreaCOLOR[8];
extern	int		PenWIDTH[8];
extern	BOOL	FillAreas, MemMap, DisplaySymbol, SetRefno, DoTime;    
extern	long	InternalRefno;
extern	int		NumQuadSegs, QuadOff, NumDescBlocks, MapVersion;
extern	LONG	LenQuad, MaxQuadLevel, MaxQuadType,NumQuadSegsProcessed, DescBlockOffset;
extern	int		QuadLevelAt, QuadTypeNext;  
extern	unsigned	short	MaxDisplayPoints, MostPoints;
extern	short	ProcessGCmdStrings;  
extern	BOOL	ReorgFile;
extern	BOOL	InGRCmd; 
extern	BOOL	InGraphicsProcessor;
long	Numcont=0;    
time_t	CurrentChangeDate=0;
extern	long	CurStreetNumbers[4], FromStreet, ToStreet, CurStreet;
long	CurGCmdStringLoc=0,CurSNamesLoc=0,CurDescLoc=0, CurTextStringLoc=0,CurTAGLoc=0,CurTextHeaderLoc=0;  
short	CurlTAG=0;
HANDLE	hGCmdString=0, hTextString=0;
short	lGCmdString=0, lTextString=0;
char	RefIndexFile[128];
extern	short	CurrentType;  
long	ModPenLoc;
int		ModifyPen=-1;  
BOOL	Visible=TRUE, TextIsVisible=TRUE, DescIsVisible=TRUE, PickNET=FALSE;
int		LocateOpt=1;  
double	HPAreaOffset=-100;
WORD	nRead;
LPPOINT	lpPoints2;
mnmxCor	NewBounds;
HANDLE	hSavePoly=0,hSavePolyParts=0;
short	nSavePoly;   
int		NumSymVectors;
HANDLE	hSymVectors; 
DPOINT	PickPointBase;
POINT	PointLoc, BPLoc, MPLoc, EPLoc, TXLoc;
int		PointRot, TXFact,  TXFont, TXColor;  
double	TXRot, PTRot;
char	TXChar;  
long	JumpBackSeg=-1, JumpBackElement=-1;

typedef struct {
				double	AZM;
				double	Dist;
				} SYMBOLVECTOR;
typedef SYMBOLVECTOR	FAR *LPSYMBOLVECTOR;

void OpenRefIndex2 (BOOL Delete);

extern	long	MinFileTime, MaxFileTime;
extern	HFILE	ReorgfileFID;
extern	BOOL	CALLBACK EnumCtrlProc(HWND hCtrl,LONG lParam); 
extern	int		QuadTypeNext;
extern	long	lBlocksRead, nBlocksRead, nBlocksIn, nBlocksOut, TotDisplayTime;
extern	BOOL	DoGraphics, FoundInvalidRec,HaveVarFillColor;
extern	BOOL	UseRefOrTAGIndex;
extern	char	EditName[128],NewName[128];   
extern	int		FileProjectionType;
extern	BOOL	ForAllVis, ShowDeletedOpt;
extern	long	LastRef;
extern	char	OrthoDisplayName[32],PickedOrthoName[32];
extern	BOOL	PickOrtho;   
extern	HANDLE	hHighlightArea;   
extern	char	CurrentTAG[10], CurrentUDI[66];
extern	int		CurrentUDILen;   
extern	BOOL	GetMaskArea,OutlineZoomArea,MaskZoomArea,IgnoreBounds,AutoClearOffset,DisableMarginPan,MaskOffsetLine;
extern	mnmxCor	MinMax, *pMinMax;
extern	short	CurrentSymNum, CurrentSymParent;
extern	char	CurrentSymName[34], CurrentTAG[10];
extern	double	OffsetLineOffset;
extern	POINT	PickPoint;
extern	int		destX, destY, destW, destH, rect_width, rect_height;
extern	WORD 	bmX, bmY;
extern	HANDLE		hDibInfo, hImage;
extern	HBITMAP		hBitmap;
extern	LPBITMAPINFO	pDibInfo;
extern	char	FullBM[128];
extern	char	PMDataFile[128], PMMacroFile[128];
extern	float	ShadowPct, BorderPct;
extern	COLORREF	WindowColor; 
extern	HANDLE	hPDChunk;
extern	int		ShadowInc, SetVisByPar,MaxDimension;
extern	long	FirstMergeRecord, LastMergeRecord; 
extern	LPORTHO	CurOrtho;
extern	long	CurOrthoFrame;  
extern	int		OrthoID;
extern	HANDLE	hOrthos;  
extern	double	OrthoRes,FileDistToWinDist,BaseDistToWinDist;
extern	long	OrthoUse;
extern	int		TSize, nBytes, MaxBrush;
extern	HRGN	hRgn;
extern	LONG	pu; 
extern	LPINT	CurElementPnt;
extern	WORD	CurElement;
extern	BOOL	PickingByRefno, Pick, FileMode, FirstError, Display, SolidAreas, ParIsVisible, Highlight;
extern	int		PltType;
extern	LONG	CurrentSeg, ItemSeg, CurrentRefno;
extern	int		CurrentDesc, CurrentiPen;
extern	WORD	CurrentItem;
extern	int		FileNum;
extern	int		FileInIndex;
extern	RECT	MainRect;
extern	HANDLE	hRefIdx,hTAGIdx;
extern	REFINDEXDATA	RefIdxData;
extern	BOOL	NoRefIndex,ForceRefIndex,ForceTAGIndex;
extern	long	CurrentRefno;
extern	double FileDistToBaseDist;
extern	double	WindowToFileFactor;
extern	TAGBOX	TAGBox;
extern	HCURSOR	hCursor, OldCursor;
extern	char	ThemeDB[256]; 
extern	char	TagFile[144];
extern	int		QuadTypeNext;
extern	BOOL	DoPaint, HavePaint;
extern	HPEN	CurrentPen;
extern	HPEN	pens[MAXPENS], HighlightPen;
extern	HBRUSH	brushes[MAXPENS], HighlightBrush;
extern	int		maxbrush;
extern	char	PltName[128];
extern	char	CfgName[128];
extern	char	PickName[128];
extern	int		NumPicked, MaxPick;
extern	PICKDATA	PickList[MAXPICKITEMS];
extern	long	PickedStreets[MAXPICKITEMS][4];
extern	double		WidthFactor;
extern	HWND	hWndMain;
extern	HANDLE	hInst;
extern	MNMXCORD	ZoomBoxRect;
extern	BOOL        WindowIsZoomed;
extern	long		RefMktVal;
extern  HPEN		h0Pen, hRedPen;
extern	HBRUSH		hRedBrush, hGreenBrush, hBlueBrush, hBackBrush1;
extern	LPVISLIST	CurVis;
extern	LPVIEWPORT	CurView, pViewports[MAX_VIEWPORTS], pViewportsD[MAX_VIEWPORTS];
extern	HANDLE		hViewports[MAX_VIEWPORTS];
extern	int			NumViewports, NumViewportsToDisplay, DisplayViewID;
extern	LPTHEME		CurTheme;
extern	LPVOID		TranFileToBase, TranBaseToFile, TranBMToBase, TranBaseToBM;
extern	HANDLE		hTranFileToBase, hTranBaseToFile, hTranBMToBase, hTranBaseToBM;
extern	LPORTHO		CurOrtho;
extern	BOOL		Printing, PrintMerging;
extern	int			nPnts;
extern	int			PickAp;
extern	double		PickApW;
extern	BOOL		UpdateSegment;
extern	int			CommandViewport;
extern	BOOL		AddLBUTTON; 
extern	int		 	idTimer, Fid, FidConfig;
extern	long		UsedDescOffset,TranPointOffset, ColorPaletteOffset, GraphicsOffset, ContinuationOffset; 
extern	long		NewPrimeOffset;
extern	long		LenQuadSeg, LenQuad;
extern	long	QuadOffLoc;  
extern	short	StretchMode;  

extern	short	InvalidRecOpt;

void StartFastPick (void)
{
	FastPick = TRUE;
	FastPickFileNum = -1;
	return;
}

void EndFastPick (void)
{
	CloseMap();
	FastPick = FALSE;
}

void OpenTAGIndex (BOOL Delete)
{	BTVARDESC	BTVar[2];
	time_t ltime;
	char		TAGIndexFile[128];

	if (hTAGIdx && !Delete) return;
	if (PltType == 5) return;
	if (PltType < 4)
	{
		_fstrcpy (TAGIndexFile,PltName);  
		ExpandText (TAGIndexFile);
		TAGIndexFile[_fstrlen(TAGIndexFile)-3]=0;
		_fstrcat (TAGIndexFile,"tin");                  
    }
    else
    {   
    	char	Name[128];
    	
    	_fstrcpy (Name,PltName);
    	ExpandText (Name);
    	_fullpath (TAGIndexFile,Name,sizeof(TAGIndexFile));
    	*_fstrrchr (TAGIndexFile,'\\') = 0;
    	_fstrcat (TAGIndexFile,"\\tagindex.rin");
    }
    if (Delete && ForceTAGIndex)
    {   
    	OFSTRUCT OFStruct;
    	
    	GSSiOpenFile (TAGIndexFile,&OFStruct,OF_DELETE);
    	return;
    }
	ltime = 0;
	if (ForceTAGIndex)
	{
		hTAGIdx = BT_OPEN (TAGIndexFile, ltime, BT_WRITE, 0);
	}
	else
	{
		if (ExistFile (TAGIndexFile))
			hTAGIdx = BT_OPEN (TAGIndexFile, ltime, BT_READ, 0);
	}
	if (!hTAGIdx && ForceTAGIndex)
	{
		BTVar[0].BT_VARTYP=BT_CHAR;
		BTVar[0].BT_VARLEN=8;
		BTVar[0].BT_VAROFF=0;
		BTVar[1].BT_VARTYP=BT_CHAR;
		BTVar[1].BT_VARLEN=32;
		BTVar[1].BT_VAROFF=8;
		BT_CREATE (TAGIndexFile, 8, FALSE, 2, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
		hTAGIdx = BT_OPEN (TAGIndexFile, 0, BT_WRITE, 0);
	}
	return;
}

void BuildTAGIndex (LPSTR Prefix, LPSTR UDI, int len)
{   

	if (ForceTAGIndex)
	{	
		char str[34];
		_fstrncpy (str,UDI,len);
		str[len]=0;
		RefIdxData.FileInIndex = FileInIndex;
		RefIdxData.Segment = ItemSeg;
		RefIdxData.Offset = CurrentItem;    
		_fstrncpy(TAGKey.PREFIX,Prefix,8); 
		_fstrncpy(TAGKey.UDI,str,32);
		BT_PUT (hTAGIdx,(LPSTR)&TAGKey,(LPSTR)&RefIdxData);
	}
	return;
}

void BuildRefIndex (void)
{   int ii;

	if (ForceRefIndex)
	{	RefIdxData.FileInIndex = FileInIndex;
		RefIdxData.Segment = ItemSeg;
		RefIdxData.Offset = CurrentItem;  
		BT_PUT (hRefIdx,(LPSTR)&CurrentRefno,(LPSTR)&RefIdxData);
	}
	return;
}

void OpenRefIndex (BOOL Delete)
{
	if (NoRefIndex && !Delete) return; 
	OpenRefIndex2 (Delete);
	OpenTAGIndex (Delete);
	return;
}
void OpenRefIndex2 (BOOL Delete)
{	BTVARDESC	BTVar[1];
	time_t ltime;

	if (hRefIdx && !Delete) return;
	if (PltType == 5) return;
	if (PltType < 4)
	{
		_fstrcpy (RefIndexFile,PltName); 
		ExpandText (RefIndexFile);
		RefIndexFile[_fstrlen(RefIndexFile)-3]='\0';
		_fstrcat (RefIndexFile,"rin");
    }
    else
    {   
    	char	Name[128];
    	
    	_fstrcpy (Name,PltName);
    	ExpandText (Name);
    	_fullpath (RefIndexFile,Name,sizeof(RefIndexFile));
    	*_fstrrchr (RefIndexFile,'\\') = 0;
    	_fstrcat (RefIndexFile,"\\refindex.rin");
    }
    if (Delete && ForceRefIndex)
    {   
    	OFSTRUCT OFStruct;
    	
    	GSSiOpenFile (RefIndexFile,&OFStruct,OF_DELETE);  
    	return;
    }
	ltime = 0;
	if (ForceRefIndex)
	{   
		hRefIdx = BT_OPEN (RefIndexFile, ltime, BT_WRITE, 0);
	}
	else 
	{   
		if (ExistFile (RefIndexFile))
			hRefIdx = BT_OPEN (RefIndexFile, ltime, BT_READ, 0);
	}
	if (!hRefIdx && ForceRefIndex)
	{
		BTVar[0].BT_VARTYP=BT_INTEGER;
		BTVar[0].BT_VARLEN=4;
		BTVar[0].BT_VAROFF=0;
		BT_CREATE (RefIndexFile, 8, FALSE, 1, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
		ltime = 0;
		hRefIdx = BT_OPEN (RefIndexFile, ltime, BT_WRITE, 0);
	}
	return;
}

void CloseRefIndex ()
{   
	BT_CLOSE (hRefIdx);
	hRefIdx = 0;
	BT_CLOSE (hTAGIdx);
	hTAGIdx = 0;
	return;
}
 
long PickByRefno (long Refno,LPSTR Prefix, LPSTR UDI,short PickFile)
{   int	st, iview;
	BOOL RIOpened, SavePick;
	long	rtn;
	LPVIEWPORT	SaveView;
	LPVISLIST	SaveVis; 
	HANDLE		hVisList;
    
    SaveView = CurView;
    SaveVis = CurVis;

	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis (); 
	
	if (!FastPick)
	{
    	CloseMap();
 	    if (hRefIdx)
 	    	CloseRefIndex (); 
 	}
	UseRefOrTAGIndex=TRUE;
	for (iview=0;iview<NumViewports;iview++)
	{   
    	CurView = pViewports[iview];
    	if (CurView)      
	    for (FileNum=0;FileNum<CurView->NumFiles;FileNum++)
	    {    
    	 	 if (PickFile >= 0 && FileNum != PickFile)
    	 	 	goto NextFile;
	    	 CurView->CurFile = FileNum;
	    	 PltType = CurView->FileType[CurView->CurFile];
			 if (PltType<4)
			     _fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]);
	         else if (PltType == 4) 
	         {
	         	if (_fstrstr(CurView->lpFiles[CurView->CurFile],"FILELIST.TXT"))
	         		goto NextFile; /* not yet supported*/
	         	else
			    	_fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]);
			 }
	         else
	         	goto NextFile;
	        if (!FastPick || FileNum != FastPickFileNum || !hRefIdx)
	        {
	        	CloseRefIndex ();
	    		OpenRefIndex (FALSE);
	    	}
	    	if (Prefix)
	    	{   
	    		_fstrncpy(TAGKey.PREFIX,Prefix,8);
	    		_fstrncpy(TAGKey.UDI,UDI,32);
				st = BT_FIND (hTAGIdx,(LPSTR)&TAGKey,BT_FIRST,BT_EQ,(LPSTR)&RefIdxData);
	    	}
	    	else
				st = BT_FIND (hRefIdx,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&RefIdxData);
		    if (st && !FastPick)
		    	CloseRefIndex ();
			else
		    {
				PickingByRefno=TRUE; 
				SavePick = Pick;
				Pick = TRUE;
			    NumPicked = 0; 
			    PickList[NumPicked].View = CurView;
				PickList[NumPicked].FileNum = FileNum;  
				PickList[NumPicked].SubFile = CurView->SubFile;
				PickList[NumPicked].FileInIndex = RefIdxData.FileInIndex;
				PickList[NumPicked].Segment = RefIdxData.Segment;
				PickList[NumPicked].Refno = Refno;
				PickList[NumPicked].Desc = 0;
				if (!FastPick)
					CloseRefIndex ();
				PickList[NumPicked].Offset = RefIdxData.Offset; 
				PickList[NumPicked].Element = 0;
				if (!PickedItemMinMax (NumPicked,&PickList[NumPicked].Rect))
					goto NextFile;
			    NumPicked = 1;
				PickList[NumPicked-1].FileInIndex = RefIdxData.FileInIndex;
				PickList[NumPicked-1].Segment = RefIdxData.Segment;
				PickList[NumPicked-1].Offset = RefIdxData.Offset; 
				PickList[NumPicked-1].Type = 2; 
				if (CurrentType == GF_AREA)
					PickList[NumPicked-1].Type = 3; 
				else if (CurrentType == GF_POINT)
					PickList[NumPicked-1].Type = 1; 
				else if (CurrentType == GF_TEXT)
					PickList[NumPicked-1].Type = 4; 
				rtn = CurrentRefno;  
				PickingByRefno=FALSE;
				Pick = SavePick;
				goto Exit;
			}
	NextFile:;
		}
    }
	CurView = SaveView;
	rtn = FALSE;
Exit:                       
	CurView = SaveView;
	CurVis = SaveVis;
	GlobalUnlock (hVisList);	
	GlobalFree (hVisList);
	UseRefOrTAGIndex=FALSE;
	return (rtn);
} 

long GetMaxRefno (int PickFile)
{   int	st, iview;
	BOOL RIOpened, SavePick;
	long	MaxRef = TMPRF$, Refno;
	LPVIEWPORT	SaveView;
	LPVISLIST	SaveVis; 
	HANDLE		hVisList;
    
    SaveView = CurView;
    SaveVis = CurVis;

	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis (); 
	
    CloseMap();
    if (hRefIdx)CloseRefIndex (); 
	UseRefOrTAGIndex=TRUE;
	for (iview=0;iview<NumViewports;iview++)
	{   
    	CurView = pViewports[iview];
    	if (CurView)      
	    for (FileNum=0;FileNum<CurView->NumFiles;FileNum++)
	    {    
    	 	 if (PickFile >= 0 && FileNum != PickFile)
    	 	 	goto NextFile;
	    	 CurView->CurFile = FileNum;
	    	 PltType = CurView->FileType[CurView->CurFile];
			 if (PltType<4)
			     _fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]);
	         else if (PltType == 4) 
	         {
	         	if (_fstrstr(CurView->lpFiles[CurView->CurFile],"FILELIST.TXT"))
	         		goto NextFile; /* not yet supported*/
	         	else
			    	_fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]);
			 }
	         else
	         	goto NextFile;
	    	OpenRefIndex (FALSE);
			if (!BT_FIND (hRefIdx,(LPSTR)&Refno,BT_LAST,BT_ANY,(LPSTR)&RefIdxData))
				MaxRef = max (MaxRef,Refno);
	    	CloseRefIndex ();
	NextFile:;
		}
    }
	CurView = SaveView;
	CurVis = SaveVis;
	GlobalUnlock (hVisList);	
	GlobalFree (hVisList);
	UseRefOrTAGIndex=FALSE;
	return (MaxRef);
} 

BOOL ProcessGraphicsRec (HDC hDC, LPINT ipnt,LPSTR BeginSeg)
{	int		idesc, ItemLen, ipen;
	LPLONG	pRefno;
	long	remlen;
	LPBYTE	Pcode;
	int		x1,y1,x2,y2;  
	DPOINT	dPoint;
	LPSTR	lpTAG, pTXChar, lpColon;  
	static	HPEN	hTempPen=0;
	static	HBRUSH	hTempBrush=0;
	LPPOINT lpPoints; 
	int		ltag, len;
	char	str[32];
	int		TempPattern, TempWidth, TempStyle,ii; 
	clock_t	starttime, endtime, starttime2;
	LPFLOAT	pSize, pRot;  
	static	BOOL	AlreadyProcessed;
	HANDLE	hMem=0; 
	long	TotBlockLen=0, LastItemLen=0, LastItemLenActual; 
	WORD	LastCurrentItem=0; 
	static	long	StartElement=-1; 
	static	BOOL	Deleted=FALSE;   
	static	long	debugrefno=-97891940; 
	HPSTR	pCoords;  
	static	POINT	LinkPoint;
	POINT	WinPoint;
	LPSHORT	pPartLen;
	BOOL	rtn=FALSE; 
	static	BOOL	SkipToNextHeader=FALSE; 
	static	HPEN	SaveCurrentPen=0;
	
    if (DoTime)
    	starttime=GetTickCount();
	InGraphicsProcessor = TRUE;
	ShowValue (hDC,TRUE);
	if (!Pick && hDC && CurrentPen)
	{
        SelectObject(hDC, CurrentPen);
	}   
		if (StartElement > 0)
			ipnt += StartElement;
		StartElement = -1;
        while (*ipnt != 0)
        {   
        	if (DoTime)
        		starttime2=GetTickCount();
//        	starttime2 = clock();  this call alone slows system by 150%
        	Pcode = (LPBYTE) ipnt;
        	ipnt++;
        	CurElementPnt = ipnt;
			CurElement=(LPSTR) ipnt - (LPSTR)(BeginSeg +2);
            
            if (SkipToNextHeader && (*Pcode != 12 && *Pcode != 92 && Pcode != 13))
				SkipSubRec (Pcode,&ipnt);            
            else if (CopyRec)
            	CopySubRec (Pcode,&ipnt);
            else
		    switch (*Pcode)
	        {   
	            case 2: /* pen up */
		 		{
					ipnt+=2;
					pu++;
				}
	            break;

	            case 3: /* set pen */
		 		{   ipen = *ipnt;   
		 			CurrentiPen = ipen;
		 			ipnt++;
		 			CurrentPen = pens[ipen]; 
		 			if (!CurrentPen)
		 				CurrentPen = pens[0];
				}
	            break;

	            case 4: /* put line */
		 		{
					nPnts = 2;
		 		    lpPoints2 = (LPPOINT) ipnt;
 		    		ipnt = ipnt + nPnts * 2;  
 		    		CurrentType = GF_LINE;
 		    		goto ProcessPolyLine;
				}
	            break;

	            case 5: /* put area */

		 		{   ipen = *ipnt; 
		 			CurrentiPen = ipen;
		 			ipnt++;
		 		    nPnts = *ipnt;
		 		    ipnt++;
		 		    lpPoints2 = (LPPOINT)ipnt;
		 		    ipnt = ipnt + nPnts * 2;
		 		    CurrentType = GF_AREA;  
		 		    
				    if (hCoords) 
				    {
				    	nCoords += nPnts;
				    	hCoords = GlobalReAlloc (hCoords,(long)nCoords*4,GMEM_MOVEABLE);
					    pCoords = GlobalLock (hCoords);
					    pCoords += lCoords;
					    BufWrite (&pCoords,&lCoords,lpPoints2,(long)nPnts*4);  
					    GlobalUnlock (hCoords);
					    lpPoints2 = GlobalLock (hCoords);
					    nPnts = nCoords;
				    }
		 		    
		 		    
		 		    if (Visible)
		 		    {   HPEN hSavePen=0;
		 		    
						if (!Pick && hDC)
						{	
							if (!hTempPen || !CurVis->WantType[1] || nPoly)
								hSavePen = SelectObject(hDC, h0Pen);
							else
				        		hOldPen = SelectObject (hDC,hTempPen);
							 
							if (!hTempBrush) 
							{
								if (ipen) 
								{
									if (brushes[ipen])
										SelectObject(hDC, brushes[ipen]); 
									else
										SelectObject(hDC, hRedBrush);
								}
							} 
							else
				        		hOldBrush = SelectObject (hDC,hTempBrush);
							
						}
						if (nPoly)
						{
							HPPOINT	lpPoints3; 
							long	lbuf=0;
							
							pCoords = GlobalLock (hPolyBuffer);
							pCoords += (nPolyPoints*sizeof(POINT));
							pPartLen = GlobalLock (hPolyPartLen);
							pPartLen+=iPoly;
							*pPartLen = nPnts;
							GlobalUnlock (hPolyPartLen); 
							BufWrite (&pCoords,&lbuf,lpPoints2,(long)nPnts*sizeof(POINT)); 
							if (!iPoly)
								LinkPoint = *lpPoints2;
							else
							{
								lpPoints3 = pCoords;
								*lpPoints3 = LinkPoint;
								nPolyPoints++; 
							}
							nPolyPoints += nPnts; 
							iPoly++;   
							GlobalUnlock (hPolyBuffer);
						}           
						if (iPoly == nPoly)
						{    
							if (nPoly)
							{
								lpPoints2 = GlobalLock (hPolyBuffer);    
								nPnts = nPolyPoints;
							}
							if (FileMode)
							{	
								HANDLE	hPoints; 
								LPPOINT	lpPoints3; 
								int		i;
								
							    hPoints = GSSiGlobAlloc (GMEM_MOVEABLE,(long)nPnts*sizeof(POINT));
							 	lpPoints3 = (LPPOINT) GlobalLock (hPoints);
							 	lpPoints = lpPoints3;
								for (i=0;i<nPnts;i++,lpPoints++,lpPoints2++)
								{   
									dPoint = FilePtToBasePt (*lpPoints2);
									*lpPoints = BasePtToWinPt (dPoint);
								}
								if (ItemInRegion (hRgn,lpPoints3, nPnts))
								{	if (SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
										GWPolygon (hDC,lpPoints3,nPnts);
								}
								GlobalUnlock (hPoints);
								GlobalFree (hPoints);
							}
							else 
							{
								if (Pick)
									PickPolygon (lpPoints2,nPnts,9999999);
								else
								{	if (SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
									{
										if (FillAreas)
										{
											GWPolygon (hDC,lpPoints2,nPnts);  
											if (hTempPen && CurVis->WantType[1] && nPoly)
											{   
												short	i;
												
								        		hOldPen = SelectObject (hDC,hTempPen);
												pPartLen = GlobalLock (hPolyPartLen);
								        		for (i=0;i<nPoly;i++)
								        		{
													GWPolyline (hDC,lpPoints2,*pPartLen); 
													lpPoints2+=*pPartLen++;
													if (i)
														lpPoints2++;
								        		}   
								        		GlobalUnlock (hPolyPartLen);
								        	}
										}
										else
											GWPolyline (hDC,lpPoints2,nPnts);
									}
								}
							}
							if (hPolyBuffer)
							{
								GlobalUnlock (hPolyBuffer);  
								GSSiGlobFree (&hPolyBuffer);
								GlobalUnlock (hPolyPartLen);  
								GSSiGlobFree (&hPolyPartLen);
								nPoly = 0;
							}
						}
						if (hCoords)
						{
							GlobalUnlock (hCoords);
							GSSiGlobFree (&hCoords);
						}
						if (hSavePen) SelectObject(hDC, hSavePen);
					}
				}
				break;

	            case 6: /* put polyline */

		 		{   nPnts = *ipnt;
		 		    ipnt++;
		 		    lpPoints2 = (LPPOINT) ipnt;
		 		    ipnt = ipnt + nPnts * 2;
		 		    CurrentType = GF_POLYLINE;
				    if (hCoords) 
				    {
				    	nCoords += nPnts;
				    	hCoords = GlobalReAlloc (hCoords,(long)nCoords*4,GMEM_MOVEABLE);
					    pCoords = GlobalLock (hCoords);
					    pCoords += lCoords;
					    BufWrite (&pCoords,&lCoords,lpPoints2,nPnts*4);  
					    GlobalUnlock (hCoords);
					    lpPoints2 = GlobalLock (hCoords);
					    nPnts = nCoords;
				    }
ProcessPolyLine:    if (TSize > 0) CurrentType = GF_TEXT;
					if (!Pick && hDC)
					{
						if (hTempPen)
						{   
							SaveCurrentPen = CurrentPen;
							CurrentPen = hTempPen;
//				       		hOldPen = SelectObject (hDC,hTempPen);
				       	}
				    }
					if (Visible & !AlreadyProcessed)
		 		    {
						if (FileMode)
						{	
							HANDLE	hPoints; 
							LPPOINT	lpPoints3; 
							int		i;
							
						    hPoints = GSSiGlobAlloc (GMEM_MOVEABLE,(long)nPnts*sizeof(POINT));
						 	lpPoints3 = (LPPOINT) GlobalLock (hPoints);
						 	lpPoints = lpPoints3;
							for (i=0;i<nPnts;i++,lpPoints++,lpPoints2++)
							{
								dPoint = FilePtToBasePt (*lpPoints2);
								*lpPoints = BasePtToWinPt (dPoint);
							}
							if (ItemInRegion (hRgn,lpPoints3,nPnts))
					 		    if (SetDisplayChar (hDC,GF_POLYLINE,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
									GWPolyline (hDC,lpPoints3,nPnts); 
							GlobalUnlock (hPoints);
							GlobalFree (hPoints);
						}

						else 
						{
							if (Pick)
								PickPolyline (lpPoints2,nPnts);
							else
							{
								if (SetDisplayChar (hDC,CurrentType,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
								GWPolyline (hDC,lpPoints2,nPnts);
							}
						} 
						if (hCoords)
						{
							GlobalUnlock (hCoords);
							GSSiGlobFree (&hCoords);
						}
					}
				}
				break;

				case 7: /* block minmax */
				{	pMinMax = (LPMINMAX) ipnt;
					ipnt += 4;
					if (!BlockInWindow (*pMinMax)) *ipnt = 0;
                }
                break;

                case 8:	/*	description */
		 		{   idesc = *ipnt;
		 			CurrentDesc = idesc;
	    			CurDescLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
		 			ipnt++;
		 			DescScan(idesc,0);
                	DescIsVisible = GetVisibility (idesc);
                	if (DescIsVisible && TextIsVisible)
                		Visible=TRUE;
                	else
                		Visible=FALSE;
                }
                break;

                case 9:	/*	refno	*/
                {   
	    			CurTAGLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
                	pRefno =(LPLONG) ipnt;
                	CurrentRefno = *pRefno; 
                	if (CurrentRefno==debugrefno)
                		ii=1; 
                	ipnt += 2; 
                	SetIntRefno (CurrentRefno);
					LastRef=LONG_MIN;  
                	BuildRefIndex (); 
                	ltag = *++Pcode;   
                	CurlTAG = ltag;
                	if (ltag) 
                	{   LPSTR	pTAGList;
                	
                		lpTAG = (LPSTR)ipnt;
                		lpColon = _fstrchr (lpTAG,':');    
                		*lpColon = 0;
						SetGlobalValue2 (hPrefix,lpTAG,0); 
						_fstrcpy(CurrentTAG,lpTAG);
						len = ltag - (lpColon - lpTAG + 1);
						SetUDIValueLen (lpTAG,++lpColon,len); 
						SetGlobalValueLen (lpTAG,lpColon,len); 
						CurrentUDILen = len;
						_fstrncpy (CurrentUDI,lpColon,len);
						CurrentUDI[len]=0;
						BuildTAGIndex (lpTAG,lpColon,len);
						ipnt = (LPINT) (lpTAG + ltag + ltag%2);  
						if (CurView->hTAGList)
						{
							pTAGList = GlobalLock(CurView->hTAGList);
							while (*pTAGList)
							{
								if (!_fstricmp (lpTAG,pTAGList)) goto TAGOut;
								pTAGList+=10;
							
							}
							_fstrcpy (pTAGList,lpTAG);
					TAGOut:	GlobalUnlock (CurView->hTAGList); 
						}
	                 	if (lpColon)
	                 		*(--lpColon) = ':';   
                	}
                	else
                	{   
						_fstrcpy(CurrentTAG,"REFNO");
						SetGlobalValue2 (hPrefix,"REFNO",0); 
						if (SetRefno)  
							sprintf (str,"%.2f",(((double)CurrentRefno) - ZERO$)/100); 
						else
							*str = 0;
						SetGlobalValue2 (hUDI,str,0);
						CurrentUDILen = _fstrlen(str);
						_fstrcpy (CurrentUDI,str);
                 	}
                }
                break;    
                
	            case 10: /* street number */
		 		{   
	    			CurSNamesLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
		 			_fmemmove (CurStreetNumbers,(LPLONG)ipnt,16); 
		 			if (CurStreetNumbers[0])
		 				ii=1;
					ipnt+=(2*4);
				}
	            break;
                
                case 92: /* deleted item */
                	Deleted = TRUE; 
                	goto ProcessHeader;
				case 12: /* item minmax */
				{	
					Deleted = FALSE;
	ProcessHeader:
					SkipToNextHeader=FALSE;
					ShowValue (hDC,FALSE);
					pMinMax = (LPMINMAX)ipnt;
					CurrentItemMinMax = *pMinMax;
					ItemSeg = CurrentSeg;
					CurrentItem=(LPSTR) ipnt - (LPSTR)(BeginSeg +2); 
					LastItemLenActual = CurrentItem - LastCurrentItem; 
					if (LastItemLenActual != LastItemLen)
						ii=1;
					LastCurrentItem = CurrentItem;
					ipnt += 4; 
					if (*ipnt < 0)  
						CurrentChangeDate=1;
					else
						CurrentChangeDate=0;
					ItemLen = abs(*ipnt);
					ipnt++;  
					LastItemLen = (ItemLen + 6)*2;
					TotBlockLen += LastItemLen;    
					TSize = 0;  
					_fmemset (CurStreetNumbers,0,16);
					HaveVarFillColor=ItemIsHighlighted=FromStreet=ToStreet=CurStreet=CurTextHeaderLoc=CurGCmdStringLoc=CurTextStringLoc=CurTAGLoc=CurSNamesLoc=nPoly=iPoly=GRStartTime=GREndTime=0;
					_fmemset (CurStreetNumbers,0,16);
					GSSiGlobFree (&hPolyBuffer);   
               		Visible=DescIsVisible=TextIsVisible=TRUE;
					AlreadyProcessed=FALSE;
					if (ReorgfileFID)
					{   
						if (!Deleted)
							ReorgOut2 (Pcode,ItemLen*2 + 12);
						remlen = nBytes - ((LPSTR) ipnt - BeginSeg); 
						if (ItemLen >= remlen)
							ipnt++;
						ipnt+=ItemLen;
					}
					else if (Deleted==ShowDeletedOpt || !BlockInWindow (*pMinMax))
					{	
						remlen = nBytes - ((LPSTR) ipnt - BeginSeg); 
						if (ItemLen >= remlen)
							ipnt++;
						ipnt+=ItemLen;
                        nBlocksOut++;  
                        if (!ItemLen)
                        	SkipToNextHeader=TRUE;
					}
					else
					{
			 			CurrentiPen = *++Pcode;
			 			if (CurrentiPen)
			 				CurrentPen = pens[CurrentiPen];
			 			if (!CurrentPen)
			 				CurrentPen = pens[0];   
			 			nBlocksIn++;
			 		}
                }
                break;

				case 13: /* continuation offset */
				{	
					ShowValue (hDC,FALSE);
					LastItemLenActual = (LPSTR) ipnt - (LPSTR)(BeginSeg +2) - LastCurrentItem; 
					if (LastItemLenActual != LastItemLen)
						ii=1;
					ContinuationOffset = *(LPLONG)ipnt; 
					if (ContinuationOffset >=0)  
					{
						Numcont++; 
						rtn = TRUE; 
					} 
					else
						SkipToNextHeader=FALSE;
				/*	ContinuationOffset=-1;*/
					ipnt += 2;
					*ipnt = 0;
					TotBlockLen += 2; 
                }
                break;

				case 14: /* text size */
				{	TSize = *ipnt;
					ipnt++;
                	TextIsVisible = GetTextVisibility (TSize);
                	if (DescIsVisible && TextIsVisible)
                		Visible=TRUE;
                	else
                		Visible=FALSE;
                }
                break;

				case 15: /* point coordinates */
				{	PointLoc.x = *ipnt++;
					PointLoc.y = *ipnt++;
					WinPoint = FileCoordToWinCoord(PointLoc); 
					PointRot = *ipnt++;
					if (Pick & Visible)
					{
						AlreadyProcessed=TRUE;
						PickPointItem (PointLoc,1,0,CurrentDesc);			 		    
					} 
					else if (!DisplaySymbol & Visible)
					{
						AlreadyProcessed = TRUE;
						DisplayPointItem (hDC,WinPoint,1,PointRot/1000,CurrentDesc);
					}
                }
                break;

				case 16: /* line coordinates */
				{	
		 		    lpPoints2 = (LPPOINT) ipnt;
					BPLoc.x = *ipnt++;
					BPLoc.y = *ipnt++;
					EPLoc.x = *ipnt++; 
					EPLoc.y = *ipnt++; 
		 		    nPnts = 2; 
		 		    goto ProcessBaseRec;
                }
                break;

				case 17: /* curve coordinates */
				{	
					DPOINT	BP,POC,EP; 
					int		st;
					
					BP.x = *ipnt++;
					BP.y = *ipnt++;
					POC.x = *ipnt++; 
					POC.y = *ipnt++; 
					EP.x = *ipnt++; 
					EP.y = *ipnt++; 
		ProcessBaseRec: 
					if (Visible && (Pick || !DisplaySymbol))  
					{    
						if (*Pcode == 17)
						{
				 		    hMem = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
				 		    lpPoints2 = GlobalLock (hMem);   
				 		    lpPoints = lpPoints2;
				 		    nPnts = 0;
		                    st =  CurvePoints(&BP,&POC,&EP, &nPnts,  &lpPoints);
					    }
						AlreadyProcessed=TRUE; 
						if (Pick)
							PickPolyline (lpPoints2,nPnts);	
						else		 		    
							GWPolyline (hDC,lpPoints2,nPnts);
					} 
					GSSiGlobFree (&hMem);
                }
                break;

				case 18: /* text character */
				{	HFONT	hFont, OldFont; 
					int		fheight, fwidth, angle, weight, nchar;
					BOOL	italic;
				
					TXLoc.x = *ipnt++;
					TXLoc.y = *ipnt++;
					TXFact  = *ipnt++; 
					TXRot   = *ipnt++; 
					TXFont  = HIBYTE(*ipnt); 
					TXColor = LOBYTE(*ipnt++); 
					nchar	= *ipnt++;
					pTXChar = (LPSTR)ipnt; 
					ipnt    += (nchar+1)/2; 
					italic = FALSE;
					weight = FW_MEDIUM;
					fheight = TXFact + 3200;
					fwidth  = 0.80 * fheight;
					angle  = ((((double) TXRot / 1000 ) + PY)/RADDEG)*10-900;
					hFont = CreateFont (fheight,0,angle,angle,weight,italic,FALSE,FALSE,ANSI_CHARSET,OUT_TT_PRECIS,
								CLIP_CHARACTER_PRECIS|CLIP_LH_ANGLES,PROOF_QUALITY,/*FIXED_PITCH|*/FF_DONTCARE, 
								"Arial");
					
					OldFont = SelectObject (hDC,hFont);
					TextOut (hDC,TXLoc.x,TXLoc.y,pTXChar,nchar); 
					SelectObject (hDC, OldFont);
					DeleteObject (hFont);
                }
                break;  
                
				case 19: /* grahpics text */
				{	HFONT	hFont, OldFont; 
					int		fheight, nchar, angle, weight, ltext, twidth, theight, xmove, ymove;
					BOOL	italic; 
					LPGRTEXTHEADER	pGRTextHeader;
					LPSTR	pText; 
					HANDLE	hText;
					LPSTR	pText2;
					LOGFONT	LogFont;    
					DWORD	TextExt;   
					DPOINT	pt, pt2;    
					double	THeight;
					short	x,y,TSize;   
					COLORREF	OldColor; 
					short	FontWeights[4]={FW_THIN,FW_NORMAL,FW_BOLD,FW_HEAVY};
				    
 		    		CurrentType = GF_TEXT;
				    pGRTextHeader = ipnt;  
   	    			CurTextHeaderLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
				    ipnt += sizeof(GRTEXTHEADER) / 2;  
				    nchar = pGRTextHeader->lText;  
				    pText = ipnt;   
				    ipnt += nchar/2;
				    TXRot = LTWOPI(-PTRot); 
				    if (CurView->PassID || Pick)
				    {
					    hText = GSSiGlobAlloc (GMEM_MOVEABLE,512); 
					    pText2 = GlobalLock (hText);
					    _fstrncpy (pText2,pText,nchar);  
						SetGlobalValue2 (hTEXT,pText2,0);
						_fmemset (&LogFont,0,sizeof(LOGFONT));
						switch (pGRTextHeader->HeightPrecision)
						{
							case 0:  
								THeight = (double)pGRTextHeader->Height;
								break;
							case 1:
								THeight = (double)pGRTextHeader->Height/10;
								break;
							case 2:
								THeight = (double)pGRTextHeader->Height/100;
								break;
							case 3:
								THeight = (double)pGRTextHeader->Height/1000;
								break;
						}
						LogFont.lfHeight = -IDNINT(THeight * BaseDistToWinDist);  
						TSize = IDNINT((double)-LogFont.lfHeight / PixelsPerHInch); 
	                	TextIsVisible = GetTextVisibility (TSize);
	                	if (DescIsVisible && TextIsVisible)
	                		Visible=TRUE;
	                	else
	                		Visible=FALSE;
					    if (Visible && (Display || Pick))
					    {   
	                		POINT	RectPoints[4];
					    	TEXTMETRIC	TextMet;
					    	
						    SaveDC (hDC);
						    if (Pick)
							{
								LogFont.lfHeight = -IDNINT(THeight / FileDistToBaseDist);  
								SetDisplayMode (CurView->hDC,GF_MAPMODE);
								TXRot = -TXRot;
							}	
							else 
						    {
								SetDisplayMode (hDC,GF_TEXTMODE);
								if (pGRTextHeader->Opaque)
									SetBkMode (hDC,OPAQUE);
								else 
									SetBkMode (hDC,TRANSPARENT);
							}	 
							angle  = IDNINT(3600-((TXRot/RADDEG)*10));
							LogFont.lfEscapement = angle;   
							LogFont.lfWeight = FontWeights[pGRTextHeader->Weight];    
							LogFont.lfOutPrecision = OUT_TT_PRECIS;
							LogFont.lfQuality = PROOF_QUALITY; 
							_fstrcpy (LogFont.lfFaceName,FontNames[pGRTextHeader->FontNum]);
							hFont = CreateFontIndirect((LPLOGFONT)&LogFont);
							OldFont = SelectObject (hDC,hFont);
							GetTextMetrics (hDC,&TextMet); 
							ltext = _fstrlen(pText2);
							TextExt = GetTextExtent (hDC,pText2,ltext);
							twidth = LOWORD(TextExt);
							theight = HIWORD(TextExt); 
							switch (pGRTextHeader->hJust)
							{
								case 0:
									xmove = 0;
									break;
								case 1:
									xmove = -twidth/2;
									break;
								case 2:
									xmove = -twidth;
									break;            
							}  
							switch (pGRTextHeader->vJust)
							{
								case 0:
									ymove = theight - TextMet.tmDescent;
									break;
								case 1:
									ymove = theight;
									break;
								case 2:
									ymove = theight/2;
									break;
								case 3:
									ymove = 0;
									break;    
							}
							if (!Pick)
							{ 
								ymove = -ymove;
								pt.x = TXLoc.x;
								pt.y = TXLoc.y;
							}
							else
							{
								pt.x = TXLoc.x/CurView->FileFactor;
								pt.y = TXLoc.y/CurView->FileFactor; 
							}  
                            
                            if (ShowBaseLine)
                            {
                            	DPOINT pt3;
                            	POINT	p[2];
                            	
                            	p[0]=TXLoc;
								pt3 = dnewpt (pt,TXRot,twidth);  
								p[1].x = IDNINT(pt3.x);
								p[1].y = IDNINT(pt3.y); 
								Polyline (hDC,p,2);
							} 
							
							pt2 = dnewpt (pt,TXRot,xmove);  
							pt = dnewpt (pt2,TXRot+HALFPI,ymove);
							x = IDNINT(pt.x);
							y = IDNINT(pt.y);
		                    if (!Pick && ItemIsHighlighted)
		                    {
								OldColor = SetTextColor (hDC,RGB(255,0,0));
							//	SetBkMode (hDC,OPAQUE); 
							//	SetBkColor (hDC,RGB(255,0,0)); 
							}
							if (Pick)
							{   
								POINT	RectPoints[4];
								
								RectPoints[0].x = x;
								RectPoints[0].y = y;
								pt2 = dnewpt (pt,TXRot,twidth);
								RectPoints[1].x = IDNINT(pt2.x);  
								RectPoints[1].y = IDNINT(pt2.y);  
								pt = dnewpt (pt2,TXRot+HALFPI,-theight);
								RectPoints[2].x = IDNINT(pt.x);  
								RectPoints[2].y = IDNINT(pt.y);  
								pt2 = dnewpt (pt,TXRot,-twidth);
								RectPoints[3].x = IDNINT(pt2.x);  
								RectPoints[3].y = IDNINT(pt2.y);  
								PickPolygon (RectPoints,4,9999999);							
//								Polygon (hDC,RectPoints,4);							
							}
							else
							{
								TextOut (hDC,x,y,pText2,ltext);    
			                    if (ItemIsHighlighted)
									OldColor = SetTextColor (hDC,OldColor); 
							}
							SelectObject (hDC, OldFont);
							DeleteObject (hFont);  
							RestoreDC (hDC,-1);
						} 
						else
					    {   
					    	LPSTR	pString;
					    	
			    			CurTextStringLoc = CurrentSeg + (long)((LPSTR) pGRTextHeader - (LPSTR)BeginSeg); 
							lTextString = nchar; 
							GSSiGlobFree (&hTextString);   
							hTextString = GSSiGlobAlloc (GMEM_MOVEABLE,512);
							pString = GlobalLock (hTextString);
							_fstrcpy (pString,pText2);
							GlobalUnlock (hTextString);
					    }
						GlobalUnlock (hText);
						GlobalFree (hText); 
					}
                }
                break;  
                
	        	case 20:	/* point symbol */    
	        		pSize = (LPFLOAT)ipnt;
	        		ipnt += 2;
	        		pRot = (LPFLOAT)ipnt;    
	        		PTRot = *pRot;
	        		ipnt += 2;
	        		lpPoints = ipnt; 
	        		if (Pick)
	        			TXLoc = *lpPoints; 
	        		else
	        		{
						WinPoint = FileCoordToWinCoord(*lpPoints); 
	        			TXLoc = WinPoint; 
	        		}
//					ProjectFilePt (lpPoints); DisplayPointItem converts to windows coord
	        		ipnt += 2;   
 		    		CurrentType = GF_POINT;
					if (Pick & Visible)
					{
						AlreadyProcessed=TRUE;
						PickPointItem (*lpPoints,*pSize,*pRot,CurrentDesc);			 		    
					} 
					else if (Visible)
					{
						AlreadyProcessed = TRUE; 
						HighlightPointSym=FALSE;
						if (SetDisplayChar (hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
							DisplayPointItem (hDC,WinPoint,*pSize,*pRot,CurrentDesc);
					}
					//	TextOut (hDC,lpPoints->x,lpPoints->y,CurrentUDI,_fstrlen(CurrentUDI));

	        	break;
	        	
	        	case 21: /* set pen color, width and style*/
	        	{   
	        		int	pwidth;           
	        		TempLineColor = *(LPLONG)ipnt;
	        		ipnt += 2; 
	        		TempWidth = *ipnt++;
	        		TempStyle = *ipnt++;  
	        		if (Pick || !hDC)
	        			break; 
                    if (*PenCOLOR >= 0) 
                    	TempLineColor = *PenCOLOR;  
                    if (*PenWIDTH > 0)
                    	TempWidth = *PenWIDTH;   
		 			pwidth = IDNINT(WidthFactor*TempWidth);  
					hTempPen = CreatePen(PS_SOLID,pwidth,AutoYellow(TempLineColor)); 
					SaveCurrentPen = 0;
	        	}
	        	break;
	        	
	        	case 22: /* set brush pattern color and forecolor*/
	        	{   
	        		TempPattern = *ipnt++;           
	        		TempFillColor = *(LPLONG)ipnt; 
	        		GlobalColors[0]=TempFillColor;
	        		ipnt += 2; 
	        		TempBackColor = *(LPLONG)ipnt;
	        		ipnt += 2;  
	        		if (Pick || !hDC)
	        			break;
					hTempBrush = CreateSolidBrush(TempFillColor);
	        	}
	        	break;
	        	
	        	case 23: /* clear temp pen and brush */
	        	{              
	        		if (Pick || !hDC)
	        			break;
					if (hTempBrush)
					{
        SelectObject(hDC, GetStockObject(BLACK_BRUSH));
        				//SelectObject(hDC, hOldBrush);
						DeleteObject (hTempBrush); 
						hTempBrush = 0;
					}
					if (hTempPen)
					{
				        //SelectObject(hDC, hOldPen);
        SelectObject(hDC, GetStockObject(BLACK_PEN));
        				if (SaveCurrentPen) 
        					CurrentPen = SaveCurrentPen;
						DeleteObject (hTempPen);
						hTempPen = 0;
					}
	        	}
	        	break; 
	        	
	        	case 27: // Multipolygon indicator
	        	{
	        		short	n; 
	        		long	PolyBufferLen;
	        		
					GSSiGlobFree (&hPolyBuffer); 
					GSSiGlobFree (&hPolyPartLen);  
	        		nPoly = n = *ipnt++; 
	        		iPoly = 0;
	        		PolyBufferLen = sizeof(POINT) * (nPoly - 1);   
	        		while (n--)
	        			PolyBufferLen += *ipnt++; 
	        		hPolyBuffer = GSSiGlobAlloc (GMEM_MOVEABLE,(long)PolyBufferLen*sizeof(POINT));  
	        		hPolyPartLen = GSSiGlobAlloc (GMEM_MOVEABLE,(long)nPoly*2);  
	        		nPolyPoints = 0;
                }
	        	break;
	        	
	        	case 30: /* intersection data: totlen/2, num_from_int, (ref,street, 1..num)
	        										   num_to_int,	 (ref,street, 1..num) */
	       		{   
	       			LPVOID	pIntData;
	       			if (hIntData)
	       			{
	       				pIntData = GlobalLock (hIntData);
	       				_fmemmove (pIntData,(LPVOID)(ipnt+1),*ipnt*2); 
	       				GlobalUnlock (hIntData);
	       			}
	        		ipnt += *ipnt+1;	
	        	}
	        	break;   
	        	
	        	case 31: /* from and to street */
	        	{  
	        		_fmemmove (&FromStreet,ipnt,4);
	        		_fmemmove (&ToStreet,ipnt+6,4);
	        		ipnt += 12;
	        	}
	        	break;  
	        	
	        	case 33: /* jump to long rec and back */
	        	{
					ShowValue (hDC,FALSE);
					LastItemLenActual = (LPSTR) ipnt - (LPSTR)(BeginSeg +2) - LastCurrentItem; 
					if (LastItemLenActual != LastItemLen)
						ii=1;
					ContinuationOffset = *(LPLONG)ipnt; 
					if (ContinuationOffset >=0)
						Numcont++; 
					ipnt += 2;    
					JumpBackSeg = CurrentSeg;
					JumpBackElement = ipnt - BeginSeg;
					JumpBackElement++;
					TotBlockLen += 2; 
				}
				break; 
				
	        	case 34: /* jump to long rec and back */
	        	{
					ContinuationOffset = JumpBackSeg; 
					StartElement = JumpBackElement;
					*ipnt = 0;	
				}
				break; 
				
			    case 35: /* point array */
			
				{   nPnts = *ipnt;
				    ipnt++; 
				    if (hCoords) 
				    {
				    	nCoords += nPnts;
				    	hCoords = GlobalReAlloc (hCoords,(long)nCoords*4,GMEM_MOVEABLE);
				    }
				    else 
				    {
				    	hCoords = GSSiGlobAlloc (GMEM_MOVEABLE,(long)nPnts*4);
				    	nCoords = nPnts;
				    	lCoords = 0;
				    }
				    pCoords = GlobalLock (hCoords);
				    pCoords += lCoords;
				    BufWrite (&pCoords,&lCoords,ipnt,nPnts*4); 
				    GlobalUnlock (hCoords);
				    ipnt += nPnts * 2;
				}
				break;   
				
				case 36: /* null code */
				break; 
				
				case 37: 
				{
					
					GRStartTime = *(LPLONG)ipnt;
					ipnt+=2;
					GREndTime = *(LPLONG)ipnt;
					ipnt+=2; 
					if (GRStartTime > TimeRangeEnd || GREndTime < TimeRangeBeg)
						Visible = FALSE;
				}
				break;
	        	
				case 40: /* command string */
				{                    
					LPSTR	pString, pEnd;
					
	    			CurGCmdStringLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
					lGCmdString = *ipnt++; 
					GSSiGlobFree (&hGCmdString);   
					hGCmdString = GSSiGlobAlloc (GMEM_MOVEABLE,4096);
					pString = GlobalLock (hGCmdString);
					_fstrncpy (pString,(LPSTR)ipnt,lGCmdString);
					pEnd = pString + lGCmdString;
					*pEnd = 0; 
					if (Visible)
					switch (ProcessGCmdStrings)
					{                          
						case 0:
							break;
						case 1:
							if (Pick)
								break; 
							goto DoCmd;
						case 2:
							if (!Pick)
								break;
						case 3: 
				DoCmd:  
						{
							LPVIEWPORT	SaveView=CurView;
							HANDLE		SaveH1=hTranFileToBase, SaveH2=hTranBaseToFile;
							
							hTranFileToBase=0;
							hTranBaseToFile=0;
							InGRCmd = TRUE;
							ExpandText (pString); 
							InGRCmd = FALSE;
							_fstrncpy (pString,(LPSTR)ipnt,lGCmdString); 
							CurView = SaveView;
							hTranFileToBase=SaveH1;
							hTranBaseToFile=SaveH2;
						} 
					}
					ipnt += lGCmdString/2; 
					GlobalUnlock (hGCmdString);
				}
				break;	
				
                default: 
                	ProcessInvalidRecord ((short)*Pcode,ipnt,1);
                break;
			} 
			if (DoTime)
				TypeTime[*Pcode] += (GetTickCount() - starttime2);
        }
    
    TotBlockLen += 1;
	if (!Pick && hDC)
	{
        SelectObject(hDC, GetStockObject(BLACK_BRUSH));
        SelectObject(hDC, GetStockObject(BLACK_PEN));
    }
    if (hSpecialPen)
    {
    	DeleteObject (hSpecialPen);
    	hSpecialPen = 0;
    }  
    if (hSpecialBrush)
    {
    	DeleteObject (hSpecialBrush);
    	hSpecialBrush = 0;
    }  
    if (DoTime)  
    	TotDisplayTime+=(GetTickCount()-starttime);
	InGraphicsProcessor = FALSE;
    return rtn;
}
void ProcessInvalidRecord (short Pcode,LPSHORT ipnt,short from)
{   
	HFILE		Fid;
	OFSTRUCT	OFStruct;
	char		str[256];
	
	switch (InvalidRecOpt)
	{
		case 0:
			CloseMap (); 
			DoPaint = FALSE;
			if (FirstError)
				MessageBox(hWndMain, "Invalid graphics record encountered",PltName, MB_OK);
			FirstError = FALSE;  
			break;
		case 1:  
			Fid = GSSiOpenFile ("invalid.txt",&OFStruct,OF_READWRITE);
			if (Fid == HFILE_ERROR)
				Fid = GSSiOpenFile ("invalid.txt",&OFStruct,OF_CREATE);
			else
				_llseek (Fid,0,2); 
			sprintf (str,"Invalid graphics record: %s  (%ld,%ld,%i,%i)",PltName,CurrentSeg,CurElement,Pcode,from);
			ExpandText (str);
			fputstring (str,Fid);
			_lclose (Fid);	
			break;
		case 2:
			break;
	}
	FoundInvalidRec = TRUE; 
	if (ipnt)
		*ipnt = 0;
	return;
}
  

int	GetRecDesc (LPSTR rec, int l)
{	LPSTR	Endpnt;
	LPBYTE	Pcode;
	LPINT	ipnt;
     
    ipnt = rec;
    Endpnt = rec + l;
    while (*ipnt != 0 && ipnt < Endpnt)
    {   Pcode = (LPBYTE) ipnt;
    	ipnt++;

	    switch (*Pcode)
        {   
            case 8:	/*	description */
	 		{   
	 			return (*ipnt);
            }
            break;

            default: 
            	SkipSubRec (Pcode,&ipnt);
            break;

		}
    }

    return 0;
}

int GWPolygon (HDC hDC, LPPOINT lpPoints, unsigned short npnts)
{	HANDLE Handle;
	LPPOINT	lpNewPoints, lpPntNew;
	unsigned short	i;
	
	if (!Display || !hDC) return (0);  
	MostPoints = max (MostPoints,npnts);
	if (npnts > MaxDisplayPoints)
		return 0;
	if (!FileProjectionType && (FileMode || CurView->FileFactor <= 1)) 
	{
		if (DoGraphics)
			i = Polygon (hDC,lpPoints,npnts);
	}
	else                                 
	{
		Handle = GSSiGlobAlloc (GMEM_MOVEABLE,(long)npnts * sizeof (POINT));
		lpNewPoints = (LPPOINT)GlobalLock (Handle);  
		if (!CurView->FileFactor) CurView->FileFactor=1;
		for (i=0,lpPntNew = lpNewPoints;i<npnts;i++,lpPntNew++,lpPoints++)
		{
			ProjectFilePt (lpPoints);
			lpPntNew->x = lpPoints->x / CurView->FileFactor;
			lpPntNew->y = lpPoints->y / CurView->FileFactor;
		} 
		if (DoGraphics)
		{
			i = Polygon (hDC,lpNewPoints,npnts);  
		}
		GlobalUnlock (Handle);
		GlobalFree (Handle);
	}
	if (i)
		CurView->CurVisType[CurrentDesc]=3;
	return (i);
	
}
int GWPolyline (HDC hDC, LPPOINT lpPoints, unsigned short npnts)
{
	HANDLE Handle;
	LPPOINT	lpNewPoints, lpPntNew;
	unsigned short	i;
	
	if (!Display || !hDC) return (0);
	if (npnts > MaxDisplayPoints)
		return 0;
	if (!FileProjectionType && (FileMode || CurView->FileFactor <= 1))
	{
		if (DoGraphics)
			i = Polyline (hDC,lpPoints,npnts);
	}
	else
	{
		if (!(Handle = GSSiGlobAlloc (GMEM_MOVEABLE,npnts * sizeof (POINT)))) return (-1);
		lpNewPoints = (LPPOINT)GlobalLock (Handle);
		if (!CurView->FileFactor) CurView->FileFactor=1;
		for (i=0,lpPntNew = lpNewPoints;i<npnts;i++,lpPntNew++,lpPoints++)
		{   
			ProjectFilePt (lpPoints);
			lpPntNew->x = lpPoints->x / CurView->FileFactor;
			lpPntNew->y = lpPoints->y / CurView->FileFactor;
		}  
		if (DoGraphics)
			i = Polyline (hDC,lpNewPoints,npnts);
		GlobalUnlock (Handle);
		GlobalFree (Handle);
	} 
	if (i)
	{
		if (TSize)
			CurView->CurVisType[CurrentDesc]=2;
		else
			CurView->CurVisType[CurrentDesc]=1;
	}
	return (i);

}

POINT CurrentMidPoint (int Type,LPDOUBLE MidPointAZ,LPDOUBLE Length,BOOL Clip)
{   POINT	MidPoint, StartPoint, LastPoint, Point;
	LPINT	ipnt;
	int		nPnts, i, cenpt;
	LPPOINT lpPoints;
	long	X, Y; 
	short	ib,ie, np;
    
    *Length = 0; 
    *MidPointAZ = 0;
    ipnt = CurElementPnt;
	switch (Type)
	{	case GF_AREA:
			ipnt++;
		case GF_POLYLINE:
		    nPnts = *ipnt;
		    ipnt++;
		    break;
		case GF_LINE:
			nPnts = 2; 
			break;
		default:
			return (MidPoint);
	}
    if (Clip)
    {     
	    np=0; 
	    ib = -1;
	    lpPoints = (LPPOINT)ipnt;  
    	for (i=0;i<nPnts;i++,lpPoints++) 
    	{
	    	Point = *lpPoints;
	    	ProjectFilePt (&Point); 
	    	Point.x /= CurView->FileFactor;
	    	Point.y /= CurView->FileFactor;
	    	if (PtInBounds (Point))
	    	{
	    		if (ib <0)
	    			ib = i; 
	    		np++;
	    		ie = i+1;
	    	}
	    	else
	    	{ 
	    		if (ib >= 0)
	    		{    
	    			if (np == 1) 
	    			{
	    				ie = i+1;
	    				np++;
	    			}
	    			else
	    				ie = i;
	    			break;
	    		}
	    	} 
	    }
    }
    else
    {
    	ib = 0;
    	ie = nPnts;  
    	np = nPnts;
    }           
    if (!np)
    {	
    	MidPoint.x=0;
    	MidPoint.y=0;
    	return (MidPoint);
    }    
    lpPoints = (LPPOINT)ipnt;  
    lpPoints += ib;
    X = lpPoints->x;
    Y = lpPoints->y;  
    if (np == 1)
    {   
    	if (ib)
    	{
		    LastPoint = *lpPoints--; 
		    ProjectFilePt (&LastPoint); 
		    StartPoint = *lpPoints++; 
		    ProjectFilePt (&StartPoint);
	 	}
	 	else
	 	{ 
		    StartPoint = *lpPoints++; 
		    ProjectFilePt (&StartPoint); 
		    LastPoint = *lpPoints; 
		    ProjectFilePt (&LastPoint); 
	    }  
	}
    else
    {
	    StartPoint = *lpPoints++; 
	    ProjectFilePt (&StartPoint); 
	    LastPoint = StartPoint;
	    cenpt = ib + np/2;
	    for (i=ib+1;i<ie;i++,lpPoints++)
	    {   
	    	Point = *lpPoints;
	    	ProjectFilePt (&Point); 
	    	X += lpPoints->x;
	        Y += lpPoints->y;
		    if (i==cenpt)
		    	MidPoint = *lpPoints;
		    
	        *Length += idist (Point,LastPoint);
	        LastPoint = Point;
	    } 
    }         
    if (np > 0)
    {   
    	if (StartPoint.x != LastPoint.x || StartPoint.y != LastPoint.y)
    		*MidPointAZ = getaz (StartPoint,LastPoint);
    }  
    if (Type == GF_AREA || np < 3)
    {
	    MidPoint.x = X / np;
	    MidPoint.y = Y / np;  
	}
	return (MidPoint);
}

BOOL ProcessPrimarySeg (HWND hWndDlg, int DlgItemSym, int DlgItemPar, BOOL SetTran)
{	HANDLE 		hpltBuf;
	LPSTR		LPpltBuf;
	LPINT		ipnt;
	LPLONG		pOffset;
	int			idesc, ndesc,iparent;
	char		DescName[34], cdesc[8], str[32];
	BOOL		parent, SymIsVisible, Invisible;
	int			pwidth; 
	long		ii;
	double		ifac,penwidth=0;
	BOOL		DupSet;
    LOGBRUSH	NDB;
    LRESULT     lResult; 
    BOOL		ValidDesc;
	RGBQUAD 	rgb;
	WORD		nRead, nBytes=0; 
	int			Pcode; 
	int			ipen, i;  
	COLORREF	Color;  
	long		StartLoc;
	LPSTR		pStart, pLoc;
	
	ifac = 0;
	if (CurVis)
		if (CurVis->WantType[9])
			ifac = WidthFactor;
		
	if (DlgItemPar)
		DupSet = TRUE;
	else
		DupSet = FALSE;


    if (!PltName[0] || Fid<0) return (FALSE);

    nRead = _lread (Fid,&nBytes,2);
    hpltBuf = GSSiGlobAlloc (GMEM_MOVEABLE,(DWORD)nBytes);
    LPpltBuf = GlobalLock (hpltBuf);
    StartLoc = _llseek (Fid,0,1);
    nRead = _lread (Fid,LPpltBuf,(WORD)nBytes);
    ipnt = (LPINT)LPpltBuf;   
    pStart = ipnt;
        while (*ipnt > 0)
        {   Pcode = *ipnt;
        	ipnt++;
		    switch (Pcode)
	        {
			    case 1: /* color palette */
		 		{   ipen = *ipnt;
		 			if (ipen > maxbrush) maxbrush=ipen; 
		 			ipnt++;
		 			if (ipen == ModifyPen)
		 				ModPenLoc = (LPSTR) ipnt - LPpltBuf + 2;
		 			rgb.rgbRed = *ipnt;
		 			ipnt++;
		 			rgb.rgbGreen = *ipnt;
		 			ipnt++;
		 			rgb.rgbBlue = *ipnt;
		 			ipnt++;
		 			Color = RGB(rgb.rgbRed,rgb.rgbGreen,rgb.rgbBlue);
		 			GetPenRedefColor(ipen,&Color);
                    if (*PenCOLOR >= 0) 
                    	Color = *PenCOLOR;  
                    if (*PenWIDTH > 0)
                    	penwidth = *PenWIDTH;   
		 			pwidth = IDNINT(ifac*penwidth);  
		 			if (penwidth > 1)
		 				pwidth = max ((long)pwidth,IDNINT(penwidth));
					pens[ipen] =   CreatePen(PS_SOLID,pwidth,AutoYellow(Color));
                    if (*AreaCOLOR >= 0) 
                    	Color = *AreaCOLOR;  
					if (SolidAreas)
		 				brushes[ipen] = CreateSolidBrush(Color);
		 			else
		 			{ 
						NDB.lbStyle = BS_HATCHED;
						NDB.lbColor = Color;
						NDB.lbHatch	= HS_DIAGCROSS;
		 				brushes[ipen] =CreateBrushIndirect(&NDB);
                    }
		 		}
	            break;

                case 101:	/*	used description offset	*/
                {   pOffset = (LPLONG)ipnt;
                	UsedDescOffset = *pOffset; 
                	ipnt += 2;
                }
                break;

			    case 102: /* color palette offset */
                {   pOffset = (LPLONG)ipnt;
                	ColorPaletteOffset = *pOffset; 
                	ipnt += 2;
                }
                break;

                case 103: /* transformation points */
                {   pOffset = (LPLONG)ipnt;
                	TranPointOffset = *pOffset;
                	ipnt += 2;
                }
                break;

                case 200: /* quad tree offset */
                {   pOffset = (LPLONG)ipnt;
                	GraphicsOffset = *pOffset;
                	ipnt += 2;
                }
                break;

                case 201: /* desc block offset */
                {   pOffset = (LPLONG)ipnt;
                	DescBlockOffset = *pOffset;
                	ipnt += 2;
                }
                break;

                case 202: /* date range */
                {   
               		MinFileTime = *(LPLONG)ipnt;
                	ipnt += 2; 
                	MaxFileTime = *(LPLONG)ipnt;
                	ipnt += 2;
                }
                break;

                case 11: /* get used description list*/
                {   
                    short SymNameLen=8;
                    
                	if (MapVersion >= 8)
                		SymNameLen = 32;
                	parent = FALSE;    
                	moredesc:   ndesc = *ipnt;
                	ipnt++;
                	for (i=0;i<ndesc;i++)
                	{	idesc=abs (*ipnt); 
                		pLoc = ipnt;
                		if (*ipnt < 0 && !parent)
                			if (!GetInVisibility(idesc)) ToggleInVisibility(idesc);
                		ipnt++;
                		iparent=*ipnt;
                		ipnt++;
                    	_fmemmove (DescName,ipnt,SymNameLen); 
                    	DescName[SymNameLen]=0;
                    	Truncate (DescName);
                    	if (!_fstricmp (DescName,"PARCEL"))
                    		ParcelLoc = StartLoc + (pLoc - pStart);
                    	if (!_fstricmp(DescName,"ALL"))
                    		iparent=0;
                    	ipnt+=(SymNameLen/2);
                    	if (!DuplicateDesc(idesc,DupSet))
                    	{
	                		if (iparent == SetVisByPar && SetVisByPar > 0)
	                		{	SymIsVisible = GetVisibility(idesc);
	                			if (SymIsVisible != ParIsVisible) ToggleVisibility(idesc);
	                			if (parent) AddParToList(idesc);
	                		}
	                    	if (DlgItemSym<0)
	                    	{       
	                    		if (DlgItemSym == -9999)
	                    		{  
	                    			if (!_fstricmp (CurrentSymName,DescName)) 
	                    				CurrentSymNum = idesc;
	                    		}
	                    		else
	                    		{
	                    			if (idesc==-DlgItemSym) 
	                    			{
	                    				_fstrcpy (CurrentSymName,DescName);
	                    				CurrentSymParent = iparent;
	                    			}
	                    		}
	                    	} 
	                    	if (_fstricmp(DescName,"        ")>0)
	                    		ValidDesc=TRUE;
	                    	else
	                    		ValidDesc=FALSE; 
	                    	if (DlgItemPar >= 0)
	                    	{
		                    	if(GetVisibility(idesc))
		                    		_fstrcat (DescName,"\t<on>\t");
		                    	else
		                    		_fstrcat (DescName,"\t\t");
		                    	sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,idesc);
		                		if (DlgItemSym > 0 && ValidDesc)
		                		{
		                			if (parent)
		                			{
		                				if (DlgItemPar > 0)
		                	    			lResult=SendDlgItemMessage ((HWND) hWndDlg, DlgItemPar,
		                				     	LB_ADDSTRING, NULL, (LPARAM) DescName); 
		                			}
		                			else if (DlgItemSym)
		                				lResult=SendDlgItemMessage ((HWND)hWndDlg,
		                	          		DlgItemSym,LB_ADDSTRING,NULL,(LPARAM) DescName); 
		                		}
		                	}
		                	else if (parent) 
		                	{   
		                		if(GetVisibility(idesc))
		                		{
			                		if (_fstricmp (DescName,"ALL"))   
				                   		sprintf (str,"(%s)",DescName);
	                	    		lResult=SendDlgItemMessage ((HWND) hWndDlg, -DlgItemPar,
				                				     	CB_ADDSTRING, NULL, (LPARAM) str);
		                	    }
		                	}
	                	}
                	}
                	if (DlgItemPar < -3200) goto Exit;
                	parent = TRUE;
                	if (ndesc) goto moredesc;
                }
                break;

				case 13: /* continuation offset */
				{	ContinuationOffset = *(LPLONG)ipnt;
					ipnt += 2;
					*ipnt = 0; 
					if (ContinuationOffset >=0)
						ii=1;
                }
                break;

                default:
                	ProcessInvalidRecord (Pcode,ipnt,3);
                break;

			}
        }

Exit:  	GlobalUnlock (hpltBuf);
    	GlobalFree	(hpltBuf);  
    	if (SetTran)     
    		ReadTranRecord();
    	return (TRUE);

}     

BOOL ReadTranRecord (void)
{
	char		TranData[140];
	double 		XINCH[4],YINCH[4],XBASE[4],YBASE[4];
	double		RSQMIN;
	LPSTR		lpCor;
   	int	i;

	_llseek(Fid,TranPointOffset,0);
	_lread (Fid,&nRead,2);
    _lread (Fid,&TranData,nRead-4); 
    lpCor =(LPSTR)  &TranData;
    XINCH[0] = dread (lpCor,16); 
    lpCor+=16;
    XINCH[2] = dread (lpCor,16);
    lpCor+=16;
    YINCH[0] = dread (lpCor,16);
    lpCor+=16;
    YINCH[1] = dread (lpCor,16);
    lpCor+=16;
    XBASE[0] = dread (lpCor,16);
    lpCor+=16;
    XBASE[2] = dread (lpCor,16);
    lpCor+=16;
    YBASE[0] = dread (lpCor,16);
    lpCor+=16;
    YBASE[1] = dread (lpCor,16);
    XINCH[1] = XINCH[0];
    XINCH[3] = XINCH[2];
    YINCH[2] = YINCH[1];
    YINCH[3] = YINCH[0];
    XBASE[1] = XBASE[0];
    XBASE[3] = XBASE[2];
    YBASE[2] = YBASE[1];
    YBASE[3] = YBASE[0]; 
    if (GetGlobalBVal ("[%ConvertMap]")) 
	for (i=0;i<4;i++)
	{	
		DPOINT	Point;
		
		Point.x = XBASE[i];
		Point.y = YBASE[i];
		ComputeNewMapControlPoint (&Point);
        XBASE[i] = Point.x;
        YBASE[i] = Point.y;                      
    	XBASE[i]+=MapAdjustX;
    	YBASE[i]+=MapAdjustY;
    }
	CloseTRANS2 (&hTranFileToBase); 
	CloseTRANS2 (&hTranBaseToFile);
    hTranFileToBase = STRAN2 (XINCH,YINCH,XBASE,YBASE,4,(LPFLOAT)&RSQMIN,1);
    hTranBaseToFile = STRAN2 (XBASE,YBASE,XINCH,YINCH,4,(LPFLOAT)&RSQMIN,1); 
    if (FileProjectionType)
    { 
    	DPOINT DPoint; 
	                              
		for (i=0;i<4;i++)
		{	                              
            DPoint.x = XINCH[i];
            DPoint.y = YINCH[i];
            ProjectFilePtD (&DPoint);
            XINCH[i] = DPoint.x;
            YINCH[i] = DPoint.y;
        }
		CloseTRANS2 (&hTranFileToBase); 
		CloseTRANS2 (&hTranBaseToFile);
        hTranFileToBase = STRAN2 (XINCH,YINCH,XBASE,YBASE,4,(LPFLOAT)&RSQMIN,1);
        hTranBaseToFile = STRAN2 (XBASE,YBASE,XINCH,YINCH,4,(LPFLOAT)&RSQMIN,1); 
    }

	return TRUE;
}

BOOL ProcessPickedItem (int Item,BOOL DoDisplay)
{	LPINT		ipnt, EndItem;
    HANDLE 		hpltBuf;
	LPSTR		LPpltBuf;
	ITEM		*ItemHeader;
	HDC			hDC=0;
	OFSTRUCT	OFStruct;
	POINT		CenterPoint, WinPoint;
	BOOL		SaveDisplay, SaveIgnoreBounds;   
	HANDLE		hVisList;
	LPVISLIST	SaveVis;   
	int			ii;
	
	CurView = PickList[Item].View;
    if (!GetPickName (Item))
    	return (FALSE);

	if (!PickName[0]) return(FALSE);
	_fstrcpy (PltName,PickName);

	CloseMap ();
	if (!OpenMap (CurView->hWnd,CurView->hDC)) return(FALSE);
	
	hVisList=GSSiGlobAlloc (GHND,sizeof(VISLIST));
	if (!hVisList) return(MemError());
	SaveVis = CurVis;
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
	
	CurrentSeg = PickList[Item].Segment;
    _llseek (Fid,CurrentSeg,0);
    nRead = _lread (Fid,&nBytes,2);
    hpltBuf = GSSiGlobAlloc (GMEM_MOVEABLE,(DWORD)nBytes+2);
    LPpltBuf = GlobalLock (hpltBuf);
    nRead = _lread (Fid,LPpltBuf,nBytes);
    if (nRead != nBytes || PickList[Item].Offset > nRead) 
    {
    	InvalidItem (NULL);
    	return FALSE;
    }
    ipnt = (LPINT)(LPpltBuf + PickList[Item].Offset);
    ItemHeader = (LPITEM) ipnt;
    if (InvalidItem (ItemHeader))
    	return FALSE;
    EndItem = ipnt + abs(ItemHeader->Len);
    EndItem+=6;
    *EndItem = 0;
    SaveDisplay = Display; 
    Display = DoDisplay;
    if (Display)
    {
		CurView->hRgn = CreateVPRgn();
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	DeleteObject(CurView->hRgn);
	    SetDisplayMode (CurView->hDC, GF_MAPMODE); 
	    CurView->PassID=2;  
	    hDC = CurView->hDC;
	    ii=SaveDC (hDC);
	    OpenBasePens();
	}
	SaveIgnoreBounds = IgnoreBounds;
	IgnoreBounds = TRUE;
    ProcessGraphicsRec (hDC,ipnt,LPpltBuf);  
    IgnoreBounds = SaveIgnoreBounds;
    if (Display)
    	CloseBasePens();
    Highlight = FALSE;
	CloseMap();
	GlobalUnlock (hpltBuf);
	GlobalFree	(hpltBuf);
	hpltBuf=0; 
	if (Display)
		RestoreDC (hDC,ii);
	Display = SaveDisplay;  
	GlobalUnlock (hVisList);
	GlobalFree	(hVisList);
	CurVis = SaveVis;
	return(TRUE);
}  

BOOL InvalidItem (LPITEM ItemHeader)
{   
	LPBYTE	Pcode;
	BYTE	NullCode=0;
	
	if (ItemHeader)
		Pcode = (LPBYTE) ItemHeader; 
	else
		Pcode = &NullCode;
	if (*Pcode !=12 && !(*Pcode == 92 && !ShowDeletedOpt))
	{   
		HaltMapDisplay();
		MessageBox (GetFocus(),"Reference and TAG indexes need to be recreated","Graphic record location error",MB_ICONEXCLAMATION);
        PostMessage(hWndMain, WM_CLOSE, 0, 0L);  
        exit(1);
		return TRUE;
	}
	return FALSE;
}

BOOL PickedItemMinMax (int Item, LPMNMXCORD lpRect)
{	LPINT		ipnt, EndItem;
    HANDLE 		hpltBuf;
	LPSTR		LPpltBuf;
	ITEM		*ItemHeader;
	HDC			hDC;
	OFSTRUCT	OFStruct;  
	BOOL		rtn=FALSE; 
	
	if (!FastPick ||
		PickList[Item].FileNum != FastPickFileNum ||
		PickList[Item].FileInIndex != FastPickFII)
	{   
		if (FastPick)
		{
			FastPickFileNum = PickList[Item].FileNum; 
			FastPickFII = PickList[Item].FileInIndex;
		}
	    if (!GetPickName (Item)) return FALSE;
	
		if (!PickName[0]) return FALSE;
		_fstrcpy (PltName,PickName);
	
		CloseMap (); 
		if (!OpenMap (CurView->hWnd,CurView->hDC)) return FALSE; 
	}
    if (_llseek (Fid,PickList[Item].Segment,0) != PickList[Item].Segment)
    	goto Exit2;
    nRead = _lread (Fid,&nBytes,2);
    if (nRead != 2)
    	goto Exit2; 
    if (nBytes < 0)
    {
    	InvalidItem (NULL);
    	goto Exit;
    }
    hpltBuf = GSSiGlobAlloc (GMEM_MOVEABLE,(DWORD)nBytes+4);
    LPpltBuf = GlobalLock (hpltBuf);
    nRead = _lread (Fid,LPpltBuf,nBytes); 
    if (nRead != nBytes || PickList[Item].Offset > nRead) 
    {
    	InvalidItem (NULL);
    	goto Exit;
    }
    ipnt = (LPINT)(LPpltBuf + PickList[Item].Offset);
    ItemHeader = (LPITEM) ipnt;  
    if (InvalidItem (ItemHeader))
    	goto Exit;
	GetItemMinMax (&ItemHeader->MinMax,lpRect); 
    EndItem = ipnt + abs(ItemHeader->Len);
    EndItem+=6;
    *EndItem = 0;
	Pick=TRUE; 
	IgnoreBounds = TRUE;
    ProcessGraphicsRec (CurView->hDC,ipnt,LPpltBuf);
	if (PickingByRefno)
	{
		PickList[Item].PickedPoint.x = (lpRect->xmn + lpRect->xmx)/2;    
		PickList[Item].PickedPoint.y = (lpRect->ymn + lpRect->ymx)/2;
	}    
    IgnoreBounds = FALSE; 
    rtn = TRUE;
Exit:
    Pick=FALSE;         
	GlobalUnlock (hpltBuf);
	GlobalFree	(hpltBuf);
	hpltBuf=0; 
Exit2:
	if (!FastPick)
		CloseMap();
	return rtn;
}  

int	PickItems (HWND hWnd,DPOINT InPickPointBase) 
{
	HCURSOR	hcurSave;  
	int		Rtn;
	
    PickPointBase = InPickPointBase;
	SetGlobalValueReal ("%PICK_POINT_X",PickPointBase.x);
	SetGlobalValueReal ("%PICK_POINT_Y",PickPointBase.y);
    
	if (!MemMap && hWnd)
	{	
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		HaltMapDisplay ();  
	}
    SetPickAp();  
    
    Rtn = PickItems2 (hWnd,PickPointBase);

	if (!MemMap)
		GSSiSetCursor (hcurSave);    
	return Rtn;
}

int	PickItems2 (HWND hWnd,DPOINT PickPointBase)
{	MNMXCORL	SaveBounds;
	MNMXCORD	SaveWBounds, SaveNewBounds;
	
	int			iPickAp, idum; 
	POINT		PickPoint, PickPoint2;
	LPVISLIST	SaveVis;
	POINT		WinPT1, WinPT2;
	DPOINT		WPoint1, WPoint2, PickPointBase2; 
	BOOL		SavePick, Rtn;
    
    if (PickAp == 0)
    {
		Rtn = PickNearestItems (PickPointBase);
		return (Rtn);
	}
	SaveVis = CurVis;

	SaveBounds = CurView->Bounds;
	SaveWBounds = CurView->WBounds; 
	SaveNewBounds = CurView->NewBounds;
	
	
	CurView->WBounds.xmn = PickPointBase.x - PickApW;
	CurView->WBounds.xmx = PickPointBase.x + PickApW;
	CurView->WBounds.ymn = PickPointBase.y - PickApW;
	CurView->WBounds.ymx = PickPointBase.y + PickApW; 
    SetLocalProjection (&CurView->WBounds);
	PickPointBase2 = PickPointBase;
	PickPointBase2.x += PickApW; 
	
    
    CurView->PassID = 0; 
    SavePick = Pick;
	Pick=TRUE;
	NumPicked=0;
	CurView->CurFile=-1;
	SelectVisList (TRUE);
	IncrementFile ();
	Printing = TRUE;
	PickName[0] = '\0';
	while (GetNextViewportFile (FALSE))
	{   
		if (PltType == 3)
			idum=0;
		else if (PltType < 5)
		{
			DisplayPlotInit(hWnd);
			if (OpenMap (hWnd, 1))
			{   
				_fstrcpy (PickName,PltName);
				PickPoint = BasePtToFilePt(PickPointBase);
				PickPoint2 = BasePtToFilePt(PickPointBase2); 
				if (!CurView->FileFactor) CurView->FileFactor=1;
				iPickAp = idist(PickPoint,PickPoint2)/CurView->FileFactor;
				PickPoint.x = PickPoint.x/CurView->FileFactor;
				PickPoint.y = PickPoint.y/CurView->FileFactor;
				CurView->Bounds.xmn = PickPoint.x - iPickAp;
				CurView->Bounds.ymn = PickPoint.y - iPickAp;
				CurView->Bounds.xmx = PickPoint.x + iPickAp;
				CurView->Bounds.ymx = PickPoint.y + iPickAp;
		
				while (DisplaySeg (NULL));
			} 
		}
		else if (PickOrtho)
		{   
			DPOINT	OrthoPoints[4];
			
			OrthoPoints[0].x=CurOrtho->Bounds.xmn;
			OrthoPoints[0].y=CurOrtho->Bounds.ymn;
			OrthoPoints[1].x=CurOrtho->Bounds.xmn;
			OrthoPoints[1].y=CurOrtho->Bounds.ymx;
			OrthoPoints[2].x=CurOrtho->Bounds.xmx;
			OrthoPoints[2].y=CurOrtho->Bounds.ymx;
			OrthoPoints[3].x=CurOrtho->Bounds.xmx;
			OrthoPoints[3].y=CurOrtho->Bounds.ymn;
			if (POINT_IN_AREAD (PickPointBase, 4, OrthoPoints))
				_fstrcpy (PickedOrthoName,CurOrtho->OrigName);
		}
	}
    Printing = FALSE;
	Pick=SavePick;
	CurView->Bounds = SaveBounds;
	CurView->WBounds = SaveWBounds; 
	CurView->NewBounds = SaveNewBounds;
//	SetDisplayMode (CurView->hDC,GF_TEXTMODE);
//	SelectClipRgn (CurView->hDC,NULL);
	CurVis = SaveVis;  
	return (NumPicked);

}

double PointInPickArea (POINT	Point)
{
	double	iPickAp,d=DBL_MAX; 
	DPOINT	DPoint;
    
	if (FileDistToBaseDist == 0) return d;

    SetPickAp();    
	d = idist (Point,PickPoint);
    if (PickAp)
    {
		iPickAp = PickAp * WindowToFileFactor;  
		if (d <= iPickAp)
			return (d);
		else
			return (DBL_MAX); 
	}
    return (d);
}

BOOL PickPolygon (LPPOINT lpPoints,int nPnts, double Offdist)
{   POINT	PickPoint;
	HANDLE Handle;
	DPOINT	huge *lpNewPoints;
	DPOINT	huge *lpDpoints;
	int	i, Type; 
	MNMXCORD	Rect;  
	DPOINT	BeginPoint, EndPoint,PickedPoint, LastPoint, PickPointW;  
	POINT	BeginPointFile,EndPointFile,PickedPointFile;
	double	Area=0,Perim=0;    
	BOOL	Selected=FALSE;
    
    if (PickNET)
    	return FALSE;
    if (hHighlightArea)
    	return (PickPolyInArea(0,lpPoints,nPnts));
	PickPointW.x = (CurView->WBounds.xmn + CurView->WBounds.xmx) / 2;
	PickPointW.y = (CurView->WBounds.ymn + CurView->WBounds.ymx) / 2;
	
    Handle = GSSiGlobAlloc (GMEM_MOVEABLE,(long)nPnts*sizeof(DPOINT));
 	lpDpoints = (LPPOINT) GlobalLock (Handle); 
 	lpNewPoints = lpDpoints;
	for (i=0;i<nPnts;i++,lpPoints++,lpDpoints++)
	{   
		*lpDpoints = FilePtToBasePt (*lpPoints);  
		if (!i)
			BeginPoint = *lpDpoints;
		else  
		{
			Perim += ldistp(LastPoint,*lpDpoints);
			Area += ( LastPoint.y - lpDpoints->y) * (lpDpoints->x + LastPoint.x) / 2;
		}
		LastPoint = *lpDpoints;
	} 
	Perim += ldistp(LastPoint,BeginPoint);
	Area += (LastPoint.y - BeginPoint.y) * (BeginPoint.x + LastPoint.x) / 2;  
	EndPoint = LastPoint;
     
    if (PickingByRefno)
    	Selected = TRUE;
    else if (POINT_IN_AREAD (PickPointW, nPnts, lpNewPoints))
    	Selected = TRUE;
    else
    	Selected = FALSE;
    if (Selected)
	{   
		if (CurrentType == GF_TEXT)
			Type = 4; 
		else
			Type = 3;
		
		GetItemMinMax (pMinMax,&Rect);
		PickedPoint = PickPointW;
		PickedPointFile = BasePtToFilePt (PickPointW);
		PickListAdd (FileNum,CurView->SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,CurrentiPen,
					   Type,0,Offdist,0,Perim,CurrentItem,CurElement,BeginPoint,EndPoint,PickedPoint,fabs(Area),&Rect,
					   BeginPointFile,EndPointFile,PickedPointFile,nPnts);
	}
	GlobalUnlock (Handle);
	GlobalFree (Handle);
	return (Selected);
}                  

BOOL PickPointItem (POINT Point, float size, float rot, int Symbol)
{ 
	return (PickPolyline (&Point,1));
} 

BOOL DisplayPointItem (HDC hDC,POINT WinPoint,float size, float rot, int Symbol)
{ 
	HANDLE hSymbol; 
	int		ii;
						   
	hSymbol = GetDictSymDesc (Symbol); 
	hSymdb = hSymbol; 
	if (!hSymbol)
	{
		Symbol = GetDictSymbolNumber ("CIRCLE");
		hSymbol = GetDictSymDesc (Symbol); 
		if (!hSymbol)
			return FALSE;
	}  
	SaveDC (hDC);  
	if (size < 0)
		size = -size;
	else
		size /= CurView->BaseUnitsPerPixel;
    SetDisplayMode (hDC, GF_TEXTMODE);
	if (size < 1) 
		size = 1;
	DisplayPointSymbol (hSymbol, hDC, size, size,rot, &WinPoint,NULL,HighlightPointSym,-1);   
	if (hSymbol != hSymdb || !hSymbol)
		ii=1;
	DestroySymbol (hSymbol);
	hSymdb = 0; 
    RestoreDC (hDC,-1);
	CurView->CurVisType[CurrentDesc]=4;
	
	return TRUE;
}

BOOL PickPolyline (LPPOINT lpPoints,int nPnts)
{
	HANDLE Handle;
	LPPOINT	lpNewPoints, lpPntNew;
	POINT	LastPoint, BeginPointFile, EndPointFile, PickedPointFile;
	DPOINT	BeginPoint, EndPoint, PickedPoint, WPoint, LastWPoint, NearPoint;
	int	i, mini;
	double	TotDist, PCT, OffDist, TotDistW, AZ, MinDist, Dist;
	BOOL	First=TRUE, rtn=FALSE;   
	MNMXCORD	Rect;
	short	Type=1;
	               
	if (nPnts > 1)
		Type = 2;
	if (PickNET)
	{
		if (!RefInNet (CurrentRefno))
			return FALSE;
	}
    if (hHighlightArea)
    	return (PickPolyInArea(1,lpPoints,nPnts));
	
	Handle = GSSiGlobAlloc (GMEM_MOVEABLE,(long)nPnts * sizeof (POINT));
	lpNewPoints = (LPPOINT)GlobalLock (Handle);
	TotDist=TotDistW=0;   
	BeginPointFile = *lpPoints;
	BeginPoint = FilePtToBasePt(*lpPoints);
	if (!CurView->FileFactor) CurView->FileFactor=1; 
	MinDist = DBL_MAX;
	for (i=0,lpPntNew = lpNewPoints;i<nPnts;i++,lpPntNew++,lpPoints++)
	{   
		*lpPntNew = *lpPoints;
		ProjectFilePt (lpPntNew);
		lpPntNew->x /= CurView->FileFactor;
		lpPntNew->y /= CurView->FileFactor;
		WPoint = FilePtToBasePt(*lpPoints);
		if (!First) 
		{
			TotDist += idist(LastPoint,*lpPntNew);
			TotDistW += ldistp(LastWPoint,WPoint);
		} 
		else
			First = FALSE;
		LastPoint = *lpPoints; 
		LastWPoint = WPoint;  
		Dist = ldistp (PickPointBase,WPoint);
		if (Dist < MinDist)
		{
			MinDist = Dist;
			NearPoint = WPoint;
			mini = i;
		}
	}
	lpPoints--;
	EndPointFile = *lpPoints;
	EndPoint = FilePtToBasePt(LastPoint);
    if (PickingByRefno)
    {
    	PCT = 0.5;
    	OffDist = 0;
    	goto Add;
    }
	if (!LineInBounds (nPnts,lpNewPoints,TotDist,&PCT,&OffDist,&AZ,
					  (LPMNMXCORL)&CurView->Bounds,&PickedPoint)) goto Exit;
    if (WantOnlyShapePoints) 
	   	PickedPoint = NearPoint; 
	for (i=0; i<NumPicked; i++)
	{
		if (CurrentRefno == PickList[i].Refno)
		{
			if (fabs(OffDist)<fabs(PickList[i].OffDist))
				PickListDelete(i);
			else
				goto Exit;
		}
	} 
Add: 
	rtn = TRUE;
	GetItemMinMax (pMinMax,&Rect); 
	if (!PickingByRefno)
		PickedPointFile = BasePtToFilePt (PickedPoint);
	PickListAdd (FileNum,CurView->SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,CurrentiPen,
				   2,PCT,OffDist,AZ,TotDistW,CurrentItem,CurElement,
				   BeginPoint,EndPoint,PickedPoint,0,&Rect,
				   BeginPointFile, EndPointFile,PickedPointFile,nPnts);
Exit:GlobalUnlock (Handle);
	GlobalFree (Handle);
	return (rtn);
} 

void PickListDelete (int item)
{   int i;
	for (i=item+1;i<NumPicked;i++)
	{
		PickList[i-1]=PickList[i];
	}
	NumPicked--;
	return;
}

void PickListAdd (int FileNum,int SubFile, short FileInIndex,long CurrentSeg,
				  long CurrentRefno,int CurrentDesc,int CurrentiPen,
				  int Type, double PCT,double OffDist,double AZ, double Length,
				  WORD CurrentItem,WORD CurrentElement,
				  DPOINT BeginPoint, DPOINT EndPoint,DPOINT PickedPoint,
				  double Area, LPMNMXCORD Rect,
				  POINT BeginPointFile, POINT EndPointFile, POINT PickedPointFile, short NumPoints )
{	int  i,j;
    
    if (CurrentRefno == DoNotPickThisRefno)
    	return;
	for (i=NumPicked-1;i>=0;i--)
	{
		if (fabs(OffDist)==fabs(PickList[i].OffDist) &&
			Length < PickList[i].Length) goto Insert; //allows segs 1 file coord long to be picked
		if (fabs(OffDist)<fabs(PickList[i].OffDist)) goto Insert;
	}
	 
	if (NumPicked >= MaxPick)
		return; 
Insert: 
	if (NumPicked >= MaxPick) 
	{
		for (j=0;j<i;j++)
		{
			PickList[j]=PickList[j+1];
			_fmemmove (PickedStreets[j],PickedStreets[j+1],16);
		}
	}
	else
	{
		i++;
		for (j=NumPicked;i<j;j--)
		{
			PickList[j]=PickList[j-1];
			_fmemmove (PickedStreets[j],PickedStreets[j-1],16);
		}
	}
	PickList[i].View = CurView;
	PickList[i].FileNum = FileNum;  
	PickList[i].SubFile = SubFile;
	PickList[i].FileInIndex = FileInIndex;
	PickList[i].Segment = CurrentSeg;
	PickList[i].Refno = CurrentRefno;
	PickList[i].Desc = CurrentDesc;  
	PickList[i].ipen = CurrentiPen;
	PickList[i].Type = Type;
	PickList[i].PCT = PCT;
	PickList[i].OffDist = OffDist;   
	PickList[i].AZ = AZ;
	PickList[i].Length = Length;
	PickList[i].Offset = CurrentItem; 
	PickList[i].Element = CurrentElement;
	PickList[i].BeginPoint = BeginPoint; 
	PickList[i].EndPoint = EndPoint;  
	PickList[i].BeginPointFile = BeginPointFile; 
	PickList[i].EndPointFile = EndPointFile;  
	PickList[i].PickedPointFile = PickedPointFile;  
	PickList[i].PickedPoint = PickedPoint;  
	PickList[i].Area = Area;
	PickList[i].Rect = *Rect; 
	PickList[i].NumPoints = NumPoints;  
	_fmemcpy (&PickedStreets[i],CurStreetNumbers,sizeof(CurStreetNumbers));
	GetVal ("%PREFIX",PickList[i].Prefix);
	GetVal ("%UDI",PickList[i].UDI);
	if (NumPicked < MaxPick)
		NumPicked++; 
	return;
}

void GetCurOrtho(int id)
{   
	CurOrtho = (LPORTHO)GlobalLock(hOrthos);
	CurOrtho += id;
	return;
}

BOOL OrthoInBuffer(LPSTR Name, long frame)
{	int i, MinOrthoID;
	long	MinUse;
	LPORTHO	MinOrtho;
	
	if (!hOrthos) OpenOrthos();
    
    MinUse = LONG_MAX;
    CurOrtho = (LPORTHO)GlobalLock(hOrthos);
	for (OrthoID=0;OrthoID<MAXORTHOBUFS;OrthoID++,CurOrtho++)
	{
//		if (!_fstrcmp (CurOrtho->Name,Name) && CurOrtho->Frame == frame) return (TRUE);
		if (!CurOrtho->Name[0]) CurOrtho->LastUsed = LONG_MIN;
		if (CurOrtho->LastUsed<MinUse)
		{
			MinUse = CurOrtho->LastUsed;	
			MinOrtho = CurOrtho; 
			MinOrthoID = OrthoID;
		}
	}
	CurOrtho = MinOrtho;
	OrthoID = MinOrthoID;
	if (CurOrtho->Name[0])
	{
		if (CurOrtho->hDib && CurOrtho->DeleteBM)
			GlobalFree (CurOrtho->hDib);
		CurOrtho->hDib = NULL;
	}
	return (FALSE);

}

void OpenOrthos ()
{	int i;
	char	txt[128];

	if (hOrthos) return; 
	_fstrcpy (txt,"[%ORTHO_BUFFERS]");
	ExpandText (txt);
	MAXORTHOBUFS = atoi (txt);
	MAXORTHOBUFS = min (max (1,MAXORTHOBUFS),128);
	hOrthos = (HANDLE) GSSiGlobAlloc (GHND,MAXORTHOBUFS*sizeof(ORTHO));
	CurOrtho = (LPORTHO) GlobalLock (hOrthos);
	for (i=0;i<MAXORTHOBUFS;i++,CurOrtho++)
	{   
		CurOrtho->Name[0]='\0';
		CurOrtho->Frame = -1;
	} 
	GlobalUnlock(hOrthos); 
	CurOrtho = NULL;
	return;
}

void UnlockHandles(void)
{
	if (CurOrtho) 
	{
		GlobalUnlock(hOrthos);
		CurOrtho = NULL;
	}
	return;
}

void CloseOrthos ()
{   int	i;

	if (!hOrthos) return;
	CurOrtho = (LPORTHO)GlobalLock(hOrthos);
	for (i=0;i<MAXORTHOBUFS;i++,CurOrtho++)
	{
		if (CurOrtho->Name[0])
		{
			if (CurOrtho->hDib && CurOrtho->DeleteBM)
				GlobalFree (CurOrtho->hDib);
		} 
	}
	GlobalUnlock (hOrthos);
	GlobalFree (hOrthos);  
	CurOrtho = NULL;
	hOrthos = 0;
	return;
}
BOOL MovePolyLine (void)
{	int	i, iPickAp, xmove, ymove, PickedPoint;
	LPPOINT	lpPoints, lpPointsMin;
	float	factor;
	long	RefDist1, RefDist2;
	POINT	RefPoint;
	int		mini;
	double	d,mindist;
    
	PickPoint = BasePtToFilePt(PickPointBase);
    SetPickAp();
	iPickAp = PickAp * WidthFactor;

    xmove = CurTheme->Xmove * WindowToFileFactor;
    ymove = CurTheme->Ymove * WindowToFileFactor;

	lpPoints = lpPoints2; 
	mindist = 33000;
	mini=-1;
	for (i=0;i<nPnts;i++,lpPoints++)
	{
		d=PointInPickArea (*lpPoints);
		if (d<mindist)
		{         
			mindist=d;
			mini=i;
			lpPointsMin = lpPoints;
		}
	}		
	if (mini>=0)
	{
		lpPoints=lpPointsMin;		
		PickedPoint = mini; 
		RefPoint = *lpPoints;
		lpPoints =lpPoints2;
		RefDist1 = idist (RefPoint,*lpPoints);
		lpPoints += nPnts-1;
		RefDist2 = idist (RefPoint,*lpPoints);
		goto Found;
	}		
	return(TRUE);	
	
Found:lpPoints = lpPoints2;
	for (i=0;i<nPnts;i++,lpPoints++)
	{   
		if (i == PickedPoint)
			factor = 1.0;
		else if (MoveOnlyPickedPoint || !i || i==(nPnts-1))
			factor = 0;
		else if (i <= PickedPoint && RefDist1 > 0)
			factor = 1.0 - idist (*lpPoints,RefPoint) /(float) RefDist1; 
		else if (i >= PickedPoint && RefDist2 > 0)
			factor = 1.0 - idist (*lpPoints,RefPoint) /(float) RefDist2;
		else
			factor = 0; 
		if (factor == 1.0)
			*lpPoints = BasePtToFilePt (CurTheme->ClassPnt[0]);
		else
		{
			lpPoints->x += xmove * factor;
			lpPoints->y += ymove * factor; 
		}
		NewBounds.xmn = min (NewBounds.xmn,lpPoints->x);
		NewBounds.ymn = min (NewBounds.ymn,lpPoints->y);
		NewBounds.xmx = max (NewBounds.xmx,lpPoints->x);
		NewBounds.ymx = max (NewBounds.ymx,lpPoints->y);
	}
	return(TRUE);
}

BOOL SavePolyLine (short type)
{	int	i, iPickAp;
	HPPOINT	lpPoints, lppoints;
	HPDPOINT	lpDpoints; 
	LPMNMXCORD	lpRect, lpRect2;    
	LPMINMAX	lpRectFile, lpRectFile2;
	BOOL	FirstIn, BothIn, NeitherIn;
	float	factor;
	long	RefDist;
	POINT	RefPoint;
	int		*npt;
    
    switch (type)
    {
    	case 1:
    	case 3:
		hSavePoly = GSSiGlobAlloc (GMEM_MOVEABLE,(long)sizeof(MNMXCORD)+(long)nPnts*sizeof(DPOINT));
		nSavePoly = nPnts;
	    lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);; 
	    lpRect->xmn = DBL_MAX;
	    lpRect->ymn = DBL_MAX;
	    lpRect->xmx = -DBL_MAX;
	    lpRect->ymx = -DBL_MAX;   
	    lpRect2 = lpRect;
	    lpRect2++;
	    lpDpoints = (LPDPOINT) lpRect2;
		lpPoints = lpPoints2; 
		for (i=0;i<nPnts;i++,lpPoints++,lpDpoints++)
		{
			*lpDpoints = FilePtToBasePt(*lpPoints); 
			lpRect->xmn = min (lpRect->xmn,lpDpoints->x);
			lpRect->ymn = min (lpRect->ymn,lpDpoints->y);
			lpRect->xmx = max (lpRect->xmx,lpDpoints->x);
			lpRect->ymx = max (lpRect->ymx,lpDpoints->y);
		}
		if (type == 3)
		{   
			LPSHORT	pPolyParts, pPartLen;
			
			if (nPoly)
			{
				hSavePolyParts = GSSiGlobAlloc (GMEM_MOVEABLE,(nPoly+1)*2);
				pPolyParts = GlobalLock (hSavePolyParts); 
				*pPolyParts++ = nPoly;
				pPartLen = GlobalLock (hPolyPartLen);
				_fmemmove (pPolyParts,pPartLen,nPoly*2);
				GlobalUnlock (hSavePolyParts);
				GlobalUnlock (hPolyPartLen); 
			}
			else
				GSSiGlobFree (&hSavePolyParts);
		}   
		break;
		
		case 2:
		hSavePoly = GSSiGlobAlloc (GMEM_MOVEABLE,(long)sizeof(mnmxCor)+(long)nPnts*sizeof(DPOINT));
	    nSavePoly = nPnts;
	    lpRectFile = (LPMINMAX) GlobalLock (hSavePoly);; 
	    lpRectFile->xmn = DBL_MAX;
	    lpRectFile->ymn = DBL_MAX;
	    lpRectFile->xmx = -DBL_MAX;
	    lpRectFile->ymx = -DBL_MAX;   
	    lpRectFile2 = lpRectFile;
	    lpRectFile2++;
	    lppoints = (LPDPOINT) lpRectFile2;
		lpPoints = lpPoints2; 
		for (i=0;i<nPnts;i++,lpPoints++,lppoints++)
		{
			*lppoints = *lpPoints; 
			lpRectFile->xmn = min (lpRectFile->xmn,lppoints->x);
			lpRectFile->ymn = min (lpRectFile->ymn,lppoints->y);
			lpRectFile->xmx = max (lpRectFile->xmx,lppoints->x);
			lpRectFile->ymx = max (lpRectFile->ymx,lppoints->y);
		}   
		break;  
	}
	GlobalUnlock(hSavePoly);
	return(TRUE);
}

void DisplayOrthoPhoto ()
{	RECT Rect;
	DPOINT	WPoint;
	POINT	Point;
	double	ScaleDiff;
    
    if (!CurOrtho) GetCurOrtho(OrthoID);
	WPoint.x = CurOrtho->Bounds.xmn;
	WPoint.y = CurOrtho->Bounds.ymn;
	Point = BasePtToWinPt (WPoint);
	Rect.left = Point.x;
	Rect.bottom = Point.y;
	WPoint.x = CurOrtho->Bounds.xmx;
	WPoint.y = CurOrtho->Bounds.ymx;
	Point = BasePtToWinPt (WPoint);
	Rect.right = Point.x;
	Rect.top = Point.y;
/*	ScaleDiff = fabs (CurOrtho->Res - CurView->BaseUnitsPerPixel);*/
	ScaleDiff = CurOrtho->Res - CurView->BaseUnitsPerPixel;
/*	if (ScaleDiff < -1.0e-10)
		FillRect (CurView->hDC,&Rect,GetStockObject(LTGRAY_BRUSH));
	else
*/	{
		if (!CurOrtho->hDib)
		{
			LoadBitMap (CurOrtho->Name,CurOrtho->Frame, &CurOrtho->hDib, &CurOrtho->DeleteBM,
						CurOrtho->BitCount,CurOrtho->Width, CurOrtho->Height);
		}			
		if (CurOrtho->hDib)
			DisplayBMInVP (CurView->hDC, CurOrtho->hDib, FALSE);
	}
	return;
} 

BOOL DisplayBMInVP (HDC hDC, HANDLE hDib, BOOL Stretch)
{   BITMAPFILEHEADER bmfHead;
	LPBITMAPINFOHEADER	pDibInfo;
	LPSTR pImage;
	int		i, VPWidth, VPHeight, bmx, bmy, vpx, vpy, BMHeight, BMWidth;
	int		vpheight, vpwidth, vpyp, bmyp, bmwidth, bmheight;
	HDC hdcMem;
	HBITMAP	hbmPrev, hNewBM;
	double	BMTopLeftWx, BMTopLeftWy, VPOrigBMx, VPOrigBMy, BMTopLeftVPx, BMTopLeftVPy;
	double	BMBotLeftWx, BMBotLeftWy, BMTopRightWx, BMTopRightWy;
	double	BMBotLeftVPx,BMBotLeftVPy,BMTopRightVPx,BMTopRightVPy;
	double	VPBotLeftBMx, VPBotLeftBMy, VPTopRightBMx, VPTopRightBMy;
    double	VPTopLeftBMx, VPTopLeftBMy;

	RECT	bm, vp;

	pDibInfo = (LPBITMAPINFOHEADER) GlobalLock (hDib);
   	pImage =(LPSTR) pDibInfo + sizeof(BITMAPINFOHEADER) + pDibInfo->biClrUsed * 4;     

    if (Stretch)
    {
	   	SetStretchBltMode(hDC, StretchMode);
	   	i=StretchDIBits (hDC,destX,destY,
					   destW, destH,
    				   0,0,
   				       (int) pDibInfo->biWidth,
   				       (int)pDibInfo->biHeight,
   				       pImage,
   				      (LPBITMAPINFO)pDibInfo,
   				      (UINT)DIB_RGB_COLORS,
   				      (DWORD) SRCCOPY);
    }
   	else
   	{                                               
	    VPWidth = CurView->DrawRect.right - CurView->DrawRect.left +1;
	    VPHeight = CurView->DrawRect.bottom - CurView->DrawRect.top+1;
   		TRANS2 (0,CurOrtho->Height-1,&BMTopLeftWx,&BMTopLeftWy,hTranBMToBase);
   		TRANS2 (0,0,&BMBotLeftWx,&BMBotLeftWy,hTranBMToBase);
		TRANS2 (BMBotLeftWx-CurOrtho->Res/2,BMBotLeftWy-CurOrtho->Res/2,
				&BMBotLeftVPx,&BMBotLeftVPy,CurView->hTranBaseToWin);
   		TRANS2 (CurOrtho->Width-1,CurOrtho->Height-1,&BMTopRightWx,
   				&BMTopRightWy,hTranBMToBase);
		TRANS2 (BMTopRightWx+CurOrtho->Res/2,BMTopRightWy+CurOrtho->Res/2,
				&BMTopRightVPx,&BMTopRightVPy,CurView->hTranBaseToWin);
		TRANS2 (CurView->WBounds.xmn,CurView->WBounds.ymn,&VPOrigBMx,&VPOrigBMy,
			    hTranBaseToBM);
		TRANS2 (CurView->WBounds.xmx,CurView->WBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
			    hTranBaseToBM);
		TRANS2 (BMTopLeftWx-CurOrtho->Res/2,BMTopLeftWy+CurOrtho->Res/2,
				&BMTopLeftVPx,&BMTopLeftVPy,CurView->hTranBaseToWin);
   		if (BMBotLeftVPx < CurView->DrawRect.left)
   		{
   			bm.left = IDNINT (VPOrigBMx);
   			vp.left = CurView->DrawRect.left;
   		}
   		else
   		{
   			bm.left = 0;
   			vp.left = IDNINT (BMBotLeftVPx);
   		}
   		if (BMTopRightVPy < CurView->DrawRect.top)
   		{
   			bm.top = IDNINT (VPTopRightBMy);
   			vp.top = CurView->DrawRect.top;
   		}
   		else
   		{
   			bm.top = CurOrtho->Height-1;
   			vp.top = IDNINT (BMTopRightVPy);
   		}

   		if (BMTopRightVPx > CurView->DrawRect.right)
   		{
   			bm.right = IDNINT (VPTopRightBMx);
   			vp.right = CurView->DrawRect.right;
   		}
   		else
   		{
   			bm.right = CurOrtho->Width-1;
   			vp.right = IDNINT (BMTopRightVPx);
   		}
   		if (BMBotLeftVPy > CurView->DrawRect.bottom)
   		{
   			bm.bottom = IDNINT (VPOrigBMy);
   			vp.bottom = CurView->DrawRect.bottom;
   		}
   		else
   		{
   			bm.bottom = 0;
   			vp.bottom = IDNINT (BMBotLeftVPy);
   		}
/*   		if (Printing)
   		{ */
   			vpx = vp.left;
   			vpy = vp.top;
   			bmx = bm.left;
   			bmy = bm.top;
   			vpwidth = vp.right - vp.left + 1;
   			vpheight = vp.bottom - vp.top + 1;
   			bmheight = bm.top - bm.bottom + 1;
   			bmwidth = bm.right - bm.left + 1;
/*   			if (CurView->WindowZoomedToOrtho && CurView->OrthoRes >= 0)
   			{	vpheight=bmheight;
   				vpwidth=bmwidth;
   			} */
   		   	SetStretchBltMode(hDC, StretchMode);
		   	i=StretchDIBits (hDC,vpx,vpy,
							   vpwidth,vpheight,
		    				   bmx,bmy-bmheight+1,
		   				       bmwidth,bmheight,
		   				       pImage,
		   				      (LPBITMAPINFO)pDibInfo,
		   				      DIB_RGB_COLORS,
		   				      SRCCOPY);

			/*hdcMem = CreateCompatibleDC(GetDC(hWndMain));
		 	hNewBM = CreateDIBitmap (hdcMem,
									 (LPBITMAPINFOHEADER)&(pDibInfo->bmiHeader),
									  CBM_INIT,
									  pImage,
							     	 (LPBITMAPINFO)pDibInfo,DIB_RGB_COLORS);
			hbmPrev = SelectObject(hdcMem, hNewBM);

			BitBlt(hDC,vpx,vpy,width,height,hdcMem,bmx,bmy,SRCCOPY);

			SelectObject(hdcMem, hbmPrev);
			DeleteDC(hdcMem);
   		}
   		else
   		  SetDIBitsToDevice (hDC,vpx,vpy,
		     				 width,
		   				     height,
		    				 bmx, bmy,
		   				     0,pDibInfo->bmiHeader.biHeight,
		   				     pImage,
		   				     (LPBITMAPINFO)pDibInfo,
		   				     DIB_RGB_COLORS);*/
    }
 	GlobalUnlock (hDib);
	return (TRUE);
}


BOOL FAR PASCAL FULLBMMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{
	RECT	rect;
	HDC		hDC;
	char	drive[4],dir[128],leaf[16],ext[6], file[128];
	int		i,n;
	static	HANDLE	hNext=0,hPrior=0;
	LPSTR	lpchr, lpFile; 
	RECT	Rect;
	
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
    { 
    	DWORD	WVer;
    	int		WinVer;

		GetWindowRect(GetDesktopWindow(), &rect);
		 WVer = GetVersion ();
		 WinVer = HIBYTE(LOWORD(WVer));
		 if (WinVer <95) 
		 	SetWindowPos(hWndDlg, (HWND) NULL, 0, 0,rect.right, rect.bottom,SWP_NOZORDER);
		 else
			SetWindowPos(hWndDlg, (HWND) HWND_TOPMOST, 0,0,rect.right, rect.bottom,SWP_NOREDRAW);
    	 hNext = GSSiGlobAlloc (GHND,128);
    	 hPrior = GSSiGlobAlloc (GHND,128);
         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
    }    
		 break; /* End of WM_INITDIALOG                                 */
    
    case WM_DESTROY:
    	 GlobalFree (hNext);
    	 GlobalFree (hPrior);
    	 break;
    	 
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
           	case IDM_CONTINUE:
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
				 sndPlaySound(NULL,SND_ASYNC);
                 EndDialog(hWndDlg, FALSE);
                 break;

            case IDOK: 
				 sndPlaySound(NULL,SND_ASYNC);
             
            	 GetClientRect (hWndDlg,&Rect);
            	 hDC = GetDC (hWndDlg);
		         _fstrupr (FullBM);
		         if (_fstrstr(FullBM,".BMP"))
				 	DisplayBMFileInRect (hDC,FullBM,Rect,TRUE);
		         else if (_fstrstr(FullBM,".PCX"))
				 	DisplayPCXFileInRect (hDC,FullBM,Rect,TRUE);
   				 ReleaseDC (hWndDlg,hDC);

		         _splitpath (FullBM,drive,dir,leaf,ext);
		         sprintf (file,"%s%s%s.WAV",drive,dir,leaf);
				 sndPlaySound(file,SND_ASYNC|SND_NODEFAULT); 
		         i=_fstrlen(leaf);
		         lpchr = &leaf[i-1];
		         while (i && isdigit(*lpchr--))
		         	i--; 
		         n = atoi(&leaf[i]);
		         if (n)
		         {
					EnableMenuItem(GetMenu(hWndDlg), IDM_PRINT_ALL, MF_BYCOMMAND | MF_ENABLED);
		         	leaf[i]=0; 
		         	lpFile = GlobalLock(hPrior);
		         	sprintf (lpFile,"%s%s%s%i%s",drive,dir,leaf,n-1,ext);
		         	if (ExistFile(lpFile))
						EnableMenuItem(GetMenu(hWndDlg), IDM_PRIOR, MF_BYCOMMAND | MF_ENABLED);
		         	else
						EnableMenuItem(GetMenu(hWndDlg), IDM_PRIOR, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
					GlobalUnlock(hPrior);
					lpFile = GlobalLock(hNext);
		         	sprintf (lpFile,"%s%s%s%i%s",drive,dir,leaf,n+1,ext);
		         	if (ExistFile(lpFile))
						EnableMenuItem(GetMenu(hWndDlg), IDM_NEXT, MF_BYCOMMAND | MF_ENABLED);
		         	else
						EnableMenuItem(GetMenu(hWndDlg), IDM_NEXT, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
					GlobalUnlock(hNext);
				 }
				 else
					EnableMenuItem(GetMenu(hWndDlg), IDM_PRINT_ALL, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
				 DrawMenuBar(hWndDlg);
				   
                 break;
                   
            case IDM_NEXT: 
            	 lpchr = GlobalLock (hNext);
            	 _fstrcpy (FullBM,lpchr);
            	 GlobalUnlock (hNext);
		         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
            	 break;

            case IDM_PRIOR: 
            	 lpchr = GlobalLock (hPrior);
            	 _fstrcpy (FullBM,lpchr);
            	 GlobalUnlock (hPrior);
		         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
            	 break; 
            	 
            case IDM_PRINT:  
            	 PrintImage (hWndDlg,FullBM,1);
            	 break;

            case IDM_PRINT_ALL:  
            	 PrintImage (hWndDlg,FullBM,2);
            	 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} /* End of FULLBMMsgProc                                      */



void ShowFullBM(BOOL FullMenu)
{   int nRc; 
	char	Dialog[16];
	
	if (FullMenu)
		_fstrcpy (Dialog,"FULL_BM2");
	else
		_fstrcpy (Dialog,"FULL_BM");
     {
      FARPROC lpfnFULLBMMsgProc;

      lpfnFULLBMMsgProc = MakeProcInstance((FARPROC)FULLBMMsgProc, hInst);
      nRc = DialogBox(hInst, Dialog, hWndMain,lpfnFULLBMMsgProc);
      FreeProcInstance(lpfnFULLBMMsgProc);
     } /*

    {
    FARPROC lpfnRBUTOPSMsgProc;

    lpfnRBUTOPSMsgProc = MakeProcInstance((FARPROC)RBUTOPSMsgProc, hInst);
    nRc = DialogBox(hInst, (LPSTR)"FULLBM", hWndMain, lpfnRBUTOPSMsgProc);
    FreeProcInstance(lpfnRBUTOPSMsgProc);

   }     */

     return;

}

BOOL BlowUp (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{	static int	BlowUpStep;
	POINT	Point, ScreenPoint1, ScreenPoint2;
	RECT	RgnRect;
	HDC		hDC;
	HRGN	hRgn;
	int		i;

 switch (Message)
   {
   	case GF_INIT:
   		BlowUpStep = 1;
       	AddLBUTTON = TRUE;
		AddGraphicsFunction (hWnd, GF_ZOOM_RECT);
   		break;

    case GF_COMPLETE:
    	if (BlowUpStep == 1)
    	{	BlowUpRec.FromBounds = ZoomBoxRect;
    		BlowUpStep = 2;
			AddGraphicsFunction (hWnd, GF_ZOOM_RECT);
    	}
    	else if (BlowUpStep == 2)
    	{	BlowUpRec.ToBounds = ZoomBoxRect;
    		BlowUpStep == 0;
			CurView->NewBounds = BlowUpRec.FromBounds;
			Point.x = BlowUpRec.ToBounds.xmn;
			Point.y = BlowUpRec.ToBounds.ymx;
			ScreenPoint1 = FileCoordToWinCoord( Point);
			Point.x = BlowUpRec.ToBounds.xmx;
			Point.y = BlowUpRec.ToBounds.ymn;
			ScreenPoint2 = FileCoordToWinCoord( Point);
			hRgn = CreateRectRgn (ScreenPoint1.x,ScreenPoint1.y,ScreenPoint2.x,ScreenPoint2.y);
			hDC = GetDC (hWnd);
			i=SelectClipRgn (hDC,hRgn);
		    GetRgnBox (hRgn,&RgnRect);
		    SetBoundsRect2 (RgnRect,CurView->hDC);
			RemoveGraphicsFunction (hWnd);
		    RedisplayViewport(FALSE,FALSE);
			ReleaseDC (hWnd,hDC);
			DeleteObject (hRgn);
		}
       	break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}


BOOL PickImage (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{
 char key;
 POINT	MousePoint;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT: 
   		HaveDown=TRUE;
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam);
		ThemePickImage (MousePoint);
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}


BOOL OffsetPickedArea (int Item, double Dist)
{
#undef lpldatablehome
//#undef lpPolyHome 

#include "offsetmn.h"
//#include "polycom.h" 
 HDC hDC;
 char key;
 POINT	MousePoint, TestPoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 LPTHEME	pTheme;
 int	np, *npt, i, st;
 long i4, loc; 
 long size;
 LPDPOINT	lpDpoint, lpDNext;
 LPPOINT	lpPoint, lpPoly;
 int     NumCrvPts, LastLine;
 char	cCor[32];
 HANDLE	hPoly; 
 LPSTR	lpColon;
 HPEN	hPen, OldPen;  
 HANDLE	hDPoints;
 HPDPOINT	lpDPoints;
 
  lpLArea OutArea;
  Area A;
  lpArea lpA = (lpArea) &A;
  lpDLine  lpLda;
  HGLOBAL heap_ptr; 
  lpA->type = 1; //indicates a standard area offset is requested
  lpA->whos_callen = 1;
  lpA->offset_dist = -50;
  lpA->LinkDesc = 351;
  lpA->fillet_desc = 1321;  
  
    	if (PickList[Item].Type == 3)
    	{  lpPoint = lpPoints2;
    		pTheme = AddTheme (GF_SAVEPOLY_THEME);
    		CurView->PassID = 2;
			ProcessPickedItem (Item,FALSE);        		
    		DeleteTheme (pTheme);
    		if (hSavePoly)
    		{   LPMNMXCORD lpRect;
    		
	    		WaitCursor (1);
                nPnts = nSavePoly; 
                lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
                lpRect++;
                lpDpoint = (LPDPOINT) lpRect;
                lpA->NumPts = nPnts ;
   
          size = (long)sizeof(DLine)*(long)(lpA->NumPts); 
          heap_ptr = GSSiGlobAlloc( GHND, size); 
          if(heap_ptr == NULL)
          {   
             MessageBox(NULL,"Offset Out of Memory", "Offset Main Error",MB_ICONSTOP);
             return TRUE;
          }   
          lpA->lpDLineBase = (lpDLine) GlobalLock(heap_ptr);
          lpLda = lpA->lpDLineBase; 
          lpDNext = lpDpoint + 1;
          for(i=0; i < lpA->NumPts; i++,lpDpoint++,lpDNext++)
          { 
            lpLda = lpA->lpDLineBase + i;
            lpLda->F.x = lpDpoint->x;
            lpLda->F.y = lpDpoint->y;
            if(i < lpA->NumPts - 1)
            { 
              lpLda->T.x = lpDNext->x;
              lpLda->T.y = lpDNext->y;
            }  
            lpLda->Refn = i;
            lpLda->Desc = 21;
            lpLda->Type = 2;           
            lpLda->ID = 123+i;
          }
          lpDpoint = (LPDPOINT) lpRect;
          lpLda->T.x = lpDpoint->x;
          lpLda->T.y = lpDpoint->y;
          lpA->NumSides = lpA->NumPts;
		  lpA->offset_dist = -OffsetLineOffset;
;
          OFFSET_MAIN(lpA, &OutArea, &i4);          
          if(i4 != 0)
          {
             MessageBox(NULL,"Sorry... Unable to offset this item",
                       "Offset Error",MB_ICONINFORMATION);
          
          }
          GlobalUnlock(heap_ptr); 
          GlobalFree(heap_ptr);                           
		  GlobalUnlock(hSavePoly);
		  GlobalFree(hSavePoly);
		  hSavePoly = NULL;
		  if(i4 != 0)
		  {
                OffsetClose();
		     	WaitCursor (-1);
		        return TRUE;
		  }
		  BasePoint.x = 0e0;
		  lpLineBase = OutArea->Lines;
		  for (i=1,OutArea->Lines++; i <= OutArea->NumSides; i++,OutArea->Lines++)
		  {
		     if(OutArea->Lines->type == 3)
		     {
		       BasePoint.x = BasePoint.x + fabs(OutArea->Lines->lngth);
		     }
		  }
		  if(BasePoint.x > 0e0)//we found some curved lines
		  { //gotta figure out how many intermediate points these curves
		    //will be broken into. BasePoint.x has the length of the curves in feet.
		    //First I need to convert the length from Base coordinates to Window
		    //CurView->BaseUnitsPerPixel holds how many feet it takes per pixel
		    //I want a point approximately every 50 pixels  
		    if (CurView->BaseUnitsPerPixel==0)
		    	NumCrvPts=3;
		    else
		    	NumCrvPts = BasePoint.x / (CurView->BaseUnitsPerPixel * 2.0);
		    //POINT BasePtToWinPt (DPOINT WPoint)
          //  SetCurvPltCtol (CurView->BaseUnitsPerPixel * 2.0);
            SetCurvPltCtol (0.05 * 2.0);
            //DPOINT WinPtToBasePt (POINT Point)
		    //Second I need to figure out how many pixels I want to include in each
		    //line segment.  Make a call to       void SetCurvPltCtol (double INCTOL);
            //so it's dividing the line properly
		  
		  }	
          size = (long)sizeof(LPPOINT)*((long)OutArea->NumSides + NumCrvPts + 10);
          heap_ptr = NULL; 
          heap_ptr = GSSiGlobAlloc( GHND, size); 
          if(heap_ptr == NULL)
          {   
             MessageBox(NULL,"Offset Out of Memory", "Offset Main Error",MB_ICONSTOP);
             return TRUE;
          }   
          lpPoint = (LPPOINT) GlobalLock(heap_ptr);
          if(lpPoint == NULL)
          {   
             MessageBox(NULL,"Offset Out of Memory", "Offset Main Error",MB_ICONSTOP);
             return TRUE;
          }
          lpPoly = lpPoint;
               OutArea->Lines = lpLineBase + 1;   
               for (np = 0,i=1; i <= OutArea->NumSides; i++,OutArea->Lines++, lpPoint++)
               { //put the first point of the line in 
                    if(OutArea->Lines->type == 2)
                    {
			          BasePoint.x = OutArea->Lines->x1;
				   	  BasePoint.y = OutArea->Lines->y1;
					  *lpPoint = BasePtToWinPt(BasePoint);
					 // lpPoint->y = lpPoint->y - CurView->DrawRect.top;
					  if(i == 1)TestPoint = *lpPoint; 
			      /*    BasePoint.x = OutArea->Lines->x2;
				   	  BasePoint.y = OutArea->Lines->y2;
					  TestPoint = BasePtToWinPt(BasePoint); */
					  np++;
					  
					}  
                    if(OutArea->Lines->type == 3)
                    {
			       /*   BasePoint.x = OutArea->Lines->x1;
				   	  BasePoint.y = OutArea->Lines->y1;
					  TestPoint = BasePtToWinPt(BasePoint); 
			          BasePoint.x = OutArea->Lines->x2;
				   	  BasePoint.y = OutArea->Lines->y2;
					  TestPoint = BasePtToWinPt(BasePoint); 
			          BasePoint.x = OutArea->Lines->rx;
				   	  BasePoint.y = OutArea->Lines->ry;
					  TestPoint = BasePtToWinPt(BasePoint);*/ 
					  if(i == 1)
					  { 
			             BasePoint.x = OutArea->Lines->x1;
				   	     BasePoint.y = OutArea->Lines->y1;
					     TestPoint  = BasePtToWinPt(BasePoint);
					  }
                         st =  CURVPLT(OutArea->Lines, &np,  &lpPoint);
                         
                    }
               }
               *lpPoint = TestPoint;
               lpPoint = lpPoly;
               np++;
                     
                hDPoints = GSSiGlobAlloc (GMEM_MOVEABLE,(long)np*sizeof(DPOINT));
                lpDPoints = GlobalLock (hDPoints);
                for (i=0;i<np;i++,lpPoint++,lpDPoints++)
					*lpDPoints = WinPtToBasePt(*lpPoint);
				GlobalUnlock (hDPoints);
                lpDPoints = GlobalLock (hDPoints);
				AddAreaToOffsetFile (np, lpDPoints);
				GlobalUnlock (hDPoints);
				GlobalFree (hDPoints);
               
                DisplayPolyOff();  
                DisplayMaskArea();
                
                /*
					SetDisplayMode (CurView->hDC, GF_TEXTMODE);
					if (!FileMode)
					{
					    CurView->hRgn = CreateVPRgn();
					    SelectClipRgn (CurView->hDC,CurView->hRgn);
					    DeleteObject(CurView->hRgn);  
					}
               		hPen = CreatePen (PS_SOLID,6,RGB(255,0,0));
                    OldPen = SelectObject (CurView->hDC,hPen);
                    i = Polyline (CurView->hDC,lpPoly,np);*/
                    GlobalUnlock(heap_ptr); 
                    GlobalFree(heap_ptr); 
                   /*                          
                    SelectObject (CurView->hDC,OldPen);
                    DeleteObject (hPen); */
                    //GlobalUnlock (hPoly);
                    //GlobalFree (hPoly); 
                    //here I free up the memory used by the OutArea
                    OffsetClose();
		    		WaitCursor (-1);
	    		}
        	} 
        	return TRUE;
} 

BOOL OffsetHighlightedLines (long FromSeq, long ToSeq, double Dist)
{
#undef lpldatablehome
//#undef lpPolyHome 

#include "offsetmn.h"
//#include "polycom.h" 
 HDC hDC;
 char key;
 POINT	MousePoint, TestPoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 LPTHEME	pTheme;
 int	np, *npt, i, st;
 long i4, loc; 
 long size;
 LPDPOINT	lpDpoint, lpDNext;
 LPPOINT	lpPoint, lpPoly;
 int     NumCrvPts, LastLine;
 char	cCor[32];
 HANDLE	hPoly; 
 LPSTR	lpColon;
 HPEN	hPen, OldPen;  
 HANDLE	hDPoints;
 HPDPOINT	lpDPoints;
 lpLArea OutArea;
 Area A;
 lpArea lpA = (lpArea) &A;
 lpDLine  lpLda;
 HGLOBAL heap_ptr; 

  lpA->type = 1;  
  lpA->whos_callen = 1;
  lpA->LinkDesc = 351;
  lpA->fillet_desc = 1321;  
  
    	if (PickList[0].Type == 2)
    	{  lpPoint = lpPoints2;
    		pTheme = AddTheme (GF_SAVEPOLY_THEME);
    		CurView->PassID = 2;
			ProcessPickedItem (0,FALSE);        		
    		DeleteTheme (pTheme);
    		if (hSavePoly)
    		{   LPMNMXCORD lpRect;
    		
	    		WaitCursor (1);
                nPnts = nSavePoly; 
                lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
                lpRect++;
                lpDpoint = (LPDPOINT) lpRect;
                lpA->NumPts = (UINT)nPnts * 2 ;
   
		  size = (long)sizeof(DLine)*(long)(lpA->NumPts+1); 
		  heap_ptr = GSSiGlobAlloc( GHND, size); 
          lpA->lpDLineBase = (lpDLine) GlobalLock(heap_ptr);
          lpLda = lpA->lpDLineBase; 
          lpDNext = lpDpoint + 1;
          for(i=0; i < lpA->NumPts; i++,lpDpoint++,lpDNext++)
          { 
            lpLda = lpA->lpDLineBase + i;
            lpLda->F.x = lpDpoint->x;
            lpLda->F.y = lpDpoint->y;
            if(i < lpA->NumPts - 1)
            { 
              lpLda->T.x = lpDNext->x;
              lpLda->T.y = lpDNext->y;
            }  
            lpLda->Refn = i;
            lpLda->Desc = 21;
            lpLda->Type = 2;           
            lpLda->ID = 123+i;
          }
          lpDpoint = (LPDPOINT) lpRect;
          lpLda->T.x = lpDpoint->x;
          lpLda->T.y = lpDpoint->y;
          lpA->NumSides = lpA->NumPts;
		  lpA->offset_dist = -Dist;
;
          OFFSET_MAIN(lpA, &OutArea, &i4);          
          if(i4 != 0)
          {
             MessageBox(NULL,"Sorry... Unable to offset this item",
                       "Offset Error",MB_ICONINFORMATION);
          
          }
          GlobalUnlock(heap_ptr); 
          GlobalFree(heap_ptr);                           
		  GlobalUnlock(hSavePoly);
		  GlobalFree(hSavePoly);
		  hSavePoly = NULL;
		  if(i4 != 0)
		  {
                OffsetClose();
		     	WaitCursor (-1);
		        return TRUE;
		  }
		  BasePoint.x = 0e0;
		  lpLineBase = OutArea->Lines;
		  for (i=1,OutArea->Lines++; i <= OutArea->NumSides; i++,OutArea->Lines++)
		  {
		     if(OutArea->Lines->type == 3)
		     {
		       BasePoint.x = BasePoint.x + fabs(OutArea->Lines->lngth);
		     }
		  }
		  if(BasePoint.x > 0e0)//we found some curved lines
		  { //gotta figure out how many intermediate points these curves
		    //will be broken into. BasePoint.x has the length of the curves in feet.
		    //First I need to convert the length from Base coordinates to Window
		    //CurView->BaseUnitsPerPixel holds how many feet it takes per pixel
		    //I want a point approximately every 50 pixels  
		    if (CurView->BaseUnitsPerPixel==0)
		    	NumCrvPts=3;
		    else
		    	NumCrvPts = BasePoint.x / (CurView->BaseUnitsPerPixel * 2.0);
		    //POINT BasePtToWinPt (DPOINT WPoint)
          //  SetCurvPltCtol (CurView->BaseUnitsPerPixel * 2.0);
            SetCurvPltCtol (0.05 * 2.0);
            //DPOINT WinPtToBasePt (POINT Point)
		    //Second I need to figure out how many pixels I want to include in each
		    //line segment.  Make a call to       void SetCurvPltCtol (double INCTOL);
            //so it's dividing the line properly
		  
		  }	
          size = (long)sizeof(LPPOINT)*((long)OutArea->NumSides + NumCrvPts + 10);
          heap_ptr = NULL; 
          heap_ptr = GSSiGlobAlloc( GHND, size); 
          if(heap_ptr == NULL)
          {   
             MessageBox(NULL,"Offset Out of Memory", "Offset Main Error",MB_ICONSTOP);
             return TRUE;
          }   
          lpPoint = (LPPOINT) GlobalLock(heap_ptr);
          if(lpPoint == NULL)
          {   
             MessageBox(NULL,"Offset Out of Memory", "Offset Main Error",MB_ICONSTOP);
             return TRUE;
          }
          lpPoly = lpPoint;
               OutArea->Lines = lpLineBase + 1;   
               for (np = 0,i=1; i <= OutArea->NumSides; i++,OutArea->Lines++, lpPoint++)
               { //put the first point of the line in 
                    if(OutArea->Lines->type == 2)
                    {
			          BasePoint.x = OutArea->Lines->x1;
				   	  BasePoint.y = OutArea->Lines->y1;
					  *lpPoint = BasePtToWinPt(BasePoint);
					 // lpPoint->y = lpPoint->y - CurView->DrawRect.top;
					  if(i == 1)TestPoint = *lpPoint; 
			      /*    BasePoint.x = OutArea->Lines->x2;
				   	  BasePoint.y = OutArea->Lines->y2;
					  TestPoint = BasePtToWinPt(BasePoint); */
					  np++;
					  
					}  
                    if(OutArea->Lines->type == 3)
                    {
			       /*   BasePoint.x = OutArea->Lines->x1;
				   	  BasePoint.y = OutArea->Lines->y1;
					  TestPoint = BasePtToWinPt(BasePoint); 
			          BasePoint.x = OutArea->Lines->x2;
				   	  BasePoint.y = OutArea->Lines->y2;
					  TestPoint = BasePtToWinPt(BasePoint); 
			          BasePoint.x = OutArea->Lines->rx;
				   	  BasePoint.y = OutArea->Lines->ry;
					  TestPoint = BasePtToWinPt(BasePoint);*/ 
					  if(i == 1)
					  { 
			             BasePoint.x = OutArea->Lines->x1;
				   	     BasePoint.y = OutArea->Lines->y1;
					     TestPoint  = BasePtToWinPt(BasePoint);
					  }
                         st =  CURVPLT(OutArea->Lines, &np,  &lpPoint);
                         
                    }
               }
               *lpPoint = TestPoint;
               lpPoint = lpPoly;
               np++;
                     
                hDPoints = GSSiGlobAlloc (GMEM_MOVEABLE,(long)np*sizeof(DPOINT));
                lpDPoints = GlobalLock (hDPoints);
                for (i=0;i<np;i++,lpPoint++,lpDPoints++)
					*lpDPoints = WinPtToBasePt(*lpPoint);
				GlobalUnlock (hDPoints);
                lpDPoints = GlobalLock (hDPoints);
				AddAreaToOffsetFile (np, lpDPoints);
				GlobalUnlock (hDPoints);
				GlobalFree (hDPoints);
               
                DisplayPolyOff();  
                DisplayMaskArea();
                
                /*
					SetDisplayMode (CurView->hDC, GF_TEXTMODE);
					if (!FileMode)
					{
					    CurView->hRgn = CreateVPRgn();
					    SelectClipRgn (CurView->hDC,CurView->hRgn);
					    DeleteObject(CurView->hRgn);  
					}
               		hPen = CreatePen (PS_SOLID,6,RGB(255,0,0));
                    OldPen = SelectObject (CurView->hDC,hPen);
                    i = Polyline (CurView->hDC,lpPoly,np);*/
                    GlobalUnlock(heap_ptr); 
                    GlobalFree(heap_ptr); 
                   /*                          
                    SelectObject (CurView->hDC,OldPen);
                    DeleteObject (hPen); */
                    //GlobalUnlock (hPoly);
                    //GlobalFree (hPoly); 
                    //here I free up the memory used by the OutArea
                    OffsetClose();
		    		WaitCursor (-1);
	    		}
        	} 
        	return TRUE;
} 


BOOL AddAreaToOffsetFile (int np, HPDPOINT lpDPoint)
{
	MNMXCORD Rect; 
	HFILE	FidAO;
	OFSTRUCT	OFStruct;        
	long	loc; 
	int		i;  
	DPOINT	BasePoint;
	
	Rect.xmn = 1.0e50;
	Rect.ymn = 1.0e50;
	Rect.xmx = -1.0e50;
	Rect.ymx = -1.0e50; 
	if (AutoClearOffset)
    	FidAO = GSSiOpenFile("areaoff",&OFStruct,OF_CREATE);
    else
    {
    	FidAO = GSSiOpenFile("areaoff",&OFStruct,OF_READWRITE);
    	if (FidAO == HFILE_ERROR)
    		FidAO = GSSiOpenFile("areaoff",&OFStruct,OF_CREATE);
    }
    loc = _llseek (FidAO,0,2);
    _lwrite (FidAO,&np,2); 
    _lwrite (FidAO,&CurView->ID,2);
    _lwrite (FidAO,&Rect,sizeof(Rect));
    for (i=0;i<np;i++,lpDPoint++)
    {   
    	BasePoint = *lpDPoint;
		_lwrite(FidAO,&BasePoint,sizeof(BasePoint));
		Rect.xmn = min(BasePoint.x,Rect.xmn);
		Rect.ymn = min(BasePoint.y,Rect.ymn);
		Rect.xmx = max(BasePoint.x,Rect.xmx);
		Rect.ymx = max(BasePoint.y,Rect.ymx);
    } 
    _llseek (FidAO,loc,0);
    _lwrite (FidAO,&np,2); 
    _lwrite (FidAO,&CurView->ID,2);
    _lwrite (FidAO,&Rect,sizeof(Rect));
    _lclose(FidAO);
	return TRUE;
}



BOOL OffsetAreaHP (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{
 POINT	MousePoint;
 DPOINT	BasePoint;  
 char	key;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT:
   		HaveDown=TRUE;
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    
		if (OffsetLineOffset < 0.0001)
        {   
	         MessageBox(NULL,"Offset distance not set", "Offset Error",MB_ICONSTOP);
	         return TRUE;
        }   
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam);
	    if (!CurView->hTranWinToBase) break;
        BasePoint=WinPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
        if (NumPicked > 0) 
        	OffsetPickedArea (NumPicked-1,OffsetLineOffset);
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL OffsetPolylinesHLT (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{
 POINT	MousePoint;
 DPOINT	BasePoint;  
 char	key;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,0, 0L); 
   		break;
    
    case GF_EXECUTE: 
		if (OffsetLineOffset < 0.0001)
        {   
	         MessageBox(NULL,"Offset distance not set", "Offset Error",MB_ICONSTOP);
	         return TRUE;
        }   
        OffsetHighlightedLines (0,100000,OffsetLineOffset);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}


void ClearPolyOff (void)
{
 OFSTRUCT OFStruct;
 int	FidAO; 

   	ClearMaskArea ();
    if (!ExistFile("areaoff")) return;
    FidAO = GSSiOpenFile("areaoff",&OFStruct,OF_DELETE);
    return;
}

void SetViewport(int iview)
{
   	CurView = pViewports[iview-1];
   	return;
}

BOOL DisplayPolyOff (void)
{
 DPOINT	BasePoint;
 BOOL	Cancel;
 LPTHEME	pTheme;
 int	np, i, nRc;
 LPDPOINT	lpDpoint;
 LPPOINT	lpPoint, lpPoly;
 char	cCor[32], str[64];
 HANDLE	hPoly; 
 LPSTR	lpColon;
 HPEN	hPen, OldPen;
 MNMXCORD Rect; 
 OFSTRUCT OFStruct;
 int	FidAO, ID; 
    
    if (!ExistFile("areaoff"))
    	return FALSE;  
    FidAO = GSSiOpenFile ("areaoff",&OFStruct,OF_READ);
    if (FidAO == HFILE_ERROR) return FALSE;
    while (_lread (FidAO,&np,2) == 2)
    {
	    _lread (FidAO,&ID,2); 
	    _lread (FidAO,&Rect,sizeof(Rect));
	    hPoly = GSSiGlobAlloc(GMEM_MOVEABLE,(np+1)*sizeof(POINT));
	    lpPoint = (LPPOINT) GlobalLock (hPoly);
	    lpPoly = lpPoint;  
	    if (MaskOffsetLine && CurView->ID == ID)
	    {   
			LPMNMXCORD	lpRect; 
	    	
	    	ClearMaskArea ();
		    CurView->hMaskArea = GSSiGlobAlloc (GHND,sizeof(MNMXCORD)+(long)np*sizeof(DPOINT));
		    CurView->NumMaskPoints = np;
		    lpRect = (LPMNMXCORD) GlobalLock (CurView->hMaskArea);
		    *lpRect = Rect; 
		    lpRect++;
		    lpDpoint = (LPDPOINT) lpRect;  
		    MaskZoomArea = TRUE;
    	}
	    
	    for (i=0;i<np;i++,lpPoint++)
	    {
			_lread(FidAO,&BasePoint,sizeof(BasePoint)); 
		    if (MaskOffsetLine && CurView->ID == ID)
				*lpDpoint++=BasePoint;
			*lpPoint = BasePtToWinPt(BasePoint);
	    }
	    if (MaskOffsetLine && CurView->ID == ID)
			GlobalUnlock (CurView->hMaskArea);
	    if (CurView->ID != ID) goto Next;
		if (!CurView->Active) goto Next;
		if (!RectInWBounds (Rect)) goto Next;
	    *lpPoint= *lpPoly;
	    np++; 
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
		if (!FileMode)
		{
		    CurView->hRgn = CreateVPRgn();
		    SelectClipRgn (CurView->hDC,CurView->hRgn);
		    DeleteObject(CurView->hRgn);  
		}
		hPen = CreatePen (PS_SOLID,6,RGB(255,0,0));
	    OldPen = SelectObject (CurView->hDC,hPen);
	    i=Polyline (CurView->hDC,lpPoly,np);
	    SelectObject (CurView->hDC,OldPen);
	    DeleteObject (hPen); 
	Next:
	    GlobalUnlock (hPoly);
	    GlobalFree (hPoly);  
	}
	_lclose(FidAO);
	return TRUE; 
}

BOOL FAR PASCAL VAROFFSETMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{
	RECT	rect;
	HDC		hDC;
	char	str[32];

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
		 if (HPAreaOffset==-500)
			SendDlgItemMessage (hWndDlg,IDC_OFF_500,BM_SETCHECK,TRUE,0L);
		 else if (HPAreaOffset==-400)
			SendDlgItemMessage (hWndDlg,IDC_OFF_400,BM_SETCHECK,TRUE,0L);
		 else if (HPAreaOffset==-350)
			SendDlgItemMessage (hWndDlg,IDC_OFF_350,BM_SETCHECK,TRUE,0L);
		 else if (HPAreaOffset==-200)
			SendDlgItemMessage (hWndDlg,IDC_OFF_200,BM_SETCHECK,TRUE,0L);
		 else if (HPAreaOffset==-100)
			SendDlgItemMessage (hWndDlg,IDC_OFF_100,BM_SETCHECK,TRUE,0L);
		 else
		 {
			 SendDlgItemMessage (hWndDlg,IDC_OFF_500,BM_SETCHECK,FALSE,0L);
			 SendDlgItemMessage (hWndDlg,IDC_OFF_400,BM_SETCHECK,FALSE,0L);
			 SendDlgItemMessage (hWndDlg,IDC_OFF_350,BM_SETCHECK,FALSE,0L);
			 SendDlgItemMessage (hWndDlg,IDC_OFF_200,BM_SETCHECK,FALSE,0L);
			 SendDlgItemMessage (hWndDlg,IDC_OFF_100,BM_SETCHECK,FALSE,0L);
			 sprintf (str,"%f",fabs(HPAreaOffset));
			 SetDlgItemText (hWndDlg,IDC_OFFSET_OTHER,str);
		 }
		 if (LocateOpt == 1)
			 SendDlgItemMessage (hWndDlg,IDC_BY_ADDRESS,BM_SETCHECK,TRUE,0L);
		 else if (LocateOpt == 2)
			 SendDlgItemMessage (hWndDlg,IDC_BY_PID,BM_SETCHECK,TRUE,0L);
		 else if (LocateOpt == 3)
			 SendDlgItemMessage (hWndDlg,IDC_WITH_MOUSE,BM_SETCHECK,TRUE,0L);

		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
                 
            case IDC_OFF_500:
            case IDC_OFF_400:
            case IDC_OFF_350:
            case IDC_OFF_200:
            case IDC_OFF_100:
            	 SetDlgItemText (hWndDlg,IDC_OFFSET_OTHER,"");
            	 break;
            	 
            case IDC_OFFSET_OTHER:
                 switch (HIWORD(lParam))
                 {	case EN_SETFOCUS:
						 SendDlgItemMessage (hWndDlg,IDC_OFF_500,BM_SETCHECK,FALSE,0L);
						 SendDlgItemMessage (hWndDlg,IDC_OFF_400,BM_SETCHECK,FALSE,0L);
						 SendDlgItemMessage (hWndDlg,IDC_OFF_350,BM_SETCHECK,FALSE,0L);
						 SendDlgItemMessage (hWndDlg,IDC_OFF_200,BM_SETCHECK,FALSE,0L);
						 SendDlgItemMessage (hWndDlg,IDC_OFF_100,BM_SETCHECK,FALSE,0L); 
				 	break;
				 }
				 break;
				 
            case IDOK: 
				 if (SendDlgItemMessage (hWndDlg,IDC_BY_ADDRESS,
				 						 (UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
				 	LocateOpt = 1;
				 else if (SendDlgItemMessage (hWndDlg,IDC_BY_PID,
				 						 (UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
				 	LocateOpt = 2;            
				 else if (SendDlgItemMessage (hWndDlg,IDC_WITH_MOUSE,
				 						 (UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
				 	LocateOpt = 3;            
				 if (SendDlgItemMessage (hWndDlg,IDC_OFF_500,
				 						 (UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
				 	HPAreaOffset=-500;
				 else if (SendDlgItemMessage (hWndDlg,IDC_OFF_400,
				 						 (UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
				 	HPAreaOffset=-400;            
				 else if (SendDlgItemMessage (hWndDlg,IDC_OFF_350,
				 						 (UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
				 	HPAreaOffset=-350;            
				 else if (SendDlgItemMessage (hWndDlg,IDC_OFF_200,
				 						 (UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
				 	HPAreaOffset=-200;            
				 else if (SendDlgItemMessage (hWndDlg,IDC_OFF_100,
				 						 (UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
				 	HPAreaOffset=-100; 
				 else
				 {
				 	GetDlgItemText (hWndDlg,IDC_OFFSET_OTHER,str,32);
				 	HPAreaOffset = atof(str); 
				 	if (HPAreaOffset==0.0)
				 	{
						MessageBox( GetFocus(), "Invalid offset entered","Error", MB_OK);
						break;
				 	}
				 }           
                 EndDialog(hWndDlg, TRUE);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} /* End of FULLBMMsgProc                                      */

HBITMAP PointAt (HDC hDC,POINT Coord,HBITMAP hSavedBM, RECT *SavedRect)
{	POINT ArrowPnt[8] = {0,0,-20,0,-8,4,-23,19,-19,23,-4,8,0,20,0,0};
 	HBRUSH hOldBrush, hRedBrush, hOldPen, hRedPen;
 	int OldMode, i;
 	static POINT	Points[8];
 	POINT ScreenCoord;

	if (hSavedBM)
	{
		RestoreScreen (hDC,hSavedBM,*SavedRect);
		DeleteObject (hSavedBM);
	}
 	SavedRect->top = INT_MAX;
 	SavedRect->bottom = INT_MIN;
 	SavedRect->left = INT_MAX;
 	SavedRect->right = INT_MIN;
 	for (i=0; i<8; i++)
 	{	Points[i].x = ArrowPnt[i].x + Coord.x;
 		Points[i].y = ArrowPnt[i].y + Coord.y;
 		SavedRect->left = min (SavedRect->left,Points[i].x);
 		SavedRect->right = max (SavedRect->right,Points[i].x);
 		SavedRect->top = min (SavedRect->top,Points[i].y);
 		SavedRect->bottom = max (SavedRect->bottom,Points[i].y);
 	}
 	hSavedBM = SaveScreen (hDC, *SavedRect);

	hRedBrush =   CreateSolidBrush(RGB(255,   0,   0));
	hRedPen =   CreatePen(PS_SOLID,1,RGB(255,   0,   0));
	hOldBrush = SelectObject (hDC,hRedBrush);
	hOldPen = SelectObject (hDC,hRedPen);
 	Polygon (hDC,Points,8);
 	SelectObject (hDC,hOldBrush);
 	DeleteObject(hRedBrush);

 	SelectObject (hDC,hOldPen);
 	DeleteObject(hRedPen);
 	ReleaseDC (hWndMain,hDC);
 	return hSavedBM;

} 
   

BOOL DrawSymbol (HDC hDC, int isym,POINT Point,int symsiz,COLORREF Color)
{   
	HBRUSH	brush, CurBrush;
	HPEN	pen, CurPen;  
	int		i;           
	DPOINT	NewPoint, SymPoint; 
	LPPOINT	pPoints, pPnt;
	LPSYMBOLVECTOR	pSymVector; 
	HANDLE	hPoints;
	
	
	if (!LoadSymbol (isym)) return FALSE;
	
	hPoints = GlobalAlloc(GMEM_MOVEABLE,NumSymVectors*sizeof(POINT));
	pPoints = GlobalLock (hPoints);   
	pSymVector = GlobalLock (hSymVectors);
	
	SymPoint.x = Point.x;
	SymPoint.y = Point.y;
	for (i=0,pPnt=pPoints;i<NumSymVectors;i++,pPnt++,pSymVector++)
	{
		NewPoint = dnewpt (SymPoint,pSymVector->AZM+PY,pSymVector->Dist*symsiz); 
		pPnt->x = IDNINT(NewPoint.x);
		pPnt->y = IDNINT(NewPoint.y);
	}
	
	brush = CreateSolidBrush (Color);
	CurBrush = SelectObject (hDC,brush); 
	pen = CreatePen (PS_SOLID,0,AutoYellow(Color));
	CurPen = SelectObject (hDC,pen);
	i = Polygon (hDC,pPoints,NumSymVectors);         
	SelectObject (hDC,CurBrush);                         
	SelectObject (hDC,CurPen);  
	DeleteObject (pen);
	DeleteObject (brush);  
	GlobalUnlock (hPoints);
	GlobalFree (hPoints);  
	UnloadSymbol();

	return TRUE;
}

BOOL LoadSymbol (int isym)
{                        
	LPSYMBOLVECTOR	pSymVector;
	
	switch (isym)
	{   
		default:
		case 3300:
			NumSymVectors = 4;
			hSymVectors = GlobalAlloc (GMEM_MOVEABLE,NumSymVectors*sizeof(SYMBOLVECTOR));
			pSymVector = GlobalLock (hSymVectors);
			
			pSymVector->AZM = 0.75 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 0.25 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.75 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.25 * PY;
			pSymVector++->Dist =  1.0;  
			
			GlobalUnlock (hSymVectors);
			break; 
			
		case 3301:
			NumSymVectors = 3;
			hSymVectors = GlobalAlloc (GMEM_MOVEABLE,NumSymVectors*sizeof(SYMBOLVECTOR));
			pSymVector = GlobalLock (hSymVectors);
			
			pSymVector->AZM = 0.5 * PY;
			pSymVector++->Dist =  1;  
			
			pSymVector->AZM = 1.25 * PY;
			pSymVector++->Dist =  1;  
			
			pSymVector->AZM = 1.75 * PY;
			pSymVector++->Dist =  1;  
			
			GlobalUnlock (hSymVectors);
			break; 
			
		case 3302:
			NumSymVectors = 4;
			hSymVectors = GlobalAlloc (GMEM_MOVEABLE,NumSymVectors*sizeof(SYMBOLVECTOR));
			pSymVector = GlobalLock (hSymVectors);
			
			pSymVector->AZM = 0.5 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.0 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.5 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 2.0 * PY;
			pSymVector++->Dist =  1.0;  
			
			GlobalUnlock (hSymVectors);
			break; 
			
		case 3303:
			NumSymVectors = 8;
			hSymVectors = GlobalAlloc (GMEM_MOVEABLE,NumSymVectors*sizeof(SYMBOLVECTOR));
			pSymVector = GlobalLock (hSymVectors);
			
			pSymVector->AZM = 0.5 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 0.75 * PY;
			pSymVector++->Dist =  0.25;  
			
			pSymVector->AZM = 1.0 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.25 * PY;
			pSymVector++->Dist =  0.25;  
			
			pSymVector->AZM = 1.5 * PY;
			pSymVector++->Dist =  1.0;  
			
			pSymVector->AZM = 1.75 * PY;
			pSymVector++->Dist =  0.25;  
			
			pSymVector->AZM = 2.0 * PY;
			pSymVector++->Dist =  1.0;  

			pSymVector->AZM = 2.25 * PY;
			pSymVector++->Dist =  0.25;  
			
			GlobalUnlock (hSymVectors);
			break; 
			
	} 
	return TRUE;
}

void UnloadSymbol (void)
{
	GlobalFree (hSymVectors);
}

BOOL GetPenRedefColor(int ipen,COLORREF *Color)
{   COLORREF	*lpNewColors;
            
	if (!CurView->hPenRedef)
		return FALSE;
	lpNewColors = GlobalLock (CurView->hPenRedef);
	lpNewColors += ipen;   
	if (*lpNewColors != ULONG_MAX)
	{
		*Color = *lpNewColors;
		GlobalUnlock (CurView->hPenRedef);
		return TRUE;
	}
	GlobalUnlock (CurView->hPenRedef);
	return FALSE;
} 

void AddPenRedef (int ipen, COLORREF Color)
{   COLORREF	*lpNewColors;
	if (!CurView->hPenRedef)
	{   int	i;
	
		CurView->hPenRedef = GlobalAlloc (GMEM_MOVEABLE,sizeof(COLORREF)*MAXPENS);
		lpNewColors = GlobalLock (CurView->hPenRedef);
		for (i=0;i<MAXPENS;i++,lpNewColors++)   
			*lpNewColors = ULONG_MAX;
		GlobalUnlock (CurView->hPenRedef);
	}
	lpNewColors = GlobalLock (CurView->hPenRedef);
	lpNewColors += ipen;   
	*lpNewColors = Color;
	GlobalUnlock (CurView->hPenRedef);
	
	return;
} 

BOOL LoadDisplayRedefFile (LPSTR Name)
{   COLORREF	*lpNewColors;
	short	HavePenRedef;
	OFSTRUCT	OFStruct;
	HFILE	Fid;  
	NEWOBJECT_v0	OldNewObject; 
	int		i,version=0,Signature;
	
	if (!*Name) return FALSE;
	Fid = GSSiOpenFile (Name,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR) return FALSE; 
	_llseek (Fid,-(4),2);
	_lread (Fid,&Signature,2);  
	if (Signature == 20852) 
	{
		_lread (Fid,&version,2);
	    if (version > 1)
	    {	_lclose(Fid);
			MessageBox( GetFocus(), "This redef file version is not recognized",Name, MB_OK);
			return(FALSE);
	    }
	}
	_llseek (Fid,0,0);
	_lread (Fid,&HavePenRedef,2); 
	if (HavePenRedef)
	{
		if (!CurView->hPenRedef)
			CurView->hPenRedef = GlobalAlloc (GMEM_MOVEABLE,sizeof(COLORREF)*MAXPENS);
		lpNewColors = GlobalLock (CurView->hPenRedef); 
		_lread (Fid,lpNewColors,sizeof(COLORREF)*MAXPENS);
		GlobalUnlock (CurView->hPenRedef);
	}
	_lread (Fid,&CurView->NumNewObjects,2);
	if (CurView->NumNewObjects) 
	{
		_lread (Fid,&CurView->NewObjectMap,3201*2); 
		if (version)
			_lread (Fid,&CurView->NewObject,CurView->NumNewObjects*sizeof(NEWOBJECT));
		else
		{	
			for (i=0;i<CurView->NumNewObjects;i++)
			{	
				_lread (Fid,&OldNewObject,sizeof(NEWOBJECT_v0)); 
				CurView->NewObject[i].Type =OldNewObject.Type;
				CurView->NewObject[i].Style =OldNewObject.Style;
				CurView->NewObject[i].R =OldNewObject.R;
				CurView->NewObject[i].G =OldNewObject.G;
				CurView->NewObject[i].B =OldNewObject.B;
				CurView->NewObject[i].Width =OldNewObject.Width; 
				CurView->NewObject[i].Handle=0;
			}
		}
	}
	_lclose (Fid);
	return TRUE;
} 

BOOL SaveDisplayRedefFile (LPSTR Name)
{   COLORREF	*lpNewColors;
	OFSTRUCT	OFStruct;
	short	HavePenRedef;  
	int		Signature, Version=1;
	HFILE	Fid;
	
	if (!*Name) return FALSE;
	Fid = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
	if (Fid == HFILE_ERROR) return FALSE;
	if (CurView->hPenRedef)
	{          
		HavePenRedef = 1;
		_lwrite (Fid,&HavePenRedef,2); 
		lpNewColors = GlobalLock (CurView->hPenRedef); 
		_lwrite (Fid,lpNewColors,sizeof(COLORREF)*MAXPENS);
		GlobalUnlock (CurView->hPenRedef);
	}
	else
	{
		HavePenRedef = 0;
		_lwrite (Fid,&HavePenRedef,2); 
	}  
	_lwrite (Fid,&CurView->NumNewObjects,2);
	if (CurView->NumNewObjects) 
	{
		_lwrite (Fid,&CurView->NewObjectMap,3201*2);
		_lwrite (Fid,&CurView->NewObject,CurView->NumNewObjects*sizeof(NEWOBJECT)); 
	}
	Signature = 20852;
    _lwrite (Fid,&Signature,2);
    _lwrite (Fid,&Version,2);
	
	_lclose (Fid);
	return TRUE;
}

BOOL ChangePenColor (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 char key;
 POINT	MousePoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 COLORREF	NewColor;
 static	HaveDown=FALSE;
 
 if (Message == GF_INIT)
 {
	if (!EditName[0])
	{
 		MessageBox(GetFocus(), "No Update File", NULL,MB_ICONEXCLAMATION|MB_OK);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
        return TRUE;
	}
   	AddLBUTTON = TRUE;
 } 
 if (Message == WM_LBUTTONDOWN || Message == GF_INIT)
 {
   	HaveDown = TRUE;
   	return TRUE;
 }
    	
 switch (Message)
   {
    case GF_CLOSE: 
        RemoveGraphicsFunction (hWnd);
    	break;
        
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam);
	    if (!CurView->hTranWinToBase) break;
        BasePoint=WinPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {
	        DoPaint=FALSE;
        	if(GetColor(hWnd,&NewColor))
        	{   
	    		_fstrcpy (PltName,EditName);
	    		PltType = 2;
				if (OpenMap (CurView->hWnd,NULL))
				{   
					int clr;
					ModifyPen = PickList[NumPicked-1].ipen;
				    _llseek(Fid,ColorPaletteOffset,0);  
				    ModPenLoc=0;
				    ProcessPrimarySeg (NULL, NULL, NULL,FALSE); 
				    if (ModPenLoc)
				    {
					    _llseek (Fid,ColorPaletteOffset+ModPenLoc,0);
					    clr = GetRValue(NewColor);
			 			_lwrite (Fid,&clr,2); 
					    clr = GetGValue(NewColor);
			 			_lwrite (Fid,&clr,2);
					    clr = GetBValue(NewColor);
			 			_lwrite (Fid,&clr,2); 
			 		}
					CloseMap ();
				} 
			    RedisplayViewport(FALSE,FALSE);
	        }
			DoPaint=TRUE;
        }
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL ChangePenNumber (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 char key, str[16];
 POINT	MousePoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 COLORREF	NewColor;
 static	HaveDown=FALSE;
 
 if (Message == GF_INIT)
 {
	if (!EditName[0])
	{
 		MessageBox(GetFocus(), "No Update File", NULL,MB_ICONEXCLAMATION|MB_OK);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
        return TRUE;
	}
   	AddLBUTTON = TRUE;
 } 
 if (Message == WM_LBUTTONDOWN || Message == GF_INIT)
 {
   	HaveDown = TRUE;
   	return TRUE;
 }
    	
 switch (Message)
   {
    case GF_CLOSE: 
        RemoveGraphicsFunction (hWnd);
    	break;
        
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam);
	    if (!CurView->hTranWinToBase) break;
        BasePoint=WinPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {
	        DoPaint=FALSE; 
	        str[0]=0;
           	if (GetTextString (hWnd,str,sizeof(str),"Enter new pen number"))
        	{   
        		int	newpen;
        		
	           	newpen = atoi (str);
	    		_fstrcpy (PltName,EditName);
	    		PltType = 2;
				if (OpenMap (CurView->hWnd,NULL))
				{   
				    _llseek (Fid,PickList[NumPicked-1].Segment +
				    			 PickList[NumPicked-1].Element+2+2,0);
		 			_lwrite (Fid,&newpen,2); 
					CloseMap ();
				} 
			    RedisplayViewport(FALSE,FALSE);
	        }
			DoPaint=TRUE;
        }
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}


BOOL ChangeRedefColor (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 char key;
 POINT	MousePoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 COLORREF	NewColor;
 static	HaveDown=FALSE;
 
 if (Message == WM_LBUTTONDOWN || Message == GF_INIT)
 {
   	HaveDown = TRUE;
   	AddLBUTTON = TRUE;
   	return TRUE;
 }
    	
 if (ForAllVis)
 {  
 	if (Message == WM_LBUTTONUP && HaveDown)
 	{
	    if(GetColor(hWnd,&NewColor))
	    {   
	    	int	idesc, newob;
	    	
	    	newob = NextNewObject();
			SetClassColor (newob,1,NewColor);
	    	for (idesc=1;idesc<3201;idesc++) 
	    	{
				if (CurView->CurVisType[idesc])
					CurView->NewObjectMap[idesc]=newob+1;
			}
		    RedisplayViewport(FALSE,FALSE);
	    }
		RemoveGraphicsFunction (hWnd); 
	} 
	return TRUE;
 }


 switch (Message)
   {
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam);
	    if (!CurView->hTranWinToBase) break;
        BasePoint=WinPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {
	        DoPaint=FALSE;
        	if(GetColor(hWnd,&NewColor))
        	{   
        		AddPenRedef (PickList[NumPicked-1].ipen,NewColor);
			    RedisplayViewport(FALSE,FALSE);
	        }
			DoPaint=TRUE;
        }
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL SetMaskArea(int Item)
{   
	LPTHEME		pTheme, SaveTheme;
	
	ClearMaskArea();
	if (!GetMaskArea) return FALSE;
   	if (PickList[Item].Type != 3) return FALSE; 
   	SaveTheme = CurTheme;
	pTheme = AddTheme (GF_SAVEPOLY_THEME); 
	CurView->PassID = 2;
	IgnoreBounds = TRUE;
	ProcessPickedItem (Item,FALSE);    
	IgnoreBounds = FALSE;;    		
    DeleteTheme (pTheme);
    CurTheme = SaveTheme;
    if (!hSavePoly) return FALSE;
    CurView->hMaskArea = hSavePoly;
    CurView->NumMaskPoints = nSavePoly;
    hSavePoly = 0;
	return TRUE;
}  

void ClearMaskArea (void)
{   
	if (!CurView)
		return;
	if (!CurView->hMaskArea) return; 
	GlobalFree (CurView->hMaskArea);
	CurView->hMaskArea = 0;
	return;
}

BOOL DisplayMaskArea (void)
{
 LPDPOINT	lpBasePoint;
 int	i;
 int	Nump; 
 LPDPOINT	lpDpoint;
 LPPOINT	lpPoint, lpPoly;
 HPEN	hPen, OldPen;
 LPMNMXCORD	lpRect; 
 HANDLE	hPoly;
    
	if (!CurView->hMaskArea) 
    	return FALSE;
    Nump = CurView->NumMaskPoints;  
    lpRect = (LPMNMXCORD) GlobalLock (CurView->hMaskArea);
	lpRect++;
	lpBasePoint = (LPDPOINT) lpRect;
	lpRect--;
    hPoly = GSSiGlobAlloc(GHND,(Nump+1)*sizeof(POINT));
    lpPoint = (LPPOINT) GlobalLock (hPoly);
    lpPoly = lpPoint; 
	if (!RectInWBounds (*lpRect))  
		Nump=0;
    for (i=0;i<Nump;i++,lpPoint++,lpBasePoint++)
    {
		*lpPoint = BasePtToWinPt(*lpBasePoint);
    }
    *lpPoint= *lpPoly;
    Nump++; 
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	if (!FileMode)
	{
	    CurView->hRgn = CreateVPRgn();
	    SelectClipRgn (CurView->hDC,CurView->hRgn);
	    DeleteObject(CurView->hRgn);  
	}
	hPen = CreatePen (PS_SOLID,4,RGB(255,0,0));
    OldPen = SelectObject (CurView->hDC,hPen); 
    if (OutlineZoomArea)
	    i=Polyline (CurView->hDC,lpPoly,Nump);
    if (MaskZoomArea)
    {   
    	HBRUSH	BkBrush, OldBrush;
    	HANDLE	hMaskPoly;
    	LPPOINT pMask;
    	
	    SelectObject (CurView->hDC,GetStockObject(NULL_PEN)); 
	    BkBrush = CreateSolidBrush (CurView->BackGroundColor);
	    OldBrush = SelectObject (CurView->hDC,BkBrush);
	    
	    hMaskPoly = GlobalAlloc (GMEM_MOVEABLE,(Nump+5)*sizeof(POINT));
	    pMask = GlobalLock (hMaskPoly);
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.bottom+1; 
	    for (i=0;i<Nump;i++,lpPoint--,pMask++)
	    	*pMask = *lpPoint;
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.bottom+1; 
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.top-1;
	    pMask->x = CurView->DrawRect.right+1;
	    pMask++->y = CurView->DrawRect.top-1;
	    pMask->x = CurView->DrawRect.right+1;
	    pMask->y = CurView->DrawRect.bottom+1;
	    GlobalUnlock (hMaskPoly);
	    pMask = GlobalLock (hMaskPoly);
	    i=Polygon (CurView->hDC,pMask,Nump+5);
	    GlobalUnlock (hMaskPoly);
	    GlobalFree (hMaskPoly);
	    SelectObject (CurView->hDC,OldBrush);
    }  
    SelectObject (CurView->hDC,OldPen);
    DeleteObject (hPen); 
    GlobalUnlock (hPoly);
    GlobalFree (hPoly); 
Exit:
    GlobalUnlock (CurView->hMaskArea);
	return TRUE; 
} 

BOOL PointInEditBounds (LPDPOINT pPoint)
{
     if (pPoint->x > EditBounds.xmx ||
         pPoint->y > EditBounds.ymx ||
         pPoint->x < EditBounds.xmn ||
         pPoint->y < EditBounds.ymn) return (FALSE);
     return (TRUE);
}

BOOL DisplayEditLimits (void)
{
	int	i;
	int	Nump; 
	LPINT	lpNump;
	LPDPOINT	lpDpoint; 
	DPOINT	DPoint;
	LPPOINT	lpPoint;
	POINT	Points[5];
    
    if (CurView->ID == CommandViewport)
		EnableMenuItem(GetMenu(hWndMain), IDM_Z_EDITLIMITS, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
	if (!CurView->UpdateFile)
    	return FALSE;  
    if (!GetLayerBounds (&EditBounds,CurView->hDC, CurView->UpdateFile))
    	return FALSE; 
    if (CurView->ID == CommandViewport)
		EnableMenuItem(GetMenu(hWndMain), IDM_Z_EDITLIMITS, MF_BYCOMMAND | MF_ENABLED);
	if (!RectInWBounds (EditBounds))  
		Nump=0; 
	else
	{   
		DPoint.x = max (EditBounds.xmn,CurView->WBounds.xmn);
		DPoint.y = max (EditBounds.ymn,CurView->WBounds.ymn);
		Points[0] = BasePtToWinPt(DPoint);
		DPoint.y = min (EditBounds.ymx,CurView->WBounds.ymx);
		Points[1] = BasePtToWinPt(DPoint);
		DPoint.x = min (EditBounds.xmx,CurView->WBounds.xmx);
		Points[2] = BasePtToWinPt(DPoint);
		DPoint.y = max (EditBounds.ymn,CurView->WBounds.ymn);
		Points[3] = BasePtToWinPt(DPoint); 
		Points[4] = Points[0]; 
		Nump=5;
    }
    lpPoint = &Points[4];
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	if (!FileMode)
	{
	    CurView->hRgn = CreateVPRgn();
	    SelectClipRgn (CurView->hDC,CurView->hRgn);
	    DeleteObject(CurView->hRgn);  
	}
    {   
    	HBRUSH	Brush, OldBrush;
    	HANDLE	hMaskPoly;
    	LPPOINT pMask;
	    HBITMAP		hBM;
	    short	OldMode, OldPen; 
		LOGBRUSH    NDB;

	    OldPen = SelectObject (CurView->hDC,GetStockObject(NULL_PEN)); 
        NDB.lbStyle = BS_HATCHED;
        NDB.lbColor = RGB(64,64,64);
        NDB.lbHatch = HS_DIAGCROSS;
        Brush =  CreateBrushIndirect(&NDB);
	    OldBrush = SelectObject (CurView->hDC,Brush);
	    
	    hMaskPoly = GlobalAlloc (GMEM_MOVEABLE,(Nump+5)*sizeof(POINT));
	    pMask = GlobalLock (hMaskPoly);
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.bottom+1; 
	    for (i=0;i<Nump;i++,lpPoint--,pMask++)
	    	*pMask = *lpPoint;
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.bottom+1; 
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.top-1;
	    pMask->x = CurView->DrawRect.right+1;
	    pMask++->y = CurView->DrawRect.top-1;
	    pMask->x = CurView->DrawRect.right+1;
	    pMask->y = CurView->DrawRect.bottom+1;
	    GlobalUnlock (hMaskPoly);
	    pMask = GlobalLock (hMaskPoly);  
	    OldMode = SetBkMode (CurView->hDC,TRANSPARENT);
	    i=Polygon (CurView->hDC,pMask,Nump+5);         
	    GlobalUnlock (hMaskPoly);
	    GlobalFree (hMaskPoly);
	    SetBkMode (CurView->hDC,OldMode);
	    SelectObject (CurView->hDC,OldBrush);
	    SelectObject (CurView->hDC,OldPen);
	    DeleteObject (Brush);
    }  
	return TRUE; 
}

BOOL WritePickedItem (int FidOut, int Item)
{	LPINT		ipnt, EndItem;
    HANDLE 		hpltBuf;
	LPSTR		LPpltBuf;
	ITEM		*ItemHeader;
	OFSTRUCT	OFStruct;
	UINT		Length;

	if (Item > NumPicked) return FALSE;
    GetPickName (Item);

	if (!PickName[0]) return FALSE;
	_fstrcpy (PltName,PickName);

	CloseMap ();
	if (!OpenMap (CurView->hWnd,CurView->hDC)) return FALSE;
    _llseek (Fid,PickList[Item].Segment,0);
    nRead = _lread (Fid,&nBytes,2);
    hpltBuf = GSSiGlobAlloc (GMEM_MOVEABLE,(DWORD)nBytes);
    LPpltBuf = GlobalLock (hpltBuf);
    nRead = _lread (Fid,LPpltBuf,nBytes);
    if (nRead != nBytes || PickList[Item].Offset > nRead) 
    {
    	InvalidItem (NULL);
    	return FALSE;
    }
    ipnt = LPpltBuf + PickList[Item].Offset;
    ItemHeader = ipnt;
    if (InvalidItem (ItemHeader))
    	return FALSE;
    EndItem = ipnt + abs(ItemHeader->Len);
    EndItem+=6;  
    Length = (LPSTR) EndItem - (LPSTR) ipnt;
    _lwrite (FidOut,&PickList[Item].Refno,4);
    _lwrite (FidOut,&Length,2);
    _lwrite (FidOut,ipnt,Length);  
    GlobalUnlock(hpltBuf);
    GlobalFree(hpltBuf);
    return TRUE;
}


BOOL ConvertPoly (LPINT ipnt,HANDLE hTranFrom,HANDLE hTranTo,LPSTR BeginSeg)
{	int		idesc, ItemLen, TSize;
	long	*pRefno;
	long	remlen;
	BYTE	*Pcode;
	int		x1,y1,x2,y2, ipen, i,ii;
    DPOINT	FilePointD, WorldPoint;
	static	long	StartElement=-1;
	static	short	nPoly=0, iPoly=0; 
         
         
		if (StartElement > 0)
			ipnt += StartElement;
		StartElement = -1;
        while (*ipnt != 0)
        {   Pcode = ipnt;
        	ipnt++;
        	CurElementPnt = ipnt; 
        	

		    switch (*Pcode)
	        {   
	            case 4: /* put line */
		 		{
		 			nPnts = 2;
					goto DoPolyline;
				}
	            break;

	            case 5: /* put area */

		 		    ipen = *ipnt;
		 			ipnt++;

	            case 6: /* put polyline */

		 		{   nPnts = *ipnt;
		 		    ipnt++;
		 DoPolyline:lpPoints2 = (LPPOINT) ipnt;
		 
		 if (CurrentRefno == 167943816 || CurrentRefno == 167943792)
		 	ii=1;
		 		    ipnt = ipnt + nPnts * 2;
					for (i=0;i<nPnts;i++,lpPoints2++)
					{
     					FilePointD.x = lpPoints2->x;
     					FilePointD.y = lpPoints2->y;
					    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
						ConvertCoord(&WorldPoint,1,3);
					    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
					    lpPoints2->x = IDNINT (FilePointD.x);
					    lpPoints2->y = IDNINT (FilePointD.y);
					}
				}
				break;
                
                case 92:
				case 12: /* item minmax */
				{	
					mnmxCor   *pMinMax;
					
					pMinMax = ipnt;  
										
					ItemSeg = CurrentSeg;
					ConvertMinMax (pMinMax,hTranFrom,hTranTo);
					nPoly = 0;    
					ipnt += 4;
					ItemLen = abs(*ipnt);
					ipnt++;
                }
                break;

		case 15: /* point symbol */
		{	
			LPINT	ipnt1=ipnt++;
			LPINT	ipnt2=ipnt++;
			
			FilePointD.x = *ipnt1;
			FilePointD.y = *ipnt2;
			PointRot = *ipnt++; 
		    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
			ConvertCoord(&WorldPoint,1,3);
		    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
		    *ipnt1 = IDNINT (FilePointD.x);
		    *ipnt2 = IDNINT (FilePointD.y);
		}
		break;
		
		case 17: /* curve symbol */  
		{
			LPINT	ipnt1=ipnt++;
			LPINT	ipnt2=ipnt++;
			FilePointD.x = *ipnt1;
			FilePointD.y = *ipnt2;
		    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
			ConvertCoord(&WorldPoint,1,3);
		    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
		    *ipnt1 = IDNINT (FilePointD.x);
		    *ipnt2 = IDNINT (FilePointD.y);
		}
		case 16: /* line symbol */
		{	
			LPINT	ipnt1=ipnt++;
			LPINT	ipnt2=ipnt++;
			LPINT	ipnt3=ipnt++;
			LPINT	ipnt4=ipnt++;
			
			FilePointD.x = *ipnt1;
			FilePointD.y = *ipnt2;
		    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
			ConvertCoord(&WorldPoint,1,3);
		    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
		    *ipnt1 = IDNINT (FilePointD.x);
		    *ipnt2 = IDNINT (FilePointD.y);
			FilePointD.x = *ipnt3;
			FilePointD.y = *ipnt4;
		    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
			ConvertCoord(&WorldPoint,1,3);
		    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
		    *ipnt3 = IDNINT (FilePointD.x);
		    *ipnt4 = IDNINT (FilePointD.y);
		}
		break;
		
		
		case 18: /* text character*/
		
		case 20: /* point symbol with size and real rot*/  
		ii=1;
		break; 
		
    	case 27: // Multipolygon indicator
    	{
    		short	n;   
       		long	PolyBufferLen;
    		
			GSSiGlobFree (&hPolyBuffer);   
    		nPoly = n = *ipnt++; 
    		iPoly = 0;
    		PolyBufferLen = nPoly - 1; // allows for linklines  
    		while (n--)
    			PolyBufferLen += *ipnt++;
    		hPolyBuffer = GSSiGlobAlloc (GMEM_MOVEABLE,(long)PolyBufferLen*2);
        }

    	case 33: /* jump to long rec and back */
    	{
			ContinuationOffset = *(LPLONG)ipnt; 
			ipnt += 2;    
			JumpBackSeg = CurrentSeg;
			JumpBackElement = ipnt - BeginSeg;
			JumpBackElement++;
		}
		break; 
				
    	case 34: /* jump to long rec and back */
    	{
			ContinuationOffset = JumpBackSeg; 
			StartElement = JumpBackElement;
			*ipnt = 0;	
		}
		break; 
				
                default: 
                	SkipSubRec (Pcode,&ipnt);
                break;

			}
        }
	return(FALSE);
}  

void ConvertMinMax (LPMINMAX pMinMax,HANDLE hTranFrom, HANDLE hTranTo)
{   
	DPOINT	FilePointD, WorldPoint, BoundsP[4];
	double	Minx=DBL_MAX,Miny=DBL_MAX,Maxx=-DBL_MAX,Maxy=-DBL_MAX; 
	short	i;
	
	BoundsP[0].x = pMinMax->xmn;
	BoundsP[0].y = pMinMax->ymn;
	BoundsP[1].x = pMinMax->xmn;
	BoundsP[1].y = pMinMax->ymx;
	BoundsP[2].x = pMinMax->xmx;
	BoundsP[2].y = pMinMax->ymx;
	BoundsP[3].x = pMinMax->xmx;
	BoundsP[3].y = pMinMax->ymn; 
	for (i=0;i<4;i++)
	{
	    TRANS2 (BoundsP[i].x,BoundsP[i].y,&WorldPoint.x,&WorldPoint.y,hTranFrom); 
		ConvertCoord(&WorldPoint,1,3); 
		Minx = min (Minx,WorldPoint.x);
		Miny = min (Miny,WorldPoint.y);
		Maxx = max (Maxx,WorldPoint.x);
		Maxy = max (Maxy,WorldPoint.y); 
	}
    TRANS2 (Minx,Miny,&FilePointD.x,&FilePointD.y,hTranTo); 
    pMinMax->xmn = max (SHRT_MIN,IDNINT(FilePointD.x));
    pMinMax->ymn = max (SHRT_MIN,IDNINT(FilePointD.y));
										
    TRANS2 (Maxx,Maxy,&FilePointD.x,&FilePointD.y,hTranTo);
    pMinMax->xmx = min (SHRT_MAX,IDNINT(FilePointD.x));
    pMinMax->ymx = min (SHRT_MAX,IDNINT(FilePointD.y));
    return;
}

 

BOOL BoundsTotallyInBounds(mnmxCor MinMax,mnmxCor Bounds)
{
	if (MinMax.xmn>=Bounds.xmn &&
		MinMax.ymn>=Bounds.ymn &&
		MinMax.xmx<=Bounds.xmx &&
		MinMax.ymx<=Bounds.ymx)
		return(TRUE);
	else
		return(FALSE);

}

void RandomizePoint (LPDPOINT DPoint,UINT Seed,double MaxDist)
{   int ir;
	double pct;
	
	ir = rand();
	pct = (2*(double)ir/RAND_MAX)-1.0; 
	DPoint->x += pct * MaxDist;
	ir = rand();
	pct = (2*(double)ir/RAND_MAX)-1.0; 
	DPoint->y += pct * MaxDist;
/*	DPoint->x += (2*((double)rand()-RAND_MAX/2)/RAND_MAX) * MaxDist;
	DPoint->y += (2*((double)rand()-RAND_MAX/2)/RAND_MAX) * MaxDist;*/
	return; 
}      

double WinDistToWorldDist (double WinDist)
{ 
	return (WinDist * CurView->BaseUnitsPerPixel);
}



