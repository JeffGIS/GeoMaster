#include "graphint.h"  
#include "umio.h"


#include "gmextern.h"

#define	MAX_ACCOUNTS	256

static int		NumAccounts=0;
static HANDLE	hVehicleLinkList[MAX_ACCOUNTS];
static HANDLE	AccountFenceHandles[MAX_ACCOUNTS];
static int		NumAccountRoutes[MAX_ACCOUNTS];
static char		AccountID[MAX_ACCOUNTS][8];

BOOL LoadAccountRoutes (int iaccount)
{
	BOOL rtn=FALSE;
	MNMXCORD	Bounds;
	HIGHLIGHTDATA	HighlightData;
	long	Refno, npnts=0;
	HANDLE	hPoly;
	int		Layer, nAreas, err, pos=BT_FIRST,len;
	HPDPOINT	pPoly;
	LPFENCEHEADER	pFenceHeader;
	char	Filelist[MAX_PATH]="[%DL]Fences\\[CAN]\\filelist.txt";
	HFILE	Fid;

	if (NumAccountRoutes[iaccount] > -1)
		return TRUE;
    ClearHighlightList (FALSE);  
	SetViewport(*pCommandViewport);
	LoadPickList ("[%DL]piklists\\routes.pik");
	Layer = GetLayerNumFromName ("Active Fences");
	NumAccountRoutes[iaccount] = 0;
    if (GetLayerBounds (&Bounds,CurView->hDC, Layer-1)) 
    {
		if (HighlightInArea (CurView->hWnd,&Bounds,TRUE,FALSE,0))
		{
			int nRoutes = BT_NUM_IN_INDEX (hHighlight);
			LPHANDLE	phRoutes;

 			AccountFenceHandles[iaccount] = GSSiGlobAlloc (1577,GMEM_MOVEABLE,sizeof(HANDLE)*nRoutes);
			phRoutes = GlobalLock (AccountFenceHandles[iaccount]);
  			while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))
			{
				pos = BT_NEXT;
				if (GetPolyPnts ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts,&hPoly,TRUE))
				{
					int	np=npnts;
					LPSTR	pTab=strchr (HighlightData.PD.UDI,'_');
					
					if (pTab)
						pTab++;
					else
						pTab = HighlightData.PD.UDI;
					pPoly = GlobalLock (hPoly);
					len = sizeof (FENCEHEADER) + npnts * sizeof(DPOINT);
					phRoutes[NumAccountRoutes[iaccount]] = GSSiGlobAlloc (1578,GMEM_MOVEABLE,len);
					pFenceHeader = GlobalLock (phRoutes[NumAccountRoutes[iaccount]]);
					strcpy (pFenceHeader->Name,pTab);
					strupr (pFenceHeader->Name);
					pFenceHeader->Type = FT_ROUTE;
					pFenceHeader->NumPoints = npnts;
					pFenceHeader->BaseOffset = 0;
					Fid = GSSiOpenFile (Filelist,0,OF_READ);
					if (Fid != HFILE_ERROR)
					{
						char	line[256], search[64];

						sprintf (search,"_%s.",pFenceHeader->Name);
						while (fgetstring (line,254,Fid))
						{
							strupr (line);
							if (strstr (line,search))
							{
								LPSTR pPar = strrchr (line,'(');
								double	off;
								char	units;

								pPar+=2;
								units = *pPar++;
								off = atof (pPar);
								if (units == 'F')
									off /= MFT;
								else
									off *= 5280/MFT;
								pFenceHeader->BaseOffset = off;
								break;
							}
						}

						GSSiClose (Fid);
					}
					memmove ((LPDPOINT)(pFenceHeader+1),pPoly,npnts * sizeof(DPOINT));
					GlobalUnlock (phRoutes[NumAccountRoutes[iaccount]]);
					GSSiGlobUlFree (&hPoly);
					NumAccountRoutes[iaccount]++;
				}
			}
			GlobalUnlock (AccountFenceHandles[iaccount]);
		}
	}
	return rtn;
}

int GetAccountNum (LPSTR Account)
{
	int	iaccount;
	static	BOOL	First=TRUE;

	if (First)
	{
		for (iaccount = 0;iaccount<MAX_ACCOUNTS;iaccount++)
		{
			NumAccountRoutes[iaccount] = -1;
			AccountFenceHandles[iaccount] = 0;
		}
		First = FALSE;
	}

	if (!Account)
	{
		for (iaccount = 0;iaccount<NumAccounts;iaccount++)
		{
			GSSiGlobFree (&hVehicleLinkList[iaccount]);
			GSSiGlobFree (&AccountFenceHandles[iaccount]);
		}
		NumAccounts = 0;

		return 0;
	}
	for (iaccount = 0;iaccount < NumAccounts; iaccount++)
	{
		if (!stricmp (AccountID[iaccount],Account))
		{
			SetGlobalValue ("CAN",Account);
			return iaccount;
		}
	}
	iaccount = NumAccounts++;
	strcpy (AccountID[iaccount],Account);
	LoadAccountRoutes (iaccount);
	return iaccount;
}

