#include "graphint.h"
#include <mmsystem.h>
 
#include "gmextern.h"

static	long	SavedCmdStringLoc[2];     
static	char	SavePickName[2][128];
static	char	LastMenuFile[MAX_PATH]="";
static	mnmxCor	DummyMnMx = {SHRT_MIN+1,SHRT_MIN+1,SHRT_MIN+1,SHRT_MIN+1};
 


BOOL LineInBounds (long NPNTS, LPPOINT POINTS,double TotDist,LPDOUBLE PCT,
                   LPDOUBLE OffDist,LPDOUBLE AZ,LPMNMXCORL BOUNDS, LPDPOINT PickedPoint,LPPOINT PickAtPoint,LPLONG pFromPtID)
#if ENABLETRACE
{GSSiEnterProg (1016);
#endif
{    LPPOINT    BPOINT, EPOINT;
     double     Dist, MinOffDist, MinPCT, MinAZ;
     DPOINT     MinPickedPoint;
     LONG       YMN, YMX, XMN, XMX, XDIFF;
     double     SLOPE, YINT, X, Y;
     short      nfound;
     long       i, MinFromPt;
     
     if (IgnoreBounds || PickDeletes)
{
#if ENABLETRACE
GSSiExitProg (1016);
#endif
        return TRUE;
}
     nfound = 0;
     BPOINT = EPOINT = POINTS; 
     if (NPNTS > 1)
        EPOINT++; 
     else
        NPNTS = 2;
     Dist = 0;
     MinOffDist=999999;
     for (i=0;i<NPNTS-1;i++,BPOINT++,EPOINT++)
     {
         XMN = min (BPOINT->x, EPOINT->x);
         XMX = max (BPOINT->x, EPOINT->x);
         YMN = min (BPOINT->y, EPOINT->y);
         YMX = max (BPOINT->y, EPOINT->y);
         if (XMN >= BOUNDS->xmn && XMX <=BOUNDS->xmx &&
             YMN >= BOUNDS->ymn && YMX <=BOUNDS->ymx) goto Found;
         XDIFF = (long)EPOINT->x - (long)BPOINT->x;
         if (XDIFF == 0)
         {    if (BPOINT->x >= BOUNDS->xmn &&
                 BPOINT->x <= BOUNDS->xmx)
                if (!(YMN > BOUNDS->ymx ||
                      YMX < BOUNDS->ymn)) goto Found;
         }
         else
         {
             SLOPE = ((double)EPOINT->y - (double)BPOINT->y)/(double)XDIFF;
             YINT  = (double)BPOINT->y - SLOPE * (double)BPOINT->x;
             if (BOUNDS->xmn >= XMN && BOUNDS->xmn <= XMX)
             {
                 Y = SLOPE * (double)BOUNDS->xmn + YINT;
                 if (Y >= BOUNDS->ymn && Y <=BOUNDS->ymx) goto Found;
             }
             if (BOUNDS->xmx >= XMN && BOUNDS->xmx <= XMX)
             {
                 Y = SLOPE * (double)BOUNDS->xmx + YINT;
                 if (Y >= BOUNDS->ymn && Y <=BOUNDS->ymx) goto Found;
             }
             if (SLOPE != 0)
             {
                 if (BOUNDS->ymn >= YMN && BOUNDS->ymn <= YMX)
                 {
                     X = ((double)BOUNDS->ymn - YINT) / SLOPE;
                     if (X >= BOUNDS->xmn && X <=BOUNDS->xmx) goto Found;
                 }
                 if (BOUNDS->ymx >= YMN && BOUNDS->ymx <= YMX)
                 {
                     X = ((double)BOUNDS->ymx - YINT) / SLOPE;
                     if (X >= BOUNDS->xmn && X <=BOUNDS->xmx) goto Found;
                 }
             }
         }
         goto Next;
Found:   if (TotDist < 0)
{
#if ENABLETRACE
GSSiExitProg (1016);
#endif
			return (TRUE);
}
         nfound++; 
       	 ComputePickPoint (BPOINT, EPOINT, Dist, TotDist,PCT,OffDist,AZ, BOUNDS,PickedPoint,PickAtPoint);
         if (fabs(*OffDist)<MinOffDist)
         {
            MinOffDist = *OffDist;
            MinPCT = *PCT;  
            MinAZ = *AZ;
            MinPickedPoint = *PickedPoint;
            MinFromPt = i;
         } 
Next:;
         Dist += idist (*BPOINT,*EPOINT);
     }
     if (!nfound)
{
#if ENABLETRACE
GSSiExitProg (1016);
#endif
        return (FALSE);
}
     else
     {
        *PCT = MinPCT;
        *OffDist = MinOffDist;
        *AZ = MinAZ;
        *PickedPoint = FilePtToBasePtD (MinPickedPoint);
        *pFromPtID = MinFromPt;
{
#if ENABLETRACE
GSSiExitProg (1016);
#endif
        return (TRUE);
}
     }
#if ENABLETRACE
}
#endif
}  

BOOL LineInBoundsD (long NPNTS, HPDPOINT POINTS,double TotDist,LPDOUBLE PCT,
                   LPDOUBLE OffDist,LPDOUBLE AZ,LPMNMXCORD BOUNDS, LPDPOINT PickedPoint,LPLONG	pFromPtID)
#if ENABLETRACE
{GSSiEnterProg (1017);
#endif
{    HPDPOINT    BPOINT, EPOINT;
     double     Dist, MinOffDist, MinPCT, MinAZ;
     DPOINT     MinPickedPoint;
     double     YMN, YMX, XMN, XMX, XDIFF;
     double     SLOPE, YINT, X, Y;
     long       i, MinFromPt;
     short		nfound;
     
     if (IgnoreBounds || PickDeletes)
{
#if ENABLETRACE
GSSiExitProg (1017);
#endif
        return TRUE;
}
     nfound = 0;
     BPOINT = EPOINT = POINTS; 
     if (NPNTS > 1)
        EPOINT++; 
     else
        NPNTS = 2;
     Dist = 0;
     MinOffDist=999999;
     for (i=0;i<NPNTS-1;i++,BPOINT++,EPOINT++)
     {
         XMN = min (BPOINT->x, EPOINT->x);
         XMX = max (BPOINT->x, EPOINT->x);
         YMN = min (BPOINT->y, EPOINT->y);
         YMX = max (BPOINT->y, EPOINT->y);
         if (XMN >= BOUNDS->xmn && XMX <=BOUNDS->xmx &&
             YMN >= BOUNDS->ymn && YMX <=BOUNDS->ymx)
              goto Found;
         XDIFF = EPOINT->x - BPOINT->x;
         if (XDIFF == 0)
         {    if (BPOINT->x >= BOUNDS->xmn &&
                 BPOINT->x <= BOUNDS->xmx)
                if (!(YMN > BOUNDS->ymx ||
                      YMX < BOUNDS->ymn))
                       goto Found;
         }
         else
         {
             SLOPE = ((double)EPOINT->y - (double)BPOINT->y)/(double)XDIFF;
             YINT  = (double)BPOINT->y - SLOPE * (double)BPOINT->x;
             if (BOUNDS->xmn >= XMN && BOUNDS->xmn <= XMX)
             {
                 Y = SLOPE * (double)BOUNDS->xmn + YINT;
                 if (Y >= BOUNDS->ymn && Y <=BOUNDS->ymx)
                  goto Found;
             }
             if (BOUNDS->xmx >= XMN && BOUNDS->xmx <= XMX)
             {
                 Y = SLOPE * (double)BOUNDS->xmx + YINT;
                 if (Y >= BOUNDS->ymn && Y <=BOUNDS->ymx)
                  goto Found;
             }
             if (SLOPE != 0)
             {
                 if (BOUNDS->ymn >= YMN && BOUNDS->ymn <= YMX)
                 {
                     X = ((double)BOUNDS->ymn - YINT) / SLOPE;
                     if (X >= BOUNDS->xmn && X <=BOUNDS->xmx)
                      goto Found;
                 }
                 if (BOUNDS->ymx >= YMN && BOUNDS->ymx <= YMX)
                 {
                     X = ((double)BOUNDS->ymx - YINT) / SLOPE;
                     if (X >= BOUNDS->xmn && X <=BOUNDS->xmx)
                      goto Found;
                 }
             }
         }
         goto Next;
Found:   if (TotDist < 0)
{
#if ENABLETRACE
GSSiExitProg (1017);
#endif
			return (TRUE);
}
         nfound++; 
       	 ComputePickPointD (BPOINT, EPOINT, Dist, TotDist,PCT,OffDist,AZ, BOUNDS,PickedPoint);
         if (fabs(*OffDist)<MinOffDist)
         {
            MinOffDist = fabs(*OffDist);
            MinPCT = *PCT;  
            MinAZ = *AZ;
            MinPickedPoint = *PickedPoint;   
            MinFromPt = i;
         } 
Next:;
         Dist += ldistp (*BPOINT,*EPOINT);
     }
     if (!nfound)
{
#if ENABLETRACE
GSSiExitProg (1017);
#endif
        return (FALSE);
}
     else
     {
        *PCT = MinPCT;
        *OffDist = MinOffDist;
        *AZ = MinAZ;
        *PickedPoint = MinPickedPoint; 
        *pFromPtID = MinFromPt;
{
#if ENABLETRACE
GSSiExitProg (1017);
#endif
        return (TRUE);
}
     }
#if ENABLETRACE
}
#endif
}

