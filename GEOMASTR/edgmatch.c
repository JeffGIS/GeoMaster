#include "graphint.h"   

#include "gmextern.h"

static	HANDLE	hCorner,hUseEdgePoint=0;
static	short	nCorner;

typedef struct {char	ID[8];
				short	nPoints;
				DPOINT	Points[3];
				double	Dist[3],
						Adjust[2];
				}FILESIDE;
static	FILESIDE	FileSide[4];
static	short	nFileSides;

BOOL LoadEdge (LPSTR JoinLineFile,LPSTR CornerCoordFile,LPSTR OrigTranPointFile,LPSTR CornerFile,LPSTR JoinFileList,LPSTR PlotFile,LPSTR NewTranFile)
{
	LPSTR		lpDot, lpEdgeID, lpSpace, lpBeg; 
	char		EdgeID[8], CornerID[8]; 
	DPOINT		Point[2], OutPoint, TempPoint;    
	double		Tol=10.0;  
	HANDLE		hEdge; 
	short		nEdge=0,i;
	LPEDGE		pEdge; 
	LPCORNER	pCorner;  
	HANDLE		hMem=GSSiGlobAlloc (1044,GMEM_MOVEABLE,4096);
	LPSTR		pFile = GlobalLock (hMem);
	LPSTR		lpstr = pFile + 512;   
	HFILE		Fid1, Fid2, Fid3;  
	HANDLE		hDLT=0;
	long		ii;
			
	nCorner = 0;
	nEdge = LoadEdgeFile (JoinLineFile,&hEdge);
	nFileSides = 0;
	Fid1 = GSSiOpenFile (CornerCoordFile,0,OF_READ);
	if (Fid1 == HFILE_ERROR)
		goto RtnFalse; 
	hCorner = GSSiGlobAlloc (1045,GHND,USHRT_MAX);
	pCorner = (LPCORNER)GlobalLock (hCorner);
	fgetstring (lpstr,1020,Fid1);
	ProcessDelimTextHeader(lpstr, 0, Fid1, &hDLT, 0, 0);
	while (fgetstring (lpstr,1020,Fid1))
	{
		GetDelimTextData(lpstr,hDLT,1020);
		GetGlobalCVal ("[UDI]",pCorner->ID,0);
		pCorner->Point.x = GetGlobalDVal ("[BPX]");
		pCorner->Point.y = GetGlobalDVal ("[BPY]");
		pCorner++;
		nCorner++; 
	} 
	GSSiClose (Fid1);
	GSSiGlobFree (&hDLT);
	GlobalUnlock (hCorner); 
	nEdge = LoadEdgeFromCorners (CornerFile,&hEdge);
	if (!nEdge)
		goto RtnFalse; 
	Fid3 = GSSiOpenFile (NewTranFile,0,OF_CREATE);
	Fid1 = GSSiOpenFile (OrigTranPointFile,0,OF_READ);
	if (Fid1 == HFILE_ERROR)
		goto RtnFalse;
	fgetstring (lpstr,1020,Fid1);
	ProcessDelimTextHeader(lpstr, 0, Fid1, &hDLT, 0, 0);
	fputstring ("# Original Transformation Points",Fid3);	     
	while (fgetstring (lpstr,1020,Fid1))
	{
		GetDelimTextData(lpstr,hDLT,1020);
		Point[0].x = GetGlobalDVal ("[BPX]");
		Point[0].y = GetGlobalDVal ("[BPY]"); 
		Point[1] = Point[0];
		sprintf (lpstr,"%f %f %f %f",Point[0].x,Point[0].y,Point[1].x,Point[1].y);  
		fputstring (lpstr,Fid3);
	} 
	GSSiClose (Fid1);
	GSSiGlobFree (&hDLT);
	Fid1 = GSSiOpenFile (CornerFile,0,OF_READ);
	if (Fid1 == HFILE_ERROR)
		goto RtnFalse;
	fputstring ("# Corner Points",Fid3);	     
	while (fgetstring (lpstr,1020,Fid1))
	{   
		lpSpace = _fstrchr (lpstr,' ');
		*lpSpace++ = 0;
		_fstrcpy (CornerID,lpstr); 
		lpBeg = lpSpace;
		lpSpace = _fstrchr (lpBeg,' ');
		*lpSpace++ = 0;
		Point[0].x = atof (lpBeg);
		Point[0].y = atof (lpSpace);
		//next line prevents edges ending at corners from being used
      	UseEdgePoint (Point[0],&OutPoint,"CORPT",nEdge,hEdge);
		if (GetCornerPoint (CornerID,&Point[1]))
		{   
			SetCornerAdjustment (CornerID,Point[1],Point[0],nEdge,hEdge);
			sprintf (lpstr,"%f %f %f %f",Point[0].x,Point[0].y,Point[1].x,Point[1].y);  
			fputstring (lpstr,Fid3);
		}
	} 
	GSSiClose (Fid1);
	Fid1 = GSSiOpenFile (JoinFileList,0,OF_READ);
	if (Fid1 == HFILE_ERROR)
		goto RtnFalse; 
	fputstring ("# Edge Points",Fid3);	     
	while (fgetstring(pFile,256,Fid1))
	{
		Fid2 = GSSiOpenFile (pFile,0,OF_READ);
		if (Fid2 != HFILE_ERROR)
		{   
			lpEdgeID = _fstrrchr (pFile,'\\');
			lpEdgeID++;
			lpDot = _fstrchr (lpEdgeID,'.');
			*lpDot = 0;
			_fstrcpy (EdgeID,lpEdgeID);
			fgetstring (lpstr,1020,Fid2);
			ProcessDelimTextHeader(lpstr, 0, Fid2, &hDLT, 0, 0);
	  		while (fgetstring (lpstr,1020,Fid2))
	  		{
      			GetDelimTextData(lpstr,hDLT,1020);
      			GetGlobalCVal ("[FILENAME]",pFile,0);
      			if (!_fstricmp (pFile,PlotFile))
      			{   
      				ii=GetGlobalLVal ("[INT_REFNO]"); 
      				if (ii == 301935)
      					ii=0;
      				Point[0].x = GetGlobalDVal ("[BPX]");
      				Point[0].y = GetGlobalDVal ("[BPY]");
      				Point[1].x = GetGlobalDVal ("[EPX]");
      				Point[1].y = GetGlobalDVal ("[EPY]"); 
      				if (ldistp (Point[0],Point[1]) > Tol)
      				{   
      					for (i=0;i<2;i++)
      					{
      						if (UseEdgePoint (Point[i],&OutPoint,EdgeID,nEdge,hEdge))
      						{
			      				sprintf (lpstr,"%f %f %f %f (%s)",Point[i].x,Point[i].y,OutPoint.x,OutPoint.y, EdgeID);  
			      				fputstring (lpstr,Fid3);
			      			}
			      		}
	      			}
      			}
		    }
			GSSiClose (Fid2);
			GSSiGlobFree (&hDLT);
		}
	}
	GSSiGlobUlFree (&hMem);
	GSSiGlobFree (&hUseEdgePoint); 
	GSSiGlobFree (&hEdge); 
	GSSiGlobFree (&hCorner);
	GSSiClose (Fid1);
	GSSiClose (Fid3);
	return TRUE; 
RtnFalse:
	GSSiGlobUlFree (&hMem);
	return FALSE;
}