BOOL GetFenceList (LPSTR Account,LPSTR Type)
{
/*$FENCE(GetList,Account,ALL or ACTIVE or INACTIVE);
$FENCE(GetList,Account,ALL or ACTIVE or INACTIVE);
returns list of fences in following format
>FENCE:name,type,status;
terminated with
>FenceListEnd;
 
Example
$FENCE(GetList,1234,ALL);
>FENCE:Fence1,Inclusive,Active;
>FENCE:Fence2,Exclusive,InActive;
>FenceListEnd;
 
 
$FENCE(Activate,Account,Name or ALL);
activates the specified fence or all fences
 
$FENCE(Deactivate,Account,Name or ALL);
deactivates the specified fence or all fences
 
$FENCE(Check,Account,lat,lon);
returns fence violations with following commands
>FenceViolation:Name,Type;
>FenceCheckEnd:;
 
Example
$FENCE(Check:1234,43.45672,-93.212344);
>FenceViolation:Fence1,Inclusive;    (point not in inclusive fence)
>FenceViolation:Fence2:Exclusive;    (point in exclusive fence)
>FenceCheckEnd:;
*/
	LPSTR	CmdMess = GlobalLock (hCmdMess);
	HFILE	Fid;
	char	Filelist[MAX_PATH]="[%DL]Fences\\[CAN]\\filelist.txt";
	char	str[260], ActInAct[10], IncExc[10], Units;
	LPSTR	pName, pEnd, pPar;
	double	Offset;
	BOOL	Critical;
	
	SetGlobalValue ("CAN",Account);
	Fid = GSSiOpenFile (Filelist,0,OF_READ);

	if (Fid != HFILE_ERROR)
	{
		while (fgetstring (str,255,Fid))
		{
			if ((pPar = strrchr (str,'(')))
			{
				*pPar++ = 0;
				if (*pPar++ == 'T')
					Critical = TRUE;
				else
					Critical = FALSE;
				Units = *pPar++;
				Offset = atof (pPar);
			}
			else
			{
				Offset = 0;
				Units = 'F';
				Critical = FALSE;
			}
			if ((pName = strrchr (str,'\\')))
			{
				pName++;
				if (!strnicmp (pName,"In_",3))
					strcpy (IncExc,"Inclusive");
				else
					strcpy (IncExc,"Exclusive");
				pName += 3;
				pEnd = strchr (pName,'.');
				*pEnd++ = 0;
				pEnd = strchr (pEnd,0);
				pEnd--;
				if (*pEnd == ' ')
					strcpy (ActInAct,"Active");
				else
					strcpy (ActInAct,"Inactive");
				if (stricmp (Type,"ALL"))
				{
					if (stricmp (Type,ActInAct))
						continue;
				}
			}
			sprintf (strchr(CmdMess,0),">Fence:%s,%s,%s,%s,%f,%c,%i;\r\n",Account,pName,IncExc,ActInAct,Offset,Units,Critical);
		}
		GSSiClose (Fid);
	}
	strcat (CmdMess,">FenceListEnd;\r\n");
	GlobalUnlock (hCmdMess);

	return TRUE;
}
BOOL GetFenceDefs (LPSTR Account)
{
	HFILE	Fid;
	int		lFile,err;
	char	FileName[MAX_PATH];
	char	str[260], ActInAct[10], IncExc[10], Units;
	LPSTR	pFile;

	sprintf (FileName,"[%%DL]fences\\%s.gfc",Account);
	lFile = GSSiLength (FileName);
	err = send (CurrentServerSocket,(LPSTR)&lFile,4,0); 
	if (lFile > 0)
	{
		HANDLE	hFile = GSSiGlobAlloc (0,GMEM_MOVEABLE,lFile);
		LPSTR	pFile = GlobalLock (hFile);

		Fid = GSSiOpenFile (FileName,0,OF_READ);
		BigRead (Fid,pFile,lFile);
		GSSiClose (Fid);
		CloseAllRequestedFiles(FALSE);  
		err = send (CurrentServerSocket,pFile,lFile,0); 
		GSSiGlobUlFree (&hFile);
	}

	return TRUE;
}

BOOL GetFenceLinks (LPSTR Account,LPSTR Type)
{
/*$FENCE(GetList,Account,ALL or ACTIVE or INACTIVE);
$FENCE(GetList,Account,ALL or ACTIVE or INACTIVE);
returns list of fences in following format
>FENCE:name,type,status;
terminated with
>FenceListEnd;
 
Example
$FENCE(GetList,1234,ALL);
>FENCE:Fence1,Inclusive,Active;
>FENCE:Fence2,Exclusive,InActive;
>FenceListEnd;
 
 
$FENCE(Activate,Account,Name or ALL);
activates the specified fence or all fences
 
$FENCE(Deactivate,Account,Name or ALL);
deactivates the specified fence or all fences
 
$FENCE(Check,Account,lat,lon);
returns fence violations with following commands
>FenceViolation:Name,Type;
>FenceCheckEnd:;
 
Example
$FENCE(Check:1234,43.45672,-93.212344);
>FenceViolation:Fence1,Inclusive;    (point not in inclusive fence)
>FenceViolation:Fence2:Exclusive;    (point in exclusive fence)
>FenceCheckEnd:;
*/
	LPSTR	CmdMess = GlobalLock (hCmdMess);
	HFILE	Fid;
	HANDLE	hList;
	char	Filelink[MAX_PATH]="[%DL]Fences\\[CAN]\\fencelinks.txt";
	LPSTR	pList, pListBeg;
	
	SetGlobalValue ("CAN",Account);
	Fid = GSSiOpenFile (Filelink,0,OF_READ);

	if (Fid != HFILE_ERROR)
	{
		int	len = GSSifilelength  (Fid);
		int	curlen = 0;

		hList = GSSiGlobAlloc (1690,GHND,len+2);
		pList = pListBeg = GlobalLock (hList);
		while (fgetstring (pList,len-curlen,Fid))
		{
			pList = strchr (pList,0);
			curlen = (int)pList - (int)pListBeg;
		}
		memcpy (CmdMess,pListBeg,curlen);
		GSSiGlobUlFree (&hList);
		GSSiClose (Fid);
	}
	strcat (CmdMess,">FenceLinkEnd;\r\n");
	GlobalUnlock (hCmdMess);

	return TRUE;
}