void CloseTAGIndex (void)
#if ENABLETRACE
{GSSiEnterProg (1018);
#endif
{   
	if (!hTAGIdx)
{
#if ENABLETRACE
GSSiExitProg (1018);
#endif
		return;
}
	BT_CLOSE (hTAGIdx);
	hTAGIdx = 0; 
{
#if ENABLETRACE
GSSiExitProg (1018);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL OpenTAGIndex (BOOL Delete,BOOL StoreBounds)
#if ENABLETRACE
{GSSiEnterProg (1019);
#endif
{	BTVARDESC	BTVar[3];
	time_t ltime;
	char		TAGIndexFile[MAX_PATH];    
	short	size=8;

	if (hTAGIdx && !Delete)
	{
		if (!ForceTAGIndex || BT_OPEN_FOR_WRITE (hTAGIdx))
{
#if ENABLETRACE
GSSiExitProg (1019);
#endif
			return FALSE;
}
	}
	if (PltType == 5)
{
#if ENABLETRACE
GSSiExitProg (1019);
#endif
		return FALSE;
}
	if (hTAGIdx && PltType == 4 && ForceTAGIndex && InRebuildRefIndex)  
{
#if ENABLETRACE
GSSiExitProg (1076);
#endif
		return FALSE;
}   
	BT_CLOSE (hTAGIdx);
	hTAGIdx = 0;
	if (PltType < 4 || MapFileType (PltName) == MT_PLT)
	{
		_fstrcpy (TAGIndexFile,PltName);  
		ExpandText (TAGIndexFile);
		if (strlen (TAGIndexFile) < 5 || !strrchr (TAGIndexFile,'.'))
			return FALSE;
		TAGIndexFile[_fstrlen(TAGIndexFile)-3]=0;
		_fstrcat (TAGIndexFile,"tin");                  
    }
    else
    {   
    	char	Name[MAX_PATH];
    	
    	_fstrcpy (Name,PltName);
    	ExpandText (Name);
		if (!*Name)
			return FALSE;
    	if (_fullpath (TAGIndexFile,Name,sizeof(TAGIndexFile)))
		{
    		*_fstrrchr (TAGIndexFile,'\\') = 0;
    		_fstrcat (TAGIndexFile,"\\tagindex.rin");
		}
		else
			return FALSE;
    }
    if (Delete && ForceTAGIndex)
    {   
    	OFSTRUCTGM OFStruct;
    	
    	GSSiRemove (TAGIndexFile);
//    	retrn TRUE;
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
		BTVar[2].BT_VARTYP=BT_INTEGER;
		BTVar[2].BT_VARLEN=4;
		BTVar[2].BT_VAROFF=40;
		if (StoreBounds)
			size = sizeof(REFINDEXDATA);
		else
			size = 8;
		BT_CREATE (TAGIndexFile, size, FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
		hTAGIdx = BT_OPEN (TAGIndexFile, 0, BT_WRITE, 0);
	}
{
#if ENABLETRACE
GSSiExitProg (1019);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL ChangePickedItemRefno (short Item, long NewRefno)
#if ENABLETRACE
{GSSiEnterProg (1026);
#endif
{
	REFINDEXDATA	RefIdxData;
	short	ID, ls, TAGHead, ID2;
	BOOL	rtn=FALSE;  
	HFILE	Fid; 
	OFSTRUCTGM	OFStruct;
	LPLONG	pRefno;  
	long	Loc, OldRef;
	BOOL	SaveDisableHalt = DisableHalt;
       		
    DisableHalt = TRUE;
	
	ProcessPickedItem (Item,FALSE);        		
    ForceRefIndex = ForceTAGIndex = TRUE;  
	if (OpenMap (CurView->hWnd,0))  
	{
		if (FidMap != HFILE_ERROR)
		{
			GSSillseek (FidMap,CurTAGLoc,0);
			BigRead (FidMap,(HPSTR)&ID2,2); 
			ID = LOBYTE (ID2);
			if (ID == 9)
			{
				Loc = GSSillseek (FidMap,0,1);
				BigRead (FidMap,(HPSTR)&OldRef,4);
				GSSillseek (FidMap,Loc,0);  
				if (NewRefno == LONG_MAX)
					NewRefno = GetNewRefno(PickName,0,0,0,0);
				BigWrite (FidMap,(HPSTR)&NewRefno,4,-1); 
				rtn = TRUE;
			   	CurrentRefno=NewRefno;
			   	FileInIndex=0;
				ItemSeg = PickList[Item].Segment;
				CurrentItem = PickList[Item].Offset;
			   	BuildRefIndex(FALSE,ItemIsDeleted);
				BT_DELETE (hRefIdx,(LPSTR)&OldRef,(LPSTR)&RefIdxData,FALSE);
			   	if (PickList[Item].Prefix)	
					BuildTAGIndex (PickList[Item].Prefix,PickList[Item].UDI,0,CurrentRefno,FALSE);
			}
	    	CloseMap(FALSE);
		} 
	}
    ForceRefIndex = ForceTAGIndex = FALSE;  
   	DisableHalt = SaveDisableHalt;  
{
#if ENABLETRACE
GSSiExitProg (1026);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

	
long GetMaxRefno (int PickFile,LPLONG pMinRef)
#if ENABLETRACE
{GSSiEnterProg (1028);
#endif
{   int	st, iview;
	BOOL RIOpened;
	long	MaxRef = TMPRF$, Refno, MinRef=LONG_MAX;
	LPVIEWPORT	SaveView;
	LPVISLIST	SaveVis; 
	HANDLE		hVisList;
	BOOL	SaveUseRORTI = UseRefOrTAGIndex;
	REFINDEXDATA	RefIdxData;
    
    SaveView = CurView;
    SaveVis = CurVis;

	hVisList=GSSiGlobAlloc ( 965,GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis (); 
	
    CloseMap(FALSE);
    CloseRefIndex (FALSE); 
	UseRefOrTAGIndex=TRUE;
	for (iview=0;iview<*pNumViewports;iview++)
	{   
    	SetCurView ( pViewports[iview]);
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
			if (!BT_FIND (hRefIdx,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&RefIdxData))
				MinRef = min (MinRef,Refno);
			if (!BT_FIND (hRefIdx,(LPSTR)&Refno,BT_LAST,BT_ANY,(LPSTR)&RefIdxData))
				MaxRef = max (MaxRef,Refno);
	    	CloseRefIndex (TRUE);
	NextFile:;
		}
    }
	SetCurView ( SaveView);
	CurVis = SaveVis;
	GlobalUnlock (hVisList);	
	GSSiGlobUlFree (&hVisList);
	UseRefOrTAGIndex=SaveUseRORTI; 
	*pMinRef = MinRef;
{
#if ENABLETRACE
GSSiExitProg (1028);
#endif
	return (MaxRef);
}
#if ENABLETRACE
}
#endif
} 
double DistToRect2 (LPDPOINT pPoint, LPMINMAX MinMax)
#if ENABLETRACE
{GSSiEnterProg (1029);
#endif
{               
	double	Dist; 
	DPOINT	p;

	if (pPoint->x >= MinMax->xmn &&
		pPoint->x <= MinMax->xmx &&
		pPoint->y >= MinMax->ymn &&
		pPoint->y <= MinMax->ymx)
	{
		Dist = 0;
{
#if ENABLETRACE
GSSiExitProg (1029);
#endif
		return Dist;
}
	}
	if (pPoint->x >= MinMax->xmn &&
		pPoint->x <= MinMax->xmx &&
		pPoint->y > MinMax->ymx) 
	{   
		p.x = pPoint->x;
		p.y = MinMax->ymx;
	} 
	else if (pPoint->x >= MinMax->xmn &&
		pPoint->x <= MinMax->xmx &&
		pPoint->y < MinMax->ymn) 
	{   
		p.x = pPoint->x;
		p.y = MinMax->ymn;
	} 
	else if (pPoint->y >= MinMax->ymn &&
		pPoint->y <= MinMax->ymx &&
		pPoint->x < MinMax->xmn) 
	{   
		p.y = pPoint->y;
		p.x = MinMax->xmn;
	} 
	else if (pPoint->y >= MinMax->ymn &&
		pPoint->y <= MinMax->ymx &&
		pPoint->x > MinMax->xmx) 
	{   
		p.y = pPoint->y;
		p.x = MinMax->xmx;
	} 
	else if (pPoint->x < MinMax->xmn &&
		pPoint->y < MinMax->ymn) 
	{   
		p.x = MinMax->xmn;
		p.y = MinMax->ymn;
	} 
	else if (pPoint->x < MinMax->xmn &&
		pPoint->y > MinMax->ymx) 
	{   
		p.x = MinMax->xmn;
		p.y = MinMax->ymx;
	} 
	else if (pPoint->x > MinMax->xmx &&
		pPoint->y > MinMax->ymx) 
	{   
		p.x = MinMax->xmx;
		p.y = MinMax->ymx;
	} 
	else if (pPoint->x > MinMax->xmx &&
		pPoint->y < MinMax->ymn) 
	{   
		p.x = MinMax->xmx;
		p.y = MinMax->ymn;
	} 
	Dist = ldistp (p,*pPoint);
{
#if ENABLETRACE
GSSiExitProg (1029);
#endif
	return Dist;
}
#if ENABLETRACE
}
#endif
}  


double DistToRect32 (LPDPOINT pPoint, LPMNMXCORL MinMax)
#if ENABLETRACE
{GSSiEnterProg (1029);
#endif
{               
	double	Dist; 
	DPOINT	p;

	if (pPoint->x >= MinMax->xmn &&
		pPoint->x <= MinMax->xmx &&
		pPoint->y >= MinMax->ymn &&
		pPoint->y <= MinMax->ymx)
	{
		Dist = 0;
{
#if ENABLETRACE
GSSiExitProg (1029);
#endif
		return Dist;
}
	}
	if (pPoint->x >= MinMax->xmn &&
		pPoint->x <= MinMax->xmx &&
		pPoint->y > MinMax->ymx) 
	{   
		p.x = pPoint->x;
		p.y = MinMax->ymx;
	} 
	else if (pPoint->x >= MinMax->xmn &&
		pPoint->x <= MinMax->xmx &&
		pPoint->y < MinMax->ymn) 
	{   
		p.x = pPoint->x;
		p.y = MinMax->ymn;
	} 
	else if (pPoint->y >= MinMax->ymn &&
		pPoint->y <= MinMax->ymx &&
		pPoint->x < MinMax->xmn) 
	{   
		p.y = pPoint->y;
		p.x = MinMax->xmn;
	} 
	else if (pPoint->y >= MinMax->ymn &&
		pPoint->y <= MinMax->ymx &&
		pPoint->x > MinMax->xmx) 
	{   
		p.y = pPoint->y;
		p.x = MinMax->xmx;
	} 
	else if (pPoint->x < MinMax->xmn &&
		pPoint->y < MinMax->ymn) 
	{   
		p.x = MinMax->xmn;
		p.y = MinMax->ymn;
	} 
	else if (pPoint->x < MinMax->xmn &&
		pPoint->y > MinMax->ymx) 
	{   
		p.x = MinMax->xmn;
		p.y = MinMax->ymx;
	} 
	else if (pPoint->x > MinMax->xmx &&
		pPoint->y > MinMax->ymx) 
	{   
		p.x = MinMax->xmx;
		p.y = MinMax->ymx;
	} 
	else if (pPoint->x > MinMax->xmx &&
		pPoint->y < MinMax->ymn) 
	{   
		p.x = MinMax->xmx;
		p.y = MinMax->ymn;
	} 
	Dist = ldistp (p,*pPoint);
{
#if ENABLETRACE
GSSiExitProg (1029);
#endif
	return Dist;
}
#if ENABLETRACE
}
#endif
}  

double MinRectDist (LPMINMAX Rect1,LPMINMAX Rect2)
#if ENABLETRACE
{GSSiEnterProg (1030);
#endif
{ 
	double MinDist=DBL_MAX, d1;
	short	i;
	
	DPOINT	Points1[4], Points2[4];
	
	Points1[0].x = Rect1->xmn;
	Points1[1].x = Rect1->xmn;
	Points1[2].x = Rect1->xmx;
	Points1[3].x = Rect1->xmx;
	Points1[0].y = Rect1->ymn;
	Points1[1].y = Rect1->ymx;
	Points1[2].y = Rect1->ymx;
	Points1[3].y = Rect1->ymn;
	Points2[0].x = Rect2->xmn;
	Points2[1].x = Rect2->xmn;
	Points2[2].x = Rect2->xmx;
	Points2[3].x = Rect2->xmx;
	Points2[0].y = Rect2->ymn;
	Points2[1].y = Rect2->ymx;
	Points2[2].y = Rect2->ymx;
	Points2[3].y = Rect2->ymn; 
	for (i=0;i<4;i++)
	{
		d1 = DistToRect2 (&Points1[i],Rect2);
		MinDist = min (d1,MinDist);
	}
	for (i=0;i<4;i++)
	{
		d1 = DistToRect2 (&Points2[i],Rect1);
		MinDist = min (d1,MinDist);
	} 
{
#if ENABLETRACE
GSSiExitProg (1030);
#endif
	return MinDist;
}
#if ENABLETRACE
}
#endif
}

double MinRectDist32 (LPMNMXCORL Rect1,LPMNMXCORL Rect2)
#if ENABLETRACE
{GSSiEnterProg (1030);
#endif
{ 
	double MinDist=DBL_MAX, d1;
	short	i;
	
	DPOINT	Points1[4], Points2[4];
	
	Points1[0].x = Rect1->xmn;
	Points1[1].x = Rect1->xmn;
	Points1[2].x = Rect1->xmx;
	Points1[3].x = Rect1->xmx;
	Points1[0].y = Rect1->ymn;
	Points1[1].y = Rect1->ymx;
	Points1[2].y = Rect1->ymx;
	Points1[3].y = Rect1->ymn;
	Points2[0].x = Rect2->xmn;
	Points2[1].x = Rect2->xmn;
	Points2[2].x = Rect2->xmx;
	Points2[3].x = Rect2->xmx;
	Points2[0].y = Rect2->ymn;
	Points2[1].y = Rect2->ymx;
	Points2[2].y = Rect2->ymx;
	Points2[3].y = Rect2->ymn; 
	for (i=0;i<4;i++)
	{
		d1 = DistToRect32 (&Points1[i],Rect2);
		MinDist = min (d1,MinDist);
	}
	for (i=0;i<4;i++)
	{
		d1 = DistToRect32 (&Points2[i],Rect1);
		MinDist = min (d1,MinDist);
	} 
{
#if ENABLETRACE
GSSiExitProg (1030);
#endif
	return MinDist;
}
#if ENABLETRACE
}
#endif
}

short DisplayDisconnected (HWND hWndDlg, short nlast,double MaxDist,HANDLE hIdx, HANDLE hTag,LPSTR udi)
#if ENABLETRACE
{GSSiEnterProg (1031);
#endif
{
	LPTAGKEY	pTag;
	LPREFINDEXDATA	pIdx; 
	short	st=0;
	char	str[256]; 
	short	idx;  
	

	if (nlast > 1)
	{   
		BOOL	Disconnected = FALSE; 
		RECT	Rect1, Rect2, IntRect;
		short	i;
							    				
		pIdx = (LPREFINDEXDATA)GlobalLock (hIdx);
		Rect1 = Rect16ToRect32 (pIdx->MinMax);  
		InflateRect (&Rect1,1,1);
		pIdx++;
		for (i=1;i<nlast;i++)
		{   
			Rect2 = Rect16ToRect32 (pIdx->MinMax);
			InflateRect (&Rect2,1,1);
			if (!IntersectRect(&IntRect,&Rect1,&Rect2))
			{   
				if (MinRectDist32 ((LPMNMXCORL)&Rect1,(LPMNMXCORL)&Rect2) > MaxDist)
					Disconnected = TRUE;
			}
			UnionRect (&Rect1,&Rect1,&Rect2);
			pIdx++;
		} 
		GlobalUnlock (hIdx); 
		if (Disconnected)
		{   
			pTag = (LPTAGKEY)GlobalLock (hTag);
			pIdx = (LPREFINDEXDATA)GlobalLock (hIdx);
			for (i=0;i<nlast;i++,pTag++,pIdx++)
			{   
		    	if (pTag->Refno)
		    		sprintf (str,"%s{%ld}",pTag->UDI,pTag->Refno);
		    	else
		    		_fstrcpy (str,pTag->UDI); 
				if ((idx=SendDlgItemMessage (hWndDlg,IDC_TAG_LIST,LB_ADDSTRING,0,(LPARAM)str)) ==
					LB_ERRSPACE)
					st = 1;
 				else if (!_fstrcmp (udi,str))
 					SendDlgItemMessage (hWndDlg,IDC_TAG_LIST,LB_SETCURSEL,idx,0);
			}
			GlobalUnlock (hIdx);
			GlobalUnlock (hTag);
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1031);
#endif
	return st;
}
#if ENABLETRACE
}
#endif
}
 

void DetermineVPDisplaySequence (void)
#if ENABLETRACE
{GSSiEnterProg (1033);
#endif
{
	short	NumDisplay=0;
	UINT	iv, iv2, itheme; 
	LPVIEWPORT	SaveVP;
	
	DetermineVPDisplaySequence2 (0,&NumDisplay);  
    for (iv=0;iv<*pNumViewports;iv++)
    {
		if (pViewportsD[iv]->Parent && pViewportsD[iv]->DisplayInParent)
		{
			for (itheme=0;itheme < pViewportsD[iv]->NumThemes;itheme++)
			{ 
				if (pViewportsD[iv]->pThemes[itheme]->ID == GF_HOTSPOT_THEME &&
					pViewportsD[iv]->pThemes[itheme]->IsActive &&
					VPIsActive(pViewportsD[iv]->pThemes[itheme]->DisplayViewport))
				{   
				    for (iv2=0;iv2<*pNumViewports;iv2++) 
				    {
				    	if (pViewportsD[iv2]->ID == pViewportsD[iv]->Parent)
				    	{   
				    		if (iv2 < iv)
				    		{
								SaveVP = pViewportsD[iv];
								pViewportsD[iv] = pViewportsD[iv2];
								pViewportsD[iv2]= SaveVP; 
								break;  
							}
						}
					}
				}
			}
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1033);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void DetermineVPDisplaySequence2 (short ParentID, LPSHORT pNumDisplay)
#if ENABLETRACE
{GSSiEnterProg (1034);
#endif
{   
	UINT	iv;
	
    for (iv=0;iv<*pNumViewports;iv++)
    {   
        if (pViewports[iv]->Parent == ParentID &&
        	pViewports[iv]->StartupFunction == GF_TOOLBAR)
        {
            pViewportsD[(*pNumDisplay)++] = pViewports[iv];
	        if (*pNumDisplay >= *pNumViewports)
{
#if ENABLETRACE
GSSiExitProg (1034);
#endif
	        	return; 
}
        	DetermineVPDisplaySequence2 (pViewports[iv]->ID,pNumDisplay);
        }
    }
    for (iv=0;iv<*pNumViewports;iv++)
    {   
        if (pViewports[iv]->Parent == ParentID &&
        	pViewports[iv]->StartupFunction != GF_TOOLBAR)
        {
            pViewportsD[(*pNumDisplay)++] = pViewports[iv];
	        if (*pNumDisplay >= *pNumViewports)
{
#if ENABLETRACE
GSSiExitProg (1034);
#endif
	        	return; 
}
        	DetermineVPDisplaySequence2 (pViewports[iv]->ID,pNumDisplay);
        }
    }
    
{
#if ENABLETRACE
GSSiExitProg (1034);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

BOOL VPIsActive (short VPID)
{   
	BOOL	rtn;
	LPVIEWPORT	SaveVP=CurView;
	
	SetViewport (VPID);
	rtn = CurViewActive ();
	CurView = SaveVP;
	return rtn;
}
BOOL CurViewActive(void)
#if ENABLETRACE
{GSSiEnterProg (1035);
#endif
{   
	short	Parent; 
	USHORT	iview;
	
	if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (1035);
#endif
		return FALSE;
}
	if (!CurView->Active)
{
#if ENABLETRACE
GSSiExitProg (1035);
#endif
		return FALSE;
}
	Parent = CurView->Parent;
	if (Parent > *pNumViewports)
		return FALSE;
	while (Parent)
	{
		if (!pViewports[Parent-1]->Active)
{
#if ENABLETRACE
GSSiExitProg (1035);
#endif
			return FALSE;
}
		Parent = pViewports[Parent-1]->Parent;
	}
    for (iview = 0; iview<*pNumViewports; iview++)  
    {
    	if (pViewports[iview]->DisplayedFullScreen && pViewports[iview]->ID != CurView->ID)
{
#if ENABLETRACE
GSSiExitProg (1035);
#endif
			return FALSE;
}
	}
{
#if ENABLETRACE
GSSiExitProg (1035);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
BOOL SymInContents (short SymNum,short SymOrPar,LPSTR Contents)
#if ENABLETRACE
{GSSiEnterProg (1039);
#endif
{
	LPSHORT	Child;
	HANDLE	hChildren=0;
	short	nChildren=0;  
	BOOL	rtn=TRUE;
	
	if (SymNum == SymOrPar)
{
#if ENABLETRACE
GSSiExitProg (1039);
#endif
		return TRUE;
}
							
	GetSymDictChildren (SymOrPar,&nChildren,&hChildren,-1,TRUE);  
	Child = (LPSHORT)GlobalLock (hChildren);
	while (nChildren--)  
	{
		if (*Child++ == SymNum)
			goto Exit;
	}
	rtn = FALSE;
Exit:
	GSSiGlobUlFree (&hChildren); 
{
#if ENABLETRACE
GSSiExitProg (1039);
#endif
	return rtn;		                
}
#if ENABLETRACE
}
#endif
}

BOOL RecomputeMinMax (LPMINMAX pMinMax,HPSTR buf, long len, short Mode,LPSHORT pDummyRec)
//Mode = 0 use defined size all items, -1 actual size all items, + = actual size of specified type
#if ENABLETRACE
{GSSiEnterProg (1040);
#endif
{ 
	HPSHORT	ipnt;
	long	Offset;
	HPBYTE	Pcode; 
	HPSTR	EndLoc;
	LPSHORT	pItemLen; 
	short		idesc, ItemLen, TSize, PointSize;
	long	*pRefno;
	long	remlen;
	short		x1,y1,x2,y2, ipen, ii;
	DWORD	i;
	double	CurPointSize; 
	LPDOUBLE	pSize;
    DPOINT	FilePointD, WorldPoint; 
    POINT	FilePoint;   
    LPPOINT	lpPoints;
    mnmxCor	NewMinMax;    
	MNMXCORD	Bounds; 
    BOOL	LongRec=FALSE;  
    HPBYTE	pCompression;
    short	Compression;
    COLORREF	RouteColor;

    if (pDummyRec) //0=have coords, 1=have dummy delete, 2=no coords or dummy delete
    	*pDummyRec = 2 ;
    if (len > USHRT_MAX)
    	LongRec=TRUE;
    MinMaxInit (&NewMinMax);
    EndLoc = buf+len;
    ipnt = (HPSHORT)EndLoc;
    *ipnt = 0;
    ipnt =(HPSHORT) buf;  
    Pcode = (HPBYTE)ipnt;  
    TLSet = FALSE;
	HaveTXLoc=TRUE;
    if (*Pcode != 12)
{
#if ENABLETRACE
GSSiExitProg (1040);
#endif
    	return FALSE;
}
    while (*ipnt)
    {   
    	Pcode = (HPBYTE)ipnt;
    	ipnt++;
        
        Compression = 0;
    	PointSize = 4;

	    switch (*Pcode)
        {   
		    case 8:	/*	description */
			{   
			 	if (!*ipnt)
			 		*ipnt = 1; //dont allow desc=0
				CurrentDesc = *ipnt;
				ipnt++;
		    }
		    break;
	
            case 4: /* put line */
	 		{
	 			nPnts = 2;
	    		CurrentType = GF_LINE;
				goto DoPolyline;
			}
            break;

            case 41: /* put line */
	 		{
	 			nPnts = 2;
               	PointSize = 16;
	    		CurrentType = GF_LINE;
				goto DoPolylineD;
			}
            break;

            case 5: /* put area */

	 		    CurrentType = GF_AREA; 
	 		    ipen = *ipnt;
	 			ipnt++;
                goto SkipPoly;
 	 		    CurrentType = GF_POLYLINE; 
            case 6: /* put polyline */
       		case 35: /* point array */
SkipPoly:
	 		{   nPnts = *(LPUSHORT)ipnt;
	 		    ipnt++;
DoPolyline:
	 		    {
		 		    HANDLE	hPnts = GSSiGlobAlloc ( 969,GMEM_MOVEABLE,(long)nPnts*PointSize);
	 		        HPPOINTS	lpCPBeg = lpCurPoints =(HPPOINTS)GlobalLock (hPnts);
	 		        
		 			PolyIsHiPrecis = FALSE;
	 		        if (pDummyRec)
	 		        	*pDummyRec = 0;
		 			hmemmove ((HPSTR)lpCurPoints,(HPSTR)ipnt,(long)nPnts*PointSize);
					for (i=0;i<nPnts;i++,lpCurPoints++)
					{   
						POINT point = POINTStoPOINT(*lpCurPoints);
						TranPointReorgFile (&point,hTranReorg);
	 					AddPointToMinMax (point,&NewMinMax);
					}
					if (hTranReorg || hTranCopyFile)  
		 				hmemmove ((HPSTR)ipnt,(HPSTR)lpCPBeg,(long)nPnts*PointSize);      
		 			
					GSSiGlobUlFree (&hPnts); 
					lpCurPoints = (HPPOINTS)ipnt;
		 		    ipnt = ipnt + (long)nPnts * 2;
				}
			}
			break;
                
			case 151: //Route Area (not closed)
				RouteOffset = *(LPDOUBLE)ipnt;
				ipnt += 4;
				RouteColor = *(LPCOLORREF)ipnt;
				ipnt += 2;
				break;

            case 51: /* put area */

	 		    CurrentType = GF_AREA;   
	 		    pCompression = Pcode + 1;
	 		    Compression = *pCompression;
	 		    ipen = *ipnt;
	 			ipnt++;
				goto SkipPolyD;
                
            case 61: /* put polyline */
	 		    CurrentType = GF_POLYLINE; 
        	case 135: /* point array (hiprecis) */

	 		{
SkipPolyD:
               	PointSize = 16;
	 			nPnts = *(LPUSHORT)ipnt;
	 		    ipnt++; 
DoPolylineD:
	 		    {
		 		    HANDLE	hPnts = GSSiGlobAlloc ( 970,GMEM_MOVEABLE,(long)nPnts*sizeof(DPOINT));
	 		        HPDPOINT	lpCPBeg = lpDCurPoints = (HPDPOINT)GlobalLock (hPnts);
	 		        
		 			PolyIsHiPrecis = TRUE;
	 		        if (pDummyRec)
	 		        	*pDummyRec = 0;
		 			hmemmove ((HPSTR)lpDCurPoints,(HPSTR)ipnt,(long)nPnts*sizeof(DPOINT));
		 		    if (CurrentType == GF_AREA && nPnts == 2)
					{
						DPOINT	RP  = TranPointReorg (lpDCurPoints,hTranReorg);
						DPOINT	POC = TranPointReorg ((lpDCurPoints+1),hTranReorg);
	 					POINT	RPFile = BasePtToFilePtReorg (RP);
						POINT	POCFile = BasePtToFilePtReorg (POC);
						double	radius=idist (RPFile,POCFile)+1;
						
	 					POCFile = RPFile;
						POCFile.x -= radius;
						AddPointToMinMax (POCFile,&NewMinMax);
	 					POCFile = RPFile;
						POCFile.x += radius;
						AddPointToMinMax (POCFile,&NewMinMax);
	 					POCFile = RPFile;
						POCFile.y -= radius;
						AddPointToMinMax (POCFile,&NewMinMax);
	 					POCFile = RPFile;
						POCFile.y += radius;
						AddPointToMinMax (POCFile,&NewMinMax);
					}

					for (i=0;i<nPnts;i++,lpDCurPoints++)
					{   
						*lpDCurPoints = TranPointReorg (lpDCurPoints,hTranReorg);
	 					FilePoint = BasePtToFilePtReorg (*lpDCurPoints);
	 					AddPointToMinMax (FilePoint,&NewMinMax);
					}
					ExpandMinMax (&NewMinMax,IDNINT(RouteOffset*= FileDistToBaseDist));
					if (hTranReorg)  
		 				hmemmove ((HPSTR)ipnt,(HPSTR)lpCPBeg,(long)nPnts*sizeof(DPOINT));   
                    lpDCurPoints = (HPDPOINT)ipnt;
		 		    ipnt = ipnt + (long)nPnts * 8;
					GSSiGlobUlFree (&hPnts);
				}
			}
			break;
                
			case 15: /* point symbol */
			{	
				short	PointRot;
				
				LPSHORT	ipnt1=ipnt++;
				LPSHORT	ipnt2=ipnt++;
				
 		        if (pDummyRec)
 		        	*pDummyRec = 0;
				FilePoint.x = *ipnt1;
				FilePoint.y = *ipnt2; 
				if (hTranReorg)
				{
					TranPointReorgFile (&FilePoint,hTranReorg);
					*ipnt1 = FilePoint.x;
					*ipnt2 = FilePoint.y;
				}
				PointRot = *ipnt++; 
				AddPointToMinMax (FilePoint,&NewMinMax);
			}
			break;
			
			case 172: // HiPrecis Curve
			{
				LPDPOINT	pDPoint;
				DPOINT		Point; 
				MNMXCORD MinMaxD;  
						
 		        if (pDummyRec)
 		        	*pDummyRec = 0;
	        	ipnt++;
	 		    CurrentType = GF_CURVE; 
				pDPoint = (LPDPOINT)ipnt;ipnt+=8; 
				*pDPoint = TranPointReorg (pDPoint,hTranReorg);
				BP = *pDPoint;
				pDPoint = (LPDPOINT)ipnt;ipnt+=8;
				*pDPoint = TranPointReorg (pDPoint,hTranReorg);
				CurPOCW = POC = *pDPoint;
				pDPoint = (LPDPOINT)ipnt;ipnt+=8;
				*pDPoint = TranPointReorg (pDPoint,hTranReorg);
				EP = *pDPoint;
	    		CurveMNMX (BP,POC,EP,&MinMaxD);
				Point.x = MinMaxD.xmn;
				Point.y = MinMaxD.ymn;
				FilePoint = BasePtToFilePtReorg (Point);
				AddPointToMinMax (FilePoint,&NewMinMax);
				Point.x = MinMaxD.xmn;
				Point.y = MinMaxD.ymx;
				FilePoint = BasePtToFilePtReorg (Point);
				AddPointToMinMax (FilePoint,&NewMinMax);
				Point.x = MinMaxD.xmx;
				Point.y = MinMaxD.ymx;
				FilePoint = BasePtToFilePtReorg (Point);
				AddPointToMinMax (FilePoint,&NewMinMax);
				Point.x = MinMaxD.xmx;
				Point.y = MinMaxD.ymn;
				FilePoint = BasePtToFilePtReorg (Point);
				AddPointToMinMax (FilePoint,&NewMinMax);
			}
			break;
			
			case 171:
				ipnt++;
			case 17: /* curve symbol */  
			{
				MNMXCORD MinMaxD;
				LPSHORT jpnt=ipnt; 
				POINT	POCFile,BPFile,EPFile; 
				   	
 		        if (pDummyRec)
 		        	*pDummyRec = 0;
	 		    CurrentType = GF_AREA; 
				BPFile.x = *ipnt++;
				BPFile.y = *ipnt++; 
				POCFile.x = *ipnt++; 
				POCFile.y = *ipnt++; 
				EPFile.x = *ipnt++; 
				EPFile.y = *ipnt++;  
				if (hTranReorg)
				{
					TranPointReorgFile ((HPPOINT)&BPFile,hTranReorg);
					TranPointReorgFile ((HPPOINT)&POCFile,hTranReorg);
					TranPointReorgFile ((HPPOINT)&EPFile,hTranReorg);
					*jpnt++ = BPFile.x;
					*jpnt++ = BPFile.y;
					*jpnt++ = POCFile.x;
					*jpnt++ = POCFile.y;
					*jpnt++ = EPFile.x;
					*jpnt++ = EPFile.y;
				} 
				BP = PointToDPoint (BPFile);
				POC = PointToDPoint (POCFile);
				EP = PointToDPoint (EPFile);
	    		CurveMNMX (BP,POC,EP,&MinMaxD);
				FilePoint.x = IDNINT(MinMaxD.xmn);
				FilePoint.y = IDNINT(MinMaxD.ymn);
				AddPointToMinMax (FilePoint,&NewMinMax); 
				FilePoint.x = IDNINT(MinMaxD.xmn);
				FilePoint.y = IDNINT(MinMaxD.ymx);
				AddPointToMinMax (FilePoint,&NewMinMax); 
				FilePoint.x = IDNINT(MinMaxD.xmx);
				FilePoint.y = IDNINT(MinMaxD.ymx);
				AddPointToMinMax (FilePoint,&NewMinMax); 
				FilePoint.x = IDNINT(MinMaxD.xmx);
				FilePoint.y = IDNINT(MinMaxD.ymn);
				AddPointToMinMax (FilePoint,&NewMinMax); 
	 		    CurrentType = GF_CURVE;
				break;
			}
			case 16: /* line symbol */
			{	
				LPSHORT	ipnt1=ipnt++;
				LPSHORT	ipnt2=ipnt++;
				LPSHORT	ipnt3=ipnt++;
				LPSHORT	ipnt4=ipnt++;
				
 		        if (pDummyRec)
 		        	*pDummyRec = 0;
 				FilePoint.x = *ipnt1;
				FilePoint.y = *ipnt2;
				if (hTranReorg)
				{
					TranPointReorgFile (&FilePoint,hTranReorg);
					*ipnt1 = FilePoint.x;
					*ipnt2 = FilePoint.y;
				}
				AddPointToMinMax (FilePoint,&NewMinMax);
				FilePoint.x = *ipnt3;
				FilePoint.y = *ipnt4;
				if (hTranReorg)
				{
					TranPointReorgFile (&FilePoint,hTranReorg);
					*ipnt3 = FilePoint.x;
					*ipnt4 = FilePoint.y;
				}
				AddPointToMinMax (FilePoint,&NewMinMax);
			}
			break;
			
			
		case 19: /* grahpics text */
		{	LPGRTEXTHEADER	pGRTextHeader,pPickedTextHeader;
			LPSTR	pText;  
			short	nchar;
			double	THeight;
				    
	        if (pDummyRec)
	        	*pDummyRec = 0;
		    pGRTextHeader = (LPGRTEXTHEADER)ipnt;  
			if (hTranReorg)
			{
		    	THeight = ReorgSizeFactor * GetTextHeadSize (pGRTextHeader); 
		    	SetTextHeadSize (pGRTextHeader,THeight); 
		    }
		    ipnt += sizeof(GRTEXTHEADER) / 2;  
		    nchar = pGRTextHeader->lText; 
		    pText = (LPSTR)ipnt;   
		    ipnt += nchar/2;
		    SetTextLocVars (&TLSet);
           	DBoundsInit (&Bounds);  
			if (ProcessTextObject (0,pGRTextHeader,pText,nchar,0,&Bounds,0,0,0))
        	{   
        		WorldPoint.x = Bounds.xmn;
        		WorldPoint.y = Bounds.ymn;
       			FilePoint = BasePtToFilePtReorg (WorldPoint); 
				AddPointToMinMax (FilePoint,&NewMinMax);
        		WorldPoint.x = Bounds.xmx;
        		WorldPoint.y = Bounds.ymx;
       			FilePoint = BasePtToFilePtReorg (WorldPoint); 
				AddPointToMinMax (FilePoint,&NewMinMax);
			} 
        }
        break; 
                
/*        case 191: //text pointer 
        {   
        	DPOINT	FromPoint;
                	
		    SaveDC (hDC);
        	lpTextTPL = ipnt;    
        	ipnt += 38/2; 
        	FromPoint = WinPtToBasePt (TXLoc);  
			{   HRGN    NewRgn, OvrLapRgn; 
			    RECT	TBRect=TextRect;
					    
			    NewRgn = CreateVPRgn (FALSE);
			    OvrLapRgn = CreateRectRgnIndirect (&TBRect);
	        	CombineRgn (NewRgn,NewRgn,OvrLapRgn,RGN_DIFF);
		  		SelectClipRgn (CurView->hDC,NewRgn); 
	            DeleteObject (OvrLapRgn);  
	            DeleteObject (NewRgn);
		  	}
			SimplePointer (hDC,&FromPoint,&lpTextTPL->points[0],1,2);
			RestoreDC (hDC,-1); 
        }
        break;    */
                
		
            case 192: //text pointer (UltiMap Style) 
            {   
            	LPUMTEXTTPL	lpTextTPL; 
            	DPOINT	TPLpoints[6];
				DPOINT	PLPoints[3]; 
                	
            	lpTextTPL = (LPUMTEXTTPL)ipnt;    
            	ipnt += 50/2; 
			    SetTextLocVars (&TLSet);   
			    for (i=0;i<6;i++)
			    	TPLpoints[i] = FPointToDPoint (lpTextTPL->points[i]);
            	DBoundsInit (&Bounds);  
            	if (ProcessUMTextPointer (0, FALSE,lpTextTPL, TPLpoints, PLPoints,&Bounds,0))
            	{   
            		WorldPoint.x = Bounds.xmn;
            		WorldPoint.y = Bounds.ymn;
	       			FilePoint = BasePtToFilePtReorg (WorldPoint); 
					AddPointToMinMax (FilePoint,&NewMinMax);
            		WorldPoint.x = Bounds.xmx;
            		WorldPoint.y = Bounds.ymx;
	       			FilePoint = BasePtToFilePtReorg (WorldPoint); 
					AddPointToMinMax (FilePoint,&NewMinMax);
				} 
            }
            break; 
                
			case 20: /* point symbol with size and real rot*/  
				{
					POINT	Point32;

	 				CurrentType = GF_POINT; 
	    			CurPointSize = *(LPFLOAT)ipnt;
        			ipnt += 2;
        			PTRot = *(LPFLOAT)ipnt;
        			ipnt += 2;
					Point32 = POINTStoPOINT (*(LPPOINTS)ipnt);
        			lpPoints = &Point32; 
        			TranPointReorgFile (lpPoints,hTranReorg); 
        			ipnt += 2;
					CurPointLocD = WorldPoint = FilePtToBasePt(*lpPoints); 
					goto PointMNMX;
				}

			case 120: 
		 	    CurrentType = GF_POINT; 
				pSize = (LPDOUBLE)ipnt;
				(*pSize) *= ReorgSizeFactor;
	    		CurPointSize = *pSize; 
        		ipnt += 4;
        		PTRot = *(LPDOUBLE)ipnt;
        		ipnt += 4;
        		lpDCurPoints = (HPDPOINT)ipnt;
        		*lpDCurPoints = TranPointReorg (lpDCurPoints,hTranReorg); 
        		CurPointLocD = WorldPoint = *lpDCurPoints;
        		ipnt += 8;
     PointMNMX: 
 		        if (pDummyRec)
 		        	*pDummyRec = 0;
     			if (Mode < 0 && !PointIsVisible (CurrentDesc))
     				break; 
				if (CurPointSize < 0)
					CurPointSize = -CurPointSize * DeviceToScreenFactor();
				else
					CurPointSize /= CurView->BaseUnitsPerPixel;
				CurPointSize *= FileDistToBaseDist;   
				if (CurrentDesc == 90)
					ii=1;
				{   
					RECT	SymRect = GetSymRect (CurrentDesc);  
					double	factor = max (((double)SymRect.right)/200,((double)SymRect.top)/200);
					
					CurPointSize *= factor;
			    }
				WorldPoint.x -= CurPointSize/2; 
				WorldPoint.y -= CurPointSize/2; 
       			FilePoint = BasePtToFilePtReorg (WorldPoint); 
				AddPointToMinMax (FilePoint,&NewMinMax); 
				WorldPoint.y += CurPointSize; 
       			FilePoint = BasePtToFilePtReorg (WorldPoint); 
				AddPointToMinMax (FilePoint,&NewMinMax); 
				WorldPoint.x += CurPointSize; 
       			FilePoint = BasePtToFilePtReorg (WorldPoint); 
				AddPointToMinMax (FilePoint,&NewMinMax); 
				WorldPoint.y -= CurPointSize; 
       			FilePoint = BasePtToFilePtReorg (WorldPoint); 
				AddPointToMinMax (FilePoint,&NewMinMax); 
			break;   
			
			case 24:
 		        if (pDummyRec)
 		        	*pDummyRec = 1;
 		        if (!AllowPickDeletes) 
 		        	NewMinMax = DummyMnMx;
			break;
						
	        default: 
	        	SkipSubRec (Pcode,&ipnt,0);
	        break;
	
		}
	}
	if (NewMinMax.xmn == SHRT_MAX && 
		NewMinMax.ymn == SHRT_MAX &&  
		NewMinMax.xmx == SHRT_MIN &&
		NewMinMax.ymx == SHRT_MIN)  
	{
		NewMinMax.xmn = SHRT_MIN; 
		NewMinMax.ymn = SHRT_MIN;  
		NewMinMax.xmx = SHRT_MAX;
		NewMinMax.ymx = SHRT_MAX;  
	}
	else
		ExpandMinMax (&NewMinMax,1);
	*pMinMax = NewMinMax;
{
#if ENABLETRACE
GSSiExitProg (1040);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
} 

void ShowTranPoint (HDC hDC,POINT point,COLORREF color)
{
	RECT	Rect; 
	short	inc=4;
	
	Rect.left = point.x-inc/2;
	Rect.right = point.x+inc/2;
	Rect.top = point.y-inc/2;
	Rect.bottom = point.y+inc/2; 
	FillRectPoly(hDC, &Rect, color);
	return;
}
 
void DisplayTransformThemeLegend(short From)
#if ENABLETRACE
{GSSiEnterProg (1041);
#endif
{
	HFILE Fid;
	OFSTRUCTGM OFStruct;
	char	str[260]; 
	long	BeginLoc;
	HANDLE	hTran1=0, hTran2=0, hcoord=GSSiGlobAlloc ( 971,GMEM_MOVEABLE,2*16*(long)MAXTRANPOINTS+MAXTRANPOINTS*2); 
	float	RSQMIN, RSQAll, RSQBest=0;
	short	N=0,i,n, nmaxresid,TotN, GetBest=1,BestN, NCoord, maxresidID, Type;   
	double	X, Y, resid, maxresid=-1;
	LPDOUBLE	XFROM,YFROM,XTO,YTO;  
	LPSHORT	ID; 
	LPSTR	lpCVT;   
	BOOL	SetTrans=FALSE;
	char	curproject[64];
	short	ConvertID=0;
	LPVIEWPORT	SaveView=CurView;
	DPOINT	FromPoint, ToPoint; 
	POINT	Points[2], MaxPoints[2];       
	HPEN	hPen, OldPen; 
	BOOL	DisplayMax=FALSE, First=TRUE, SkipThisCoord;    
	DPOINT	Point;
	
	
	SaveDC (CurView->hDC);  
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	SelectClipRgn (CurView->hDC,0);
    FillRectPoly (CurView->hDC,&CurView->DrawRect,ConvertColor(CurView->BackGroundColor,-1)); 
    if (From != 4)
    	goto Exit;
	
	Fid = GSSiOpenFile (CurTheme->DataFile,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
		goto Exit;
	if (!fgetstring (str,256,Fid))
	{
		GSSiClose (Fid);
		goto Exit;
	}
	Truncate (str);  
	_fstrlwr (str);
	if ((lpCVT = _fstrstr (str,".cvt")))
	{   
		lpCVT += 4;
		*lpCVT++ = 0;
		GetGlobalCVal ("[%ALT_PROJECTION]",curproject,0);
		SetGlobalValue("%ALT_PROJECTION",str); 
		ConvertID = atoi (lpCVT);
		ConvertCoordClose ();
		ConvertCoordInit(); 
		SetTrans = TRUE; 
		BeginLoc = GSSillseek (Fid,0,1);
	}
	else
		BeginLoc = 0;   
	if (!CurTheme->DisplayPointsOnly)
		GetBest=9999; 
	while (GetBest--)
	{   
		GSSillseek (Fid,BeginLoc,0); 
		N = NCoord = 0;
		XFROM = (LPDOUBLE)GlobalLock (hcoord);
		YFROM = XFROM+MAXTRANPOINTS;
		XTO = YFROM+MAXTRANPOINTS;
		YTO = XTO+MAXTRANPOINTS; 
		ID = (LPSHORT)(YTO+MAXTRANPOINTS);
		SkipThisCoord=FALSE;
		while (fgetstring (str,256,Fid))
		{   
			Truncate (str); 
			ExpandText (str);
			if (*str && *str != '#')
			{
				if (N != GetBest) 
				{
					if (sscanf (str,"%Flf %Flf %Flf %Flf",XFROM,YFROM,XTO,YTO) == 4) 
					{
					    XFROM++;
					    YFROM++;
					    XTO++;
					    YTO++; 
					    *ID++=N+1;
					    NCoord++;   
					} 
				} 
				else
				    SkipThisCoord=TRUE;
				N++;  
			}
		}
		GlobalUnlock (hcoord);
		XFROM = (LPDOUBLE)GlobalLock (hcoord);
		YFROM = XFROM+MAXTRANPOINTS;
		XTO = YFROM+MAXTRANPOINTS;
		YTO = XTO+MAXTRANPOINTS;
		ID = (LPSHORT)(YTO+MAXTRANPOINTS); 
		if (ConvertID == 1)
		{
			for (i=0;i<NCoord;i++)
			{
				Point.x = XFROM[i];
				Point.y = YFROM[i];
				ConvertCoord(&Point,3,1);
				XFROM[i] = Point.x;
				YFROM[i] = Point.y;
			}
		}
		else if (ConvertID == 2)
		{
			for (i=0;i<NCoord;i++)
			{
				Point.x = XTO[i];
				Point.y = YTO[i];
				ConvertCoord(&Point,3,1);
				XTO[i] = Point.x;
				YTO[i] = Point.y;
			}
		}  
		Type = CurTheme->DispersePoints+1;
		if (Type < 1 || Type > 3)
			Type = 1;
			
		if (NCoord > 1)
		{   
			short	itype=1;
			if (CurTheme->AddCommas && GetBest > 9990)  
			{
				itype = Type;
				SetGlobalValueBool ("%SHOWTIN",TRUE);
			}
		    SetViewport (CurTheme->TargetViewport);
			hTran1 = STRAN2 (1616,XFROM,YFROM,XTO,YTO,NCoord,&RSQMIN,itype,0);  
		    SetViewport (CurTheme->ReScan);
			hTran2 = STRAN2 (1617,XTO,YTO,XFROM,YFROM,NCoord,&RSQMIN,itype,0);
			if (CurTheme->AddCommas && GetBest > 9990)
				SetGlobalValueBool ("%SHOWTIN",FALSE);
		}
		SetCurView ( SaveView);
		GlobalUnlock (hcoord);
		if (First && !SkipThisCoord)
		{   
			if (N < 1)
			{
				GSSiClose (Fid);
				goto Exit;
			}
			TotN=N;
			RSQAll = RSQMIN; 
			if (!CurTheme->DisplayPointsOnly)
				GetBest = N;   
			First = FALSE;
		}
		else if (SkipThisCoord)
		{
			if (RSQMIN > RSQBest)
			{
				BestN = GetBest+1;
				RSQBest = RSQMIN;
			}
		} 
		if (GetBest < 0)
			break;
		CloseTRANS2 (&hTran1);
		CloseTRANS2 (&hTran2);
		if (!GetBest)
			GetBest = -1; 
	}
	
	GSSiClose (Fid);
	if (SetTrans)
	{
		SetGlobalValue("%ALT_PROJECTION",curproject);
		ConvertCoordClose ();
	}
	XFROM = (LPDOUBLE)GlobalLock (hcoord);
	YFROM = XFROM+MAXTRANPOINTS;
	XTO = YFROM+MAXTRANPOINTS;
	YTO = XTO+MAXTRANPOINTS;  
	ID = (LPSHORT)(YTO+MAXTRANPOINTS);
	hPen = CreatePen (PS_SOLID,CurTheme->ValConv+1,CurTheme->ClassColor[0]);
	OldPen = SelectObject (CurView->hDC,hPen);
	nmaxresid = -1;
	for (i=0;i<NCoord;i++)
	{
		if (!CurTheme->DisplayPointsOnly) 
		{
		    TRANS2 (*XFROM,*YFROM,&X,&Y,hTran1);  
	    	resid = LDIST (*XTO,*YTO,X,Y);
		    if (resid > maxresid)
		    {
		    	nmaxresid = i; 
		    	maxresidID = *ID;
		    	maxresid = resid;
		    } 
		}
	    FromPoint.x = *XFROM;
	    FromPoint.y = *YFROM;
	    ToPoint.x = *XTO;
	    ToPoint.y = *YTO;  
	    SetViewport (CurTheme->TargetViewport);
		if (PtInWBounds (&FromPoint))
		{   
			Points[0] = BasePtToWinPt (&FromPoint);
			ShowTranPoint (CurView->hDC,Points[0],CurTheme->ClassColor[0]); 

			SetViewport (CurTheme->ReScan); 
			if (PtInWBounds (&ToPoint))
			{   
				Points[1] = BasePtToWinPt (&ToPoint);
				ShowTranPoint (CurView->hDC,Points[1],CurTheme->ClassColor[0]);  
				if (CurTheme->ZeroIsMissing)
					Polyline (CurView->hDC,Points,2); 
				if (nmaxresid == i)
				{
					DisplayMax = TRUE;
					_fmemmove (MaxPoints,Points,sizeof(MaxPoints)); 
				}
			}  
		} 
		else if (nmaxresid == i)
			DisplayMax = FALSE;
		XFROM++;
		YFROM++;
		XTO++;
		YTO++;  
		ID++;
	}
	SetGlobalValueLong ("%MAXRESID_ID",(long)nmaxresid);
	SelectObject (CurView->hDC,OldPen);
	DeleteObject (hPen);
	if (DisplayMax)
	{
		hPen = CreatePen (PS_SOLID,1,CurTheme->ClassColor[1]);
		SelectObject (CurView->hDC,hPen);
		Polyline (CurView->hDC,MaxPoints,2);
		SelectObject (CurView->hDC,OldPen);
		DeleteObject (hPen);
	}
	SetCurView ( SaveView);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn (FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn);
    FillRectPoly (CurView->hDC,&CurView->DrawRect,ConvertColor(CurView->BackGroundColor,-1));
	sprintf (str,"RSQ = %f(%f-%i), Max residual = %.3Flf(%i) N=%i",RSQMIN,RSQBest,BestN,maxresid,maxresidID,NCoord); 
	TextOut (CurView->hDC,CurView->DrawRect.left+2,CurView->DrawRect.top+2,str,_fstrlen(str));
	GlobalUnlock (hcoord);
Exit:
	RestoreDC (CurView->hDC,-1);
	CloseTRANS2 (&hTran1);
	CloseTRANS2 (&hTran2);
	GSSiGlobFree (&hcoord);
{
#if ENABLETRACE
GSSiExitProg (1041);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetMapBounds (LPSTR File,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (1042);
#endif
{
	BOOL    SaveNRI = NoRefIndex;
	HANDLE	hCurView=0;  
	LPVIEWPORT	SaveVP=CurView;
	int ft;
	
    if (!CurView) 
    {
    	hCurView = GSSiGlobAlloc (0,GHND,sizeof(VIEWPORT));
    	CurView = (LPVIEWPORT)GlobalLock (hCurView);
    }
	ft = MapFileType(File);
	if (ft != MT_FILE_GEO_DB && FileType(File) == 2)
    {   
    	HANDLE	handle, hMem = GSSiGlobAlloc ( 972,GMEM_MOVEABLE,256);
    	LPSTR	pName = GlobalLock (hMem);
    	sprintf (pName,"%s\\index",File);
    	
		handle = OpenMapIndex (pName,0);
    	CloseMapIndex (pName,handle,FALSE,TRUE);  
    	GSSiGlobUlFree (&hMem);
    }
	else switch (ft)
	{
	case	MT_INDEX:
    {   
    	HANDLE	handle;
    	
		if (_fstrstr(File,"FILELIST.TXT"))
		{
			GetBinFileBounds (File,pBounds);
		}
		else
		{
			handle = OpenMapIndex (File,0);
			CloseMapIndex (File,handle,FALSE,TRUE);
		} 
    }
	break;

	case MT_DTM:
    {   
    	HANDLE		hSurf = DTMOpen (File,DBL_MAX,BT_READ,0);
		LPDTMINFO	pDTMInfo = (LPDTMINFO)GlobalLock (hSurf);  
		
		CurView->FileMNMX = pDTMInfo->Bounds;  
		GlobalUnlock (hSurf);
		DTMClose (&hSurf);
    }
	break;

	case MT_IMAGE:
	{
		HDIB32 hDib = LoadDIB32 (File,FALSE);
		MNMXCORD BitmapBounds;

    	GetImageBounds (File,hDib,&BitmapBounds,&CurView->FileMNMX);
		DestroyDIB32(hDib,FALSE);
	}
	break;

    default: 
    {
	    _fstrcpy (PltName,File);
	    CurView->HaveBounds=FALSE;
	    PltType = 4; 
	    NoRefIndex = TRUE;
	    if (!OpenMap (0,CurView->hDC))
	    {
		    NoRefIndex = SaveNRI;   
		    GSSiGlobUlFree (&hCurView);
		    CurView = SaveVP;
{
#if ENABLETRACE
GSSiExitProg (1042);
#endif
	    	return FALSE; 
}
	    }
	    CloseMap (FALSE);
	} 
	}
    *pBounds = CurView->FileMNMX;  
    NoRefIndex = SaveNRI;
    GSSiGlobUlFree (&hCurView);
    CurView = SaveVP;
{
#if ENABLETRACE
GSSiExitProg (1042);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}

void ApplyVPShadows (void)
#if ENABLETRACE
{GSSiEnterProg (1043);
#endif
{   
	short	iview;
	LPVIEWPORT	SaveVP = CurView;
	
    for (iview = 0;iview<*pNumViewports; iview++)
    {   
    	SetCurView ( pViewportsD[iview]); 
        if (CurViewActive() && CurView->Shadow)
        {
		    SetDisplayMode (CurView->hDC,GF_TEXTMODE);
		    GSSiDeleteObject(&CurView->hRgn);
	        CurView->hRgn = CreateVPRgn (TRUE,FALSE);
	        SelectClipRgn (CurView->hDC,CurView->hRgn);
	        GSSiDeleteObject(&CurView->hRgn);
 	     	DisplayShadow (CurView->hDC,&CurView->Rect,ShadowInc);
        }
    }  
    SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (1043);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL LoadVPToolBar (void)
#if ENABLETRACE
{GSSiEnterProg (1045);
#endif
{   
	BOOL	rtn=FALSE;  
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
	long	len=0, line=1; 
	int	height, width, type;
	LPSHORT	pInt, pLen;
	LPSTR	pTB, pAt, pComma, pStr; 
	char	str[1030];  
	char	tmp[256];
	RECT	rect, LogicalCoordVPRect;  
	LPRECT	pRect; 
	HANDLE	hTran=0;
	
	GSSiGlobFree (&CurView->ToolbarHandle);
	if (*CurView->PickMacroFile)
	{
		Fid = GSSiOpenFile (CurView->PickMacroFile,&OFStruct,OF_READ);
		if (Fid != HFILE_ERROR)
		{   
			CurView->ToolbarHandle = GSSiGlobAlloc ( 973,GHND,USHRT_MAX);   
			pTB = GlobalLock (CurView->ToolbarHandle);
			fgetstring (str,1024,Fid);while (*str == '#'){line++;fgetstring (str,1024,Fid);}  
			if (sscanf (str,"%i %i",&height,&width) != 2) 
				goto ErrOut;
			pInt = (LPSHORT)pTB;
			*pInt++ = height;
			*pInt = width; 
			len = 4;    
			LogicalCoordVPRect.left = LogicalCoordVPRect.top = 0;
			LogicalCoordVPRect.right = width;
			LogicalCoordVPRect.bottom = height;
			hTran = STRANRect (&LogicalCoordVPRect,&CurView->DrawRect);
//			sprintf (str,"%i %i %i %i | %i %i %i %i",LogicalCoordVPRect.left,LogicalCoordVPRect.right,LogicalCoordVPRect.top,LogicalCoordVPRect.bottom,
//													 CurView->DrawRect.left,CurView->DrawRect.right,CurView->DrawRect.top,CurView->DrawRect.bottom);
//			MessageBox (0,str,0,MB_OK);
			while (fgetstring (str,1024,Fid))
			{   
				line++;
				if (*str != '#')
				{   
					if (!(pComma = MatchLev (str,',')))
						goto ErrOut;
					*pComma++ = 0;
					pAt = pTB + len;
					pLen = (LPSHORT)pAt; 
					len += 2;
					if (sscanf (str,"%i %i %i %i",&rect.left,&rect.right,&rect.top,&rect.bottom) != 4) 
						goto ErrOut; 
					pAt = pTB + len;
					pRect = (LPRECT)pAt; 
					*pRect = rect;
					//sprintf (tmp,"%i %i %i %i",pRect->left,pRect->right,pRect->top,pRect->bottom);
					//MessageBox (0,tmp,0,MB_OK);
					TRANRect (pRect,hTran);
					//sprintf (tmp,"%i %i %i %i",pRect->left,pRect->right,pRect->top,pRect->bottom);
					//MessageBox (0,tmp,0,MB_OK);
					len += sizeof(RECT);
					(*pLen) += sizeof(RECT);
					pStr = pComma;
					if (!(pComma = MatchLev (pComma,',')))
						goto ErrOut;
					*pComma++ = 0;  
					pAt = pTB + len;
					_fstrcpy (pAt,pStr);
					len += _fstrlen (pStr)+1;
					(*pLen) += _fstrlen (pStr)+1;  
					pStr = pComma;
					if (!(pComma = MatchLev (pComma,',')))
						goto ErrOut;
					*pComma++ = 0;
					if (!_fstricmp (pStr,"MENU"))
						type = 1;
					else if (!_fstricmp (pStr,"COMMAND"))
						type = 2;    
					else
						goto ErrOut; 
					pAt = pTB + len;
					pInt = (LPSHORT)pAt;
					*pInt = type; 
					len += 2;  
					(*pLen) += 2;
					pAt = pTB + len;
					_fstrcpy (pAt,pComma);
					len += _fstrlen (pComma)+1;
					(*pLen) += _fstrlen (pComma)+1;  
				}
			} 
			len += 2;
			GlobalUnlock (CurView->ToolbarHandle);   
			CurView->ToolbarHandle = GSSiGlobalReAlloc (0,CurView->ToolbarHandle,len,GMEM_MOVEABLE);
			GSSiClose (Fid);
		}
	}     
	CloseTRANS2 (&hTran);
{
#if ENABLETRACE
GSSiExitProg (1045);
#endif
	return	rtn;  
}
ErrOut:
	GSSiGlobUlFree (&CurView->ToolbarHandle);
	GSSiClose (Fid);
	CloseTRANS2 (&hTran);
	sprintf (str,"Error at line %ld in toolbar file %s",line,CurView->PickMacroFile);
	GSSiMessageBox (str,0,MB_ICONEXCLAMATION,0);
{
#if ENABLETRACE
GSSiExitProg (1045);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}
HANDLE DisplayToolbarText (POINT CursorPoint,LPHANDLE phCmd,short WantBox)
#if ENABLETRACE
{GSSiEnterProg (1046);
#endif
{ 
	HANDLE	hBox=0; 
	LPSHORT	pInt, pLen, pType;  
	short	height, width;
	int		IconNum=0;
	LPSTR	pTB, pAt, pStr, pCmd; 
	char	str[1030];  
	LPRECT	pRect; 
	POINT	DisplayPoint;
	
	if (phCmd)
		*phCmd = 0;
	if (!CurView->ToolbarHandle)
{
#if ENABLETRACE
GSSiExitProg (1046);
#endif
		return 0;
}
	SaveDC (CurView->hDC);  
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	SelectClipRgn (CurView->hDC,0);
	pTB = GlobalLock (CurView->ToolbarHandle);
	pInt = (LPSHORT)pTB; 
	height = *pInt++;
	width = *pInt++;
	pLen = pInt;
	pAt = (LPSTR)pInt; 
	while (*pLen)
	{    
		pAt += 2;
		pRect = (LPRECT)pAt;
		pAt += sizeof(RECT);
		_fstrcpy (str,pAt);
		pAt = _fstrchr (pAt,0);
		pAt++; 
		pType = (LPSHORT)pAt;
		pAt += 2;   
		IconNum++;
		if (PtInRect (pRect,CursorPoint))
		{   
			ExpandText (str); 
			switch (WantBox)
			{
				case 2: 
					if (IconNum != (int)CurView->CurrentIcon)
					{   
						if (CurView->CurrentIcon)
							hBox = CurView->CurrentIcon;
						else
						{
							Draw3DBorder(CurView->hDC,pRect,-UP_3D,FALSE);
							CurView->CurrentIcon = (HANDLE)IconNum;
							CurView->CurrentIconRect = *pRect;
						} 
					}
					break;
				case 1: 
					DisplayPoint.x = (pRect->left + pRect->right)/2;
					DisplayPoint.y = pRect->bottom + 7;
					hBox = YellowTextBox (CurView->hWnd,str,DisplayPoint,0,0,FALSE,0);    
					break;
				case 0:
					Draw3DBorder(CurView->hDC,pRect,-DOWN_3D,FALSE);
					break;
			} 
			if (phCmd)
			{
		    	*phCmd = GSSiGlobAlloc ( 974,GMEM_MOVEABLE,4096);
		    	pCmd = GlobalLock (*phCmd); 
		    	*pCmd = 0;
		    	if (*pType == 1)
		    		_fstrcpy (pCmd,"MENU(");
		    	_fstrcat (pCmd,pAt);
	            GlobalUnlock (*phCmd); 
			}	            
            goto Exit;
		}
		pAt = _fstrchr (pAt,0);
		pAt++;
		pLen = (LPSHORT)pAt;
	}
Exit:	
	GlobalUnlock (CurView->ToolbarHandle);
	RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (1046);
#endif
	return hBox;
}
#if ENABLETRACE
}
#endif
}

void ClearSelectedToolbarIcons (void)
#if ENABLETRACE
{GSSiEnterProg (1047);
#endif
{
	if (CurView->CurrentIcon)
	{
		SaveDC (CurView->hDC);  
	    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
		SelectClipRgn (CurView->hDC,0);
		Draw3DBorder(CurView->hDC,&CurView->CurrentIconRect,-NO_3D,FALSE);
		RestoreDC (CurView->hDC,-1);   
		CurView->CurrentIcon = 0;
	}
{
#if ENABLETRACE
GSSiExitProg (1047);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL EditLastMenuFile (void)
{
	if (*LastMenuFile)
		GMEdit (hWndMain,LastMenuFile); 
	return TRUE;
}


HMENU LoadToolBarMenu (LPSTR Name,LPHANDLE phPopups)
#if ENABLETRACE
{GSSiEnterProg (1048);
#endif
{               
	HFILE	MFid;
	OFSTRUCTGM	OFStruct;
	HMENU	hMenu=0; 
	int		line=1, l;  
	long	NewLen;
	LPLONG	pNumCmd, pCmdOffset; 
	LPSHORT	pNumPop; 
	LPSTR	pCmd; 
	
	MFid = GSSiOpenFile (Name,&OFStruct,OF_READ); 
	if (MFid == HFILE_ERROR)
	{
		char	mess[256];
			
		sprintf (mess,"Menu file %s not found or locked",Name);
		MessageBox (GetFocus(),mess,0,MB_ICONEXCLAMATION);
{
#if ENABLETRACE
GSSiExitProg (1048);
#endif
		return FALSE;
}
	}
	strcpy (LastMenuFile,Name);
	hNewPopups = GSSiGlobAlloc ( 975,GHND,4096);
	phWhichCmdList = &hToolCmd; 
	if (!hToolCmd)
		hToolCmd = GSSiGlobAlloc ( 976,GHND,USHRT_MAX);
	hMenu = GMCreateMenu (MFid,&line,Name); 
	GSSiClose (MFid); 
	*phPopups = hNewPopups;
	hNewPopups = 0;
	GetCmdID (0,0);
{
#if ENABLETRACE
GSSiExitProg (1048);
#endif
	return hMenu;   
}
#if ENABLETRACE
}
#endif
}   

BOOL GMEnableMenuItem(HWND hWnd, UINT Item, UINT Action)
#if ENABLETRACE
{GSSiEnterProg (1049);
#endif
{
	HMENU	hMenu = GetMenu(hWnd); 
	BOOL	rtn;
	
	if (!hMenu)
{
#if ENABLETRACE
GSSiExitProg (1049);
#endif
		return FALSE;  
}
		
	rtn = EnableMenuItem (hMenu,Item,Action);
{
#if ENABLETRACE
GSSiExitProg (1049);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

short UnitsFromText (LPSTR str)  
{
	short Units = 0;
	
	if (!_fstricmp (str,"Meters"))
		Units = 1;
	else if (!_fstricmp (str,"Degrees")) 
		Units = 2;
	else if (!_fstricmp (str,"DM"))
		Units = 3;
	else if (!_fstricmp (str,"DMS"))
		Units = 4;
	else if (!_fstricmp (str,"MGRS"))
		Units = 5; 
	return Units;
}
 
long FillProjectionList (HWND hWndDlg,UINT cntl,LPSHORT pCurProj,UINT unitscntl,LPSHORT pCurUnits)
{   
	char	Name[MAX_PATH]="[%DL]projections"; 
	char	str[66];
	HFILE	Fid,Fid2;  
	long	TotFiles=0, Loc;    
	int		Item;
	LPSTR	pName;
	char	defaultProj[256]="";

    if (!hProjectionFile)
    {
    	hProjectionFile = GSSiGlobAlloc (0,GMEM_MOVEABLE,256); 
    	pName = GlobalLock (hProjectionFile);
		GSSiGetTempFileName(0,"gma",0,pName); 
	}	
	else
    	pName = GlobalLock (hProjectionFile);
	Fid =	GSSiOpenFile (pName,0,OF_CREATE);    
	ExpandText (Name);
	SearchFilesInDir (Name,0,Fid,&TotFiles,"*.cvt",1,FALSE,TRUE);     
	GSSillseek (Fid,0,0);  
	Loc = -1;
	_fstrcpy (Name,"[%DL]baseproj.cvt");
	ExpandText (Name);
	do
	{    
		Fid2 = GSSiOpenFile (Name,0,OF_READ);
		if (Fid2 != HFILE_ERROR)
		{   
			fgetstring (Name,250,Fid2);
        	Item = SendDlgItemMessage (hWndDlg,cntl,CB_ADDSTRING,0,(LPARAM)&Name[1]); 
        	if (*pCurProj < 0) 
        	{
				strcpy(defaultProj, &Name[1]);
        		*pCurProj = Item;
				fgetstring (str,64,Fid2);
				*pCurUnits = UnitsFromText (&str[1]);
			}
	        Item = SendDlgItemMessage (hWndDlg,cntl,CB_SETITEMDATA,(WPARAM)Item,(LPARAM)Loc);
        	GSSiClose (Fid2);
        }
        Loc = GSSillseek (Fid,0,1);
	} while (fgetstring (Name,250,Fid));

	GSSiClose (Fid);
	GlobalUnlock (hProjectionFile);
	if (*defaultProj)
	{
		*pCurProj = SendDlgItemMessage(hWndDlg, cntl, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)defaultProj);
	}
	return TotFiles;
}

BOOL GetProjectionFile (HWND hWndDlg,UINT cntl,LPSTR File)
{   
	LPSTR	pName;
	HFILE	Fid; 
	int		CurProj;
	long	Loc;
	
	*File = 0;
 	if (!hProjectionFile)
 		return FALSE;
 	pName=GlobalLock (hProjectionFile);
 	Fid = GSSiOpenFile (pName,0,OF_READ);
 	GlobalUnlock (hProjectionFile);
 	if (Fid == HFILE_ERROR)
 		return FALSE;
 	CurProj=SendDlgItemMessage(hWndDlg,IDC_PROJECTION,CB_GETCURSEL,0,0); 
 	Loc = SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_GETITEMDATA,(WPARAM)CurProj,(LPARAM)0);  
	if (Loc < 0)
		_fstrcpy (File,"[%DL]baseproj.cvt");
	else
	{
 		GSSillseek (Fid,Loc,0);
		fgetstring (File,250,Fid);
		SubstituteDL (File,TRUE);
	}
	GSSiClose (Fid);
	return TRUE;
}

void SetCoordTitle (HWND hWndDlg,short CurUnits)
{  
	short	TitleOpt=0;
	
	if (CurUnits > 1)
		TitleOpt=1;
	if (CurUnits == 5)
		TitleOpt=2;
	ShowWindow (GetDlgItem(hWndDlg,IDC_YTITLE),SW_SHOW);  
	ShowWindow (GetDlgItem(hWndDlg,IDC_Y),SW_SHOW);  
	switch (TitleOpt)
	{
		case 0:
			SetDlgItemText (hWndDlg,IDC_XTITLE,"X(Easting) :");
			SetDlgItemText (hWndDlg,IDC_YTITLE,"Y(Northing):");   
			break;
					        			
		case 1:
			SetDlgItemText (hWndDlg,IDC_XTITLE,"Longitude :");
			SetDlgItemText (hWndDlg,IDC_YTITLE,"Latitude  :");   
			break;
					        			
		case 2:
			SetDlgItemText (hWndDlg,IDC_XTITLE,"MGRS:");
			ShowWindow (GetDlgItem(hWndDlg,IDC_YTITLE),SW_HIDE);  
			ShowWindow (GetDlgItem(hWndDlg,IDC_Y),SW_HIDE);  
			break;
	} 
	switch (CurUnits)
	{
		case 0:
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEX,"529839.12");
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEY,"167422.56");   
			break;
		case 1:
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEX,"161495.28");
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEY,"51030.51");
			break;
		case 2:
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEX,"W93.26866D or -93.26866");
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEY,"N44.97625D or 44.97625");
			break;
		case 3:
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEX,"W93D16.245M or -93D16.245");
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEY,"N44D58.521M or 44D58.521");
			break;
		case 4:
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEX,"W93D16M30.5S or -93D16M30.5");
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEY,"N44D58M32.9S or 44D58M32.9");   
			break;
		case 5:
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEX,"15TVK7831480326");
			SetDlgItemText (hWndDlg,IDC_COORDEXAMPLEY,"");   
			break;
			
	}
	return;
}





 



void RemoveToolbarViewports (HWND hWnd,HDC hDC)
#if ENABLETRACE
{GSSiEnterProg (1054);
#endif
{   
	short	iv, AddV; 
	RECT	NewRect;
	
    for (iv = 0; iv<*pNumViewports; iv++)
    {   
    	AddV = pViewportsD[iv]->OnPrintAddSpaceToVP;
        if (AddV && !_fstricmp (pViewportsD[iv]->Name,"TOOLBAR"))
        {   
        	AddV--;
        	if (AddV != iv)
        	{
	        	pViewportsD[iv]->Active = FALSE; 
	        	UnionRect (&NewRect,&pViewportsD[iv]->Rect,&pViewports[AddV]->Rect); 
	        	pViewports[AddV]->Rect = NewRect;    
	        	SetupViewports (hWnd, hDC, pViewports[AddV]->ID,pViewports[AddV]->Rect,0); 
	        }
        }
    }
{
#if ENABLETRACE
GSSiExitProg (1054);
#endif
 	return;
}
#if ENABLETRACE
}
#endif
}

 

BOOL LinfitFilePoints (LPSTR FileList,LPSTR ExclusionList,short FitType,LPSTR FitProjection,LPSTR OutFile,LPSTR Symbol)
#if ENABLETRACE
{GSSiEnterProg (1056);
#endif
{
	HPDPOINT	pPoints, pRtnPoints;   
	DPOINT	Point1, Point2, OutPoint;
	short	pos=BT_FIRST;
	HANDLE	hPoints, hWeights,hFitPoints;   
	HPDPOINT	pBegPoint;
	HPDOUBLE	pWeights;
	long	Refno=1, NumPoints=0, LOFMDF, i;
	int		NFit=2; 
	BOOL	First=TRUE, Reverse1=FALSE, Reverse2=FALSE;  
	double	D1, D2, MinD, A, B, MAXDIF, Dist;    
	char	File[132], str[260]; 
	HFILE	Fid1, Fid2;
	OFSTRUCTGM	OFStruct;
	HANDLE	hDLT; 
	short	SymNum;
	MNMXCORD Bounds;  
	char	SaveAltProj[64]; 
	double	TotDist=0, AZ1,AZ2, TotPoints=0, SumWeights=0;             
    
    _fstrcpy (PltName,OutFile); 
    if (OpenMap (CurView->hWnd,CurView->hDC))
    {
		EditBounds = CurView->FileMNMX; 
		CloseMap (FALSE); 
	}
	else
{
#if ENABLETRACE
GSSiExitProg (1056);
#endif
		return FALSE;
}
   	SymNum = GetDictSymbolNumber (Symbol);
   	if (!SymNum)
{
#if ENABLETRACE
GSSiExitProg (1056);
#endif
   		return FALSE;
}
	Fid1 = GSSiOpenFile (FileList,&OFStruct,OF_READ);
	if (Fid1 == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1056);
#endif
		return FALSE; 
}
	if (FitType)
	{
	    GetGlobalCVal ("[%ALT_PROJECTION]",SaveAltProj,0);
	    SetGlobalValue("%ALT_PROJECTION",FitProjection);
	    ConvertCoordClose ();
	    ConvertCoordInit();
	}
    hPoints = GSSiGlobAlloc ( 977,GMEM_MOVEABLE,USHRT_MAX); 
    pPoints = (HPDPOINT)GlobalLock (hPoints);
    hWeights = GSSiGlobAlloc ( 978,GMEM_MOVEABLE,USHRT_MAX); 
    pWeights = (LPDOUBLE)GlobalLock (hWeights);   
    hFitPoints = GSSiGlobAlloc ( 979,GMEM_MOVEABLE,2*sizeof(DPOINT));
    pRtnPoints = (HPDPOINT)GlobalLock (hFitPoints);
	while (fgetstring (File,128,Fid1))
	{
		Fid2 = GSSiOpenFile (File,&OFStruct,OF_READ);
		if (Fid2 == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1056);
#endif
			return FALSE; 
}
		fgetstring (str,256,Fid2);
		ProcessDelimTextHeader(str, 0, Fid2, &hDLT, 0, 0);
		NumPoints = 0;	
		DBoundsInit (&Bounds);     
	  	while (fgetstring (str,256,Fid2))
	  	{
  			GetDelimTextData(str,hDLT,256);
			Point1.x = GetGlobalDVal ("[BPX]");
			Point1.y = GetGlobalDVal ("[BPY]");
			Point2.x = GetGlobalDVal ("[EPX]");
			Point2.y = GetGlobalDVal ("[EPY]");
			pPoints[NumPoints]= MidPointD (Point1,Point2);
			pWeights[NumPoints] = ldistp (Point1,Point2);
			if (FitType)
				ConvertCoord (&pPoints[NumPoints],1,3);
			AddDPointToMinMax (&pPoints[NumPoints],&Bounds);
			NumPoints++; 
  		} 
		GSSiGlobFree (&hDLT);
      	GSSiClose (Fid2); 
		switch (FitType)
		{
			case 0:
			if (LINFIT (pPoints,NumPoints,&A,&B,&MAXDIF,&LOFMDF))
			{ 
				if (fabs(B) < 1)
				{
					pRtnPoints[0].x = Bounds.xmn;
					pRtnPoints[0].y = A + pRtnPoints[0].x * B;
					pRtnPoints[1].x = Bounds.xmx;
					pRtnPoints[1].y = A + pRtnPoints[1].x * B;
				}
				else
				{                                        
					pRtnPoints[0].y = Bounds.ymn;
					pRtnPoints[0].x = (pRtnPoints[0].y - A)/B;
					pRtnPoints[1].y = Bounds.ymx;
					pRtnPoints[1].x = (pRtnPoints[1].y - A)/B;
				}
			}
			else
			{
					pRtnPoints[0].y = Bounds.ymn;
					pRtnPoints[0].x = pPoints->x;
					pRtnPoints[1].y = Bounds.ymx;
					pRtnPoints[1].x = pPoints->x;
			} 
			break;
			
			default:
			{
				double TotWeight=0, AveX=0,AveY=0;
				
				for (i=0;i<NumPoints;i++)
				{
					AveX += pPoints[i].x * pWeights[i];
					AveY += pPoints[i].y * pWeights[i];
					TotWeight += pWeights[i];
				}
				AveX /= TotWeight;
				AveY /= TotWeight;
				if (FitType == 1) 
				{
					pRtnPoints[0].x = Bounds.xmn;
					pRtnPoints[0].y = AveY;
					pRtnPoints[1].x = Bounds.xmx;
					pRtnPoints[1].y = AveY;
				}
				else
				{                                        
					pRtnPoints[0].y = Bounds.ymn;
					pRtnPoints[0].x = AveX;
					pRtnPoints[1].y = Bounds.ymx;
					pRtnPoints[1].x = AveX;
				}
			}
		}  
		ConvertCoord (&pRtnPoints[0],3,1);
		ConvertCoord (&pRtnPoints[1],3,1);
		AZ1 = getazd (&pRtnPoints[0],&pRtnPoints[1]); 
		AZ2 = LTWOPI (AZ1 + HALFPI);
		for (i=0;i<NumPoints;i++)
		{   
			ConvertCoord (&pPoints[i],3,1);
			LINSEC (pRtnPoints[0].x,pRtnPoints[0].y,AZ1, pPoints[i].x,pPoints[i].y,AZ2,&OutPoint.x,&OutPoint.y);
			TotDist += ldistp (pPoints[i],OutPoint) * pWeights[i]; 
			SumWeights += pWeights[i];
		} 
		TotPoints += NumPoints;  
      	{
			HANDLE	hSymDesc=0;
			short	NumSyms=0; 
			 
			AddToSymList (SymNum,&NumSyms,&hSymDesc); 
			Refno = GetNewRefno(PltName,0,0,0,0);
			AddPolyToMap (1,&NFit, &hFitPoints,1,Refno,0,2,SymNum,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
		    CloseMap(TRUE);  
			AddSymToMap (NumSyms,hSymDesc,0,0); 
            DestroySymList (&NumSyms,&hSymDesc);
		}
    }
    GSSiClose (Fid1);
	GSSiGlobUlFree (&hWeights);
	GSSiGlobUlFree (&hPoints); 
	GSSiGlobUlFree (&hFitPoints);
	if (FitType)
	{
		SetGlobalValue("%ALT_PROJECTION",SaveAltProj);
		ConvertCoordClose ();
		ConvertCoordInit();
    } 
    sprintf (str,"TotDist = %f, AveDist = %f",TotDist,TotDist/SumWeights);
    MessageBox (GetFocus(),str," ",MB_OK);
{
#if ENABLETRACE
GSSiExitProg (1056);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

BOOL SetEditName (BOOL DisplayError)
#if ENABLETRACE
{GSSiEnterProg (1057);
#endif
{
	
	if (!CurView->UpdateFile)
	{  
		if (DisplayError)
		{
	 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(CurView->hWnd, GF_CLOSE,0, 0L); 
{
#if ENABLETRACE
GSSiExitProg (1057);
#endif
			return FALSE;
}
		}
		*EditName = 0; 
	} 
	else
		_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
{
#if ENABLETRACE
GSSiExitProg (1057);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}  

BOOL BoundsToScreenPoly (LPMNMXCORD Bounds,LPPOINT Points,short EnlargePixels)
#if ENABLETRACE
{GSSiEnterProg (1058);
#endif
{   
	DPOINT	DPoint;
	
	DPoint.x = Bounds->xmn;
	DPoint.y = Bounds->ymn;
	Points[0] = BasePtToWinPt (&DPoint);
	DPoint.x = Bounds->xmn;
	DPoint.y = Bounds->ymx;
	Points[1] = BasePtToWinPt (&DPoint);
	DPoint.x = Bounds->xmx;
	DPoint.y = Bounds->ymx;
	Points[2] = BasePtToWinPt (&DPoint);
	DPoint.x = Bounds->xmx;
	DPoint.y = Bounds->ymn;
	Points[3] = BasePtToWinPt (&DPoint); 
	Points[0].x -= EnlargePixels;
	Points[1].x -= EnlargePixels;
	Points[2].x += EnlargePixels;
	Points[3].x += EnlargePixels;
	Points[0].y -= EnlargePixels;
	Points[1].y += EnlargePixels;
	Points[2].y += EnlargePixels;
	Points[3].y -= EnlargePixels;
{
#if ENABLETRACE
GSSiExitProg (1058);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL ScreenPolyToBounds (LPMNMXCORD pBounds,LPPOINT Points,short nPnts)
#if ENABLETRACE
{GSSiEnterProg (1059);
#endif
{   
	DPOINT	DPoint;
	UINT	i;
	
	DBoundsInit (pBounds);
	for (i=0;i<nPnts;i++) 
	{
		DPoint = WinPtToBasePt (*Points++); 
		AddDPointToMinMax (&DPoint,pBounds); 
	} 
{
#if ENABLETRACE
GSSiExitProg (1059);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}



BOOL GetRadialProfiles (DPOINT Point,LPDOUBLE pStartElev,short SurfSym,short nRadials,double LenRadial,LPHANDLE phRadials)
#if ENABLETRACE
{GSSiEnterProg (1061);
#endif
{
	MNMXCORD	Bounds;
	LPRADIAL	pRadial;  
	long		Refno, nPoly=0;  
	BOOL		SavePAP=PickAllPieces;
	HIGHLIGHTDATA	HighlightData;
	short		pos=BT_FIRST;
	LPTHEME		pTheme;
	
	
	Bounds.xmn = Point.x - LenRadial;
	Bounds.xmx = Point.x + LenRadial;
	Bounds.ymn = Point.y - LenRadial;
	Bounds.ymx = Point.y + LenRadial;
	
	ClearHighlightList (FALSE);
    PickAllPieces = TRUE;
	HighlightInArea (CurView->hWnd,&Bounds,TRUE,FALSE,0);
    PickAllPieces = SavePAP;
   	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
   	{
   		pos=BT_NEXT;
   		PickList[0]=HighlightData.PD;
    	if (PickList[0].Type == 2 && PickList[0].Desc == SurfSym)
    	{   
	   		SetConfig (PickList[0].ConfigID);
		    SetViewport (PickList[0].ViewID);
    		pTheme = AddTheme (GF_SAVEPOLY_THEME);
    		CurView->PassID = 4;
			ProcessSelectedTheme = CurView->NumThemes;
			ProcessPickedItem (0,FALSE); 
			ProcessSelectedTheme = 0;       		
    		DeleteTheme (pTheme); 
    		while (GetSavedPolys ())
    		if (hSavePoly)
    		{   LPMNMXCORD lpRect;
				HPDPOINT	lpDpoint;
    		    
    		    nPoly++;  
                nPnts = nSavePoly; 
                lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
                lpRect++;
                lpDpoint = (LPDPOINT) lpRect;
                while (nPnts--)
                	lpDpoint++;
                GSSiGlobUlFree (&hSavePoly);
             }
         }
    }
{
#if ENABLETRACE
GSSiExitProg (1061);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}

double ElevFromRadials (short nRadials,LPHANDLE hRadials)
#if ENABLETRACE
{GSSiEnterProg (1062);
#endif
{
	double 	Elev=0;
	
{
#if ENABLETRACE
GSSiExitProg (1062);
#endif
	return Elev;
}
#if ENABLETRACE
}
#endif
}

BOOL GetNextRadialPoint (LPDPOINT pNextPoint,LPDOUBLE pStartElev,short nRadials,LPHANDLE phRadials,double MaxRadialMove)
#if ENABLETRACE
{GSSiEnterProg (1063);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (1063);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

 

BOOL EndPointMacros (LPSTR PrePickMacro,LPSTR BPMacro,LPSTR EPMacro,LPSTR PostPickMacro)
#if ENABLETRACE
{GSSiEnterProg (1066);
#endif
{   
	short	pos=BT_FIRST;
	long	Sequence, Refno; 
	HIGHLIGHTDATA	HighlightData;  
	HANDLE	hStr=GSSiGlobAlloc ( 980,GMEM_MOVEABLE,2048);
	LPSTR	str = GlobalLock(hStr);
	HCURSOR	hcurSave;
	
	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
	while (ContinueProcessing && !BT_FIND (hHighlight2,(LPSTR)&Sequence,pos,BT_ANY,(LPSTR)&Refno))
	{   
		pos = BT_NEXT;
		BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData);
		PickList[0] = HighlightData.PD;
		ProcessPickedItem (0,FALSE);        		
		SavedCmdStringLoc[0] = CurGCmdStringLoc;
		_fstrcpy (SavePickName[0],PickName);	
		SetPickGlobals(0);
		sprintf (str,"$MACRO(%s)",PrePickMacro);
		ExpandText (str);
    	PickItems2 (hWndMain,HighlightData.PD.BeginPoint,FALSE,TRUE,TRUE);  
    	while (NumPicked--)
    	{
			PickList[0] = HighlightData.PD;
			ProcessPickedItem (0,FALSE);        		
			SavedCmdStringLoc[1] = CurGCmdStringLoc;
			_fstrcpy (SavePickName[1],PickName);	
			SetPickGlobals(0);
			sprintf (str,"$MACRO(%s)",BPMacro);
			ExpandText (str);
    	}
    	PickItems2 (hWndMain,HighlightData.PD.EndPoint,FALSE,TRUE,TRUE);  
    	while (NumPicked--)
    	{
			PickList[0] = HighlightData.PD;
			ProcessPickedItem (0,FALSE);        		
			SavedCmdStringLoc[1] = CurGCmdStringLoc;
			_fstrcpy (SavePickName[1],PickName);	
			SetPickGlobals(0);
			sprintf (str,"$MACRO(%s)",EPMacro);
			ExpandText (str);
    	}
		sprintf (str,"$MACRO(%s)",PostPickMacro);
		ExpandText (str);
	}    
	GSSiGlobUlFree (&hStr);
	GSSiSetCursor (hcurSave); 
{
#if ENABLETRACE
GSSiExitProg (1066);
#endif
	return TRUE; 
}
#if ENABLETRACE
}
#endif
}

BOOL UpdateCmdStringInFile (LPSTR VarName,LPSTR VarVal,short FilePos)
#if ENABLETRACE
{GSSiEnterProg (1067);
#endif
{   
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
	BOOL	rtn=TRUE; 
	short	ID, ls;  
	long	loc;
	LPSTR	pString;
	HANDLE	hStr;
	
	if (FilePos)
	{ 
		CurGCmdStringLoc=SavedCmdStringLoc[FilePos-1];
		_fstrcpy (PickName,SavePickName[FilePos-1]);	
	}
	if (!CurGCmdStringLoc)
{
#if ENABLETRACE
GSSiExitProg (1067);
#endif
		return FALSE;
}
	Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1067);
#endif
		return FALSE;  
}
	hStr = GSSiGlobAlloc ( 981,GMEM_MOVEABLE,512);
	pString = GlobalLock (hStr);
	GSSillseek (Fid,CurGCmdStringLoc,0);
	BigRead (Fid,(HPSTR)&ID,2);
	BigRead (Fid,(HPSTR)&ls,2); 
	if (ID == 40)
	{   
		loc = GSSillseek (Fid,0,1);
		BigRead (Fid,pString,ls);
		pString[ls] = 0;
		UpdateCmdStringVal (VarName,VarVal,pString,ls);
		GSSillseek (Fid,loc,0);
		BigWrite (Fid,(HPSTR)pString,ls,-1); 
	}
	else
		rtn=FALSE;   
	GSSiGlobUlFree (&hStr);
	GSSiClose (Fid);  
{
#if ENABLETRACE
GSSiExitProg (1067);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL UpdateCmdStringVal (LPSTR VarName,LPSTR VarVal,LPSTR CmdString,short MaxLen)  
#if ENABLETRACE
{GSSiEnterProg (1068);
#endif
{
	BOOL	rtn=TRUE;
	char	str[66];    
	LPSTR	pVar, pEnd; 
	short	StartLen,OldLen,NewLen,DifLen,RestLen;
	
	sprintf (str,"[%s]=",VarName);
	if (!(pVar=_fstrstr(CmdString,str)))
{
#if ENABLETRACE
GSSiExitProg (1068);
#endif
		return FALSE;   
}
	StartLen = _fstrlen (CmdString);
	pVar += _fstrlen (str);
	pEnd = MatchLev (pVar,';');   
	if (!pEnd)
		pEnd = _fstrchr (pVar,0); 
	RestLen = _fstrlen (pEnd); 
	OldLen = pEnd - pVar;
	NewLen = _fstrlen (VarVal); 
	DifLen = NewLen - OldLen; 
	if (StartLen + DifLen > MaxLen)
{
#if ENABLETRACE
GSSiExitProg (1068);
#endif
		return FALSE;             
}
	if (RestLen && DifLen != 0)
		_fmemmove ((pEnd+DifLen),pEnd,RestLen);
	_fmemmove (pVar,VarVal,NewLen);
{
#if ENABLETRACE
GSSiExitProg (1068);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  





BOOL ShowFullBM(BOOL FullMenu,int rotate,int left)
#if ENABLETRACE
{GSSiEnterProg (1070);
#endif
{   int nRc; 
	char	Dialog[16];
	
	fullBMLeft = left;
	if (FullMenu)
		_fstrcpy (Dialog,"FULL_BM2");
	else
		_fstrcpy (Dialog,"FULL_BM");
     {
      DLGPROC lpfnFULLBMMsgProc;

      lpfnFULLBMMsgProc = MakeProcInstance((DLGPROC)FULLBMMsgProc, hInst);
      nRc = DialogBox(hInst, Dialog, hWndMain,(DLGPROC)lpfnFULLBMMsgProc);
      FreeProcInstance(lpfnFULLBMMsgProc);
     } /*
	fullBMLeft=0;
    {
    FARPROC lpfnRBUTOPSMsgProc;

    lpfnRBUTOPSMsgProc = MakeProcInstance((FARPROC)RBUTOPSMsgProc, hInst);
    nRc = DialogBox(hInst, (LPSTR)"FULLBM", hWndMain, lpfnRBUTOPSMsgProc);
    FreeProcInstance(lpfnRBUTOPSMsgProc);

   }     */

{
#if ENABLETRACE
GSSiExitProg (1070);
#endif
     return nRc;
}

#if ENABLETRACE
}
#endif
}

BOOL VPIsMap (short VPID)
#if ENABLETRACE
{GSSiEnterProg (1071);
#endif
{   
	BOOL	rtn;
	LPVIEWPORT	SaveVP=CurView;
	
	if (!SetViewport (VPID))
		rtn = FALSE;
	else if (CurView->ZoomLocked)
		rtn = FALSE;
	else if (!stricmp (CurView->Name,"TOOLBAR"))
		rtn = FALSE;
	else if (CurView->Type == 1 && (!CurView->pTheme || CurView->pTheme->ID == GF_PROFILE_LINK_THEME) && CurView->NumFiles) //18/8/2004 && !(CurView->NumFiles == 1 && CurView->FileType[0] == 3))
		rtn = TRUE;  
	else if (CurView->pTheme && CurView->pTheme->ID == GF_COMPARE_VIEWPORTS_THEME)
		rtn = TRUE; 
	else if (CurView->Type == 7 || CurView->Type == 8)
		rtn = TRUE;
	else
		rtn = FALSE;
	SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (1071);
#endif
	return rtn;
}
	
#if ENABLETRACE
}
#endif
}



void SetFileRotation (void)
{
	double	Rot = GetGlobalDVal2("[%ROTATION]",0);
	float	RSQMIN;
	double	XFROM[2]={0,100}, YFROM[2]={0,0}, XTO[2], YTO[2]; 
	DPOINT	Point1, Point2;

	if (CurView->FileProjectionType == 2)
		CurView->FileProjectionType = 0; 
	if (!Rot)
		return;
	Point1.x = 0;
	Point1.y = 0; 
	Rot *= RADDEG;
	Point2 = dnewpt (Point1,Rot,100);
	XTO[0] = YTO[0] = 0;
	XTO[1] = Point2.x;
	YTO[1] = Point2.y;
	CloseTRANS2 (&hTranProjection);
	CloseTRANS2 (&hTranProjectionReverse);  
	hTranProjection = STRAN2 (1618,XFROM,YFROM,XTO,YTO,2,&RSQMIN,1,0);  
	hTranProjectionReverse = STRAN2 (1619,XTO,YTO,XFROM,YFROM,2,&RSQMIN,1,0); 
	CurView->FileProjectionType = 2; 
	return;
} 

void CreateFileTran (LPMINMAX MinMax,LPMNMXCORD pMinMaxCoord)
{
     float  RSQMIN;
     double XFILE[4], YFILE[4], XWIN[4], YWIN[4], XBASE[4], YBASE[4];
     USHORT	i;
      
     XFILE[0]=MinMax->xmn;
     XFILE[1]=MinMax->xmn;
     XFILE[2]=MinMax->xmx;
     XFILE[3]=MinMax->xmx;
     YFILE[0]=MinMax->ymn;
     YFILE[1]=MinMax->ymx;
     YFILE[2]=MinMax->ymx;
     YFILE[3]=MinMax->ymn;
     XBASE[0]=pMinMaxCoord->xmn;
     XBASE[1]=pMinMaxCoord->xmn;
     XBASE[2]=pMinMaxCoord->xmx;
     XBASE[3]=pMinMaxCoord->xmx;
     YBASE[0]=pMinMaxCoord->ymn;
     YBASE[1]=pMinMaxCoord->ymx;
     YBASE[2]=pMinMaxCoord->ymx;
     YBASE[3]=pMinMaxCoord->ymn; 
     if(!CurView->hTranBaseToVP) 
		CreateBaseToVPTran (CurView->DrawRect);  
	 for (i=0;i<4;i++)
	 	TRANS2 (XBASE[i],YBASE[i],&XWIN[i],&YWIN[i],CurView->hTranBaseToVP); 
	 CloseTRANS2(&hTranFileToBase);
	 CloseTRANS2(&hTranBaseToFile);
	 CloseTRANS2(&hTranFileToVP);
	 hTranFileToBase = STRAN2(1620, XFILE, YFILE, XBASE, YBASE, 4, &RSQMIN, 1, 0);
     hTranBaseToFile = STRAN2 (1621,XBASE,YBASE,XFILE,YFILE,4,&RSQMIN,1,0); 
     hTranFileToVP  = STRAN2 (1622,XFILE,YFILE,XWIN,YWIN,4,&RSQMIN,1,0);
     return;
}

void CreateFileTranD (LPMNMXCORD MinMax,LPMNMXCORD pMinMaxCoord)
{
     float  RSQMIN;
     double XFILE[4], YFILE[4], XWIN[4], YWIN[4], XBASE[4], YBASE[4];
     USHORT	i;
      
     XFILE[0]=MinMax->xmn;
     XFILE[1]=MinMax->xmn;
     XFILE[2]=MinMax->xmx;
     XFILE[3]=MinMax->xmx;
     YFILE[0]=MinMax->ymn;
     YFILE[1]=MinMax->ymx;
     YFILE[2]=MinMax->ymx;
     YFILE[3]=MinMax->ymn;
     XBASE[0]=pMinMaxCoord->xmn;
     XBASE[1]=pMinMaxCoord->xmn;
     XBASE[2]=pMinMaxCoord->xmx;
     XBASE[3]=pMinMaxCoord->xmx;
     YBASE[0]=pMinMaxCoord->ymn;
     YBASE[1]=pMinMaxCoord->ymx;
     YBASE[2]=pMinMaxCoord->ymx;
     YBASE[3]=pMinMaxCoord->ymn; 
     if(!CurView->hTranBaseToVP) 
		CreateBaseToVPTran (CurView->DrawRect);  
	 for (i=0;i<4;i++)
	 	TRANS2 (XBASE[i],YBASE[i],&XWIN[i],&YWIN[i],CurView->hTranBaseToVP); 
	 CloseTRANS2 (&hTranFileToBase); 
	 CloseTRANS2 (&hTranBaseToFile);  
	 CloseTRANS2 (&hTranFileToVP);  
     hTranFileToBase = STRAN2 (1623,XFILE,YFILE,XBASE,YBASE,4,&RSQMIN,1,0);
     hTranBaseToFile = STRAN2 (1624,XBASE,YBASE,XFILE,YFILE,4,&RSQMIN,1,0); 
     hTranFileToVP  = STRAN2 (1625,XFILE,YFILE,XWIN,YWIN,4,&RSQMIN,1,0);
     return;
}






BOOL LoadIndexParm (LPSTR PltName)
{   
	if (PltName && !*PltName)
		return FALSE;
	LoadDGNParm (PltName);
	LoadSHPParm (PltName,0,0);
	return TRUE;
}
BOOL DeleteRedefinedItem (LPSTR EditName)
#if ENABLETRACE
{GSSiEnterProg (862);
#endif
{
	HIGHLIGHTDATA	HighlightData;  
	long TLID;

	BT_FIND (hHighlight,(LPSTR)&TLID,BT_FIRST,BT_ANY,(LPSTR)&HighlightData);   
	PickList[0] = HighlightData.PD;
	ClearHighlightList (FALSE);
	GetPickName (0);   
	if (!_fstricmp (EditName,PickName))
	{
		DeletePickedItem (0,12,92);
	}
{
#if ENABLETRACE
GSSiExitProg (862);
#endif
	return TRUE; 
}
#if ENABLETRACE
}
#endif
}

void DisplayFileBounds (MNMXCORD Bounds)
#if ENABLETRACE
{GSSiEnterProg (863);
#endif
{	MNMXCORD	FileBounds, BoundSP;
	RECT		Rect;     
	DPOINT		BasePt;
	POINT		WinPt;
	
	SetDisplayMode (CurView->hDC,GF_TEXTMODE);
    
    BasePt.x = Bounds.xmn;
    BasePt.y = Bounds.ymn;
	WinPt = BasePtToWinPt(&BasePt);
	Rect.left = WinPt.x;
	Rect.bottom = WinPt.y;
    BasePt.x = Bounds.xmx;
    BasePt.y = Bounds.ymx;
	WinPt = BasePtToWinPt(&BasePt);
	Rect.right = WinPt.x;
	Rect.top = WinPt.y;
	FillRectPoly (CurView->hDC,&Rect,
                  RGB(127+32+IDNINT(((double)rand()/RAND_MAX)*32),
                      127+32+IDNINT(((double)rand()/RAND_MAX)*32),
                      127+32+IDNINT(((double)rand()/RAND_MAX)*32)));
{
#if ENABLETRACE
GSSiExitProg (863);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
	

void CreateDigCursor (HDC hDC)
#if ENABLETRACE
{GSSiEnterProg (1014);
#endif
{   
    HDC			hdcMem;
    HBITMAP		hbmPrev, hNewBM;  
    HPEN		OldPen, WidePen;
    POINT		P[4];  
    DPOINT		p[3];
    RECT		Rect;  
    HCURSOR		hCur,hNewDigCursor=0;   
    static		hDebugCurs=0;
    BOOL		DigIsCurCursor;
    UINT		i,ii; 
    BITMAP		bm; 
    COLORREF	WideColor = 0;
    HBRUSH		BGColor = GetStockObject (WHITE_BRUSH);  
    BYTE		bits1[128],bits2[128], allon=255;   
    double		AZ;  
    short		Pass=0, NumPass=0, Width=1;
    char		str[128];
    
    if (!CursorIsLocked && GetGlobalCVal ("[%DIGCURSOR]",str,0))
    {
	    hNewDigCursor = LoadCursor (hInst,str); 
	    AddToSpecialCursorList (hNewDigCursor);
	}
	if (!hNewDigCursor)
	{
		hdcMem = CreateCompatibleDC(hDC);
		_fmemset (bits1,allon,sizeof(bits1));
		hNewBM = CreateBitmap(32,32,1,1,bits1);
	    if (GetGlobalBVal ("[%WIDECURSOR]"))
	    {
	    	WideColor = RGB(255,255,255); 
	    	BGColor = GetStockObject (BLACK_BRUSH);
	    	Width=3; 
	    	NumPass = 3;
	    }
	    Rect.left=Rect.top=0;
	    Rect.right=Rect.bottom=32;
		_fmemset (bits2,0,128);    
		WidePen = CreatePen (PS_SOLID,Width,WideColor);
		SetBkMode (hdcMem,OPAQUE);
		SetROP2(hdcMem,R2_COPYPEN);
	NextPass:
	    hbmPrev = SelectObject (hdcMem,hNewBM); 
	    switch (Pass)
	    {
	    	case 0: 
	    		if (NumPass)
	    		{
				    //FillRectPoly (hdcMem,&Rect,RGB(255,255,255)); 
		    		OldPen = SelectObject (hdcMem,GetStockObject (BLACK_PEN)); 
		    	}
		    	else
	    		{
				    //FillRectPoly (hdcMem,&Rect,0);  
				    FillRect (hdcMem,&Rect,GetStockObject(BLACK_BRUSH));
		    		OldPen = SelectObject (hdcMem,GetStockObject (WHITE_PEN)); 
		    	}
	    		break;
	    	case 1:
			    FillRect (hdcMem,&Rect,BGColor); 
	    		OldPen = SelectObject (hdcMem,WidePen);
	    		break;
	    	case 2:   
	    		OldPen = SelectObject (hdcMem,GetStockObject (BLACK_PEN));
	    		break; 
	    }
	    P[0].x=P[0].y=15; 
	    CurrentAZ = LTWOPI (CurrentAZ);
	    P[1]=newpt(P[0],CurrentAZ,15);
	    PolylineINV (hdcMem,P,2);
	    P[3]=P[2]=newpt(P[1],CurrentAZ+HALFPI+HALFPI/2,5);
	    PolylineINV (hdcMem,&P[1],2);
	    P[2]=newpt(P[1],CurrentAZ-HALFPI-HALFPI/2,5);
	    PolylineINV (hdcMem,&P[1],2);   
	    if (AZLocked)
	    	PolylineINV (hdcMem,&P[2],2);
	    P[1]=newpt(P[0],CurrentAZ+HALFPI,15);
	    PolylineINV (hdcMem,P,2);
	    P[1]=newpt(P[0],CurrentAZ+PY,15);
	    PolylineINV (hdcMem,P,2);
	    P[1]=newpt(P[0],CurrentAZ+HALFPI*3,15);
	    PolylineINV (hdcMem,P,2);  
	    if (CursorIsLocked)
	    {   
	    	for (i=0;i<4;i++)
	    	{   
	    		AZ=CurrentAZ+(i*HALFPI)+HALFPI/2;
			    P[1]=newpt(P[0],AZ,3);
			    P[2]=newpt(P[1],AZ,8);
			    PolylineINV (hdcMem,&P[1],2);
			    P[2]=newpt(P[1],AZ-HALFPI/2,4);
			    PolylineINV (hdcMem,&P[1],2);
			    P[2]=newpt(P[1],AZ+HALFPI/2,4);
			    PolylineINV (hdcMem,&P[1],2);
			}
	    }
	    SelectObject (hdcMem,hbmPrev);  
	    SelectObject (hdcMem,OldPen);
	    GetObject(hNewBM, sizeof(BITMAP), (LPSTR) &bm);
	    switch (Pass)
	    {
	    	case 0:
			    GetBitmapBits (hNewBM,(bm.bmWidthBytes * bm.bmHeight),&bits1);  
			    if (!NumPass)  
			    {
			    	_fmemset (bits2,allon,sizeof(bits2));
			    	break;                               
			    }
	    	case 1:
	    		Pass++;
	    		if (Pass < 2 || Width > 1)
	    			goto NextPass; 
	    	case 2:
	    		GetBitmapBits (hNewBM,(bm.bmWidthBytes * bm.bmHeight),&bits2); 
	    }
	    DeleteDC (hdcMem); 
	    DeleteObject (hNewBM); 
	    DeleteObject (WidePen);
	}
    hCur = GSSiSetCursor (LoadCursor(0, IDC_ARROW));     
    DigIsCurCursor = (hCur == hDigCursor) && hCur;
    if (DigIsCurCursor)
    {   
    	if (hDigCursor && (hDigCursor != hNewDigCursor))
    	{
    		short ii =1;
    		
    		if (!InSpecialCursorList (hDigCursor))
    			ii=DestroyCursor (hDigCursor);
    		if (!ii)
    			ii=1;    
    		hDigCursor = 0;
    	}
    	if (hNewDigCursor)
    		hDigCursor = hNewDigCursor;
    	else if (Width > 1)
	    	hDigCursor = CreateCursor (hInst,15,15,32,32,bits1,bits2);  
    	else
	    	hDigCursor = CreateCursor (hInst,15,15,32,32,bits2,bits1);  
    	SetCurs ((HCURSOR)2,FALSE);
       	PostMessage(hWndMain, WM_SETCURSOR, 0, 0L);
    }
    else
    {   
    	if (hDigCursor && (hDigCursor != hNewDigCursor))
    	{
    		short ii=1; 
    		if (!InSpecialCursorList (hDigCursor))
    			ii=DestroyCursor (hDigCursor);
    		if (!ii)
    			ii=1;
    		hDigCursor = 0;
    	}
    	if (hNewDigCursor)
    		hDigCursor = hNewDigCursor;
    	else if (Width > 1)
	    	hDigCursor = CreateCursor (hInst,15,15,32,32,bits1,bits2);  
    	else
	    	hDigCursor = CreateCursor (hInst,15,15,32,32,bits2,bits1);  
//    	if (!LoadCursor (hInst,hCur))
//    		ii=1;
    }
    	
{
#if ENABLETRACE
GSSiExitProg (1014);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL HaveUpdateFile (void)
{
	if (!CurView)
		return FALSE;
	if (!CurView->UpdateFile)
	{
		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}
	return TRUE;
}