short LoadEdgeFile (LPSTR File,LPHANDLE hEdge)
{   
	LPEDGE		pEdge; 
	short	nEdge=0;
	HFILE	Fid1;
	OFSTRUCTGM	OFStruct;
	DPOINT	TempPoint; 
	HANDLE	hStr;    
	LPSTR	lpstr;   
	HANDLE	hDLT;
	
	Fid1 = GSSiOpenFile (File,&OFStruct,OF_READ);
	if (Fid1 == HFILE_ERROR)
		return 0;   
	*hEdge = GSSiGlobAlloc (1046,GHND,USHRT_MAX);
	pEdge = (LPEDGE)GlobalLock (*hEdge);   
	hStr = GSSiGlobAlloc (1047,GMEM_MOVEABLE,1024);
	lpstr = GlobalLock (hStr);
	fgetstring (lpstr,1020,Fid1);
	ProcessDelimTextHeader(lpstr, 0, Fid1, &hDLT, 0, 0);
	while (fgetstring (lpstr,1020,Fid1))
	{
		GetDelimTextData(lpstr,hDLT,1020);
		GetGlobalCVal ("[UDI]",pEdge->ID,0);
		pEdge->Point[0].x = GetGlobalDVal ("[BPX]");
		pEdge->Point[0].y = GetGlobalDVal ("[BPY]");
		pEdge->Point[1].x = GetGlobalDVal ("[EPX]");
		pEdge->Point[1].y = GetGlobalDVal ("[EPY]");
		if (fabs (pEdge->Point[0].x - pEdge->Point[1].x) >
			fabs (pEdge->Point[0].y - pEdge->Point[1].y))
		{
			if (pEdge->Point[0].x > pEdge->Point[1].x)
			{
				TempPoint = pEdge->Point[0];
				pEdge->Point[0] = pEdge->Point[1];
				pEdge->Point[1] = TempPoint;
			}
		}
		else 
			if (pEdge->Point[0].y > pEdge->Point[1].y)
			{
				TempPoint = pEdge->Point[0];
				pEdge->Point[0] = pEdge->Point[1];
				pEdge->Point[1] = TempPoint;
			}

		pEdge++;
		nEdge++; 
	} 
	GSSiGlobFree (&hDLT);
	GSSiClose (Fid1);
	GlobalUnlock (*hEdge); 
	GSSiGlobUlFree (&hStr);
	return nEdge;
}