BOOL GetFenceStops (LPSTR Account,LPSTR Type)
{
 
	LPSTR	CmdMess = GlobalLock (hCmdMess);
	HFILE	Fid;
	HANDLE	hList;
	char	Filelink[MAX_PATH]="[%DL]Fences\\[CAN]\\routestops.txt";
	LPSTR	pList, pListBeg;
	
	SetGlobalValue ("CAN",Account);
	Fid = GSSiOpenFile (Filelink,0,OF_READ);

	if (Fid != HFILE_ERROR)
	{
		int	len = GSSifilelength  (Fid);
		int	curlen = 0;

		hList = GSSiGlobAlloc (1687,GHND,len+2);
		pList = pListBeg = GlobalLock (hList);
		while (fgetstring (pList,len-curlen,Fid))
		{
			pList = strchr (pList,0);
			curlen = (int)pList - (int)pListBeg;
		}
		memcpy (CmdMess,pListBeg,curlen);
		GSSiGlobUlFree (&hList);
		GSSiClose (Fid);
	}
	strcat (CmdMess,">RouteStopsEnd;\r\n");
	GlobalUnlock (hCmdMess);

	return TRUE;
}

BOOL ActivateFence (LPSTR Account,LPSTR Name,BOOL AorD)
{
	HFILE	Fid;
	char	Filelist[MAX_PATH]="[%DL]Fences\\[CAN]\\filelist.txt";
	char	aord[12];
	char	ActInact='D', name[128];
	LPSTR	pName, pLoc;
	int		len, loc;
	BOOL	rtn=FALSE;
	LPSTR	CmdMess = GlobalLock (hCmdMess);

	if (AorD)
		ActInact = ' ';
	SetGlobalValue ("CAN",Account);
	len = GSSiLength (Filelist);
	if (len>0)
	{
		HANDLE	handle = GSSiGlobAlloc (0,GMEM_MOVEABLE,len+1);
		LPSTR	pStr=GlobalLock (handle);

		Fid = GSSiOpenFile (Filelist,0,OF_READWRITE);
		BigRead (Fid,pStr,len);
		pStr[len] = 0;
		strlwr (pStr);
		if (stricmp (Name,"ALL"))
		{
			sprintf (name,"In_%s.plt",Name);
			strlwr (name);
			if (!(pLoc = strstr (pStr,name)))
			{
				sprintf (name,"Ex_%s.plt",Name);
				strlwr (name);
				pLoc = strstr (pStr,name);
			}
			if (pLoc)
			{
				int	l = strlen (name);
				
				pLoc += l;
				loc = (int)pLoc - (int)pStr;
				GSSillseek (Fid,loc,0);
				BigWrite (Fid,&ActInact,1,-1);
				rtn = TRUE;
			}
		}
		else
		{
			rtn = TRUE;
			pLoc = pStr;
			pLoc = strstr (pLoc,".plt");
			while (pLoc)
			{
				pLoc += 4;
				loc = (int)pLoc - (int)pStr;
				GSSillseek (Fid,loc,0);
				BigWrite (Fid,&ActInact,1,-1);
				pLoc = strstr (pLoc,".plt");
			}
		}
		GSSiClose (Fid);
		GSSiGlobUlFree (&handle);
	}
	if (AorD)
		strcpy (aord,"Activate");
	else
		strcpy (aord,"Deactivate");
	if (rtn)
		sprintf (strchr (CmdMess,0),">Fence%s:%s,%s,OK;\r\n",aord,Account,Name);
	else
		sprintf (strchr (CmdMess,0),">Fence%s:%s,%s,ERR;\r\n",aord,Account,Name);
	GlobalUnlock (hCmdMess);
	return rtn;
}

BOOL LoadAccountVehicleLinkList (int iaccount)
{
	BOOL	rtn=FALSE;

	LPSTR	pList, pListBeg, pLoc;
	char	Filelist[MAX_PATH]="[%DL]Fences\\[CAN]\\fencelinks.txt";
	HFILE	Fid=GSSiOpenFile (Filelist,0,OF_READ);

	GSSiGlobFree (&hVehicleLinkList[iaccount]);
	if (Fid != HFILE_ERROR)
	{
		int	len = GSSifilelength  (Fid);
		int	curlen = 0;

		hVehicleLinkList[iaccount] = GSSiGlobAlloc (1688,GHND,len+2);
		pList = pListBeg = GlobalLock (hVehicleLinkList[iaccount]);
		while (fgetstring (pList,len-curlen,Fid))
		{
			pList = strchr (pList,0);
			curlen = (int)pList - (int)pListBeg;
		}
		GlobalUnlock (hVehicleLinkList[iaccount]);
		GSSiClose (Fid);
		rtn = TRUE;
	}
	else
		hVehicleLinkList[iaccount] = GSSiGlobAlloc (1689,GHND,4);
	return rtn;
}

BOOL FenceIsRoute (int iaccount,LPSTR FenceName)
{
	BOOL	rtn=FALSE;
	int		iroute;
	LPFENCEHEADER	pFenceHeader;

	if (AccountFenceHandles[iaccount])
	{
		LPHANDLE phRoute=GlobalLock (AccountFenceHandles[iaccount]);

		for (iroute = 0;iroute < NumAccountRoutes[iaccount];iroute++)
		{
			pFenceHeader = (LPFENCEHEADER)GlobalLock (phRoute[iroute]);

			if (!stricmp (FenceName,pFenceHeader->Name))
				rtn = TRUE;
			GlobalUnlock (phRoute[iroute]);
		}
		GlobalUnlock (AccountFenceHandles[iaccount]);
	}
	return rtn;
}

