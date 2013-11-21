#include "graphint.h"  


#include "gmextern.h"


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
	char	str[260], ActInAct[10], IncExc[10];
	LPSTR	pName, pEnd;
	
	SetGlobalValue ("CAN",Account);
	Fid = GSSiOpenFile (Filelist,0,OF_READ);

	if (Fid != HFILE_ERROR)
	{
		while (fgetstring (str,255,Fid))
		{
			if ((pName = strrchr (str,'\\')))
			{
				pName++;
				if (!strnicmp (pName,"In_",3))
					strcpy (IncExc,"Inclusive");
				else
					strcpy (IncExc,"Exclusive");
				pName += 3;
				pEnd = strrchr (pName,'.');
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
			sprintf (strchr(CmdMess,0),">Fence:%s,%s,%s,%s;\r\n",Account,pName,IncExc,ActInAct);
		}
		GSSiClose (Fid);
	}
	strcat (CmdMess,">FenceListEnd;\r\n");
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

	*CmdMess = 0;
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

BOOL CheckFence (LPSTR Account,LPSTR ID,double x,double y)
{
	DPOINT	Point;
	int		n, npicked, len;
	char	name[128];
	LPSTR	pEnd, pName;
	LPSTR	CmdMess = GlobalLock (hCmdMess);
	HFILE	Fid;
	char	Filelist[MAX_PATH]="[%DL]Fences\\[CAN]\\filelist.txt";

	*CmdMess = 0;
	SetGlobalValue ("CAN",Account);
	Point.x = x;
	Point.y = y;
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
				if ((pEnd = strrchr (name,'.')))
					*pEnd = 0;
				//sprintf (strchr (CmdMess,0),">FenceViolation:%s,%s,Exclusive;\r\n",Account,name);
				sprintf (CmdMess,">FenceViolation:%s,%s,%s,Exclusive;\r\n",Account,ID,name);
				goto Exit;
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
						*pEnd = '.';
						if (!InArea)
						{
							sprintf (CmdMess,">FenceViolation:%s,%s,%s,Inclusive;\r\n",Account,ID,name);
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
	
	sprintf (CmdMess,">FenceViolation:%s,%s;\r\n",Account,ID);
Exit:
 	GlobalUnlock (hCmdMess);

	return TRUE;
}

int PointInFence (HANDLE handle,LPDPOINT pPoint)
{
	int	rtn=0;
	LPFENCEHEADER	pFenceHeader;
	LPDPOINT	FencePoints;

	if (!handle)
		return 0;
	pFenceHeader = GlobalLock (handle);

	switch (pFenceHeader->Type)
	{
	case FT_CIRCLE:
		if (ldistpp (&pFenceHeader->Point,pPoint) <= pFenceHeader->Offset)
			rtn = 1;
		break;
	case FT_POLYGON:
		FencePoints = (LPDPOINT)(pFenceHeader+1);
		rtn = POINT_IN_AREAD (*pPoint,pFenceHeader->NumPoints, FencePoints,0,0);
		break;
	}
	GlobalUnlock (handle);
	return rtn;
}

int GetFenceCoord (LPSTR Account,LPSTR FenceName)
{
	BOOL rtn=FALSE;
	MNMXCORD	Bounds;
	HIGHLIGHTDATA	HighlightData;
	long	Refno, npnts=0;
	HANDLE	hPoly;
	int		Layer, nAreas, err;
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
				err = send (CurrentServerSocket,(LPSTR)&npnts,4,0); 
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

int UploadFence (LPSTR Account,LPSTR FenceName,BOOL First)
{
	HANDLE	hArea=0;
	long	nPoints;
	LPSTR	pArea;
	int		lArea, la=0;
	int		n, len, err;
	char	txt[128];

	send (CurrentServerSocket,"OK",2,0);
	n = 0;
	do
	{
		len = recv (CurrentServerSocket,txt,4,MSG_PEEK);
		if (len == SOCKET_ERROR)
		{	
			err = WSAGetLastError();
			WSASetLastError (0);	
			if (err != WSAEWOULDBLOCK)
			{
				ProcessSocketError (CurrentServerSocket,err);
				return FALSE;
			}
		}
		Sleep (25);
		n++;
	} while (n < 100 && len < 4);
	len = recv (CurrentServerSocket,(LPSTR)&nPoints,4,0);
	lArea = nPoints * sizeof(DPOINT);
	hArea = GSSiGlobAlloc (0,GMEM_MOVEABLE,lArea);
	pArea = GlobalLock (hArea);
	n = 0;
	do
	{
			Sleep (25);
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
					ProcessSocketError (CurrentServerSocket,err);
					return FALSE;
				}
			}
	} while (n < 100 && la < lArea);
	if (n >= 100)
	{
		GSSiGlobUlFree (&hArea);
		return FALSE;
	}
	else
	{
		char	MapFile[MAX_PATH];
		char	FenceFilelist[MAX_PATH];
		MNMXCORD	Bounds;
		HANDLE	hSymDesc=0;
		short	NumSyms=0, SymNum; 
		USHORT	np=nPoints;
		char	Prefix[10]="GEOFENCE";
		char	AreaSymbolName[20]="GEOFENCE_INCLUSIVE";
		long	NewRefno;
	
		SymNum = GetDictSymbolNumber (AreaSymbolName);
		ConvertPolyCoord ((HPDPOINT)pArea,nPoints,2,1);
		GetPolyBounds2 ((HPDPOINT)pArea,nPoints,&Bounds);
		sprintf (MapFile,"[%%DL]\\fences\\%s\\%s.plt",Account,FenceName);
		sprintf (FenceFilelist,"[%%DL]\\fences\\%s\\filelist.txt",Account);
		strcpy (PltName,MapFile);
		CreateNewMap (PltName,&Bounds,0,0,0,0,0,0,FALSE);
		AddToSymList (SymNum,&NumSyms,&hSymDesc); 
		EditBounds = Bounds;  
		NewRefno = GetNewRefno (MapFile,0,0,0,0);
		AddPolyToMap (1,&np, &hArea,0,NewRefno++,0,2,SymNum,0,Prefix,FenceName,-1,-1,-1,0,0,0,0,TRUE);
		CloseMap(TRUE);  
		AddSymToMap (NumSyms,hSymDesc,0,0); 
    	DestroySymList (&NumSyms,&hSymDesc); 
		GSSiGlobUlFree (&hArea);
		AppendFile (FenceFilelist,MapFile);
	}
	return TRUE;
}