void AddEdge (LPSTR SideID,DPOINT Point1, DPOINT Point2,LPEDGE	*pEdge,LPSHORT pnEdge)
{   
	DPOINT	TempPoint;
	
	_fstrcpy ((*pEdge)->ID,SideID);
	(*pEdge)->Point[0] = Point1;
	(*pEdge)->Point[1] = Point2;
	if (fabs ((*pEdge)->Point[0].x - (*pEdge)->Point[1].x) >
		fabs ((*pEdge)->Point[0].y - (*pEdge)->Point[1].y))
	{
		if ((*pEdge)->Point[0].x > (*pEdge)->Point[1].x)
		{
			TempPoint = (*pEdge)->Point[0];
			(*pEdge)->Point[0] = (*pEdge)->Point[1];
			(*pEdge)->Point[1] = TempPoint;
		}
	}
	else 
	{
		if ((*pEdge)->Point[0].y > (*pEdge)->Point[1].y)
		{
			TempPoint = (*pEdge)->Point[0];
			(*pEdge)->Point[0] = (*pEdge)->Point[1];
			(*pEdge)->Point[1] = TempPoint;
		}
    } 
    (*pEdge)++;
    (*pnEdge)++;
    return;
}

short LoadEdgeFromCorners (LPSTR File,LPHANDLE hEdge)
{   
	LPEDGE		pEdge; 
	short	nEdge=0, n=0, j, i, ID1, Next;
	HFILE	Fid1;
	OFSTRUCTGM	OFStruct;
	DPOINT	TempPoint, Point[4]; 
	HANDLE	hStr;    
	LPSTR	lpstr, lpSpace, pC, pSideID; 
	char	CornerID[4][8], SideID[4][2][4]; 

	Fid1 = GSSiOpenFile (File,&OFStruct,OF_READ);
	if (Fid1 == HFILE_ERROR)
		return 0;   
	*hEdge = GSSiGlobAlloc (1048,GHND,2048);
	pEdge = (LPEDGE)GlobalLock (*hEdge);   
	hStr = GSSiGlobAlloc (1049,GMEM_MOVEABLE,1024);
	lpstr = GlobalLock (hStr);
	while (fgetstring (lpstr,1020,Fid1))
	{   
		if (n>=4)
		{
			GSSiMessageBox (0,File,"Too many corners in file",MB_ICONEXCLAMATION,0); 
			nEdge = 0;
			goto Exit; 
		}
		lpSpace = _fstrchr (lpstr,' ');
		*lpSpace++ = 0;
		_fstrcpy (CornerID[n],lpstr); 
		pC = _fstrchr (CornerID[n],'C');  
		if (!pC)
			return FALSE;
		*pC = 0;
		_fstrcpy (SideID[n][0],CornerID[n]);
		*pC = 'C';
		_fstrcpy (SideID[n][1],pC);
      	if (GetCornerPoint (CornerID[n],&Point[n]))
        	n++;
    }
	if (n!=4)
	{
		GSSiMessageBox (0,File,"Not enough corners in file",MB_ICONEXCLAMATION,0); 
		nEdge = 0;
		goto Exit; 
	} 
	for (j=0;j<2;j++)
	{
		ID1=0;
		if (!_fstricmp (SideID[0][j],SideID[1][j]))
			Next=2;
		else
			Next=1;
		for (i=1;i<4;i++)
		{   
			pSideID = SideID[ID1][j];
			if (!_fstricmp (SideID[ID1][j],SideID[i][j]))
	        	AddEdge (pSideID,Point[ID1],Point[i],&pEdge,&nEdge);
	    }
	    ID1=Next;
		for (i=Next+1;i<4;i++)
		{
			pSideID = SideID[ID1][j];
			if (!_fstricmp (SideID[ID1][j],SideID[i][j]))
	        	AddEdge (pSideID,Point[ID1],Point[i],&pEdge,&nEdge);
	    }
	}
Exit: 
	GSSiClose (Fid1);
	GlobalUnlock (*hEdge); 
	GSSiGlobUlFree (&hStr);
	return nEdge;
}