BOOL VehicleLinkedToFence (int iaccount,LPSTR VehID,LPSTR FenceName)
{
	static	HANDLE	hList=0;
	BOOL	rtn=TRUE;
	LPSTR	pList, pListBeg, pLoc;
	char	fence[64]=">Fence:",vehicle[64];

	if (!VehID)
	{
		for (iaccount=0;iaccount < NumAccounts;iaccount++)
			GSSiGlobFree (&hVehicleLinkList[iaccount]);
		return TRUE;
	}
	if (!hVehicleLinkList[iaccount])
		LoadAccountVehicleLinkList (iaccount);
	hList = hVehicleLinkList[iaccount];
	pList = GlobalLock (hList);
	strcat (fence,FenceName);
	sprintf (vehicle,",%s,",VehID);
	if ((pLoc = strstr (pList,fence)))
	{
		rtn = FALSE;
		pLoc += strlen (fence);
		if (!strnicmp (pLoc,",ALL,",5))
			rtn = TRUE;
		else
		{
			LPSTR pEnd = strstr (pLoc,">Fence");

			if (pEnd)
			{
				*pEnd = 0;
				if (strstr (pLoc,vehicle))
					rtn = TRUE;
				*pEnd = '>';
			}
		}
	}
	GlobalUnlock (hList);

	return rtn;
}

BOOL RouteFenceViolation (int iaccount,LPSTR ID,LPSTR VehID,DPOINT Point,LPSTR CmdMess)
{
	LPFENCEHEADER	pFenceHeader;

	int		iroute;
	BOOL	rtn=FALSE;

	if (NumAccountRoutes[iaccount] < 0)
		LoadAccountRoutes (iaccount);
	if (AccountFenceHandles[iaccount])
	{
		LPHANDLE phRoute=GlobalLock (AccountFenceHandles[iaccount]);

		for (iroute = 0;iroute < NumAccountRoutes[iaccount];iroute++)
		{
			pFenceHeader = GlobalLock (phRoute[iroute]);
		
			if (VehicleLinkedToFence (iaccount,VehID,pFenceHeader->Name))
			{
				if (PointInFence (phRoute[iroute],&Point,0))
				{
				}
				else
				{
					sprintf (strchr (CmdMess,0),">FenceViolation:%s,%s,%s,Inclusive;\r\n",AccountID[iaccount],ID,pFenceHeader->Name);
					rtn = TRUE;
				}
			}
			GlobalUnlock (phRoute[iroute]);
		}
		GlobalUnlock (AccountFenceHandles[iaccount]);
	}
	return rtn;
}

BOOL CheckFence (LPSTR Account,LPSTR ID,LPSTR VehID,double x,double y)
{
	DPOINT	Point;
	int		n, npicked, len, iaccount;
	char	name[128], mess[256];
	LPSTR	pEnd, pName;
	LPSTR	CmdMess = GlobalLock (hCmdMess);
	HFILE	Fid;
	char	Filelist[MAX_PATH]="[%DL]Fences\\[CAN]\\filelist.txt";

	iaccount = GetAccountNum (Account);
//	sprintf (mess,"%s:%s:%s:%f %f",Account,ID,VehID,x,y);
//	AppendFile ("c:\\servlog.txt",mess);
	SetGlobalValue ("CAN",Account);
	Point.x = x;
	Point.y = y;
	
	SetViewport(*pCommandViewport);
	if (RouteFenceViolation (iaccount,ID,VehID,Point,CmdMess))
		goto Exit;
	LoadPickList ("[%DL]piklists\\fences.pik");

	npicked = n = PickItems (hWndMain,Point);
// check exclusive fences
	while (n--)
	{
		GetPickName (n);
		if ((pName = strrchr (PickName,'\\')))
		{
			pName++;
			if (!strnicmp (pName,"EX_",3))
			{
				pName += 3;
				strcpy (name,pName);
				if (VehicleLinkedToFence (iaccount,VehID,name))
				{
					if ((pEnd = strrchr (name,'.')))
						*pEnd = 0;
					//sprintf (strchr (CmdMess,0),">FenceViolation:%s,%s,Exclusive;\r\n",Account,name);
					sprintf (strchr (CmdMess,0),">FenceViolation:%s,%s,%s,Exclusive;\r\n",Account,ID,name);
					goto Exit;
				}
			}
		}
	}
// check inclusive fences
	len = GSSiLength (Filelist);
	if (len>0)
	{
		HANDLE	handle = GSSiGlobAlloc (0,GMEM_MOVEABLE,len+1);
		LPSTR	pStr=GlobalLock (handle);
		LPSTR	pLoc=pStr,pNamePicked;

		Fid = GSSiOpenFile (Filelist,0,OF_READ);
		BigRead (Fid,pStr,len);
		GSSiClose (Fid);
		pStr[len] = 0;
		strlwr (pStr);
		while (pLoc)
		{
			if ((pLoc = strstr (pLoc,"\\in_")))
			{
				BOOL	InArea = FALSE;
				
				if ((pEnd = strstr (pLoc,".plt")))
				{
					if (*(pEnd+4) == ' ')
					{
						*pEnd = 0;
						n = npicked;
						while (n--)
						{
							GetPickName (n);
							if ((pNamePicked = strrchr (PickName,'\\')))
							{
								LPSTR ppEnd = strrchr (pNamePicked,'.');
								
								if (ppEnd)
									*ppEnd = 0;
								if (!stricmp (pLoc,pNamePicked))
									InArea = TRUE;
							}
						}
						pLoc += 4;
						strcpy (name,pLoc);
						strupr (name);
						*pEnd = '.';
						if (!InArea && !FenceIsRoute (iaccount,name) && VehicleLinkedToFence (iaccount,VehID,name))
						{
							sprintf (strchr (CmdMess,0),">FenceViolation:%s,%s,%s,Inclusive;\r\n",Account,ID,name);
							GSSiGlobUlFree (&handle);
							goto Exit;
						}
					}
				}
				pLoc += 4;
			}
		}
		GSSiGlobUlFree (&handle);
	}
	
	sprintf (strchr (CmdMess,0),">FenceViolation:%s,%s;\r\n",Account,ID);
Exit:
//	AppendFile ("c:\\servlog.txt",CmdMess);
 	GlobalUnlock (hCmdMess);

	return TRUE;
}

int PointInFence (HANDLE handle,LPDPOINT pPoint,int LastDist)
{
	int	rtn=0,ii;
	LPFENCEHEADER	pFenceHeader;
	LPDPOINT	FencePoints;
	DPOINT		IntPoint;
	double		OffDist, PolyDist;

	if (!handle)
		return 0;
	pFenceHeader = GlobalLock (handle);

	switch (pFenceHeader->Type)
	{
	case FT_CIRCLE:
		{
			double d;

			if (pFenceHeader->OffsetUnits == 'F' || pFenceHeader->OffsetUnits == 'f')
				d = ConvertDist (pFenceHeader->Offset,1);
			else
				d = ConvertDist (pFenceHeader->Offset,2);
			if (ldistpp (&pFenceHeader->Point,pPoint) <= d)
				rtn = 1;
		}
		break;
	case FT_POLYGON:
		FencePoints = (LPDPOINT)(pFenceHeader+1);
		rtn = POINT_IN_AREAD (*pPoint,pFenceHeader->NumPoints, FencePoints,1,0,0,&pFenceHeader->hAccelerator);
		if (rtn)
			ii=1;
		break;
	case FT_ROUTE:
		FencePoints = (LPDPOINT)(pFenceHeader+1);//FencePoints[pFenceHeader->NumPoints-1]
		if (GetPerpendicularOffsetToPoly (pPoint,pFenceHeader->NumPoints, FencePoints,&IntPoint,&OffDist,&PolyDist,LastDist))
		{
			OffDist = GetBaseDist (pPoint, &IntPoint);
			if (OffDist <= pFenceHeader->BaseOffset)
				rtn = max (1,IDNINT(PolyDist));
		}
		if (!rtn)
			ii=1;
		break;
	}
	GlobalUnlock (handle);
	return rtn;
}

DPOINT PointOnFence (HANDLE handle,LPDOUBLE pdist)
{
	LPFENCEHEADER	pFenceHeader;
	LPDPOINT	FencePoints;
	DPOINT		Point={0,0};

	if (handle)
	{
		pFenceHeader = GlobalLock (handle);

		switch (pFenceHeader->Type)
		{
		case FT_CIRCLE:
			Point = dnewpt (pFenceHeader->Point, 0, *pdist);
			break;
		case FT_POLYGON:
		case FT_ROUTE:
			FencePoints = (LPDPOINT)(pFenceHeader+1);
			Point = PointAtDistOnPoly (FencePoints,pFenceHeader->NumPoints,*pdist,0,0);
			break;
		}
		GlobalUnlock (handle);
	}
	return Point;
}

int GetFenceCoord (LPSTR Account,LPSTR FenceName)
{
	BOOL rtn=FALSE;
	MNMXCORD	Bounds;
	HIGHLIGHTDATA	HighlightData;
	long	Refno, npnts=0;
	HANDLE	hPoly;
	int		Layer, nAreas, err=0;
	HPDPOINT	pPoly;

    ClearHighlightList (FALSE);  
	SetGlobalValue ("CAN",Account);
	SetGlobalValue ("FENCEFILE",FenceName);
	SetViewport(*pCommandViewport);
	Layer = GetLayerNumFromName ("PickFence");
    if (GetLayerBounds (&Bounds,CurView->hDC, Layer-1)) 
    {
		LoadPickList ("[%DL]piklists\\namedfence.pik");
		if (HighlightInArea (CurView->hWnd,&Bounds,TRUE,FALSE,0))
		{
   			BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData);  
			if (GetPolyPnts ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts,&hPoly,TRUE))
			{
				int	np=npnts;

				if (HighlightData.PD.Type == 2)
					np =-npnts;
				err = send (CurrentServerSocket,(LPSTR)&np,4,0); 
				pPoly = GlobalLock (hPoly);
				err = send (CurrentServerSocket,(LPSTR)pPoly,npnts*sizeof(DPOINT),0); 
				GSSiGlobUlFree (&hPoly);
			}
		}
	}
	if (!npnts)
		err = send (CurrentServerSocket,(LPSTR)&npnts,4,0); 
	
	return err;
}

BOOL TCPGetFile (SOCKET socket,LPSTR ToPath,LPSTR FromPath,BOOL Delete)
{
	int		lArea, la=0, st;
	int		n, len, err;
	char	txt[512];
	HFILE	Fid;
	long	Flen, totlen=0;

	sprintf (txt,"$TCPFILE(SEND,%s,%i)\r\n",FromPath,Delete);
	st = send (socket,txt,strlen(txt),0);
	n = 0;
	do
	{
		len = recv (socket,txt,4,MSG_PEEK);
		if (len == SOCKET_ERROR)
		{	
			err = WSAGetLastError();
			WSASetLastError (0);	
			if (err != WSAEWOULDBLOCK)
			{
				ProcessSocketError (CurrentServerSocket,err,"In TCPGetFile-1");
				return FALSE;
			}
		}
		Sleep (25);
		n++;
	} while (n < 100 && len < 4);
	if (n > 99)
		return FALSE;
	len = recv (socket,(LPSTR)&Flen,4,0);
	Fid = GSSiOpenFile (ToPath,0,OF_CREATE);
	while (totlen < Flen)
	{
		Sleep (10);
		len = recv (socket,txt,500,0);
		if (len > 0)
		{
			BigWrite (Fid,txt,len,-1);
			totlen += len;
		}
		else 
		{
			err = WSAGetLastError();
			Flen = 0;
			WSASetLastError (0);
		}
	}
	GSSiClose (Fid);
	return TRUE;
}