BOOL GetCornerPoint (LPSTR CornerID,LPDPOINT pOutPoint)
{
	LPCORNER pCorner;
	short	i;
	
	if (!hCorner)
		return FALSE;
	pCorner = (LPCORNER)GlobalLock (hCorner);
	for (i=0;i<nCorner;i++,pCorner++)
	{
		if (!_fstricmp (pCorner->ID,CornerID))
		{
			*pOutPoint = pCorner->Point;
			GlobalUnlock (hCorner);
			return TRUE;
		}
	}
	GlobalUnlock (hCorner);
	return FALSE;
} 

BOOL GetEdgePoints (LPSTR EdgeID,LPDPOINT pEdgePoints,short nEdge,HANDLE hEdge)
{
	LPEDGE	pEdge;
	short	i;    
	
	pEdge = (LPEDGE)GlobalLock (hEdge);
	for (i=0;i<nEdge;i++,pEdge++)
	{
		if (!_fstricmp (pEdge->ID,EdgeID))
			goto GotEdge;
	}
	GlobalUnlock (hEdge);
	return FALSE;
GotEdge:
	*pEdgePoints++ = pEdge->Point[0];
	*pEdgePoints = pEdge->Point[1]; 
	GlobalUnlock (hEdge);
	return TRUE;
}

BOOL UseEdgePoint (DPOINT InPoint,LPDPOINT pOutPoint,LPSTR EdgeID,
				   short nEdge,HANDLE hEdge)
{   
	LPCORNER pCorner; 
	DPOINT	EdgePoints[2];
	static	short	nPoints;
	short	i;
	LPDPOINT	pPoints;
	double	AZ1,AZ2, EdgeDist;
	
	if (!hUseEdgePoint)
	{
		nPoints = 0;
		hUseEdgePoint = GSSiGlobAlloc (1050,GHND,USHRT_MAX);
	}                   
	pPoints = (LPDPOINT)GlobalLock (hUseEdgePoint);
	for (i=0;i<nPoints;i++,pPoints++)
	{ 
		if (ldistp (InPoint,*pPoints) < P_TOL)
		{
			GlobalUnlock (hUseEdgePoint);
			return FALSE;
		}
	}
	GlobalUnlock (hUseEdgePoint);
/*	if (hCorner)
	{
		pCorner = GlobalLock (hCorner);
		for (i=0;i<nCorner;i++,pCorner++)
		{
			if (ldistp (InPoint,pCorner->Point) < P_TOL)
			{
				GlobalUnlock (hCorner);
				return FALSE;
			}
		}
		GlobalUnlock (hCorner);  
	} */
	*pPoints = InPoint;
	nPoints++;  
	if (!GetEdgePoints (EdgeID,EdgePoints,nEdge,hEdge))
		return FALSE;
GotEdge:  
	AZ1 = getazd (&EdgePoints[0],&EdgePoints[1]); 
	AZ2 = LTWOPI (AZ1 + HALFPI);
	LINSEC (EdgePoints[0].x,EdgePoints[0].y,AZ1, InPoint.x,InPoint.y,AZ2,&pOutPoint->x,&pOutPoint->y);
	EdgeDist = ldistp (EdgePoints[0],*pOutPoint);
	ApplyCornerAdjustment (EdgeID,EdgeDist,pOutPoint,nEdge,hEdge);
	GlobalUnlock (hEdge);
	return TRUE;
} 