BOOL TCPSaveFile (LPSTR ToPath,int FileLength)
{
	HFILE	Fid;
	HANDLE	hFile;
	LPSTR	pFile, pFileb;
	int		lnread, n=0;
	int		totread=0;
	BOOL	rtn=TRUE;
	int		st, ierr;

	if (FileLength <= 0)
		return FALSE;
	Fid = GSSiOpenFile (ToPath,0,OF_CREATE);
	if (Fid == HFILE_ERROR)
	{
		st = send (CurrentServerSocket,"CC",2,0);
		rtn = FALSE;
		totread = -1;
	}
	else
	{
		st = send (CurrentServerSocket,"OK",2,0);
		hFile = GSSiGlobAlloc (1701,GMEM_MOVEABLE,FileLength);
		pFile = pFileb = GlobalLock (hFile);
		while (totread < FileLength)
		{
			lnread = recv (CurrentServerSocket,pFile,FileLength-totread,0);
			if (lnread == SOCKET_ERROR)
			{
				ierr = WSAGetLastError ();
				WSASetLastError (0);	
				if (ierr != WSAEWOULDBLOCK)
				{
					totread = -ierr;
					rtn = FALSE;
					break;
				}
				Sleep (25);
				n++;
				if (n > 100)
				{
					lnread = -WSAETIMEDOUT;
					break;
				}
			}
			else
			{
				n = 0;
				totread += lnread;
				pFile += lnread;
			}
		}
		if (rtn)
			BigWrite (Fid,pFileb,FileLength,-1);
		GSSiGlobUlFree (&hFile);
		GSSiClose (Fid);
	}
	st = send (CurrentServerSocket,(LPSTR)&totread,4,0);
	if (st == SOCKET_ERROR)
	{
		WSASetLastError (0);	
		rtn = FALSE;
	}
	return rtn;
}

BOOL TCPSendFile (LPSTR FromPath,int BlockSize,BOOL Delete)
{
	int		lArea, la=0, st;
	int		n, len, ln, lnsent, err;
	HANDLE	htxt = GSSiGlobAlloc (0,GMEM_MOVEABLE,BlockSize);
	LPSTR	txt = GlobalLock (htxt);
	HFILE	Fid;
	long	Flen, totlen=0;
	BOOL	rtn=TRUE;

	if (!htxt)
	{
		len = -1;
		st = send (CurrentServerSocket,(LPSTR)&len,4,0);
		return FALSE;
	}
	len = GSSiLength (FromPath);
	st = send (CurrentServerSocket,(LPSTR)&len,4,0);
	if (len>0)
	{
		Fid = GSSiOpenFile (FromPath,0,OF_READ);

		while (totlen < len)
		{
			Sleep (10);
			ln = BigRead (Fid,txt,BlockSize);
			lnsent = send (CurrentServerSocket,txt,ln,0);
			if (lnsent == SOCKET_ERROR)
			{
				rtn = FALSE;
				break;
			}
			totlen += lnsent;
			GSSillseek (Fid,totlen,0);//allows for lnsent to be less than ln
		}
		GSSiClose (Fid);
		if (Delete)
			GSSiRemove (FromPath);
	}
	GSSiGlobUlFree (&htxt);
	return rtn;
}

int UploadFence (LPSTR Account,LPSTR FileLength,LPSTR FileLengthCmp)
{
	HANDLE	hArea=0;
	long	nPoints;
	LPSTR	pArea;
	int		lArea, la=0, st;
	int		n, len, err, iaccount, TotLen=0, lFile,lFileCmp;
	HANDLE	hMem, hMemCmp;
	LPSTR	pFile, pFileCmp;
	char	txt[128];
	char	MapFile[MAX_PATH];
	char	FenceBackupFile[MAX_PATH];
	char	FenceFilelist[MAX_PATH];
	HFILE	Fid;

	if (!stricmp (FileLength,"*RENAME*"))
	{
		HaltMapDisplay(FALSE,TRUE); 
		CloseAllRequestedFiles (FALSE);  
		GetPolygonPickAccelerator (1);
//		sprintf (MapFile,"[%%DL]\\fences\\%s_new",Account);
		sprintf (FenceFilelist,"[%%DL]fences\\%s",Account);
//		ExpandText (MapFile);
		sprintf (FenceBackupFile,"[%%DL]fences\\%s.gfb",Account);
		if (GSSiLength (FenceBackupFile) > 0)
		{
			ExpandText (FenceFilelist);
			DeleteDirAndContents (FenceFilelist);
			makedirectories (FenceFilelist,TRUE,FALSE);
			sprintf (FenceBackupFile,"[%%DL]fences\\%s.gfb",Account);
			RecallFencesFromFile (FenceBackupFile,Account);
	//		GSSirenamefile (MapFile,FenceFilelist);
			iaccount = GetAccountNum (Account);
			GSSiGlobFree (&AccountFenceHandles[iaccount]);
			NumAccountRoutes[iaccount] = -1;
			LoadAccountRoutes (iaccount);
			st = send (CurrentServerSocket,"OK",2,0);
			return TRUE;
		}
		else
			st = send (CurrentServerSocket,"ERR",2,0);
		return FALSE;
	}
	
	lFile = atoi (FileLength);
	lFileCmp = atoi (FileLengthCmp);
	st = send (CurrentServerSocket,"OK1",4,0);
	hMemCmp = GSSiGlobAlloc (0,GMEM_MOVEABLE,lFileCmp+4096);
	pFileCmp = GlobalLock (hMemCmp);
	n = 0;
	do
	{
		Sleep (25);
		len = recv (CurrentServerSocket,pFileCmp+TotLen,lFileCmp,0);
		if (len == SOCKET_ERROR)
		{	
			err = WSAGetLastError();
			WSASetLastError (0);	
			if (err != WSAEWOULDBLOCK)
			{
				ProcessSocketError (CurrentServerSocket,err,"In UploadFence-1");
				GSSiGlobUlFree (&hMemCmp);
				return FALSE;
			}
		}
		else 
			TotLen += len;
		n++;
	} while (n < 200 && TotLen < lFileCmp);
	if (n < 200)
	{
		hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,lFile+4096*4);
		pFile = GlobalLock (hMem);
		len = DecompressBinaryRecordUnsafe (pFile,pFileCmp,lFileCmp);
		sprintf (FenceBackupFile,"[%%DL]fences\\%s.gfc",Account);
		Fid = GSSiOpenFile (FenceBackupFile,0,OF_CREATE);
		BigWrite (Fid,pFileCmp,lFileCmp,-1);
		GSSiClose (Fid);
		if (len == lFile)
		{
			sprintf (FenceBackupFile,"[%%DL]fences\\%s.gfb",Account);
			Fid = GSSiOpenFile (FenceBackupFile,0,OF_CREATE);
			BigWrite (Fid,pFile,len,-1);
			GSSiClose (Fid);
			st = send (CurrentServerSocket,"OK2",4,0);
		}
		else
			st = send (CurrentServerSocket,"ERR",4,0);

		GSSiGlobUlFree (&hMem);
		GSSiGlobUlFree (&hMemCmp);
		st = send (CurrentServerSocket,"OK2",4,0);
	}
	else
		st = send (CurrentServerSocket,"ERR",4,0);
	/*
	len = recv (CurrentServerSocket,(LPSTR)&nPoints,4,0);
	lArea = nPoints * sizeof(DPOINT);
	hArea = GSSiGlobAlloc (0,GMEM_MOVEABLE,lArea);
	pArea = GlobalLock (hArea);
	n = 0;
	Sleep (25);
	do
	{
		n++;
		len = recv (CurrentServerSocket,&pArea[la],lArea-la,0);
		if (len > 0)
		{
			la += len;
		}
		else
		{
			err = WSAGetLastError();
			WSASetLastError (0);	
			if (err != WSAEWOULDBLOCK)
			{
				GSSiGlobUlFree (&hArea);
				ProcessSocketError (CurrentServerSocket,err,"In UploadFence-2");
				return FALSE;
			}
			Sleep (250);
		}
	} while (n < 100 && la < lArea);
	if (n >= 100)
	{
		GSSiGlobUlFree (&hArea);
		return FALSE;
	}
	else
	{
		MNMXCORD	Bounds;
		HANDLE	hSymDesc=0;
		short	NumSyms=0, SymNum; 
		int		np=nPoints;
		char	Prefix[10]="GEOFENCE";
		long	NewRefno;
		int		ptype;
	
		st = send (CurrentServerSocket,"OK",2,0);
		ptype = 0;
		if (FenceType == 2)
		{
			ptype = 1;
			SymNum = GetDictSymbolNumber ("GEOFENCE_ROUTE");
		}
		else if (!strnicmp (FenceName,"Ex_",3))
			SymNum = GetDictSymbolNumber ("GEOFENCE_INCLUSIVE");
		else
			SymNum = GetDictSymbolNumber ("GEOFENCE_EXCLUSIVE");
		GetPolyBounds (hArea,nPoints,&Bounds,TYPE_AREA);
		sprintf (MapFile,"[%%DL]\\fences\\%s_new\\%s.plt",Account,FenceName);
		sprintf (FenceFilelist,"[%%DL]\\fences\\%s_new\\filelist.txt",Account);
		strcpy (PltName,MapFile);
		CreateNewMap (PltName,&Bounds,0,0,0,0,0,0,FALSE);
		AddToSymList (SymNum,&NumSyms,&hSymDesc); 
		EditBounds = Bounds;  
		NewRefno = GetNewRefno (MapFile,0,0,0,0);
		AddPolyToMap (1,&np, &hArea,ptype,NewRefno++,0,2,SymNum,0,Prefix,FenceName,-1,-1,-1,0,0,0,0,TRUE,0);
		CloseMap(TRUE);  
		AddSymToMap (NumSyms,hSymDesc,0,0); 
    	DestroySymList (&NumSyms,&hSymDesc); 
		GSSiGlobUlFree (&hArea);
		if (First)
			GSSiRemove (FenceFilelist);
		sprintf (MapFile,"[%%DL]\\fences\\%s\\%s.plt (%i%c%f)",Account,FenceName,Critical,*Units,FenceOffset);
		strcat (MapFile," ");
		AppendFile (FenceFilelist,MapFile);
	}*/
	HaltMapDisplay(FALSE,TRUE); 
	CloseAllRequestedFiles (FALSE);  
	return TRUE;
}