BOOL SetCornerAdjustment (LPSTR CornerID,DPOINT ToPoint,DPOINT FromPoint,short nEdge,HANDLE hEdge)
{    
	short 	i;
	char	SideID[2][4];
	LPSTR	pC;    
	DPOINT	EdgePoints[2];
	double	SideDist;
	
	pC = _fstrchr (CornerID,'C');  
	if (!pC)
		return FALSE;
	*pC = 0;
	_fstrcpy (SideID[0],CornerID);
	*pC = 'C';
	_fstrcpy (SideID[1],pC);
	if (!GetEdgePoints (SideID[0],EdgePoints,nEdge,hEdge))
		return FALSE;
	SideDist = ldistp (ToPoint,EdgePoints[0]);
	AddSidePoint (SideID[0],ToPoint,FromPoint,SideDist,EdgePoints);
	if (!GetEdgePoints (SideID[1],EdgePoints,nEdge,hEdge))
		return FALSE;
	SideDist = ldistp (ToPoint,EdgePoints[0]);
	AddSidePoint (SideID[1],ToPoint,FromPoint,SideDist,EdgePoints);
	return TRUE;
}

void AddSidePoint (LPSTR SideID,DPOINT ToPoint,DPOINT FromPoint,double ToDist,LPDPOINT EdgePoints)
{   
	short	i; 
	double	AZ1,AZ2, FromDist, Adjust;   
	DPOINT	OutPoint;
	
	AZ1 = getazd (&EdgePoints[0],&EdgePoints[1]); 
	AZ2 = LTWOPI (AZ1 + HALFPI);
	for (i=0;i<nFileSides;i++)
	{
		if (!_fstricmp(SideID,FileSide[i].ID))
			goto GotSide;
	}            
	_fstrcpy (FileSide[nFileSides].ID,SideID);
	FileSide[nFileSides].nPoints=0;
	i = nFileSides++;
GotSide:
	LINSEC (EdgePoints[0].x,EdgePoints[0].y,AZ1, FromPoint.x,FromPoint.y,AZ2,&OutPoint.x,&OutPoint.y);
	FromDist = ldistp (OutPoint,EdgePoints[0]);
	Adjust = ToDist - FromDist;
	if (FileSide[i].nPoints)
	{ 
		if (FromDist < FileSide[i].Dist[0])
		{
			FileSide[i].Points[2] = FileSide[i].Points[0];
			FileSide[i].Points[0] = OutPoint;
			FileSide[i].Dist[2] = FileSide[i].Dist[0];
			FileSide[i].Dist[0] = FromDist;
			FileSide[i].Adjust[1] = FileSide[i].Adjust[0];
			FileSide[i].Adjust[0] = Adjust;
		}
		else
		{
			FileSide[i].Points[2] = OutPoint;
			FileSide[i].Dist[2] = FromDist;
			FileSide[i].Adjust[1] = Adjust;
		} 
		FileSide[i].Points[1] = MidPointD (FileSide[i].Points[0],FileSide[i].Points[2]);
		FileSide[i].Dist[1] = (FileSide[i].Dist[0] + FileSide[i].Dist[2]) / 2;
		FileSide[i].nPoints = 2;
	}
	else  
	{
		FileSide[i].Points[0]=OutPoint;
		FileSide[i].Dist[0] = FromDist;    
		FileSide[i].Adjust[0] = Adjust;
		FileSide[i].nPoints = 1;
	}
	return;
}

BOOL ApplyCornerAdjustment (LPSTR EdgeID,double EdgeDist,LPDPOINT pPoint,short nEdge,HANDLE hEdge)
{   
	DPOINT	EdgePoints[2];
	short	i;  
	double	PCTAdjust, AZ, Adjust=0;
	
	if (!GetEdgePoints (EdgeID,EdgePoints,nEdge,hEdge))
		return FALSE;
	EdgeDist = ldistp (*pPoint,EdgePoints[0]);
	for (i=0;i<nFileSides;i++)
	{    
		if (!_fstricmp (FileSide[i].ID,EdgeID))
		{
			if (FileSide[i].nPoints == 2)
			{
				if (EdgeDist >= FileSide[i].Dist[0] && 
					EdgeDist <= FileSide[i].Dist[1])
				{   
					PCTAdjust = 1.0-(EdgeDist - FileSide[i].Dist[0])/(FileSide[i].Dist[1]-FileSide[i].Dist[0]);
					Adjust = PCTAdjust * FileSide[i].Adjust[0];
				}
				else 
				if (EdgeDist >= FileSide[i].Dist[1] && 
					EdgeDist <= FileSide[i].Dist[2])
				{ 
					PCTAdjust = 1.0-(EdgeDist - FileSide[i].Dist[1])/(FileSide[i].Dist[2]-FileSide[i].Dist[1]);
					Adjust = PCTAdjust * FileSide[i].Adjust[0];
				}
				EdgeDist += Adjust; 
				AZ = getazd (&EdgePoints[0],&EdgePoints[1]);
				*pPoint = dnewpt (EdgePoints[0],AZ,EdgeDist);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL MatchTranFunction (LPSTR Arg1,LPSTR Arg2,LPSTR Arg3)
{
	LPOFSTRUCTGM	pOFStruct;
	LPSTR	pFile; 
	char	id[8]; 
	double	MatchTol, AvDist, AZ;  
	long	FileNameLoc,TranDataLoc,nfix=0,len;
	short	nEdge,st;
	HANDLE	hEdge, hBTMatch;
	LPSTR	EdgeID, lpEndPar, str;
	DPOINT	EdgePoints[2], Points[2], NewPoint;  
	BTVARDESC	BTVar[2]; 
	typedef	struct	{char	ID[4];
			 		 double	Dist;} MATCHKEY;
	MATCHKEY	MatchKey, MatchKeyLast;
	typedef	struct	{long	FileNameLoc, TranDataLoc;} MATCHDATA;
	MATCHDATA	MatchData, MatchDataLast;      
	HANDLE	hMem = GSSiGlobAlloc (1051,GMEM_MOVEABLE,3*2048);   
	BOOL	rtn=FALSE; 
	HFILE	Fid1, Fid2;
	
	str = GlobalLock(hMem);
	pOFStruct = (LPOFSTRUCTGM) (str + 2048);   
	pFile = str + 2048 + 1024;
	MatchTol = atof (Arg3);
	nEdge = LoadEdgeFile (Arg1,&hEdge);
	if (!nEdge)
		goto Exit;
	Fid1 = GSSiOpenFile (Arg2,pOFStruct,OF_READ);
	if (Fid1 == HFILE_ERROR)
	{
		GSSiGlobFree (&hEdge); 
		goto Exit; 
	}
	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_REAL;
	BTVar[1].BT_VARLEN=8;
	BTVar[1].BT_VAROFF=4;
	GSSiGetTempFileName (0,"gm",0,pFile); 
	BT_CREATE (pFile, sizeof(MatchData), FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hBTMatch = BT_OPEN (pFile, 0, BT_WRITE, 0);
	FileNameLoc = GSSillseek (Fid1,0,1);
	while (fgetstring(pFile,256,Fid1))
	{
		Fid2 = GSSiOpenFile (pFile,pOFStruct,OF_READ);
		if (Fid2 != HFILE_ERROR)
		{   
			TranDataLoc = GSSillseek (Fid2,0,1);
			while (fgetstring (str,200,Fid2))
			{
				if ((EdgeID = _fstrchr (str,'(')))
				{       
					EdgeID++;
					lpEndPar = _fstrchr (EdgeID,')');
					*lpEndPar = 0;
					if (!GetEdgePoints (EdgeID,EdgePoints,nEdge,hEdge))
						goto Exit;
					sscanf (str,"%Flf %Flf %Flf %Flf",&Points[0].x, &Points[0].y,
											  		  &Points[1].x, &Points[1].y);
					MatchKey.Dist = ldistp (EdgePoints[0],Points[1]);
					_fstrncpy (MatchKey.ID,EdgeID,4); 
					MatchData.FileNameLoc = FileNameLoc;
					MatchData.TranDataLoc = TranDataLoc;
					BT_PUT (hBTMatch,(LPSTR)&MatchKey,(LPSTR)&MatchData);
				}
				TranDataLoc = GSSillseek (Fid2,0,1);
			}
	    }
	    GSSiClose (Fid2);
		FileNameLoc = GSSillseek (Fid1,0,1);
	}
	st = BT_FIND (hBTMatch,(LPSTR)&MatchKeyLast,BT_FIRST,BT_ANY,(LPSTR)&MatchDataLast);  
	while (!BT_FIND (hBTMatch,(LPSTR)&MatchKey,BT_NEXT,BT_ANY,(LPSTR)&MatchData))
	{
		if (!_fstrcmp (MatchKey.ID,MatchKeyLast.ID) &&  
			MatchData.FileNameLoc != MatchDataLast.FileNameLoc &&
			MatchKey.Dist - MatchKeyLast.Dist <= MatchTol)
		{   
			GetEdgePoints (MatchKey.ID,EdgePoints,nEdge,hEdge);
			AvDist = (MatchKey.Dist + MatchKeyLast.Dist)/2;
			AZ = getazd (&EdgePoints[0],&EdgePoints[1]);
			NewPoint = dnewpt (EdgePoints[0],AZ,AvDist);
			GSSillseek (Fid1,MatchData.FileNameLoc,0);
			fgetstring (pFile,200,Fid1);
			Fid2 = GSSiOpenFile (pFile,pOFStruct,OF_READWRITE);
			GSSillseek (Fid2,MatchData.TranDataLoc,0);
			fgetstring (str,200,Fid2);
			len = _fstrlen (str); 
			sscanf (str,"%Flf %Flf %Flf %Flf %s",&Points[0].x, &Points[0].y,
											  &Points[1].x, &Points[1].y,id);
			sprintf (str,"%f %f %f %f %s",Points[0].x,Points[0].y,NewPoint.x,NewPoint.y, id);  
			GSSillseek (Fid2,MatchData.TranDataLoc,0);
			BigWrite (Fid2,(HPSTR)str,(UINT)len,-1);
			GSSiClose (Fid2);
			GSSillseek (Fid1,MatchDataLast.FileNameLoc,0);
			fgetstring (pFile,200,Fid1);
			Fid2 = GSSiOpenFile (pFile,pOFStruct,OF_READWRITE);
			GSSillseek (Fid2,MatchDataLast.TranDataLoc,0);
			fgetstring (str,200,Fid2); 
			len = _fstrlen (str); 
			sscanf (str,"%Flf %Flf %Flf %Flf %s",&Points[0].x, &Points[0].y,
											  &Points[1].x, &Points[1].y,id);
			sprintf (str,"%f %f %f %f %s",Points[0].x,Points[0].y,NewPoint.x,NewPoint.y, id);  
			GSSillseek (Fid2,MatchDataLast.TranDataLoc,0);
			BigWrite (Fid2,(HPSTR)str,(UINT)len,-1);
			GSSiClose (Fid2);
			MatchKeyLast.ID[0]=0;    
			nfix++;
		}
		else
		{
			MatchKeyLast = MatchKey;
			MatchDataLast = MatchData;
		}
	}
	GSSiGlobFree (&hEdge); 
	GSSiClose (Fid1);
	BT_CLOSEANDDELETE (&hBTMatch);
	rtn = TRUE;
Exit: 
	GSSiGlobUlFree (&hMem);
	return rtn;
}  