int UploadFenceLinks (LPSTR Account)
{
	HANDLE	hBuffer=0;
	long	lBuffer;
	LPSTR	pBuffer;
	int		lb=0, st;
	int		n, len, err;
	char	txt[128];
	char	FenceLinkList[MAX_PATH];

	st = send (CurrentServerSocket,"OK",2,0);
	n = 0;
	Sleep (25);
	do
	{
		len = recv (CurrentServerSocket,txt,4,MSG_PEEK);
		if (len == SOCKET_ERROR)
		{	
			err = WSAGetLastError();
			WSASetLastError (0);	
			if (err != WSAEWOULDBLOCK)
			{
				ProcessSocketError (CurrentServerSocket,err,"In UploadFence-1");
				return FALSE;
			}
		}
		Sleep (250);
		n++;
	} while (n < 100 && len < 4);
	len = recv (CurrentServerSocket,(LPSTR)&lBuffer,4,0);
	hBuffer = GSSiGlobAlloc (0,GMEM_MOVEABLE,lBuffer);
	pBuffer = GlobalLock (hBuffer);
	n = 0;
	Sleep (25);
	do
	{
		n++;
		len = recv (CurrentServerSocket,&pBuffer[lb],lBuffer-lb,0);
		if (len > 0)
		{
			lb += len;
		}
		else
		{
			err = WSAGetLastError();
			WSASetLastError (0);	
			if (err != WSAEWOULDBLOCK)
			{
				GSSiGlobUlFree (&hBuffer);
				ProcessSocketError (CurrentServerSocket,err,"In UploadFenceLinks-2");
				return FALSE;
			}
			Sleep (250);
		}
	} while (n < 100 && lb < lBuffer);
	if (n >= 100)
	{
		GSSiGlobUlFree (&hBuffer);
		return FALSE;
	}
	else
	{
		HFILE	Fid;
		int		iaccount;

		GlobalUnlock (hBuffer);
		st = send (CurrentServerSocket,"OK",2,0);
		iaccount = GetAccountNum (Account);
		GSSiGlobFree (&hVehicleLinkList[iaccount]);
		sprintf (FenceLinkList,"[%%DL]\\fences\\%s\\fencelinks.txt",Account);
		Fid = GSSiOpenFile (FenceLinkList,0,OF_CREATE);
		if (Fid != HFILE_ERROR)
		{
			pBuffer = GlobalLock (hBuffer);
			BigWrite (Fid,pBuffer,lBuffer,-1);
			GSSiClose (Fid);
			GlobalUnlock (hBuffer);
		}
		GSSiGlobFree (&hBuffer);
	}
	CloseAllRequestedFiles (FALSE);
	return TRUE;
}
int UploadRouteStops (LPSTR Account)
{
	HANDLE	hBuffer=0;
	long	lBuffer;
	LPSTR	pBuffer;
	int		lb=0, st;
	int		n, len, err;
	char	txt[128];
	char	FenceLinkList[MAX_PATH];

	st = send (CurrentServerSocket,"OK",2,0);
	n = 0;
	Sleep (25);
	do
	{
		len = recv (CurrentServerSocket,txt,4,MSG_PEEK);
		if (len == SOCKET_ERROR)
		{	
			err = WSAGetLastError();
			WSASetLastError (0);	
			if (err != WSAEWOULDBLOCK)
			{
				ProcessSocketError (CurrentServerSocket,err,"In UploadFence-1");
				return FALSE;
			}
		}
		Sleep (250);
		n++;
	} while (n < 100 && len < 4);
	len = recv (CurrentServerSocket,(LPSTR)&lBuffer,4,0);
	hBuffer = GSSiGlobAlloc (0,GMEM_MOVEABLE,lBuffer);
	pBuffer = GlobalLock (hBuffer);
	n = 0;
	Sleep (25);
	do
	{
		n++;
		len = recv (CurrentServerSocket,&pBuffer[lb],lBuffer-lb,0);
		if (len > 0)
		{
			lb += len;
		}
		else
		{
			err = WSAGetLastError();
			WSASetLastError (0);	
			if (err != WSAEWOULDBLOCK)
			{
				GSSiGlobUlFree (&hBuffer);
				ProcessSocketError (CurrentServerSocket,err,"In UploadFenceLinks-2");
				return FALSE;
			}
			Sleep (250);
		}
	} while (n < 100 && lb < lBuffer);
	if (n >= 100)
	{
		GSSiGlobUlFree (&hBuffer);
		return FALSE;
	}
	else
	{
		HFILE	Fid;
		int		iaccount;

		GlobalUnlock (hBuffer);
		st = send (CurrentServerSocket,"OK",2,0);
		iaccount = GetAccountNum (Account);
		GSSiGlobFree (&hVehicleLinkList[iaccount]);
		sprintf (FenceLinkList,"[%%DL]\\fences\\%s\\routestops.txt",Account);
		Fid = GSSiOpenFile (FenceLinkList,0,OF_CREATE);
		if (Fid != HFILE_ERROR)
		{
			pBuffer = GlobalLock (hBuffer);
			BigWrite (Fid,pBuffer,lBuffer,-1);
			GSSiClose (Fid);
			GlobalUnlock (hBuffer);
		}
		GSSiGlobFree (&hBuffer);
	}
	CloseAllRequestedFiles (FALSE);
	return TRUE;
}
BOOL RestoreFences (LPSTR CAN,LPSTR File)
{
	BOOL rtn;
	int	iaccount;

	HaltMapDisplay(FALSE,TRUE); 
	CloseAllRequestedFiles (FALSE);
	rtn = RecallFencesFromFile (File,CAN);
	iaccount = GetAccountNum (CAN);
	GSSiGlobFree (&AccountFenceHandles[iaccount]);
	NumAccountRoutes[iaccount] = -1;
	LoadAccountRoutes (iaccount);

	return rtn;
}

