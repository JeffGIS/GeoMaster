#include "graphint.h"  
#include "umio.h"

#include "gmextern.h"

static	time_t	AVLAdjustTime=0;
static	HANDLE	hDummyVehicles=0;
static	short	nDummyVehicles=0;
static	short	StatusSymbol[8], NotHeardFromSymbol, StoppedSymbol, MovingSymbol,DirectionSymbol;
static	HANDLE	hVehSaveScreen[MAX_VIEWPORTS];
static	long	VehSaveScreenID[MAX_VIEWPORTS];  
static	BOOL	FirstVeh=TRUE;
static	double	LastSize=0;
static	HANDLE	Fences[MAX_FENCES];
static	MNMXCORD	PMBounds={0,0,-1,-1};
static	WNDPROC	g_OldEdit=0;
static	int		VehStatHeight, VehStatWidth;
static	HANDLE	hLastBox=0;
static	HDC		hDCLastBox;
static	int		LastVeh=-1;	
static	int		maxinrow=5;
static	int		VehInfoHeight=132;
static	LPVIEWPORT	VehStatusVP;
static	int		NumFencesInList=0, NumFences=0;
static	char	FenceName[MAX_FENCES][64];
static	short	FenceInclusive[MAX_FENCES];
static	short	FenceMethod[MAX_FENCES];
static	BOOL	FenceActive[MAX_FENCES];
static	double	FenceOffset[MAX_FENCES];
static	char	FenceOffsetUnits[MAX_FENCES];
static	BOOL	FenceCritical[MAX_FENCES];
static	DPOINT	FencePoint[MAX_FENCES];
static	RECT	NullRect={0,0,0,0};
static	HWND	hWndGeoFence = 0;
static	char	HistoryReplayFile[MAX_PATH]="";
static	int		nInReplayList=0;
static	char	VehicleReplayList[MAXVEHICLES][32];

BOOL VehicleInReplayList (LPSTR ID)
{
	int	i;

	if (!ID)
	{
		VehReplayFid = HFILE_ERROR;
		return TRUE;
	}
	if (VehReplayFid == HFILE_ERROR)
		return FALSE;
	for (i=0;i<nInReplayList;i++)
	{
		if (!stricmp (ID,VehicleReplayList[i]))
			return TRUE;
	}
	return FALSE;
}

int  GetFenceID (LPSTR pName)
{
	int	i;

	for (i=0;i<NumFencesInList;i++)
		if (!stricmp (pName,FenceName[i]))
			return i;
	return -1;
}

BOOL GetFenceListFromDB (LPSTR CAN)
{
	HANDLE	hDB=0, Handle;
	char	str[512],type[64], TAG[100];
	BOOL	err;
	char	TempName[MAX_PATH], GFD[32]="[GEOFENCEDISPLAY]";
	char	MapFile[MAX_PATH+64], FileList[MAX_PATH], FenceDB[MAX_PATH];
	char	FenceTypeAndName[80], InEx[8];
	HFILE	FidTemp, Fid;
	int		i;
	char CurDir[MAX_PATH]="[%GEOFENCEDIR]";

	ExpandText (CurDir);
	if (!*CurDir)
	{
		strcpy (CurDir,"[%DL]fences\\");
		strcat (CurDir,CAN);
		ExpandText (CurDir);
	}

	DestroyFence (0);	
	NumFencesInList = 0;
	GSSiGetTempFileName(0,"gm",0,TempName);
	FidTemp = GSSiOpenFile (TempName,0,OF_CREATE);
	ExpandText (GFD);
	SetGlobalValue ("GEOFENCEDISPLAY","");
	sprintf (FenceDB,"FLIST=%s\\fencelist.gmd",CurDir);
	if (OpenDataFile (FenceDB,"",BT_READ,&hDB))
	while (FetchDBRec (hDB))
	{
		strcpy (FenceName[NumFencesInList],"[FLIST.NAME]");
		ExpandText (FenceName[NumFencesInList]);
		strcpy (str,"[FLIST.INCLUSIVE]");
		ExpandText (str);
		FenceInclusive[NumFencesInList] = atoi (str);
		strcpy (str,"[FLIST.X] [FLIST.Y]");
		ExpandText (str);
		FencePoint[NumFencesInList] = CurrentPoint = atopt (str,&err);
		strcpy (str,"[FLIST.OFFSET]");
		ExpandText (str);
		FenceOffset[NumFencesInList] = atof (str);
		strcpy (str,"[FLIST.OFFSETUNITS]");
		ExpandText (str);
		if (!*str)
			*str = 'F';
		FenceOffsetUnits[NumFencesInList] = *str;
		strcpy (str,"[FLIST.CRITICAL]");
		ExpandText (str);
		FenceCritical[NumFencesInList] = atob (str);
		strcpy (str,"[FLIST.ACTIVE]");
		ExpandText (str);
		FenceActive[NumFencesInList] = atob (str);
		strcpy (type,"[FLIST.TYPE]");
		ExpandText (type);
		if (!stricmp (type,"POLYGON"))
			FenceMethod[NumFencesInList] = 0;
		else if (!stricmp (type,"CIRCLE"))
			FenceMethod[NumFencesInList] = 1;
		else
			FenceMethod[NumFencesInList] = 2;
		sprintf (TAG,"GEOFENCE:%s",FenceName[NumFencesInList]);
//		$FENCE(LOAD,[%GEOFENCETYPE],GEOFENCE:[%GEOFENCENAME],[%GEOFENCEOFFSET],[%WX] [%WY])
		if (FenceInclusive[NumFencesInList])
			strcpy (InEx,"In");
		else
			strcpy (InEx,"Ex");
		sprintf (FenceTypeAndName,"%s_%s",InEx,FenceName[NumFencesInList]);
		SetGlobalValue ("%NEWGEOFENCE",FenceTypeAndName);
		Handle = LoadFence (type,TAG,&FencePoint[NumFencesInList],FenceOffset[NumFencesInList],FenceOffsetUnits[NumFencesInList]);
		sprintf (str,"$GMDUPDATE([%%GEOFENCEDIR]\\fencelist.gmd,NAME=%s,HANDLE=%i)",FenceName[NumFencesInList],(int)Handle);
		fputstring (str,FidTemp);
		NumFencesInList++;
	}
	SetGlobalValue ("%NEWGEOFENCE","");
	CloseDataFile (TRUE, &hDB); 
	GSSillseek (FidTemp,0,0);
	while (fgetstring (str,256,FidTemp))
	{
		ExpandText (str);
	}
	GSSiClose (FidTemp);
	GSSiRemove (TempName);
	{
		strcpy (FileList,CurDir);
		strcat (FileList,"\\filelist.txt");
		ExpandText (FileList);
		Fid = GSSiOpenFile (FileList,0,OF_CREATE);
		for (i=0;i<NumFencesInList;i++)
		{
			if (FenceInclusive[i])
				sprintf (MapFile,"%s\\In_%s.plt (%i%c%f)",CurDir,FenceName[i],FenceCritical[i],FenceOffsetUnits[i],FenceOffset[i]);
			else
				sprintf (MapFile,"%s\\Ex_%s.plt (%i%c%f)",CurDir,FenceName[i],FenceCritical[i],FenceOffsetUnits[i],FenceOffset[i]);
			strcat (MapFile," ");
			fputstring (MapFile,Fid);
		}
		GSSiClose (Fid);
		OrderFenceFilelist (FileList);
	}
	SetGlobalValue ("GEOFENCEDISPLAY",GFD);
	return TRUE;
}

BOOL RecallFencesFromFile (LPSTR Pathname,LPSTR CAN)
{
    char	str[MAX_PATH+2],str2[MAX_PATH+2],TempName[MAX_PATH],CurDir[MAX_PATH]="[%GEOFENCEDIR]",SaveName[MAX_PATH];
    HFILE	Fid, Fid2, FidIn; 
    int		TotFiles=0, lCurDir, FileLen, NameLen; 
	HANDLE	hFile, hDB=0;
	LPSTR	pFile, pName;
	LPSTR	pDot = strrchr (Pathname,'.');

	if (!pDot)
		strcat (Pathname,".gfb");

	FidIn = GSSiOpenFile (Pathname,0,OF_READ);
	if (FidIn == HFILE_ERROR)
		return FALSE;
	ExpandText (CurDir);
	if (!*CurDir)
	{
		sprintf (CurDir,"[%%DL]fences\\%s",CAN);
		ExpandText (CurDir);
	}
	lCurDir = strlen (CurDir);
	GSSiGetTempFileName(0,"gm",0,TempName);
	Fid =	GSSiOpenFile (TempName,0,OF_CREATE);
	SearchFilesInDir (CurDir,"",Fid,&TotFiles,"*.*",1,TRUE,TRUE); 
	GSSillseek (Fid,0,0);
	while (fgetstring (str,MAX_PATH,Fid))
	{   
		GSSiRemove (str);
	}
	GSSiClose (Fid); 
	GSSiRemove (TempName);
	while (BigRead (FidIn,&NameLen,4)==4)
	{
		BigRead (FidIn,SaveName,NameLen);
		BigRead (FidIn,&FileLen,4);
		hFile = GSSiGlobAlloc (0,GMEM_MOVEABLE,FileLen);
		pFile = GlobalLock (hFile);
		BigRead (FidIn,pFile,FileLen);
		pName = strrchr (SaveName,'\\');
		sprintf (str,"%s%s",CurDir,pName);
		ExpandText (str);
		Fid2 = GSSiOpenFile (str,0,OF_CREATE);
		BigWrite (Fid2,pFile,FileLen,-1);
		GSSiClose (Fid2);
		GSSiGlobUlFree (&hFile);
		if (!stricmp (&str[max(0,strlen(str)-4)],".plt"))
		{
			if (!IS_BASE[2])
				ConvertFileCoordinates (str,2,1,2);   
		}
	}
	GSSiClose (FidIn);
	sprintf (TempName,"%s%s",CurDir,"\\filelist.txt");
	ExpandText (TempName);
	Fid2 = GSSiOpenFile (TempName,0,OF_READ);
	if (Fid2 != HFILE_ERROR)
	{
		char	TempFilelistName[MAX_PATH];

		sprintf (TempFilelistName,"%s%s",CurDir,"\\filelist.tmp");
		ExpandText (TempFilelistName);
		Fid = GSSiOpenFile (TempFilelistName,0,OF_CREATE);
		while (fgetstring (str,254,Fid2))
		{
			LPSTR	pBS = strrchr (str,'\\');

			if (pBS)
			{
				sprintf (str2,"%s%s",CurDir,pBS);
				ExpandText (str2);
				fputstring (str2,Fid);
			}
		}
		GSSiClose (Fid);
		GSSiClose (Fid2);
		GSSiRemove (TempName);
		GSSiRename (TempFilelistName,TempName);
	}
	GetFenceListFromDB (CAN);
	return TRUE;
}

BOOL BackupFences (LPSTR CAN,LPSTR File)
{
	BOOL rtn;

	HaltMapDisplay(FALSE); 
	CloseAllRequestedFiles (FALSE);
	rtn = SaveFencesToFile (File,CAN);
	return rtn;
}


BOOL SaveFencesToFile (LPSTR Pathname,LPSTR CAN)
{
    char	str[MAX_PATH+2],TempName[MAX_PATH],TempPlt[MAX_PATH],CurDir[MAX_PATH]="[%GEOFENCEDIR]",SaveName[MAX_PATH];
    HFILE	Fid, Fid2, Fid3, FidOut; 
    int		TotFiles=0, lCurDir, FileLen, NameLen; 
	HANDLE	hFile;
	LPSTR	pDot = strrchr (Pathname,'.');
	char	FenceLinkFile[MAX_PATH], RouteStopsFile[MAX_PATH];

	if (!pDot)
		strcat (Pathname,".gfb");
	FidOut = GSSiOpenFile (Pathname,0,OF_CREATE);
	if (FidOut == HFILE_ERROR)
		return FALSE;
	ExpandText (CurDir);
	if (!*CurDir)
	{
		strcpy (CurDir,"[%DL]fences\\[CAN]");
		ExpandText (CurDir);
	}
	lCurDir = strlen (CurDir);
	sprintf (FenceLinkFile,"%s\\fencelinks.txt",CurDir);
	sprintf (RouteStopsFile,"%s\\routestops.txt",CurDir);
	CreateFencesLinksFromDB (FenceLinkFile);
	CreateRouteStopsFromDB (RouteStopsFile);
	GSSiGetTempFileName(0,"gm",0,TempName);
	Fid =	GSSiOpenFile (TempName,0,OF_CREATE);
	SearchFilesInDir (CurDir,"",Fid,&TotFiles,"*.*",1,TRUE,TRUE); 
	GSSillseek (Fid,0,0);
	while (fgetstring (str,MAX_PATH,Fid))
	{   
		LPSTR	pFile=str;
		int		lstr=strlen (str);

		if (!stricmp (&str[max(0,lstr-4)],".rin")) 
			continue;
		if (!_fstrnicmp (str,CurDir,lCurDir))
			pFile = &str[lCurDir+1];
		sprintf (SaveName,"%s\\%s",CAN,pFile);
		NameLen = strlen (SaveName)+1;
		Fid2 = GSSiOpenFile (str,0,OF_READ);
		FileLen = GSSifilelength (Fid2);
		if (FileLen < 1)
			MessageBox (0,str,0,MB_ICONEXCLAMATION);

		hFile = GSSiGlobAlloc (0,GMEM_MOVEABLE,FileLen);
		pFile = GlobalLock (hFile);
		BigRead (Fid2,pFile,FileLen);
		GSSiClose (Fid2);
		if (!stricmp (&str[max(0,lstr-4)],".plt")) 
		{
			GSSiGetTempFileName(0,"gm",0,TempPlt);
			Fid3 = GSSiOpenFile (TempPlt,0,OF_CREATE);
			BigWrite (Fid3,pFile,FileLen,-1);
			GSSiClose (Fid3);
			ConvertFileCoordinates (TempPlt,1,2,2);    
			Fid2 = GSSiOpenFile (TempPlt,0,OF_READ);
			FileLen = GSSifilelength (Fid2);
			GSSiGlobUlFree (&hFile);
			hFile = GSSiGlobAlloc (0,GMEM_MOVEABLE,FileLen);
			pFile = GlobalLock (hFile);
			BigRead (Fid2,pFile,FileLen);
			GSSiClose (Fid2);
			GSSiRemove (TempPlt);
		}
		BigWrite (FidOut,&NameLen,4,-1);
		BigWrite (FidOut,SaveName,NameLen,-1);
		BigWrite (FidOut,&FileLen,4,-1);
		BigWrite (FidOut,pFile,FileLen,-1);
		GSSiGlobUlFree (&hFile);
	}
	GSSiClose (Fid); 
	GSSiClose (FidOut);
	GSSiRemove (TempName);
	return TRUE;
}

void ClearVehicleMenuRects (void)
{
	LPVEHLOCATION	pVehLoc;
	UINT	iveh;
	RECT	NullMenuRect={0,0,-1,-1};

	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		
		pVehLoc->MenuRect = NullMenuRect;
		GlobalUnlock (hVehicle[iveh]);
	} 
	return;
};


BOOL StartVehInfoMenu (LPVIEWPORT pVP)
{
	VehStatusVP = pVP;
	pVP->hWndDlg = CreateDialog(hInst, (LPSTR)"VEHICLE_STATUS", hWndMain, VEHICLE_STATUSMsgProc);
	return TRUE;
}

BOOL SetVehicleInfo (LPSTR info,LPSTR VehID,BOOL ForceDisplay)
{
	short	ii;
	char	tempdbug[34];
	static	char	CurVehID[64];

	if (ForceDisplay)
		strcpy (CurVehID,VehID);
	else if (stricmp (CurVehID,VehID))
		return FALSE;
	if (!hWndVehStatus)
		return FALSE;
	if (*info)
		ii=1;
	else if (GetDlgItemText (hWndVehStatus,IDC_VEHICLEINFO,tempdbug,32))
		ii=1;
	SetDlgItemText (hWndVehStatus,IDC_VEHICLEINFO,info);
	return TRUE;
}

void ClearVehicleInfoRect (void)
{
	SetVehicleInfo ("","",TRUE);
//	RestoreScreen2 (hDCLastBox, hLastBox,0,FALSE);
	hLastBox = 0;
	LastVeh=-1;	
	return;
}

BOOL OrderFenceFilelist (LPSTR FenceFilelist)
{
	char	TempName[MAX_PATH], File[MAX_PATH+2];
	HFILE	Fid1, Fid2;

	Fid1 = GSSiOpenFile (FenceFilelist,0,OF_READ);
	if (Fid1 == HFILE_ERROR)
		return FALSE;
	GSSiGetTempFileName (0,"gm",0,(LPSTR)TempName);
	Fid2 = GSSiOpenFile (TempName,0,OF_CREATE);
	while (fgetstring (File,MAX_PATH,Fid1))
	{
		strlwr (File);
		if (strstr (File,"\\ex_"))
			fputstring (File,Fid2);
	}
	GSSillseek (Fid1,0,0);
	while (fgetstring (File,MAX_PATH,Fid1))
	{
		strlwr (File);
		if (strstr (File,"\\in_"))
			fputstring (File,Fid2);
	}
	GSSillseek (Fid2,0,0);
	GSSiClose (Fid1);
	Fid1 = GSSiOpenFile (FenceFilelist,0,OF_CREATE);
	while (fgetstring (File,MAX_PATH,Fid2))
		fputstring (File,Fid1);
	GSSiClose (Fid1);
	GSSiClose (Fid2);
	GSSiRemove (TempName);
	return TRUE;
}

BOOL ImportFences (LPSTR IPAddress,LPSTR CPort,LPSTR Account)
{
	USHORT	port;  
	SOCKET	sock;
	long	Buflen=USHRT_MAX*8;
	HANDLE	hIB;
	LPSTR	InputBuffer;
	int		err;
	int		lb=0,n=0,i, len,ii,lFileCmp;
	char	SearchStr[32];
	LPSTR	pLoc, pEnd;
	BOOL	rtn=FALSE;
	long	NewRefno=5100000;
	char	FenceFilelist[MAX_PATH];
	BOOL	SaveProcessing = Processing;
	char	Mess[256], str[256];

	DestroyFence (0);	
	if (!*IPAddress)
		return TRUE;
	port = atol (CPort);
	if (!port)
		return TRUE;
	Processing = TRUE;

	if (OpenTCPIPSocket (hWndMain,IPAddress,port,&sock,"","","","",FALSE))
	{
		char	response[4];

		sprintf (str,"$FENCE(GETDEFS,%s)\r\n",Account);
		send (sock,str,strlen(str),0);
		n = 0;
		do
		{
			Sleep (25);
			n++;
			err = recv (sock,response,4,MSG_PEEK);
		} while (n < 200 && err < 4);
		if (n < 200)
		{
			int	lFile;

			n = 0;
			recv (sock,(LPSTR)&lFileCmp,4,0);
			if (lFileCmp > 0)
			{
				HANDLE hFileCmp = GSSiGlobAlloc (0,GMEM_MOVEABLE,lFileCmp+4096);
				LPSTR  pFileCmp = GlobalLock (hFileCmp);
				HANDLE hFile = GSSiGlobAlloc (0,GMEM_MOVEABLE,2*1024*1024);
				LPSTR  pFile = GlobalLock (hFile);

				do
				{
					Sleep (25);
					n++;
					err = recv (sock,&pFileCmp[lb],lFileCmp-lb,0);
					if (err > 0)
					{
						lb += err;
					}
				} while (n < 150 && lb < lFileCmp);
				if (n < 150)
				{
					char	FenceFileBackup[MAX_PATH];
					HFILE	Fid;

					sprintf (FenceFileBackup,"[%%DL]fences\\%s.gfb",Account);
					lFile = DecompressBinaryRecordUnsafe (pFile,pFileCmp,lFileCmp);
					Fid = GSSiOpenFile (FenceFileBackup,0,OF_CREATE);
					BigWrite (Fid,pFile,lFile,-1);
					GSSiClose (Fid);
					HaltMapDisplay(FALSE); 
					CloseAllRequestedFiles (FALSE);
					BlockSocketProcessing (5);
					RecallFencesFromFile (FenceFileBackup,Account);
					BlockSocketProcessing (FALSE);
					ProcessText ("$MACRO([%DL]macros\\CreateStopPoints.txt)");
				}
				GSSiGlobUlFree (&hFile);
				GSSiGlobUlFree (&hFileCmp);
			}
		}
Exit:
		CloseTCPIPSocket (sock,TRUE);
	}
	StatusWindowUpdate2 ("", 0,0);
	Processing = SaveProcessing;
	return rtn;
}
BOOL ImportFences_old (LPSTR IPAddress,LPSTR CPort,LPSTR Account)
{
	USHORT	port;  
	SOCKET	sock;
	long	Buflen=USHRT_MAX*8;
	HANDLE	hIB;
	LPSTR	InputBuffer;
	int		err;
	int		lb=0,n=0,i, len,ii;
	char	SearchStr[32];
	LPSTR	pLoc, pEnd;
	BOOL	rtn=FALSE;
	long	NewRefno=5100000;
	char	FenceFilelist[MAX_PATH];
	BOOL	SaveProcessing = Processing;
	char	Mess[256];

	DestroyFence (0);	
	sprintf (FenceFilelist,"[%%GEOFENCEDIR]\\filelist.txt");
	GSSiRemove (FenceFilelist);
	if (!*IPAddress)
		return TRUE;
	port = atol (CPort);
	if (!port)
		return TRUE;
	Processing = TRUE;
	sprintf (SearchStr,">Fence:%s,",Account);

	if (OpenTCPIPSocket (hWndMain,IPAddress,port,&sock,"","","","",FALSE))
	{
		hIB = GSSiGlobAlloc (0,GMEM_MOVEABLE,Buflen);
		InputBuffer = GlobalLock (hIB);
		sprintf (InputBuffer,"$FENCE(GetList,%s,ALL)\r\n",Account);
		send (sock,InputBuffer,strlen(InputBuffer),0);
		do
		{
				Sleep (25);
				n++;
				err = recv (sock,&InputBuffer[lb],Buflen,0);
				if (err > 0)
				{
					lb += err;
					InputBuffer[lb] = 0;
					if (strstr (InputBuffer,"FenceListEnd;"))
					{
						rtn = TRUE;
						pLoc = InputBuffer;
						ii = strlen (InputBuffer);
						while ((pLoc = strstr (pLoc,SearchStr)))
						{
							pLoc += strlen (SearchStr);
							pEnd = strchr (pLoc,',');
							*pEnd = 0;
							strupr (pLoc);
							strcpy (FenceName[NumFencesInList],pLoc);
							if (strstr (FenceName[NumFencesInList],"MICHEL"))
								ii=1;
							sprintf (Mess,"Define fence %s",pLoc);
							StatusWindowUpdate2 (Mess, 0,0);
							*pEnd++ = ',';
							pLoc = pEnd;
							pEnd = strchr (pLoc,',');
							if (!strnicmp (pLoc,"Inclusive",9))
								FenceInclusive[NumFencesInList] = 1;
							else
								FenceInclusive[NumFencesInList] = 0;
							*pEnd++ = ',';
							pLoc = pEnd;
							pEnd = strchr (pLoc,',');
							if (!strnicmp (pLoc,"Active",6))
								FenceActive[NumFencesInList] = TRUE;
							else
								FenceActive[NumFencesInList] = FALSE;
							*pEnd++ = ',';
							pLoc = pEnd;
							pEnd = strchr (pLoc,',');
							FenceOffset[NumFencesInList] = atof (pLoc);
							*pEnd++ = ',';
							pLoc = pEnd;
							pEnd = strchr (pLoc,',');
							FenceOffsetUnits[NumFencesInList] = *pLoc;
							pLoc = pEnd + 1;
							if (*pLoc == '0')
								FenceCritical[NumFencesInList++] = FALSE;
							else
								FenceCritical[NumFencesInList++] = TRUE;
						}
						break;
					}
				}
		} while (n < 150);
		if (n < 150)
		{
			LPSTR	pNextFence;

			n = 0;
			lb = 0;
			sprintf (InputBuffer,"$FENCE(GetLinks,%s,ALL)\r\n",Account);
			send (sock,InputBuffer,strlen(InputBuffer),0);
			do
			{
				Sleep (25);
				n++;
				err = recv (sock,&InputBuffer[lb],Buflen,0);
				if (err > 0)
				{
					lb += err;
					InputBuffer[lb] = 0;
					if ((pEnd = strstr (InputBuffer,">FenceLinkEnd;")))
					{
						*pEnd = 0;
						rtn = TRUE;
						pLoc = InputBuffer;
						while ((pLoc = strstr (pLoc,">Fence:")))
						{
							pLoc += 7;
							if ((pNextFence = strstr (pLoc,">Fence:")))
								*pNextFence = 0;
							pEnd = strchr (pLoc,',');
							*pEnd++ = 0;
							SetGlobalValue ("%GEOFENCENAME",pLoc);
							pLoc = pEnd;
							while ((pEnd = strchr (pLoc,',')))
							{
								*pEnd++ = 0;
								SetGlobalValue ("%VEHICLE",pLoc);
								pLoc = pEnd;
								ProcessText ("$GMDUPDATE([%GEOFENCEDIR]\\fencelink.gmd,VID=[%VEHICLE];FENCENAME=[%GEOFENCENAME],VID2=[%VEHICLE])");
							}
							if (pNextFence)
								*pNextFence = '>';
						}
						break;
					}
				}
			} while (n < 150);
		}
		if (n < 150)
		{
			LPSTR	pNextFence;

			n = 0;
			lb = 0;
			sprintf (InputBuffer,"$FENCE(GetStops,%s,ALL)\r\n",Account);
			send (sock,InputBuffer,strlen(InputBuffer),0);
			do
			{
				Sleep (25);
				n++;
				err = recv (sock,&InputBuffer[lb],Buflen,0);
				if (err > 0)
				{
					lb += err;
					InputBuffer[lb] = 0;
					if ((pEnd = strstr (InputBuffer,">RouteStopsEnd;")))
					{
						*pEnd = 0;
						rtn = TRUE;
						pLoc = InputBuffer;
						while ((pLoc = strstr (pLoc,">Route:")))
						{
							pLoc += 7;
							if ((pNextFence = strstr (pLoc,">Route:")))
								*pNextFence = 0;
							pEnd = strchr (pLoc,',');
							*pEnd++ = 0;
							SetGlobalValue ("%GEOFENCENAME",pLoc);
							pLoc = pEnd;
							while ((pEnd = strchr (pLoc,',')))
							{
								LPSTR loc[4];

								*pEnd++ = 0;
								GetParmLoc (4,';',pLoc,loc);
								SetGlobalValue ("%GEOFENCESTOPDIST",loc[0]);
								SetGlobalValue ("%GEOFENCESTOPNAME",loc[1]);
								SetGlobalValue ("%GEOFENCESTOPTIME",loc[2]);
								SetGlobalValue ("%GEOFENCESTOPDELAY",loc[3]);
								pLoc = pEnd;
								ProcessText ("$GMDUPDATE([%GEOFENCEDIR]\\routestops.gmd,ROUTE=[%GEOFENCENAME];DIST=[%GEOFENCESTOPDIST],NAME=[%GEOFENCESTOPNAME];TIME=[%GEOFENCESTOPTIME];DELAY=[%GEOFENCESTOPDELAY])");
							}
							if (pNextFence)
								*pNextFence = '>';
						}
						break;
					}
				}
			} while (n < 150);
		}
		if (n < 150)
		for (i=0;i<NumFencesInList;i++)
		{
			HANDLE	hArea=0;
			long	nPoints;
			LPSTR	pArea;
			int		lArea, la=0;
			char	InEx[4]="In";
			char	AreaSymbolName[20]="GEOFENCE_INCLUSIVE";
			
			if (!FenceInclusive[i])
			{
				strcpy (InEx,"Ex");
				strcpy (AreaSymbolName,"GEOFENCE_EXCLUSIVE");
			}
			sprintf (InputBuffer,"$FENCE(GetCoord,%s,%s_%s)\r\n",Account,InEx,FenceName[i]);
			sprintf (Mess,"Request fence definition for  %i of %i %s",i+1,NumFencesInList,FenceName[i]);
			StatusWindowUpdate2 (Mess, 0,0);
			send (sock,InputBuffer,strlen(InputBuffer),0);
			n = 0;
			do
			{
				len = recv (sock,InputBuffer,Buflen,MSG_PEEK);
				if (len == SOCKET_ERROR)
				{	
					err = WSAGetLastError();
					WSASetLastError (0);	
					if (err != WSAEWOULDBLOCK)
					{
					    ProcessSocketError (sock,err,"In ImportFences-1");
						goto Exit;
					}
				}
				Sleep (25);
				n++;
			} while (n < 150 && len < 4);
			len = recv (sock,(LPSTR)&nPoints,4,0);
			sprintf (Mess,"Request %i fence coordinates for %i of %i %s",nPoints,i+1,NumFencesInList,FenceName[i]);
			StatusWindowUpdate2 (Mess, 0,0);
			if (labs(nPoints) < 1)
				goto Exit;
			if (nPoints == 2)
				FenceMethod[i] = 1;
			else if (nPoints < 0)
				FenceMethod[i] =2;
			else
				FenceMethod[i] = 0;
			lArea = labs(nPoints) * sizeof(DPOINT);
			hArea = GSSiGlobAlloc (0,GMEM_MOVEABLE,lArea);
			pArea = GlobalLock (hArea);
			n = 0;
			do
			{
					Sleep (25);
					n++;
					len = recv (sock,&pArea[la],lArea-la,0);
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
							ProcessSocketError (sock,err,"In ImportFences-2");
							goto Exit;
						}
					}
			} while (n < 150 && la < lArea);
			if (n >= 150)
			{
				GSSiGlobUlFree (&hArea);
				break;
			}
			else
			{
				char	MapFile[MAX_PATH];
				MNMXCORD	Bounds;
				HANDLE	hSymDesc=0;
				short	NumSyms=0, SymNum; 
				int		np=labs (nPoints);
				char	Prefix[10]="GEOFENCE";
				char	TAG[80], un[2]=" ";
				char	FenceTypeAndName[128];
				int		seq;
				HANDLE	hFence;
				int		typ=0, width=0;
				long	color=-1;

				if (nPoints < 0)
				{
					typ = 1;
					color = RGB (255,220,200);
					if (FenceOffsetUnits[i] == 'M')
						width = (-FenceOffset[i] * 5280)/3.2808333;
					else
						width = (-FenceOffset[i])/3.2808333;
				}
				SymNum = GetDictSymbolNumber (AreaSymbolName);
				ConvertPolyCoord ((HPDPOINT)pArea,np,2,1);
				GetPolyBoundsD (hArea,np,&Bounds,TYPE_AREA);
				CurrentPoint = FencePoint[i] = ComputePolylineMidpoint (hArea,np);
				SetGlobalValue ("%GEOFENCENAME",FenceName[i]);
				switch (FenceMethod[i])
				{
				case 0:
					SetGlobalValue ("%GEOFENCETYPE","POLYGON");
					break;
				case 1:
					SetGlobalValue ("%GEOFENCETYPE","CIRCLE");
					break;
				case 2:
					SetGlobalValue ("%GEOFENCETYPE","ROUTE");
					break;
				}
				SetGlobalValueLong ("%GEOFENCEINCLUSIVE",(int)FenceInclusive[i]); 
				seq = GetGlobalLVal ("[%GEOFENCESEQ]");
				seq++;
				SetGlobalValueLong ("%GEOFENCESEQ",seq); 
				SetGlobalValueReal ("%GEOFENCEOFFSET",FenceOffset[i]);
				un[0] = FenceOffsetUnits[i];
				SetGlobalValue ("%GEOFENCEOFFSETUNITS",un);
				SetGlobalValueBool ("%GEOFENCECRITICAL",FenceCritical[i]);
				sprintf (FenceTypeAndName,"%s_%s",InEx,FenceName[i]);
				SetGlobalValue ("%NEWGEOFENCE",FenceTypeAndName);
				sprintf (MapFile,"[%%GEOFENCEDIR]\\%s_%s.plt",InEx,FenceName[i]);
				strcpy (PltName,MapFile);
				CreateNewMap (PltName,&Bounds,0,0,0,0,0,0,FALSE);
				AddToSymList (SymNum,&NumSyms,&hSymDesc); 
				EditBounds = Bounds; 
				if (FenceMethod[i] == 2)
				{
					double	RouteOffset = FenceOffset[i];
					
					if (FenceOffsetUnits[i] == 'F')
						RouteOffset *= FTM;
					else
						RouteOffset *= (5280 * FTM);
					AddPolyToMap (1,&np, &hArea,typ,NewRefno++,0,2,SymNum,0,Prefix,FenceName[i],-1,color,width,0,0,0,0,TRUE,&RouteOffset);
				}
				else
					AddPolyToMap (1,&np, &hArea,typ,NewRefno++,0,2,SymNum,0,Prefix,FenceName[i],-1,color,width,0,0,0,0,TRUE,0);
			    CloseMap(TRUE);  
				AddSymToMap (NumSyms,hSymDesc,0,0); 
    			DestroySymList (&NumSyms,&hSymDesc); 
				GSSiGlobUlFree (&hArea);
				AppendFile (FenceFilelist,MapFile);
				sprintf (TAG,"%s:%s",Prefix,FenceName[i]);
				//hFence = LoadFence ("POLYGON",TAG,&CurrentPoint,0);
				ProcessText("$GMDUPDATE([%GEOFENCEDIR]\\fencelist.gmd,NAME=[%GEOFENCENAME],TYPE=[%GEOFENCETYPE];INCLUSIVE=[%GEOFENCEINCLUSIVE];X=[%WX];Y=[%WY];OFFSET=[%GEOFENCEOFFSET];OFFSETUNITS=[%GEOFENCEOFFSETUNITS];CRITICAL=[%GEOFENCECRITICAL];ACTIVE=T;HANDLE=$FENCE(LOAD,[%GEOFENCETYPE],GEOFENCE:[%GEOFENCENAME],[%GEOFENCEOFFSET],[%WX] [%WY]))");

				ProcessText("IF(!$GMDFIND([%GEOFENCEDIR]\\fencelink.gmd,FENCENAME=[%GEOFENCENAME],,64,1)){$GMDUPDATE([%GEOFENCEDIR]\\fencelink.gmd,VID=ALL;FENCENAME=[%GEOFENCENAME],VID2=ALL)};");
			}
		}
Exit:
		SetGlobalValue ("%NEWGEOFENCE","");
		CloseTCPIPSocket (sock,TRUE);
		GSSiGlobUlFree (&hIB);
		OrderFenceFilelist (FenceFilelist);
		ProcessText ("$MACRO([%DL]macros\\CreateStopPoints.txt)");
	}
	GetFenceListFromDB (Account);
	StatusWindowUpdate2 ("", 0,0);
	Processing = SaveProcessing;
	return rtn;
}

BOOL ExportFences2 (LPSTR pMemCmp,int lFile, int lFileCmp,LPSTR IPAddress,LPSTR CPort,LPSTR Account)
{
	BOOL	rtn=FALSE;
	char	response[32];
	int		lresponse;
	int		ier, n, err,ii;
	USHORT	port;
	SOCKET	sock;
	char	str[256];

	port = atol (CPort);
	if (!port)
		return FALSE;
	if (OpenTCPIPSocket (hWndMain,IPAddress,port,&sock,"","","","",TRUE))
	{
		char	response[4];

		sprintf (str,"$FENCE(UPLOAD,%s,%i,%i)\r\n",Account,lFile,lFileCmp);
		send (sock,str,strlen(str),0);
		n = 0;
		lresponse = 0;
		do
		{
			Sleep (25);
			n++;
			err = recv (sock,&response[lresponse],4,0);
			if (err > 0)
				lresponse += err;
		} while (n < 200 && lresponse < 4);
		if (n < 200)
		{
			if (!strnicmp (response,"OK1",3))
			{
				ier = send (sock,pMemCmp,lFileCmp,0);
				if (ier != lFileCmp)
					MessageBox (0,"Failed to transmit fence definitions",0,MB_ICONEXCLAMATION);
				else
				{
					n = 0;
					lresponse = 0;
					do
					{
						Sleep (25);
						n++;
						err = recv (sock,&response[lresponse],4,0);
						if (err > 0)
							lresponse += err;
					} while (n < 200 && lresponse < 4);
					if (n < 200)
					{
						if (!strnicmp (response,"OK2",3))
							rtn = TRUE;
					}
				}
			}
		}
		else
			ii=1;
		CloseTCPIPSocket (sock,TRUE);
	}
	return rtn;
}

BOOL ExportFences2_OLD (LPSTR FenceName,int FenceType,double Offset,char OffsetUnits,BOOL Critical,LPSTR IPAddress,LPSTR CPort,LPSTR Account,BOOL First)
{
	USHORT	port;  
	SOCKET	sock;
	int		err,ii;
	char	str[256];
	int		lb=0,n=0,i, len, st;
	char	SearchStr[32];
	LPSTR	pLoc, pEnd;
	BOOL	rtn=FALSE;
	long	NewRefno=5100000;
	MNMXCORD	Bounds;
	HIGHLIGHTDATA	HighlightData;
	long	npnts=0;
	HANDLE	hPoly=0;
	int		Layer, nAreas, Refno;
	HPDPOINT	pPoly;
	char	SavePickFile[MAX_PATH]=""; 
	BOOL	PickSubVPSave = PickSubVP;

	if (!*IPAddress)
		return TRUE;
	port = atol (CPort);
	if (!port)
		return TRUE;
	sprintf (SearchStr,">Fence:%s,",Account);

    ClearHighlightList (FALSE);  
	SetGlobalValue ("%NEWGEOFENCE",FenceName);
	SetViewport(*pCommandViewport);
	Layer = GetLayerNumFromName ("New GeoFence");
    GetLayerBounds (&Bounds,CurView->hDC, Layer-1);
	GSSiGetTempFileName (0,"gmb",0,SavePickFile);
    SaveVisFile (SavePickFile,TRUE,"");
	ProcessText ("$PIK(LAYER:ALL,0);$PIK(LAYER:New GeoFence,1)");
	//LoadPickList ("[%DL]piklists\\newfence.pik");
	PickSubVP = FALSE;
	st = HighlightInArea (CurView->hWnd,&Bounds,TRUE,FALSE,0);
	PickSubVP = PickSubVPSave;
	LoadPickList (SavePickFile);
   	GSSiRemove (SavePickFile);
	if (st)
	{
		BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData);  
		if (GetPolyPnts ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts,&hPoly,TRUE))
			goto SendFence;
	}
	return FALSE;

SendFence:

    ClearHighlightList (FALSE);  

	if (OpenTCPIPSocket (hWndMain,IPAddress,port,&sock,"","","","",TRUE))
	{
		char	response[4];

		sprintf (str,"$FENCE(UPLOAD,%s,%s,%i,%f,%c,%i,%i)\r\n",Account,FenceName,FenceType,Offset,OffsetUnits,Critical,First);
		send (sock,str,strlen(str),0);
		n = 0;
		do
		{
			Sleep (25);
			n++;
			err = recv (sock,response,4,0);
		} while (n < 200 && err < 0);
		if (n < 200)
		{
			send (sock,(LPSTR)&npnts,4,0);
			pPoly = GlobalLock (hPoly);
			ConvertPolyCoord (pPoly,npnts,1,2);
			send (sock,(LPSTR)pPoly,npnts*sizeof(DPOINT),0);
			GlobalUnlock (hPoly);
			rtn = TRUE;
			n = 0;
			do
			{
				Sleep (25);
				n++;
				err = recv (sock,response,4,0);
			} while (n < 200 && err < 0);
		}
		else
			ii=1;
		CloseTCPIPSocket (sock,TRUE);
	}
	GSSiGlobFree (&hPoly);
	return rtn;
}

int RemoveLinksForFence (LPSTR FenceName)
{
	HANDLE	hDB=0;
	int		NumRemoved = 0;
	char	SQL[]="FENCENAME=@[%FNM]";

	if (OpenDataFile ("FLINK=[%GEOFENCEDIR]\\fencelink.gmd",SQL,BT_WRITE,&hDB))
	{
		SetGlobalValue ("%FNM",FenceName);
		if (FetchDBRec (hDB))
		do
		{
			LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB);
			LPOPENFILEDATA	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
			LPGWDHEADER		lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);
			
			GWDDeleteRecord (lpGWDHead,SQLPtr->Offset); 
			SQLPtr->lastreadtime = 0;
			GlobalUnlock (FilePtr->FileHandle);
			GlobalUnlock (SQLPtr->OFHandle); 
			GlobalUnlock (hDB);
			NumRemoved++;
			SetGlobalValue ("%FNM",FenceName);

		}while (FetchDBRec (hDB));
		CloseDataFile (TRUE, &hDB); 
	}
	return NumRemoved;
}

int RemoveRouteStopsForFence (LPSTR FenceName)
{
	HANDLE	hDB=0;
	int		NumRemoved = 0;
	char	SQL[]="ROUTE=@[%FNM]";

	if (OpenDataFile ("FLINK=[%GEOFENCEDIR]\\routestops.gmd",SQL,BT_WRITE,&hDB))
	{
		SetGlobalValue ("%FNM",FenceName);
		if (FetchDBRec (hDB))
		do
		{
			LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB);
			LPOPENFILEDATA	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
			LPGWDHEADER		lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);
			
			GWDDeleteRecord (lpGWDHead,SQLPtr->Offset); 
			SQLPtr->lastreadtime = 0;
			GlobalUnlock (FilePtr->FileHandle);
			GlobalUnlock (SQLPtr->OFHandle); 
			GlobalUnlock (hDB);
			NumRemoved++;
			SetGlobalValue ("%FNM",FenceName);

		}while (FetchDBRec (hDB));
		CloseDataFile (TRUE, &hDB); 
	}
	return NumRemoved;
}

BOOL DeleteFence (LPSTR FenceName)
{
	char	FenceFilelist[MAX_PATH],FenceFilelist2[MAX_PATH];
	char	str[260], SearchStr[128];
	char	FencePltToDelete[MAX_PATH]="";
	HFILE	Fid, Fid2;
	BOOL	rtn=FALSE;

	sprintf (FenceFilelist,"[%%GEOFENCEDIR]\\filelist.txt");
	Fid = GSSiOpenFile (FenceFilelist,0,OF_READ);

	if (Fid == HFILE_ERROR)
		return FALSE;
	sprintf (SearchStr,"_%s.",FenceName);
	strlwr (SearchStr);
	sprintf (FenceFilelist2,"[%%GEOFENCEDIR]\\filelist2.txt");
	Fid2 = GSSiOpenFile (FenceFilelist2,0,OF_CREATE);
	while (fgetstring (str,255,Fid))
	{
		strlwr (str);
		if (!strstr (str,SearchStr))
			fputstring (str,Fid2);
		else
			strcpy (FencePltToDelete,str);
	}
	GSSiClose (Fid);
	GSSiClose (Fid2);
	if (*FencePltToDelete)
	{
		LPSTR	pPlt;

		strlwr (FencePltToDelete);
		if ((pPlt = strstr (FencePltToDelete,".plt")))
		{
			strcpy (pPlt,".plt");
			GSSiRemove (FencePltToDelete);
			strcpy (pPlt,".rin");
			GSSiRemove (FencePltToDelete);
			strcpy (pPlt,".tin");
			GSSiRemove (FencePltToDelete);
		}
	}
	GSSiRemove (FenceFilelist);
	GSSiRename (FenceFilelist2,FenceFilelist);
	RemoveFenceFromList (FenceName);
	RemoveLinksForFence (FenceName);
	RemoveRouteStopsForFence (FenceName);
	return rtn;
}

BOOL ActivateDeactivateFence (LPSTR FenceName,BOOL Active)
{
	char	FenceFilelist[MAX_PATH],FenceFilelist2[MAX_PATH];
	char	str[260], SearchStr[128];
	HFILE	Fid, Fid2;
	BOOL	rtn=FALSE;

	sprintf (FenceFilelist,"[%%GEOFENCEDIR]\\filelist.txt");
	Fid = GSSiOpenFile (FenceFilelist,0,OF_READ);

	if (Fid == HFILE_ERROR)
		return FALSE;
	sprintf (SearchStr,"_%s.",FenceName);
	strlwr (SearchStr);
	sprintf (FenceFilelist2,"[%%GEOFENCEDIR]\\filelist2.txt");
	Fid2 = GSSiOpenFile (FenceFilelist2,0,OF_CREATE);
	while (fgetstring (str,255,Fid))
	{
		strlwr (str);
		if (!strstr (str,SearchStr))
			fputstring (str,Fid2);
		else
		{
			LPSTR ploc = strstr (str,".plt");

			if (ploc)
			{
				ploc += 4;
				if (Active)
					*ploc = ' ';
				else
					*ploc = 'N';
			}
			fputstring (str,Fid2);
		}
	}
	GSSiClose (Fid);
	GSSiClose (Fid2);
	GSSiRemove (FenceFilelist);
	GSSiRename (FenceFilelist2,FenceFilelist);
	SetGlobalValue ("%GEOFENCENAME",FenceName);
	if (Active)
		ProcessText("$GMDUPDATE([%GEOFENCEDIR]\\fencelist.gmd,NAME=[%GEOFENCENAME],ACTIVE=T;)");
	else
		ProcessText("$GMDUPDATE([%GEOFENCEDIR]\\fencelist.gmd,NAME=[%GEOFENCENAME],ACTIVE= ;)");

	return rtn;
}

BOOL CreateFencesLinksFromDB (LPSTR File)
{
	int		err,ii,n;
	char	str[512];
	HANDLE	hDB=0, hBuffer;
	HFILE	Fid;
	long	lBuffer;
	LPSTR	pBuffer;


	Fid = GSSiOpenFile (File,0,OF_CREATE);
	if (OpenDataFile ("FLINK=[%GEOFENCEDIR]\\fencelink.gmd","FENCENAME>''",BT_READ,&hDB))
	{
		char	LastFence[66]="", FenceName[66], VehicleName[64];

		*str = 0;
		while (FetchDBRec (hDB))
		{
			strcpy (FenceName,"[FLINK.FENCENAME]");
			ExpandText (FenceName);
			if (stricmp (FenceName,LastFence))
			{
				if (*LastFence)
					fputstring (str,Fid);
				strcpy (LastFence,FenceName);
				sprintf (str,">Fence:%s,",FenceName);
			}
			strcpy (VehicleName,"[FLINK.VID]");
			ExpandText (VehicleName);
			sprintf (strchr (str,0),"%s,",VehicleName);
		}
		fputstring (str,Fid);
		CloseDataFile (TRUE,&hDB);
	}
	fputstring (">FenceLinkEnd;",Fid);
	GSSiClose (Fid);
	return TRUE;
}

BOOL CreateRouteStopsFromDB (LPSTR File)
{
	int		err,ii,n;
	char	str[512];
	HANDLE	hDB=0;
	HFILE	Fid;

	Fid = GSSiOpenFile (File,0,OF_CREATE);
	if (OpenDataFile ("STOPS=[%GEOFENCEDIR]\\routestops.gmd","",BT_READ,&hDB))
	{
		char	LastFence[66]="", FenceName[66], VehicleName[64];

		while (FetchDBRec (hDB))
		{
			strcpy (FenceName,"[STOPS.ROUTE]");
			ExpandText (FenceName);
			if (stricmp (FenceName,LastFence))
			{
				if (*LastFence)
					fputstring (str,Fid);
				strcpy (LastFence,FenceName);
				sprintf (str,">Route:%s,",FenceName);
			}
			strcat (str,"[STOPS.DIST];[STOPS.NAME];[STOPS.TIME];[STOPS.DELAY],");
			ExpandText (str);
		}
		fputstring (str,Fid);
		CloseDataFile (TRUE,&hDB);
	}
	fputstring (">RouteStopsEnd;",Fid);
	GSSiClose (Fid);
	return TRUE;
}

BOOL ExportFencesLinks (LPSTR IPAddress,LPSTR CPort,LPSTR Account)
{
	USHORT	port;  
	SOCKET	sock;
	int		err,ii,n;
	char	str[4096];
	char	TempName[MAX_PATH];
	HANDLE	hDB=0, hBuffer;
	HFILE	Fid;
	long	lBuffer;
	LPSTR	pBuffer;

	if (!*IPAddress)
		return TRUE;
	port = atol (CPort);
	if (!port)
		return TRUE;
	if (OpenTCPIPSocket (hWndMain,IPAddress,port,&sock,"","","","",FALSE))
	{
		char	response[4];

		sprintf (str,"$FENCE(UPLINKS,%s)\r\n",Account);
		send (sock,str,strlen(str),0);
		n = 0;
		do
		{
			Sleep (250);
			n++;
			err = recv (sock,response,4,0);
		} while (n < 200 && err < 0);

		if (OpenDataFile ("FLINK=[%GEOFENCEDIR]\\fencelink.gmd","FENCENAME>''",BT_READ,&hDB))
		{
			char	LastFence[66]="", FenceName[66], VehicleName[64];

			GSSiGetTempFileName (0,"gm",0,(LPSTR)TempName);
			Fid = GSSiOpenFile (TempName,0,OF_CREATE);
			*str = 0;
			while (FetchDBRec (hDB))
			{
				strcpy (FenceName,"[FLINK.FENCENAME]");
				ExpandText (FenceName);
				if (stricmp (FenceName,LastFence))
				{
					if (*LastFence)
						fputstring (str,Fid);
					strcpy (LastFence,FenceName);
					sprintf (str,">Fence:%s,",FenceName);
				}
				strcpy (VehicleName,"[FLINK.VID]");
				ExpandText (VehicleName);
				sprintf (strchr (str,0),"%s,",VehicleName);
			}
			fputstring (str,Fid);
			CloseDataFile (TRUE,&hDB);
		}
		fputstring (">FenceLinkEnd;",Fid);
		GSSillseek (Fid,0,0);
		lBuffer = GSSifilelength (Fid);
		send (sock,(LPSTR)&lBuffer,4,0);
		if (lBuffer)
		{
			hBuffer = GSSiGlobAlloc (1576,GMEM_MOVEABLE,lBuffer);
			pBuffer = GlobalLock (hBuffer);
			BigRead (Fid,pBuffer,lBuffer);
			send (sock,pBuffer,lBuffer,0);
			GSSiGlobUlFree (&hBuffer);
		}
		GSSiClose (Fid);
		GSSiRemove (TempName);
		n = 0;
		do
		{
			Sleep (250);
			n++;
			err = recv (sock,response,4,0);
		} while (n < 200 && err < 0);
		CloseTCPIPSocket (sock,TRUE);
	}
	return TRUE;
}

BOOL ExportRouteStops (LPSTR IPAddress,LPSTR CPort,LPSTR Account)
{
	USHORT	port;  
	SOCKET	sock;
	int		err,ii,n;
	char	str[4096];
	char	TempName[MAX_PATH];
	HANDLE	hDB=0, hBuffer;
	HFILE	Fid;
	long	lBuffer;
	LPSTR	pBuffer;

	if (!*IPAddress)
		return TRUE;
	port = atol (CPort);
	if (!port)
		return TRUE;
	if (OpenTCPIPSocket (hWndMain,IPAddress,port,&sock,"","","","",FALSE))
	{
		char	response[4];

		sprintf (str,"$FENCE(UPSTOPS,%s)\r\n",Account);
		send (sock,str,strlen(str),0);
		n = 0;
		do
		{
			Sleep (250);
			n++;
			err = recv (sock,response,4,0);
		} while (n < 200 && err < 0);

		if (OpenDataFile ("STOPS=[%GEOFENCEDIR]\\routestops.gmd","",BT_READ,&hDB))
		{
			char	LastFence[66]="", FenceName[66], VehicleName[64];

			GSSiGetTempFileName (0,"gm",0,(LPSTR)TempName);
			Fid = GSSiOpenFile (TempName,0,OF_CREATE);
			*str = 0;
			while (FetchDBRec (hDB))
			{
				strcpy (FenceName,"[STOPS.ROUTE]");
				ExpandText (FenceName);
				if (stricmp (FenceName,LastFence))
				{
					if (*LastFence)
						fputstring (str,Fid);
					strcpy (LastFence,FenceName);
					sprintf (str,">Route:%s,",FenceName);
				}
				strcat (str,"[STOPS.DIST];[STOPS.NAME];[STOPS.TIME];[STOPS.DELAY],");
				ExpandText (str);
			}
			fputstring (str,Fid);
			CloseDataFile (TRUE,&hDB);
		}
		fputstring (">RouteStopsEnd;",Fid);
		GSSillseek (Fid,0,0);
		lBuffer = GSSifilelength (Fid);
		send (sock,(LPSTR)&lBuffer,4,0);
		if (lBuffer)
		{
			hBuffer = GSSiGlobAlloc (1576,GMEM_MOVEABLE,lBuffer);
			pBuffer = GlobalLock (hBuffer);
			BigRead (Fid,pBuffer,lBuffer);
			send (sock,pBuffer,lBuffer,0);
			GSSiGlobUlFree (&hBuffer);
		}
		GSSiClose (Fid);
		GSSiRemove (TempName);
		n = 0;
		do
		{
			Sleep (250);
			n++;
			err = recv (sock,response,4,0);
		} while (n < 200 && err < 0);
		CloseTCPIPSocket (sock,TRUE);
	}
	return TRUE;
}

BOOL ExportFencesEnd (LPSTR IPAddress,LPSTR CPort,LPSTR Account)
{
	USHORT	port;  
	SOCKET	sock;
	int		err,ii,n;
	char	str[256];
	BOOL	rtn=FALSE;

	if (!*IPAddress)
		return FALSE;
	port = atol (CPort);
	if (!port)
		return FALSE;
	port--;
	if (OpenTCPIPSocket (hWndMain,IPAddress,port,&sock,"","","","",FALSE))
	{
		char	response[4]="";

		sprintf (str,"$FENCE(UPLOAD,%s,*RENAME*)\r\n",Account);
		send (sock,str,strlen(str),0);
		n = 0;
		do
		{
			Sleep (25);
			n++;
			err = recv (sock,response,2,MSG_PEEK);
		} while (n < 200 && err < 2);
		err = recv (sock,response,2,0);
		CloseTCPIPSocket (sock,TRUE);
		if (!strnicmp (response,"OK",2))
			rtn = TRUE;
	}
	return rtn;
}

BOOL ExportFences (LPSTR IPAddress,LPSTR CPort,LPSTR Account)
{
	char	FenceFilelist[MAX_PATH];
	char	str[260], Type[16], IncExc[16], ActInAct[16];
	char	txt[512]="";
	LPSTR	pName, pEnd, pName2;
	HFILE	Fid;
	BOOL	First = TRUE;
	long	Tot, Done=0;
	int		ifence;
	char	Account_New[128];
	char	TempName[MAX_PATH];
	HANDLE	hMem, hMemCmp;
	LPSTR	pMem, pMemCmp;
	int		lFile, lFileCmp;
	BOOL	rtn;

	WaitCursor (1);
	//GSSiGetTempFileName (0,"gm",0,(LPSTR)TempName);
	sprintf (TempName,"[%%DL]FenceBackup\\%s_[%%SYS_CLOCK]",Account);
	ExpandText (TempName);
	HaltMapDisplay(FALSE); 
	CloseAllRequestedFiles (FALSE);
	BlockSocketProcessing (6);
	if (!SaveFencesToFile (TempName,Account))
	{
		WaitCursor (-1);
		sprintf (str,"Unable to create fence backup file\r%s",TempName);
		MessageBox (0,str,0,MB_ICONEXCLAMATION);
		return FALSE;
	}
	BlockSocketProcessing (FALSE);
	lFile = GSSiLength (TempName);
	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,lFile);
	pMem = GlobalLock (hMem);
	Fid = GSSiOpenFile (TempName,0,OF_READ);
	BigRead (Fid,pMem,lFile);
	GSSiClose (Fid);
	hMemCmp = GSSiGlobAlloc (0,GMEM_MOVEABLE,lFile+4096);
	pMemCmp = GlobalLock (hMemCmp);
	lFileCmp = CompressBinaryRecord (pMem,pMemCmp,lFile);
	GSSiGlobUlFree (&hMem);
	rtn = ExportFences2 (pMemCmp,lFile,lFileCmp,IPAddress,CPort,Account);
	GSSiGlobUlFree (&hMemCmp);
/*
	sprintf (Account_New,"%s_new",Account);

	sprintf (FenceFilelist,"[%%GEOFENCEDIR]\\filelist.txt");
	Fid = GSSiOpenFile (FenceFilelist,0,OF_READ);

	if (Fid != HFILE_ERROR)
	{
		Tot = GSSillseek (Fid,0,2);
		GSSillseek (Fid,0,0);
		if (hWndGeoFence)
		    PctBox (GetDlgItem(hWndGeoFence,IDC_UPLOADSTATUS), Tot, Done,0); 
		while (fgetstring (str,255,Fid))
		{
			if (FileType (str))
			{
				if ((pName = strrchr (str,'\\')))
				{
					pName++;
					pEnd = strrchr (pName,'.');
					*pEnd++ = 0;
					pEnd = strchr (pEnd,0);
					pEnd--;
				}
				if ((pName2 = strchr (pName,'_')))
					pName2++;
				else
					pName2 = pName;
				ifence = GetFenceID (pName2);
				ExportFences2 (pName,FenceMethod[ifence],FenceOffset[ifence],FenceOffsetUnits[ifence],FenceCritical[ifence],IPAddress,CPort,Account,First);
				First = FALSE;
			}
			Done = GSSillseek (Fid,0,1);
			if (hWndGeoFence)
			    PctBox (GetDlgItem(hWndGeoFence,IDC_UPLOADSTATUS), Tot, Done,0); 
		}
		GSSiClose (Fid);
	}
	ExportFencesLinks (IPAddress,CPort,Account_New);
	ExportRouteStops (IPAddress,CPort,Account_New);
	*/
	if (rtn)
		rtn = ExportFencesEnd (IPAddress,CPort,Account);

	WaitCursor (-1);
/*	if (rtn && hWndGeoFence)
    	 PostMessage(hWndGeoFence, WM_CLOSE, 0, 0L);
		 */
	return rtn;
}

BOOL AddFenceToList (LPSTR Name,LPSTR Method,LPSTR Type,LPSTR Active,LPSTR CPoint,LPSTR COffset,char OffsetUnits,BOOL Critical)
{
	BOOL	err;

	if (NumFencesInList + 1 >= MAX_FENCES)
		return FALSE;
	strcpy (FenceName[NumFencesInList],Name);
	FenceInclusive[NumFencesInList] = atoi(Type);
	if (!stricmp (Method,"POLYGON"))
		FenceMethod[NumFencesInList] = 0;
	else if (!stricmp (Method,"CIRCLE"))
		FenceMethod[NumFencesInList] = 1;
	else
		FenceMethod[NumFencesInList] = 2;
	FencePoint[NumFencesInList] = atopt (CPoint,&err);
	FenceOffset[NumFencesInList] = atof (COffset);
	FenceOffsetUnits[NumFencesInList] = OffsetUnits;
	FenceCritical[NumFencesInList] = Critical;
	FenceActive[NumFencesInList++] = atob(Active);
	return TRUE;
}

BOOL RemoveFenceFromList (LPSTR Name)
{
	HANDLE	hDB=0;
	char	SQL[]="NAME=@[%FNM]";
	int	n=0,i;

	if (!NumFencesInList)
		return FALSE;
	for (i=0;i<NumFencesInList;i++)
	{
		if (stricmp (FenceName[i],Name))
		{
			strcpy (FenceName[n],FenceName[i]);
			FenceMethod[n] = FenceMethod[i];
			FenceInclusive[n] = FenceInclusive[i];
			FenceOffset[n] = FenceOffset[i];
			FenceOffsetUnits[n] = FenceOffsetUnits[i];
			FenceCritical[n] = FenceCritical[i];
			FenceActive[n++] = FenceActive[i];
		}
	}
	NumFencesInList = n;

	if (OpenDataFile ("FLIST=[%GEOFENCEDIR]\\fencelist.gmd",SQL,BT_WRITE,&hDB))
	{
		SetGlobalValue ("%FNM",Name);
		if (FetchDBRec (hDB))
		{
			LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB);
			LPOPENFILEDATA	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
			LPGWDHEADER		lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);
			
			GWDDeleteRecord (lpGWDHead,SQLPtr->Offset); 
			SQLPtr->lastreadtime = 0;
			GlobalUnlock (FilePtr->FileHandle);
			GlobalUnlock (SQLPtr->OFHandle); 
			GlobalUnlock (hDB);
		}
		CloseDataFile (TRUE, &hDB); 
	}
	return TRUE;
}

double GetBaseOffset (double offset,char units)
{
	if (units == 'M')
		return ((offset * 5280)/MFT);
	return (offset/MFT);
}

HANDLE	LoadFence (LPSTR Type,LPSTR TAG,LPDPOINT pPoint,double Offset,char OffsetUnits)
{
	HANDLE handle=0, hPoly=0;
	int	itype, NumPoints;
	LPFENCEHEADER	pFenceHeader;
	HPDPOINT	pPoints,Points;
	char	Prefix[64], UDI[64];
	LPSTR	pSC;

	if (NumFences + 1 >= MAX_FENCES)
	{
		MessageBox (0,"Number of fences exceeds max",0,MB_ICONEXCLAMATION);
		return 0;
	}

	if (!stricmp (Type,"POLYGON"))
		itype = FT_POLYGON;
	else if (!stricmp (Type,"CIRCLE"))
		itype = FT_CIRCLE;
	else if (!stricmp (Type,"ROUTE"))
		itype = FT_ROUTE;
	else
		return 0;
	strcpy (Prefix,TAG);
	if ((pSC = strchr (Prefix,':')))
	{
		*pSC++ = 0;
		strcpy (UDI,pSC);
	}
	else
		*UDI = 0;
	if (!PickByRefno(0,Prefix,UDI,-1))
		return 0; 

	switch (itype)
	{
	case FT_CIRCLE:
		handle = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(FENCEHEADER));
		pFenceHeader = GlobalLock (handle);
		pFenceHeader->Type = itype;
		pFenceHeader->Offset = Offset;
		pFenceHeader->OffsetUnits = OffsetUnits;
		pFenceHeader->BaseOffset = GetBaseOffset (Offset,OffsetUnits);
		pFenceHeader->Point = *pPoint;
		pFenceHeader->hAccelerator = 0;
		pFenceHeader->NumPoints = 0;
		strcpy (pFenceHeader->Name,TAG);
		break;
	case FT_ROUTE:
	case FT_POLYGON:
		if (!GetPolyPnts ((LPPICKDATAHEADER)&PickList[0],FALSE,&NumPoints,&hPoly,TRUE))
			return 0;
		Points = GlobalLock (hPoly);
		handle = GSSiGlobAlloc (1574,GMEM_MOVEABLE,sizeof(FENCEHEADER)+NumPoints * sizeof(DPOINT));
		pFenceHeader = GlobalLock (handle);
		if (NumPoints == 2)
		{
			itype  = FT_CIRCLE;
			Offset = ldistpp (&Points[0],&Points[1]);
			pPoint = Points;
		}
		pFenceHeader->Type = itype;
		pFenceHeader->Offset = Offset;
		pFenceHeader->BaseOffset = GetBaseOffset (Offset,OffsetUnits);
		pFenceHeader->OffsetUnits = OffsetUnits;
		pFenceHeader->Point = *pPoint;
		pFenceHeader->hAccelerator = 0;
		pFenceHeader->NumPoints = NumPoints;
		strcpy (pFenceHeader->Name,TAG);
		pPoints = (LPDPOINT)(pFenceHeader+1);
		memmove (pPoints,Points,NumPoints * sizeof(DPOINT));
		GSSiGlobUlFree (&hPoly);
		break;
	}
	GlobalUnlock (handle);
	Fences[NumFences++] = handle;
	return handle;
}


void DestroyFence (HANDLE handle)
{
	int	i;
	LPFENCEHEADER	pFenceHeader;

	if (!handle)
	{
		for (i=0;i<NumFences;i++)
		{
			pFenceHeader = GlobalLock (Fences[i]);
			GSSiGlobFree (&pFenceHeader->hAccelerator);
			GSSiGlobUlFree (&Fences[i]);
		}
		NumFences = 0;
	}
	else
	{
		int	j=0;

		for (i=0;i<NumFences;i++)
		{
			if (handle == Fences[i])
			{
				pFenceHeader = GlobalLock (handle);
				GSSiGlobFree (&pFenceHeader->hAccelerator);
				GlobalUnlock (handle);
				GSSiGlobFree (&Fences[i]);
			}
			else
				Fences[j++]=Fences[i];
		}
		NumFences = j;
	}
	return;
}

void DestroyVehicles (int VPID)
{   
	short	i;
	
	if (!VPID)
	{
		for (i=0;i<NumVehicles;i++)
		{
			LPVEHLOCATION	pVehLoc;  
			pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[i]);
			GSSiGlobFree (&pVehLoc->hHist);
			GSSiGlobUlFree (&hVehicle[i]); 
		}
		NumVehicles = 0; 
		NumSortedVehicles = 0;
	}
	if (FirstVeh)
		return;
	if (VPID)
		DestroySavedScreen (&hVehSaveScreen[VPID-1],VehSaveScreenID[VPID-1]);
	else
		for (i=0;i<MAX_VIEWPORTS;i++) 
			DestroySavedScreen (&hVehSaveScreen[i],VehSaveScreenID[i]);

	return;
} 

BOOL PickVehicles (DPOINT PickPointBase,int PickAp,LPDOUBLE pNearDist)
{
	LPVEHLOCATION	pVehLoc;  
	UINT			iveh; 
	double			MinDist;
	int				MinWidth;
	DPOINT			NearPoint; 
	short			Item;  
	MNMXCORD		Rect;    
	POINT			FilePoint={0,0};   
	double			AZ[3];
	int				ii;
	
	if (!CurrentConfig || CurView->ID != *pCommandViewport)
		return FALSE;
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (pVehLoc->nLoc && !IsRectEmpty (&pVehLoc->LastDrawRect))
		{   
			MinWidth = min (RECTWIDTH(&pVehLoc->LastDrawRect),RECTHEIGHT(&pVehLoc->LastDrawRect));
			if (MinWidth > 0)
			{
				NearPoint = pVehLoc->Loc[pVehLoc->nLoc-1];
				MinDist = ldistp (NearPoint,PickPointBase);
				DBoundsInit (&Rect);
				AddDPointToMinMax (&NearPoint,&Rect);
				InflateBounds (&Rect,MinWidth * CurView->Scale/2);
				if (!PickAp || MinDist <= (PickApW + (MinWidth * CurView->BaseUnitsPerPixel)/2))
				{    
					AZ[0]=AZ[1]=AZ[2]=0;
					SetGlobalValue ("%PREFIX","VEHICLE"); 
					SetGlobalValue ("%UDI",pVehLoc->ID); 

					if ((Item = PickListAdd (-1,0,0,0,0,pVehLoc->Symbol,0,
								 1,0,MinDist,AZ,0,0,0,
								 NearPoint,NearPoint,NearPoint,NearPoint,0,0,&Rect,
								 FilePoint,FilePoint,FilePoint,1,NULL_ELEV,&NearPoint,&NearPoint)))
					{
						ii=1;
					//	_fstrcpy (PickList[Item-1].Prefix,"VEHICLE");
					//	_fstrcpy (PickList[Item-1].UDI,pVehLoc->ID);
					}
				}
			}
		}
		if (pVehLoc->nLoc && !IsRectEmpty (&pVehLoc->LastFlagRect))
		{   
			MinWidth = min (RECTWIDTH(&pVehLoc->LastFlagRect),RECTHEIGHT(&pVehLoc->LastFlagRect));
			if (MinWidth > 0)
			{
				NearPoint = ScreenPtToBasePt (RectMid (&pVehLoc->LastFlagRect));
				MinDist = ldistp (NearPoint,PickPointBase);
				DBoundsInit (&Rect);
				AddDPointToMinMax (&NearPoint,&Rect);
				InflateBounds (&Rect,MinWidth * CurView->Scale/2);
				if (!PickAp || MinDist <= (PickApW + (MinWidth * CurView->BaseUnitsPerPixel)/2))
				{    
					AZ[0]=AZ[1]=AZ[2]=0;
					SetGlobalValue ("%PREFIX","VEHICLE"); 
					SetGlobalValue ("%UDI",pVehLoc->ID); 

					if ((Item = PickListAdd (-1,0,0,0,0,pVehLoc->Symbol,0,
								 1,0,MinDist,AZ,0,0,0,
								 NearPoint,NearPoint,NearPoint,NearPoint,0,0,&Rect,
								 FilePoint,FilePoint,FilePoint,1,NULL_ELEV,&NearPoint,&NearPoint)))
					{
						ii=1;
					//	_fstrcpy (PickList[Item-1].Prefix,"VEHICLE");
					//	_fstrcpy (PickList[Item-1].UDI,pVehLoc->ID);
					}
				}
			}
		}
		GlobalUnlock (hVehicle[iveh]);
	} 
	return TRUE;             
}  

double SetVehSymAndColor (LPVEHLOCATION pVehLoc,LPDPOINT pLoc,LPBOOL pMoving)
{   time_t	Now=time(0), LocTime[MAXVEHLOC];
	short	nPnts=0, nSpeedSyms;                                               
	long	LOFMDF, TimeDiff; 
	UINT	i;
	DPOINT	Loc[MAXVEHLOC], AVLoc;  
	double	AZ = TWOPI, A,B, MAXDIF, Dist, Hours, Miles; 
	char	str[64];
	
	Now+=AVLAdjustTime;
	*pMoving = FALSE;
	*pLoc = pVehLoc->Loc[pVehLoc->nLoc-1]; 
	if (pVehLoc->ComputeSpeed || pVehLoc->ComputeAZ)
	{
		for (i=0;i<pVehLoc->nLoc;i++)
		{
			if (Now - pVehLoc->LocTime[i] <= pVehLoc->ComputeTimeSpan) 
			{
				LocTime[nPnts] = pVehLoc->LocTime[i];
				Loc[nPnts++] = pVehLoc->Loc[i];      
			}
		}
		if (!NotHeardFromSymbol && stricmp (pVehLoc->ID,"GPS"))
		{
			if (pVehLoc->StatusSymbol > 0)
				pVehLoc->Symbol = pVehLoc->StatusSymbol;
		}
		else
		switch (nPnts)
		{
			case 0:
				pVehLoc->Symbol = NotHeardFromSymbol;
				break;
			case 1:
				AZ = pVehLoc->AZ;
				break;
			default:
				if (LINFIT (Loc,nPnts,&A,&B,&MAXDIF,&LOFMDF))
				{   
					*pLoc = AverageDPoints (Loc,nPnts);
					Dist = GetBaseDist (&Loc[0],&Loc[nPnts-1]);
					if (!Dist)
						pVehLoc->Symbol = StoppedSymbol;
					else 
					{   
						TimeDiff = LocTime[nPnts-1] - LocTime[0]; 
						Hours = (double)TimeDiff / (double)3600; 
						Miles = ConvertDist (Dist,4);
						if (Hours)
							pVehLoc->Speed = Miles/Hours;
						else
							pVehLoc->Speed = 0; 
						if ((nSpeedSyms = GetGlobalLVal2 ("[%NUMSPEEDSYMBOLS]",0)))
						{   
							short	ispeed;
							char	SymName[34];
							
							for (ispeed = nSpeedSyms-1;ispeed>=0; ispeed--)
							{   
								sprintf (str,"[%%SPEEDSYMBOLSPEED(%i)]",ispeed+1);
								if (!ispeed || pVehLoc->Speed >= GetGlobalDVal2 (str,0))
								{
									sprintf (str,"[%%SPEEDSYMBOL(%i)]",ispeed+1); 
									GetGlobalCVal (str,SymName,"CIRCLE");
//									pVehLoc->Symbol = GetDictSymbolNumber (SymName); 
									if (ispeed)
										pVehLoc->AZ = AZ = getazd (&Loc[nPnts-2],&Loc[nPnts-1]);
									break;
								}
							}
						}
						else
						{
							if (pVehLoc->Speed < GetGlobalDVal2 ("[%MINVEHSPEED]",5))
								pVehLoc->Symbol = StoppedSymbol;
							else
							{   
								pVehLoc->AZ = AZ = getazd (&Loc[0],&Loc[nPnts-1]);
								pVehLoc->Symbol = MovingSymbol; 
								*pLoc = Loc[nPnts-1]; 
								*pMoving = TRUE;
							}
						}
					}
				}
				break;
		}
	}
	else
		pVehLoc->Speed = 0;
	return AZ;
}  

BOOL GetVehicleLoc (LPSTR VehID,LPDPOINT Loc,short VPID)
{   
	LPVEHLOCATION	pVehLoc;
	UINT	iveh;
	
	if (!TrackingStatus)
		return FALSE;
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!_fstricmp (pVehLoc->ID,VehID) && (!VPID || VPID == *pCommandViewport))
			goto GotVeh;
		GlobalUnlock (hVehicle[iveh]);
	}
	return FALSE;   
GotVeh:
	*Loc = pVehLoc->Loc[pVehLoc->nLoc-1];
	GlobalUnlock (hVehicle[iveh]);
    return TRUE;      
}

void DestroyDummyVehicles (void)
{
	LPDUMMYVEHICLE pDummyVehicle; 
	UINT	i;

	if (AVLTimer)
		KillTimer(hWndMain,AVLTimer);
	AVLTimer = 0;                               
	if (hDummyVehicles)
	{
		pDummyVehicle = (LPDUMMYVEHICLE)GlobalLock (hDummyVehicles); 
		for (i=0;i<nDummyVehicles;i++)  
		{
			GSSiGlobFree (&pDummyVehicle->hPoly);
			GSSiGlobFree (&pDummyVehicle->hMoveList);
			pDummyVehicle++;
		}
		nDummyVehicles = 0;
		GSSiGlobUlFree (&hDummyVehicles);
	}  
	DestroyVehicles(0);
	return; 
}  

BOOL OpenDummyVehicleFile(LPSTR FileName,long InDelay)
{   
	OFSTRUCT	OFStruct;
	HFILE		Fid, Fid2;
	LPDUMMYVEHICLE pDummyVehicle;    
	LPMOVELIST	pMoveList;
	char		str[260], MoveListFile[MAX_PATH], str2[70], Drive[8], Dir[256]; 
	LPSTR		pComma, pTAB1, pTAB2, pBS;
	long		StartTime;  
	static	UINT	Delay=0;
	
	StartTime = GetGlobalLVal2 ("[%AVLSTARTTIME]",0);
	if (StartTime)
		AVLAdjustTime = StartTime - time(0);
//	else
		AVLAdjustTime = 0;
	if (InDelay < 0)
	{ 
		if (!Delay)
			return FALSE;
		AVLFactor = labs (InDelay);
		KillTimer (hWndMain,AVLTimer);
		SetTimer(hWndMain, AVLTimer, Delay/AVLFactor, (FARPROC) 0);
		return TRUE; 
	} 
	DestroyDummyVehicles ();
	if (!FileName)
		return TRUE; 
	Delay = InDelay; 
	SetGlobalValueLong ("%VEHCOMPUTETIMESPAN",(InDelay*3)/1000);
	Fid = GSSiOpenFile (FileName,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
		return FALSE;
	_splitpath (OFStruct.szPathName,Drive,Dir,0,0);
	hDummyVehicles = GSSiGlobAlloc (1336,GHND,USHRT_MAX);  
	pDummyVehicle = (LPDUMMYVEHICLE)GlobalLock (hDummyVehicles);
	while (fgetstring (str,256,Fid))
	{
		if ((pComma = _fstrchr (str,'\t')))
		{
			*pComma = 0;
			_fstrncpy (pDummyVehicle->Image,pComma,128);
		}
		_fstrncpy (pDummyVehicle->Name,str,32);
		if (PickByRefno(0,"ROUTE",pDummyVehicle->Name,-1))
			if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&pDummyVehicle->nPnts,&pDummyVehicle->hPoly))
			{
				HPDPOINT	pPoint = (HPDPOINT)GlobalLock (pDummyVehicle->hPoly);
				
				pDummyVehicle->RouteLength = GetPolyLengthD (pPoint,pDummyVehicle->nPnts);
				SetVehicleLoc (pDummyVehicle->Name,*pPoint,0,0,0,TRUE,TRUE,0,0);
				GlobalUnlock (pDummyVehicle->hPoly);
			}
		sprintf (MoveListFile,"%s%s%s.txt",Drive,Dir,pDummyVehicle->Name); 
		if ((Fid2 = GSSiOpenFile (MoveListFile,&OFStruct,OF_READ)) != HFILE_ERROR)
		{   
			StartTime = 0;
			pDummyVehicle->hMoveList = GSSiGlobAlloc (1337,GHND,USHRT_MAX);
			pMoveList = (LPMOVELIST)GlobalLock (pDummyVehicle->hMoveList);
			while (fgetstring (str,64,Fid2))
			{   
				long	loc = GSSillseek (Fid2,0,1); 
				double	EndMP;
				
				if (fgetstring (str2,64,Fid2))
					EndMP = atof (str2);
				else
					EndMP = (pDummyVehicle->RouteLength * MFT)/5280;
				GSSillseek (Fid2,loc,0);
				if ((pTAB1 = _fstrchr (str,'\t')))
				{
					*pTAB1++ = 0;
					if ((pTAB2 = _fstrchr (pTAB1,'\t')))
					{
						*pTAB2++ = 0; 
						pMoveList->StartMilePoint = atof (str);  
						pMoveList->Value = atof (pTAB2);
						if (!_fstricmp (pTAB1,"speed")) 
						{
							pMoveList->Command = 1;
							pMoveList->EndMilePoint = EndMP;
							pMoveList->EndTime = StartTime + ((pMoveList->EndMilePoint - pMoveList->StartMilePoint) / pMoveList->Value) * 3600;
						}
						else
						{
							pMoveList->Command = 2; 
							pMoveList->EndMilePoint = pMoveList->StartMilePoint;
							pMoveList->EndTime = StartTime + pMoveList->Value * 60;
						}
						pMoveList->StartTime = StartTime;
						StartTime = pMoveList->EndTime + 1;
						pDummyVehicle->nMoveList++;
						pMoveList++;
					}
				}
			}
			GlobalUnlock (pDummyVehicle->hMoveList);
			GSSiClose (Fid2);
		}
		else
		{
			sprintf (str,"%s%shistory\\%s.txt",Drive,Dir,pDummyVehicle->Name);
			if (ExistFile (str))
				_fstrcpy (pDummyVehicle->HistoryFile,str);
		}
		pDummyVehicle++;  
		nDummyVehicles++;
	}
	GlobalUnlock (hDummyVehicles);
	GSSiClose (Fid);
	AVLTimer = 6;
	SetTimer(hWndMain, AVLTimer, Delay, (FARPROC) 0); 
	time (&AVLStartTime); 
	return TRUE;
}

BOOL ComputeDummyVehicleLocations (time_t AVLTime)
{
	LPDUMMYVEHICLE pDummyVehicle;  
	DPOINT	DPoint;  
	double	Dist;
	UINT	i,j; 
	char	str[130];
	OFSTRUCT	OFStruct;
	
	if (!hDummyVehicles)
		return FALSE;    
	AVLTime *= GetGlobalDVal2 ("[%AVLSPEEDFACTOR]",1.0);
	AVLTime += GetGlobalLVal2 ("[%AVLSTARTTIME]",0);
	pDummyVehicle = (LPDUMMYVEHICLE)GlobalLock (hDummyVehicles); 
	for (i=0;i<nDummyVehicles;i++,pDummyVehicle++) 
	{   
/*		if (pDummyVehicle->hMoveList && pDummyVehicle->hPoly)
		{
			LPMOVELIST	pMoveList=(LPMOVELIST)GlobalLock (pDummyVehicle->hMoveList);
			
			for (j=0;j<pDummyVehicle->nMoveList;j++,pMoveList++)
			{
				if (AVLTime >= pMoveList->StartTime && AVLTime <= pMoveList->EndTime)
				{
					HPDPOINT	pPoly = (HPDPOINT)GlobalLock (pDummyVehicle->hPoly);
					
					Dist = pMoveList->StartMilePoint +
						   (pMoveList->EndMilePoint - pMoveList->StartMilePoint) * (AVLTime - pMoveList->StartTime) / (pMoveList->EndTime - pMoveList->StartTime);
					Dist *= (5280 * FTM);		
					DPoint = PointAtDistOnPoly (pPoly,pDummyVehicle->nPnts,Dist); 
					GlobalUnlock (pDummyVehicle->hPoly);
					SetVehicleLoc (pDummyVehicle->Name,DPoint,0,0);  
					break;
				}
			}
			GlobalUnlock (pDummyVehicle->hMoveList); 
		}*/
		
		if (*pDummyVehicle->HistoryFile)
		{
			HFILE	Fid = GSSiOpenFile (pDummyVehicle->HistoryFile,&OFStruct,OF_READ); 
			time_t	LastTime=LONG_MAX, Time; 
			HANDLE	hDLT;
			LPSTR	pData;
			DPOINT	Point1, Point2; 
			char	CPoint[32];
			
			while (fgetstring (str,128,Fid))
			{   
				if (!_fstrnicmp (str,">EndHistory:",12))
					break;
				if ((pData = _fstrchr (str,':')))
				{
					pData++;
					ProcessDelimTextHeader("VEHID,VEHSEQ,VEHTIME,VEHLAT,VEHLON,VEHSPEED",0,Fid,&hDLT,0);
				    GetDelimTextData(pData,hDLT); 
					GSSiGlobFree (&hDLT); 
					_fstrcpy (str,"$CLK([VEHTIME],1)");
					ExpandText (str);
					Time = atol (str); 
					_fstrcpy (CPoint,"[VEHLON]");
					ExpandText (CPoint);
					Point2.x = - (dread (CPoint,3) + dread (&CPoint[3],7)/60);
					_fstrcpy (CPoint,"[VEHLAT]");
					ExpandText (CPoint);
					Point2.y =   (dread (CPoint,2) + dread (&CPoint[2],7)/60);
					if (AVLTime >= LastTime && AVLTime <= Time) 
					{   
						double	Dist, AZ;
						ConvertCoord (&Point1,2,1);
						ConvertCoord (&Point2,2,1);  
						Dist = ldistp (Point1,Point2) * ((double)AVLTime - LastTime)/((double)Time - LastTime); 
						AZ = getazd (&Point1,&Point2);
						Point2 = dnewpt (Point1,AZ,Dist);
						SetVehicleLoc (pDummyVehicle->Name,Point2,0,0,0,TRUE,TRUE,Time,0);  
						break;
					} 
					LastTime = Time; 
					Point1 = Point2;
				}
			}
			GSSiClose (Fid);
		}
	} 
	GlobalUnlock (hDummyVehicles);
	return TRUE;
}  

short GetVehSymbolFromStatus (int Status)
{   
	static	BOOL First=TRUE;
	char	SymName[16];  
	short	i;
	
	if (Status < 0)
		return Status;
	if (First)
	{
		First = FALSE;
		for (i=0;i<8;i++)
		{
			sprintf (SymName,"STATUS%i",i+1);
			StatusSymbol[i] = GetDictSymbolNumber (SymName);
		}
	}
	if (Status < 1 || Status > 8)
		return 0;
	return (StatusSymbol[Status-1]);
}

BOOL SetVehicleStatus (LPSTR VehID, int Status,BOOL IsFenceStatus,BOOL Redisplay)
{
	UINT	iveh;
	short	iSymbol;  
	LPVEHLOCATION	pVehLoc;  
	
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!_fstricmp (pVehLoc->ID,VehID))  
		{
			if (IsFenceStatus)
				pVehLoc->FenceStatus = Status;
			else
			{
				pVehLoc->Status = Status; 
				pVehLoc->StatusSymbol = GetVehSymbolFromStatus (Status);
			}
			if (Redisplay)
			{
				VehicleStatusChanged = TRUE;
 				UpdateVehicleStatusDlg ();
			}
			GlobalUnlock (hVehicle[iveh]);
			return TRUE;
		}
		GlobalUnlock (hVehicle[iveh]);
	}             
	return FALSE;
}

BOOL SetVehicleColor (LPSTR VehID, COLORREF Color)
{
	UINT	iveh;
	short	iSymbol;  
	LPVEHLOCATION	pVehLoc;  
	
	if (NumVehicles)
	{
		for (iveh=0;iveh<NumVehicles;iveh++)
		{
			pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
			if (!_fstricmp (pVehLoc->ID,VehID))  
			{
				pVehLoc->Color = Color; 
				GlobalUnlock (hVehicle[iveh]); 
				return TRUE;
			}
			GlobalUnlock (hVehicle[iveh]);
		}             
		UpdateVehicleStatusDlg ();
	}
	return FALSE;
}

BOOL SetVehicleSpeed (LPSTR VehID, double Speed)
{
	UINT	iveh;
	short	iSymbol;  
	LPVEHLOCATION	pVehLoc;  
	
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!_fstricmp (pVehLoc->ID,VehID))  
		{
			pVehLoc->Speed = Speed; 
			GlobalUnlock (hVehicle[iveh]); 
			return TRUE;
		}
		GlobalUnlock (hVehicle[iveh]);
	}             
	return FALSE;
}

BOOL GetVehicleDriver (LPSTR VehID,LPSTR Driver)
{
	UINT	iveh;
	LPVEHLOCATION	pVehLoc;

	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!stricmp (pVehLoc->ID,VehID))  
		{
			strcpy (Driver,pVehLoc->Driver);
			GlobalUnlock (hVehicle[iveh]);
			return TRUE;
		}
		GlobalUnlock (hVehicle[iveh]);
	}
	*Driver = 0;
	return FALSE;
}

BOOL SetVehicleDriver (LPSTR VehID,LPSTR Driver)
{
	UINT	iveh;
	LPVEHLOCATION	pVehLoc;

	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!stricmp (pVehLoc->ID,VehID))  
		{
			strcpy (pVehLoc->Driver,Driver);
			GlobalUnlock (hVehicle[iveh]);
			return TRUE;
		}
		GlobalUnlock (hVehicle[iveh]);
	}
	return FALSE;
}

double GetVehicleSpeed (LPSTR VehID)
{
	UINT	iveh;
	short	iSymbol;  
	LPVEHLOCATION	pVehLoc;
	double	Speed;
	
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!_fstricmp (pVehLoc->ID,VehID))  
		{
			Speed = pVehLoc->Speed; 
			GlobalUnlock (hVehicle[iveh]); 
			return Speed;
		}
		GlobalUnlock (hVehicle[iveh]);
	}             
	return -1;
}
double GetVehicleHeading (LPSTR VehID)
{
	UINT	iveh;
	short	iSymbol;  
	LPVEHLOCATION	pVehLoc;
	double	Heading;
	
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!_fstricmp (pVehLoc->ID,VehID))  
		{
			Heading = pVehLoc->AZ; 
			GlobalUnlock (hVehicle[iveh]); 
			return Heading;
		}
		GlobalUnlock (hVehicle[iveh]);
	}             
	return 0;
}

BOOL ClearVehicleHistory (int iTime)
{
	UINT	iveh;
	LPVEHLOCATION	pVehLoc;  

	if (iTime < 0)
		VehicleTrackMaxTime = abs (iTime);
	else
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (iTime == INT_MAX)
		{
			pVehLoc->nLoc = 0;
			pVehLoc->NumHist = 0;
			GSSiGlobFree (&pVehLoc->hHist);
		}
		else if (pVehLoc->nLoc)
		{
			if (!iTime)
			{
				pVehLoc->LocTime[0] = pVehLoc->LocTime[pVehLoc->nLoc-1];
				pVehLoc->Loc[0] = pVehLoc->Loc[pVehLoc->nLoc-1];
				pVehLoc->Speeds[0] = pVehLoc->Speeds[pVehLoc->nLoc-1];
				pVehLoc->Heading[0] = pVehLoc->Heading[pVehLoc->nLoc-1];
				pVehLoc->nLoc = 1;
				if (pVehLoc->NumHist)
				{
					LPDPOINT	pHistPoint=GlobalLock (pVehLoc->hHist);
					LPLONG		pHistTime=(LPLONG)(pHistPoint + MAX_VEHICLE_TRACK_POINTS);

					pHistPoint[0] = pHistPoint[pVehLoc->NumHist-1];
					pHistTime[0] = pHistTime[pVehLoc->NumHist-1];
					pVehLoc->NumHist = 1;
					GlobalUnlock (pVehLoc->hHist);
				}
			}
			else
			{
				int	i = 0;
				while (i < pVehLoc->nLoc)
				{
					if (pVehLoc->LocTime[i] > iTime)
						break;
					i++;
				}
				if (i == pVehLoc->nLoc)
					i--;
				memmove (&pVehLoc->Loc[0],&pVehLoc->Loc[i],(pVehLoc->nLoc-i)*sizeof(DPOINT));
				memmove (&pVehLoc->LocTime[0],&pVehLoc->LocTime[i],(pVehLoc->nLoc-i)*sizeof(time_t));
				memmove (&pVehLoc->Speeds[0],&pVehLoc->Speeds[i],(pVehLoc->nLoc-i)*sizeof(short));
				memmove (&pVehLoc->Heading[0],&pVehLoc->Heading[i],(pVehLoc->nLoc-i)*sizeof(short));
				pVehLoc->nLoc -= i;
			}
		}

		GlobalUnlock (hVehicle[iveh]);
	}             
	return TRUE;
}

BOOL GetVehicleHistory (LPSTR VehID, LPSTR VarName,BOOL Clear)
{
	UINT	iveh;
	short	iSymbol;  
	LPVEHLOCATION	pVehLoc;  
	
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!_fstricmp (pVehLoc->ID2,VehID))  
		{
			if (VarName)
			{
				HANDLE	hStr=GSSiGlobAlloc (1697,GMEM_MOVEABLE,USHRT_MAX*4);
				LPSTR	pStr = GlobalLock (hStr);
				UINT	i;

				itoa (pVehLoc->nLoc,pStr,10);
				if (pVehLoc->nLoc)
				{
					sprintf (strchr (pStr,0),"!%ld|%.1f|%.1f",pVehLoc->LocTime[0],pVehLoc->Loc[0].x,pVehLoc->Loc[0].y);
					for (i=1;i<pVehLoc->nLoc;i++)
						sprintf (strchr (pStr,0),"!%ld|%.1f|%.1f",pVehLoc->LocTime[i]-pVehLoc->LocTime[0],
																  pVehLoc->Loc[i].x-pVehLoc->Loc[0].x,
																  pVehLoc->Loc[i].y-pVehLoc->Loc[0].y);
				}
				SetGlobalValue (VarName,pStr);
				GSSiGlobUlFree (&hStr);
				if (Clear)
					pVehLoc->nLoc = 0;
			}
			else
				pVehLoc->nLoc = 0;
			GlobalUnlock (hVehicle[iveh]); 
			return TRUE;
		}
		GlobalUnlock (hVehicle[iveh]);
	}             
	return FALSE;
}

BOOL SetVehicleTrackColor (LPSTR VehID, COLORREF Color)
{
	UINT	iveh;
	short	iSymbol;  
	LPVEHLOCATION	pVehLoc;  
	
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!_fstricmp (pVehLoc->ID2,VehID))  
		{
			pVehLoc->TrackColor = Color; 
			GlobalUnlock (hVehicle[iveh]); 
			return TRUE;
		}
		GlobalUnlock (hVehicle[iveh]);
	}             
	return FALSE;
}

void RemoveVehicleFromSortList (int iveh)
{
	int	n=0, i;

	for (i=0;i<NumSortedVehicles;i++)
		if (VehOrder[i] != iveh)
			VehOrder[n++] = VehOrder[i];
	NumSortedVehicles = n;
	return;
}

void AddVehicleToSortList (int iveh)
{
	int	i=NumSortedVehicles, st;
	LPVEHLOCATION	pVehLocNew = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]), pVehLoc;
	
	VehOrder[i] = iveh;
	while (i--)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[VehOrder[i]]);
		if (!stricmp (pVehLoc->ID,"GPS"))
			st = -1;
		else
			st = stricmp (pVehLocNew->Desc,pVehLoc->Desc);
		GlobalUnlock (hVehicle[VehOrder[i]]);
		if (st > 0)
			break;
		VehOrder[i+1] = VehOrder[i];
		VehOrder[i] = iveh;
	}
	GlobalUnlock (hVehicle[iveh]);
	NumSortedVehicles++;
	return;
}

BOOL RemoveVehicle (LPSTR VehID)
{
	UINT	iveh,i;  
	LPVEHLOCATION	pVehLoc;  
	
	VehicleStatusChanged = TRUE;
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!_fstricmp (pVehLoc->ID,VehID))  
		{
			RemoveVehicleFromSortList (iveh);
			GSSiGlobFree (&pVehLoc->hHist);
			goto DoRemove;
		}
		GlobalUnlock (hVehicle[iveh]);
	} 
	return FALSE;
DoRemove:
	GSSiGlobUlFree (&hVehicle[iveh]);
	for (i=iveh;i<NumVehicles-1;i++)
		hVehicle[i] = hVehicle[i+1];
	NumVehicles--;
//	UpdateAllVehicles(TRUE); 
	return TRUE;
}

BOOL DefineVehicle (LPSTR VehID,LPSTR ID2,LPSTR VehDesc,LPSTR SymName,LPSTR Color,LPSTR Size,LPSTR ComputeTimeSpan)
{
	UINT	iveh,ii;  
	LPVEHLOCATION	pVehLoc;  
	char	txt[64];  
	long	TimeSpan=atol (ComputeTimeSpan);
	
	if (!TimeSpan)
		TimeSpan = 7;
	VehicleStatusChanged = TRUE;
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!_fstricmp (pVehLoc->ID,VehID))  
		{
			RemoveVehicleFromSortList (iveh);
			goto SetVals;
		}
		GlobalUnlock (hVehicle[iveh]);
	}   
	iveh = NumVehicles;
	if (NumVehicles >= MAXVEHICLES)
		MessageBox (0,"Vehicle limit reached",0,MB_ICONEXCLAMATION);
	else
		NumVehicles++;
	hVehicle[iveh] = GSSiGlobAlloc (1338,GHND,sizeof(VEHLOCATION));  
	pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);   
SetVals:
	_fstrcpy (pVehLoc->ID,VehID);
	pVehLoc->Symbol = pVehLoc->StatusSymbol = GetDictSymbolNumber (SymName);     
	pVehLoc->Color = GetColorFromName (Color);  
	pVehLoc->TrackColor = RGB(0,255,0);
	pVehLoc->Size[0] = atol(Size);
	pVehLoc->Size[1] = pVehLoc->Size[0] / 2; 
	pVehLoc->ComputeTimeSpan = TimeSpan;
	_fstrcpy (pVehLoc->Desc,VehDesc); 
	strcpy (pVehLoc->ID2,ID2);
    if (*VehDesc == ' ')
		ii=1;
	pVehLoc->ComputeAZ = TRUE; 
	GlobalUnlock (hVehicle[iveh]);
	AddVehicleToSortList (iveh);
	return TRUE;
} 

MNMXCORD GetVehicleBounds (LPSTR VehID)
{
	MNMXCORD	Bounds; 
	UINT	iveh;
	LPVEHLOCATION	pVehLoc;  
	
	DBoundsInit (&Bounds);
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if ((!_fstricmp (VehID,"ALL") || !_fstricmp (pVehLoc->ID,VehID)) && pVehLoc->nLoc && pVehLoc->Display)
		{  
			AddDPointToMinMax (&pVehLoc->Loc[pVehLoc->nLoc-1],&Bounds);
		}
		GlobalUnlock (hVehicle[iveh]);
	}             
	
	return Bounds;
}

BOOL FlashVehicle (LPSTR VehID)
{
	UINT	iveh,ii;
	LPVEHLOCATION	pVehLoc; 
	RECT	Rect;
	LPVIEWPORT	SaveVP = CurView;
	
	if (InDisplayProcessing)
		return FALSE;
	SetViewport (*pCommandViewport);
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_SCREENMODE);  
	SelectClipRgn (CurView->hDC,0);
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (pVehLoc->Display && !stricmp (pVehLoc->ID,VehID))
		{
			if (!IsRectEmpty (&pVehLoc->Rect))
			{
 				if (!InvertRect (CurView->hDC,&pVehLoc->Rect))
					ii=1;
			}
			if (hWndVehStatus)
				SendMessage (hWndVehStatus,WM_COMMAND,IDC_FLASHVEHICLE,iveh);
		}
		GlobalUnlock (hVehicle[iveh]);
	}             
	RestoreDC (CurView->hDC,-1);
	CurView = SaveVP;
	
	return TRUE;
}

BOOL SetVehicleLoc (LPSTR VehID,DPOINT DPoint,int Speed,int Heading,int Status,BOOL DisplayVehicles,BOOL ValidCoord,int VehTime,BOOL FromHist)
{   
	HFILE		Fid;
	OFSTRUCT	OFStruct;    
	char		str[260], file[128],txt[64]; 
	DPOINT		CLPoint,LastPt;
	double		AZ, Length, seconds=0;  
	short		iveh;
	COLORREF	Color;
	LPVEHLOCATION	pVehLoc;  
	LPSTR		pLoc, pEnd;
	char		IniName[128]="[%DL]geomastr.ini", SymDefaultDef[128];
	static		BOOL	First=TRUE;
	HPDPOINT	HistPoints;
	LPLONG		HistTimes;
	
	if (!VehTime)
		VehTime = time(0);
	if (First)
	{
		GetGlobalCVal ("[%NOTHEARDFROMSYMBOL]",str,"SQUARE");
		NotHeardFromSymbol = GetDictSymbolNumber (str);
		GetGlobalCVal ("[%STOPPEDSYMBOL]",str,"CIRCLE");
		StoppedSymbol = GetDictSymbolNumber (str);
		GetGlobalCVal ("[%MOVINGSYMBOL]",str,"ARROW1");
		MovingSymbol = GetDictSymbolNumber (str);
		GetGlobalCVal ("[%DIRECTIONSYMBOL]",str,"DIRECTIONLINE");
		DirectionSymbol = GetDictSymbolNumber (str);
		First = FALSE;
	}	
	if (VehicleInReplayList (VehID) && !FromHist)
		return TRUE;
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!_fstricmp (pVehLoc->ID,VehID))
			goto GotVeh;
		GlobalUnlock (hVehicle[iveh]);
	}             
	hVehicle[iveh] = GSSiGlobAlloc (1338,GHND,sizeof(VEHLOCATION));  
	pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
	_fstrcpy (pVehLoc->ID,VehID); 
	_fstrcpy (pVehLoc->ID2,VehID); 
	GetGlobalCVal ("[%DEFAULTVEHICLEDEF]",SymDefaultDef,"CIRCLE|Red|10|[%VEHID]");    
	ExpandText (IniName);
	GetPrivateProfileString ("VEHICLES",VehID,SymDefaultDef,str,128,IniName); 
	pLoc = str;
	pEnd = _fstrchr (pLoc,'|');
	*pEnd++ = 0;
	pVehLoc->Symbol = GetDictSymbolNumber (pLoc);     
	pLoc = pEnd;
	pEnd = _fstrchr (pLoc,'|');
	*pEnd++ = 0;
	pVehLoc->Color = GetColorFromName (pLoc);   
	pVehLoc->TrackColor = RGB(255,0,0);
	pLoc = pEnd;
	pEnd = _fstrchr (pLoc,'|');
	*pEnd++ = 0;
	pVehLoc->Size[0] = atol(pLoc);
	pVehLoc->Size[1] = pVehLoc->Size[0] / 2;     
	pLoc = pEnd;
	_fstrcpy (pVehLoc->Desc,pLoc);  
	ExpandText (pVehLoc->Desc);
	if (!_fstricmp (pVehLoc->ID,"GPS"))
	{   
		pVehLoc->ComputeAZ = TRUE; 
		pVehLoc->Display = TRUE; 
		pVehLoc->ComputeSpeed = TRUE; 
		pVehLoc->Color = RGB(255,0,0);
		pVehLoc->Size[0] = GetGlobalLVal2 ("[%GPSSYMBOLSIZE]",8); 
		pVehLoc->ComputeTimeSpan =  GetGlobalLVal2("[%VEHCOMPUTETIMESPAN]",7);
	}
	else 
	{
		pVehLoc->StatusSymbol = pVehLoc->Symbol;
		pVehLoc->ComputeAZ = TRUE;
		pVehLoc->Display = TRUE;
	} 
	if (NumVehicles >= MAXVEHICLES)
		MessageBox (0,"Vehicle limit reached",0,MB_ICONEXCLAMATION);
	else
	{
		NumVehicles++;
		AddVehicleToSortList (iveh);
	}

GotVeh:
	if (pVehLoc->nLoc == MAXVEHLOC)                                           
	{
		_fmemmove (pVehLoc->Loc,&pVehLoc->Loc[1],(MAXVEHLOC-1)*sizeof(DPOINT));
		_fmemmove (pVehLoc->LocTime,&pVehLoc->LocTime[1],(MAXVEHLOC-1)*sizeof(time_t));
		_fmemmove (pVehLoc->Speeds,&pVehLoc->Speeds[1],(MAXVEHLOC-1)*sizeof(short));
		_fmemmove (pVehLoc->Heading,&pVehLoc->Heading[1],(MAXVEHLOC-1)*sizeof(short));
	}
	else
		pVehLoc->nLoc++;
	if (ValidCoord)
	{
		if (!pVehLoc->NumHist)
		{
			pVehLoc->hHist = GSSiGlobAlloc (1571,GMEM_MOVEABLE,MAX_VEHICLE_TRACK_POINTS*sizeof(DPOINT)+MAX_VEHICLE_TRACK_POINTS*sizeof(long));
		}
		HistPoints = GlobalLock (pVehLoc->hHist);
		HistTimes = (LPLONG)(HistPoints + MAX_VEHICLE_TRACK_POINTS);
		if (pVehLoc->NumHist < MAX_VEHICLE_TRACK_POINTS)
		{
			HistTimes[pVehLoc->NumHist] = VehTime;
			HistPoints[pVehLoc->NumHist++] = DPoint;
		}
		else
		{
			memmove (HistPoints,&HistPoints[1],(MAX_VEHICLE_TRACK_POINTS-1)*sizeof(DPOINT));
			memmove (HistTimes,&HistTimes[1],(MAX_VEHICLE_TRACK_POINTS-1)*sizeof(long));
			HistPoints[MAX_VEHICLE_TRACK_POINTS-1] = DPoint;
			HistTimes[MAX_VEHICLE_TRACK_POINTS-1] = VehTime;
		}
		GlobalUnlock (pVehLoc->hHist);
	}
	pVehLoc->Loc[pVehLoc->nLoc-1] = DPoint;
	pVehLoc->LocTime[pVehLoc->nLoc-1] = VehTime;
	pVehLoc->Speeds[pVehLoc->nLoc-1] = Speed;
	if (!_fstricmp (pVehLoc->ID,"GPS"))
	{   
		dpointtoa (txt,&DPoint);
		SetGlobalValue("%GPSLOCATION",txt);  
	}
	GlobalUnlock (hVehicle[iveh]);
//	if (DisplayVehicles)
//		UpdateAllVehicles(TRUE); 
	if (!InDisplayProcessing && !HavePendingDisplay && DisplayVehicles)
	{
		HavePendingDisplay = TRUE;
     	PostMessage(hWndMain, WM_COMMAND, IDM_DISPLAY_VEHICLES, 0L);  
    }
	return TRUE;
}

void SavePreVehicleVPDisplay (void)
{
	HDC hDC = ScreenBufferDC (CurView->hWnd,CurView->hDC);
	RECT	rect=CurView->ScreenRect;
	
	if (BufferedScreen)
		GetClientRect(hWndMain,&rect);

	DestroySavedScreen (&hVehSaveScreen[CurView->ID-1],VehSaveScreenID[CurView->ID-1]);
	hVehSaveScreen[CurView->ID-1] = SaveScreen2 (hWndMain,hDC,rect,CurView,&VehSaveScreenID[CurView->ID-1]);
	return;
}

BOOL DisplayAllVehicles (BOOL MainVP,BOOL Force)
{   
	short	iview; 
	UINT	TimerInc; 
	time_t	CurTime; 
	LPVIEWPORT	SaveVP=CurView; 

	BOOL	rtn=FALSE;

	if (!GetGlobalLVal2 ("[%ALLOWVEHICLEDISPLAY]",TRUE) || !CurrentConfig)
		goto Exit;
	if (!NumVehicles && !Force)
		goto Exit;
//	return FALSE;
	if (BlockVehicleDisplay)
	{
		BlockVehicleDisplay++;
		goto Exit;
	}
	if (InDisplayProcessing)
		goto Exit;

	if (MainVP)
		SetViewport (*pCommandViewport);  
	if (FirstVeh)
	{   
		_fmemset (hVehSaveScreen,0,sizeof(hVehSaveScreen));
		FirstVeh = FALSE;
	}
	//if (!hVehSaveScreen[CurView->ID-1])
	SavePreVehicleVPDisplay ();
    SelectVisList (FALSE);
	if (MainVP)
		rtn = DisplayAllVehicles2 (nAVLVP,AVLViewports);
	else
		rtn = DisplayAllVehicles2 (1,&SaveVP);
	HavePendingDisplay = FALSE;
Exit: 
	SetCurView ( SaveVP);
	return rtn;
}

BOOL ShowBufferedScreen (BOOL Display,BOOL ResetDC,int VPID,LPRECT pUpdateRect)
{
	if (BufferedScreen && !MemMap)
	{
		HDC		hDC, hDCBuf;
		RECT	Rect; 
		HBITMAP	hBitmap, hTempBM; 
		short	i,ii,iview;   
		HRGN	hRgnMain;
		POINT	Point;

        hDC  = GetDC (hWndMain);
		SetDisplayMode (hDC, GF_SCREENMODE); 
		if (VPID)
			Rect = pViewports[VPID-1]->ScreenRect;
		else
			GetClientRect (hWndMain,&Rect);
		if (!IsRectEmpty (&PromptRect))
			SubtractRect (&Rect,&Rect,&PromptRect);
		hRgnMain = CreateRectRgnIndirect (&Rect);  
		for (i=0;i<*pNumViewports;i++) 
		{
			if (pViewports[i]->hWndDlg)
			{
				RECT	Rect2;
				HRGN	hRgn2;
				int		TypeRegion;

				GetWindowRect (pViewports[i]->hWndDlg,&Rect2);
				ScreenRectToClientRect (hWndMain,&Rect2);
				hRgn2 = CreateRectRgnIndirect (&Rect2);  
				TypeRegion = CombineRgn (hRgnMain,hRgnMain,hRgn2,RGN_DIFF);
				GSSiDeleteObject (&hRgn2);
			}
		}
		hDCBuf = hDCScreenBuffer;//ScreenBufferDC (CurView->hWnd,hDC);  
		for (iview=0;iview<*pNumViewports;iview++)  
			pViewports[iview]->hDC = hDCBuf;    
	  	SelectClipRgn (hDC,hRgnMain);
        if (Display)
		{
			if (pUpdateRect)
				ii=BitBlt(hDC, pUpdateRect->left, pUpdateRect->top,
							 RECTWIDTH(pUpdateRect),RECTHEIGHT(pUpdateRect),
               				 CurView->hDC,
               	  			 pUpdateRect->left,pUpdateRect->top, SRCCOPY);  
			else
				ii=BitBlt(hDC, 0, 0, Rect.right-Rect.left+1,
                             Rect.bottom-Rect.top+1,
               				 CurView->hDC,
               	  			 Rect.left,Rect.top, SRCCOPY);  
		}
		SelectClipRgn (hDC,0);
		DeleteObject (hRgnMain);
		if (ResetDC)
			for (i=0;i<*pNumViewports;i++) 
			{
				pViewports[i]->hDC = hDC;  
				if (pViewports[i]->hWndDlg)
				{
					//SendMessage(pViewports[i]->hWndDlg, GSSI_REPOSITION,1, (LPARAM)pViewports[i]); 
					ShowWindow (pViewports[i]->hWndDlg,SW_SHOW);
					InvalidateRect (pViewports[i]->hWndDlg,0,TRUE);
				}
			}
        ReleaseDC (hWndMain,hDC); 
        DisplayCycle++;  
//		if (Display)
//			UpdateVehicleStatusDlg ();
		DisplayLenSocketBuffer (-1);
/*		GetCursorPos (&Point); 
		SetCursorPos (Point.x,Point.y); 
     	PostMessage (hWndMain,WM_MOUSEMOVE,0,MAKELPARAM (Point.x,Point.y));*/
		if (!pUpdateRect)
			DisplayAllToolbars  (1);
       	RedrawActiveFunctions(GF_REDRAW);  
		return TRUE;
	}
	return FALSE;
}

void UpdateVehicleStatusDlg (void)
{
	if (BlockVehicleDisplay)
		return;
	if (hWndVehStatus && VehicleStatusChanged)
    	PostMessage(hWndVehStatus, WM_COMMAND, IDC_UPDATESTATUS, 0); 
	//DisplayAllVehicles (TRUE);
	if (!FirstVeh)
		UpdateAllVehicles (TRUE);
	return;
}

BOOL DisplayVehicleResponseArea (DPOINT Point,int Status)
{
	int	idesc = GetSymbolNum ("RESPONSEAREA");
	int	nPnts;
	HANDLE	hCoords;
	double	Radius=GetGlobalDVal2 ("[%RESPONSERADIUS]",10*5280*FTM);
	LPDPOINT	Points;

	if (/*Status != 6 ||*/ !GetGlobalBVal2 ("[%DISPLAYRESPONSEAREAS]",FALSE))
		return FALSE;
	hCoords = CreateCirclePoly (Point,Radius,&nPnts,0);
    Points = (HPDPOINT)GlobalLock (hCoords);
	GWPolygonD (CurView->hDC,Points,nPnts,1,0,idesc,FALSE,TRUE,0);
	GSSiGlobUlFree (&hCoords);
	return TRUE;
}

BOOL ClearVehicles (void)
{
	if (!CurView)
		return FALSE;
	RestoreScreen2 (CurView->hDC, hVehSaveScreen[CurView->ID-1],
	    					  VehSaveScreenID[CurView->ID-1],FALSE);
	ClearFlags ();
	return TRUE;
}

BOOL DisplayAllVehicles2 (short nVP,LPVIEWPORT *pVP)
{   
	double		AZ=0;  
	long		startseg; 
	DPOINT		ProjectedLoc;
	short		iveh, isveh, i;
	double		size;
	LPVIEWPORT	SaveVP = CurView; 
	COLORREF	VehTextColor=GetGlobalLVal2("[%VEHTEXTCOLOR]",0);
	LPVEHLOCATION	pVehLoc;
	LPSTR		Txt;  
	double		TrackBorder, TrackBorderPCT; 
	char		AVLTrackVehicle[32], NoName[8]="%%%"; 
	BOOL		InBounds,DisplayVehicles=FALSE;
	MNMXCORD	SaveBounds;
	BOOL		Moving=FALSE, InWindow;  
	DPOINT		MidPoint;
	double		MPAZ, AZDiff;  
	short		DestinationSymbol = GetSymbolNum ("DESTINATION"); 
	double		DestSize=GetGlobalDVal2 ("[%DESTINATIONSYMBOLSIZE]",25);
	BOOL		ShowAllVehTrails = GetGlobalBVal2 ("[%SHOWVEHICLETRACKS]",FALSE);
	POINT		ProjectedLocScreen;
	double		AZ2=0; 
	char		TempTxt[32];
	int			Symbol;
	static		int	VHCircle=0;
	COLORREF	Color;
	//HDC			SavehDC;
	HDC	hDC,hDCMain = GetDC (CurView->hWnd);
	static		BOOL		debug=FALSE;
	int			ii;
	
	if (!GetGlobalLVal2 ("[%ALLOWVEHICLEDISPLAY]",TRUE))
	{
		ReleaseDC (CurView->hWnd,hDCMain);
		return FALSE;
	}
	if (BlockVehicleDisplay)// || HaveMeterPrompts())
	{
		ReleaseDC (CurView->hWnd,hDCMain);
		BlockVehicleDisplay++;
		return FALSE;
	}
	if ((BufferedScreen && HaveMeterPrompts ()) || hLastBox || InVehicleDisplay || InDisplayProcessing)
	{
		ReleaseDC (CurView->hWnd,hDCMain);
		return FALSE;
	}
	ClearFlags ();
	InVehicleDisplay = TRUE;
	//SavehDC = CurView->hDC;
	//if (BufferedScreen)
	//	CurView->hDC = ScreenBufferDC (CurView->hWnd,CurView->hDC);  
	{
		
		if (MemMap)
    		hDC = hdcMemMap; 
   		else if (Printing)
    		hDC = CurView->hDC;
    	else
    		hDC = hDCMain;	
		//ClearMeterPrompts (hDC);
		hDC = ScreenBufferDC (CurView->hWnd,hDC);
		for (i=0;i<*pNumViewports;i++)  
			pViewports[i]->hDC = hDC;    
	}

	if (NumVehicles)
	{	
		GetGlobalCVal ("[%AVLFOLLOW]",AVLTrackVehicle,0);
		if (HaveMeterPrompts ())
		{
			RECT	rect, intrect;

			if (BufferedScreen)
				ClearMeterPrompts (CurView->hDC);
			else if (GetMeterRect (&rect))
			{
		//		if (IntersectRect (&intrect,&rect,&CurView->ScreenRect))
		//			ClearMeterPrompts (CurView->hDC);
			}
		}
		//if (!FirstVeh)
		{
			int	ivp;
			LPVIEWPORT	SaveVP = CurView;

			for(ivp=0;ivp<nAVLVP;ivp++)
			{
				RECT	MeterRect, intrect;
				BOOL	DoClip = FALSE;
				
				CurView = AVLViewports[ivp];
				if (!VPIsActive (CurView->ID))
					continue;
				if (GetMeterRect (&MeterRect))
				{
					if (IntersectRect (&intrect,&MeterRect,&CurView->ScreenRect))
					{
						int TypeRegion;
						HRGN	OvrLapRgn;

						GSSiDeleteObject(&CurView->hRgn);
						CurView->hRgn = CreateVPRgn (FALSE,FALSE);
						OvrLapRgn = CreateRectRgnIndirect (&MeterRect);  
						TypeRegion = CombineRgn (CurView->hRgn,CurView->hRgn,OvrLapRgn,RGN_DIFF);
						DeleteObject (OvrLapRgn);

	  					SelectClipRgn (CurView->hDC,CurView->hRgn);
	  					GSSiDeleteObject(&CurView->hRgn);
						DoClip = TRUE;
					}
				}
				if (BufferedScreen && !	DoClip && ivp)
				{
					GSSiDeleteObject(&CurView->hRgn);
					CurView->hRgn = CreateVPRgn (FALSE,FALSE);
	  				SelectClipRgn (CurView->hDC,CurView->hRgn);
	  				GSSiDeleteObject(&CurView->hRgn);
					DoClip = TRUE;
				}
				RestoreScreen2 (AVLViewports[ivp]->hDC, hVehSaveScreen[AVLViewports[ivp]->ID-1],
	    					 		  VehSaveScreenID[AVLViewports[ivp]->ID-1],DoClip);
				//ShowBufferedScreen (TRUE,FALSE);
			}
			CurView = SaveVP;
		}
		ProcessPassiveFunctions (CurView->hWnd,GF_REDISPLAYCOORD,0,0);
		for (isveh=0;isveh<NumSortedVehicles;isveh++)
		//for (iveh=0;iveh<NumVehicles;iveh++)
		{   
			RECT	NullVehicleRect={0,0,-1,-1};
			
	
			iveh = VehOrder[isveh];

			InWindow = TRUE;
			if (hVehicle[iveh]) 
			{
				pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
				pVehLoc->Rect = NullVehicleRect;
				RectInit (&pVehLoc->LastDrawRect);
				RectInit (&pVehLoc->LastFlagRect);
				if (!pVehLoc->nLoc ||
					(VehReplayFid == HFILE_ERROR && !pVehLoc->Display) || 
					(VehReplayFid != HFILE_ERROR && !VehicleInReplayList (pVehLoc->ID)))
					goto NextVeh;
				i = nVP; 
				while (i--)
				{
					RectInit (&pVehLoc->LastDrawRect);
					SetCurView ( pVP[i]);
					if (!VPIsActive (CurView->ID))
						continue;

					SelectVisList (FALSE);
					SetDisplayMode (CurView->hDC, GF_TEXTMODE);
					GSSiDeleteObject(&CurView->hRgn);
					CurView->hRgn = CreateVPRgn (FALSE,FALSE);
	  				SelectClipRgn (CurView->hDC,CurView->hRgn);
	  				GSSiDeleteObject(&CurView->hRgn);
				//	if (CurView->ID == *pCommandViewport)
					{
						size=(double)pVehLoc->Size[0]/100; 
						if (!_fstricmp (pVehLoc->ID,"GPS"))
							Txt = NoName;
						else
							Txt = pVehLoc->Desc;
						strcpy (TempTxt,Txt);
						Txt = TempTxt;
						if (!*Txt)
							strcpy (Txt,"%%%");
					}
				/*	else
					{
						size=(double)pVehLoc->Size[1]/100;
						Txt = 0;  
						goto NextView;
					}*/
	//				RestoreScreen2 (CurView->hDC, pVehLoc->hSaveScreen[CurView->ID-1],
	//			    					 		  pVehLoc->SaveScreenID[CurView->ID-1],FALSE);
	//			    DestroySavedScreen (&pVehLoc->hSaveScreen[CurView->ID-1],
	//			    					 pVehLoc->SaveScreenID[CurView->ID-1]);
		            ProjectedLoc = pVehLoc->Loc[pVehLoc->nLoc-1]; 
					AZ = SetVehSymAndColor (pVehLoc,&ProjectedLoc,&Moving);
					ProjectedLocScreen = BasePtToScreenPt (&ProjectedLoc);
					InBounds = PtInRect (&CurView->ScreenRect,ProjectedLocScreen);
					if (ShowAllVehTrails || GetGlobalBVal2 ("[%DISPLAYRESPONSEAREAS]",FALSE))
						InBounds = TRUE;
					if (InBounds && (pVehLoc->Symbol || pVehLoc->StatusSymbol))
					{   
						BOOL 	SaveDM = DisplayMarkers;
				    	int		OldMode; 
						COLORREF	OldColor;  
						short	SaveHT = CurView->HalfTone;
						BOOL	SaveCTG = CurView->ConvertToGray;
						char	Desc[80];
						
						CurView->HalfTone = 0;
						CurView->ConvertToGray = FALSE;
						DisplayMarkers = TRUE;      
						if (pVehLoc->StatusSymbol > 0) 
						{   
							DPOINT	Points[2];
							
							if (pVehLoc->Speed)
							{ 
								Points[0] = ProjectedLoc;
								Points[1] = dnewpt (ProjectedLoc,AZ,size*5*pVehLoc->Speed*CurView->BaseUnitsPerPixel); 
								GWPolylineD (CurView->hDC, Points, 2,DirectionSymbol);
							}
							pVehLoc->Symbol = pVehLoc->StatusSymbol;
							AZ2 = 0;
						}
						else
							AZ2 = AZ;
						OldMode = SetBkMode (CurView->hDC,OPAQUE);
						OldColor  = SetTextColor (CurView->hDC,VehTextColor);
						if (GetDictSymDescription (pVehLoc->Symbol,Desc))
						{
							ExpandText (Desc);
							{
								LPSTR	pBar=_fstrchr (Desc,'|');
								
								if (pBar)
								{   
									*pBar++ = 0; 
									if (!atob (pBar))
										SetBkMode (CurView->hDC,TRANSPARENT);
									SetTextColor (CurView->hDC,atol(Desc));
								}
							} 
						}
						DisplayVehicleResponseArea (ProjectedLoc,pVehLoc->Status);
						if (ShowAllVehTrails && pVehLoc->NumHist>1)
						{
							HPDPOINT	HistPoints = GlobalLock (pVehLoc->hHist);
							LPLONG		HistTimes = (LPLONG)(HistPoints+MAX_VEHICLE_TRACK_POINTS);
							HPEN	hPen = CreatePen (PS_SOLID,IDNINT(3*DeviceToScreenFactor),pVehLoc->TrackColor);
							HPEN	OldPen = SelectObject (CurView->hDC,hPen); 
							int		np,ih;
							int		Now;
							int		VehicleTrackStartTime;

							if (VehicleInReplayList (pVehLoc->ID))
							{
								Now = HistTimes[pVehLoc->NumHist-1];
							}
							else
							{
								Now = time (0);
							}
							VehicleTrackStartTime = Now - VehicleTrackMaxTime;
							for (ih=0;ih<pVehLoc->NumHist;ih++)
								if (HistTimes[ih] > VehicleTrackStartTime)
									break;
							np = pVehLoc->NumHist - ih;

							if (np > 1)
								GWPolylineD (CurView->hDC,&HistPoints[ih],np,0); 
							SelectObject (CurView->hDC,OldPen);
						    GSSiDeleteObject(&hPen);
							GlobalUnlock (pVehLoc->hHist);
						}
						pSymbolRect = &pVehLoc->Rect; 
						if (GetGlobalBVal2 ("[%USETEXTFLAG]",FALSE))
						{
							if (!VHCircle)
								VHCircle = GetDictSymbolNumber ("VHCIRCLE")+10000;
							Symbol = VHCircle;
							Color = RGB(255,0,0);
							DisplayMarker (ProjectedLoc,0,Txt,size,AZ2,Color,FALSE,-Symbol,0,0,&pVehLoc->LastDrawRect,0,&pVehLoc->LastFlagRect);
						}
						else
						{
							char	SaveLabelFont[128];
							
							GetGlobalCVal ("[%LABELFONT]",SaveLabelFont,"Arial");
							SetGlobalValue ("%LABELFONT","Arial Rounded MT Bold");
							Symbol = pVehLoc->Symbol;
							Color = pVehLoc->Color;
							DisplayMarker (ProjectedLoc,0,Txt,size,AZ2,Color,FALSE,-Symbol,0,0,&pVehLoc->LastDrawRect,0,0);
 							SetGlobalValue ("%LABELFONT",SaveLabelFont);
						}
						DisplayMarkers = SaveDM;  
	//					pVehLoc->hSaveScreen[CurView->ID-1] = SaveScreen2 (CurView->hDC,*pSymbolRect,CurView,&pVehLoc->SaveScreenID[CurView->ID-1]);
						pSymbolRect = NULL;
				    	SetBkMode (CurView->hDC,OldMode); 
						SetTextColor (CurView->hDC,OldColor);
						CurView->HalfTone = SaveHT;
						CurView->ConvertToGray =SaveCTG;
					} 
					else if (!InBounds)
						InWindow = FALSE;
				
					if (TrackingStatus && !_fstricmp (pVehLoc->ID,AVLTrackVehicle) && CurView->ID == *pCommandViewport && !InDisplayProcessing)
					{
						RECT	TrackRect = CurView->ScreenRect;
						POINT	MidPointScreen = RectMid (&TrackRect);
						double	RotateWithVehicle = GetGlobalDVal2 ("[%ROTATEWITHVEHICLE]",0);

			            TrackBorderPCT = 1.0 - GetGlobalDVal2 ("[%TRACKBORDER]",0.2);
			            if (!HaveDestination || !GetGlobalBVal ("[%USEDESTINATION]"))
							TrackRect = FactorRect (&TrackRect,TrackBorderPCT );
			            MidPoint = ScreenPtToBasePt (MidPointScreen);
			            MPAZ = getazd (&ProjectedLoc,&MidPoint);
			            AZDiff = LTWOPI(DeltaAZ (AZ,MPAZ));
			            if (AZDiff > PY)
			            	AZDiff = TWOPI - AZDiff;
						InBounds = PtInRect (&TrackRect,ProjectedLocScreen);
						if (!InWindow || (!InBounds && (Moving && AZDiff > PY/2)) ||
							(RotateWithVehicle && (AZ < TWOPI && fabs(DeltaAZ (LTWOPI(-(AZ - HALFPI)),CurView->Rotation)) > RotateWithVehicle)) ||
							!PtInRect (&CurView->ScreenRect,ProjectedLocScreen))
						{   
							GlobalUnlock (hVehicle[iveh]);
							IgnoreAVLTimer = TRUE;     
							if (RotateWithVehicle)
								CurView->Rotation = LTWOPI(-(AZ - HALFPI));
							CenterWindow (ProjectedLoc,FALSE); 
							RedisplayWindow();
							goto Exit;
						} 
						if (HaveDestination && GetGlobalBVal ("[%USEDESTINATION]")) 
						{
				    		MNMXCORD	Rect = Destination;    
				    		double		Size;
				    		
				    		AddDPointToMinMax (&ProjectedLoc,&Rect); 
				    		Size = BoundsArea (&Rect);
				    		if (Size < LastSize)
				    		{   
				    			LastSize = Size;
					    	//	Rect = FactorBounds (&Rect,1.5); 
					    		if ((Rect.xmx - Rect.xmn)*1.45 < (CurView->WBounds.xmx - CurView->WBounds.xmn) &&
					    			(Rect.ymx - Rect.ymn)*1.45 < (CurView->WBounds.ymx - CurView->WBounds.ymn)) 
								{   
									GlobalUnlock (hVehicle[iveh]);
									IgnoreAVLTimer = TRUE;
									Rect = FactorBounds (&Rect,1.1);
									ZoomToRect (Rect,FALSE); 
									RedisplayWindow();
									goto Exit;
								} 
							}
							else
								LastSize = Size;
				    	} 
					}
NextView:;
				}
NextVeh:
				GlobalUnlock (hVehicle[iveh]); 
			}
		}
	} 
	for (i=0;i<*pNumViewports;i++)  
	{
		if (!stricmp (pViewports[i]->Name,"Post Vehicle Viewport"))
		{
			static ii=1;
			BOOL	SaveBufferedScreen = BufferedScreen;

			SetViewport (i+1);
			BufferedScreen=FALSE;
//			ShowBufferedScreen (FALSE);
			if (ii)
				RedisplayViewport (2,FALSE); 
//			ShowBufferedScreen (FALSE);
			BufferedScreen =  SaveBufferedScreen;
		}
	}
	SetCurView ( SaveVP);	
	if (HaveDestination)
	{    
		POINT	WinPoint;
		
		MidPoint = MinMaxMidPointD (&Destination);    
		WinPoint=BasePtToWinPt (&MidPoint);
       	DisplayPointItem (CurView->hDC,WinPoint,DestSize*DeviceToScreenFactor,0,DestinationSymbol,0);
	}
	DisplayVehicles = TRUE;
Exit:
	ShowBufferedScreen (DisplayVehicles,TRUE,0,0);
	SetCurView ( SaveVP);	
//	CurView->hDC = SavehDC;
	InVehicleDisplay = FALSE;
	ReleaseDC (CurView->hWnd,hDCMain);
	return TRUE;
}

BOOL UpdateAllVehicles (BOOL MainVP)
{   
	LPVIEWPORT	SaveVP=CurView;

    if (InDisplayProcessing)
    	return FALSE;
    SetConfig (1);
    if (!*pNumViewports)
    	return FALSE;
	SetViewport (*pCommandViewport); 
	if (!CurView)
		return FALSE;  
	if (!nAVLVP)
		AVLViewports[nAVLVP++] = CurView;
    DisplayAllVehicles2 (nAVLVP,AVLViewports);
	HavePendingDisplay = FALSE;
	CurView = SaveVP;
	return TRUE;
}  

BOOL CreateVehHistMap (LPSTR DataFile,LPSTR MapFile,LPSTR vehid,LPSTR radio,
					   LPSTR SymName,COLORREF Color,BOOL AddLabel,BOOL Connect,LPMNMXCORD pBounds)
{
	LPSTR		pLoc;
	int			SymSize=-30;
	HANDLE		hGRText=0;
	char		UDI[64];
	int			Seq;
	LPSTR		Time;
	HANDLE		hSymDesc=0;
	short		NumSyms=0; 
	long		Refno=1;
	DPOINT		DPoint, Point1;
	BOOL		Err;
	short		SymNum=GetDictSymbolNumber (SymName);
	short		arrowLineSym=GetDictSymbolNumber ("ARROW_LINE");
	short		solidLineSym=GetDictSymbolNumber ("SOLID_LINE");
	HFILE		FidTemp;
	char		str[256];
	int			startloc;
	BOOL		FirstPoint=TRUE;
	int			pass=0;

	if (!AddLabel)
		SymSize = -20;
	DBoundsInit (pBounds);
	FidTemp = GSSiOpenFile (DataFile,0,OF_READ);
	fgetstring (str,16,FidTemp);
	fgetstring (str,16,FidTemp);
	fgetstring (str,16,FidTemp);
	fgetstring (str,16,FidTemp);
	fgetstring (str,16,FidTemp);
	fgetstring (str,16,FidTemp);
	fgetstring (str,16,FidTemp);
	startloc = GSSillseek (FidTemp,0,1);
	while (fgetstring (str,250,FidTemp))
	{
		pLoc = strrchr (str,'\t');
		pLoc++;
		DPoint = atopt (pLoc,&Err);
		ConvertCoord(&DPoint,2,1);
		AddDPointToMinMax (&DPoint,pBounds);
	}
	strcpy (PltName,MapFile);
	CreateNewMap (PltName,pBounds,0,0,0,0,0,0,FALSE);
	AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	hGRText = GSSiGlobAlloc (0,GHND,sizeof(GRTEXT));
	if (!Connect)
		pass++;
NextPass:
	GSSillseek (FidTemp,startloc,0);
	FirstPoint = TRUE;
	while (fgetstring (str,250,FidTemp))
	{
		LPGRTEXT lpGRText;
		HANDLE	hGRT=0;

		pLoc = strchr (str,'\t');
		*pLoc++ = 0;
		Seq = atoi (str);
		Time = pLoc;
		pLoc = strchr (Time,'\t');
		*pLoc++ = 0;
		pLoc = strrchr (pLoc,'\t');
		pLoc++;
		DPoint = atopt (pLoc,&Err);
		ConvertCoord(&DPoint,2,1);
		sprintf (UDI,"%s-%i %s",vehid,Seq,Time);
		lpGRText = (LPGRTEXT)GlobalLock (hGRText);
		lpGRText->version = 1;    
		lpGRText->length = sizeof(GRTEXT);
		lpGRText->FontNum = 0;
		lpGRText->Shadow = 1;
		_fstrcpy (lpGRText->cHeight,"12P"); 
   		lpGRText->hJust = 1;
		lpGRText->vJust = 2;  
	    sprintf (lpGRText->Text,"%i",Seq); 
	    lpGRText->ltext = _fstrlen (lpGRText->Text)+1;
	    lpGRText->ltext += lpGRText->ltext % 2;  
	    GlobalUnlock (hGRText);
		if (!pass)
		{
			if (!FirstPoint)
			{
				HANDLE hPt=GSSiGlobAlloc (0,GMEM_MOVEABLE,2*sizeof(DPOINT));
				LPDPOINT	pPoints = GlobalLock (hPt);
				int	nPt = 2;

				pPoints[0] = Point1;
				pPoints[1].x = (Point1.x + DPoint.x) / 2;
				pPoints[1].y = (Point1.y + DPoint.y) / 2;
				GlobalUnlock (hPt);
				AddToSymList (arrowLineSym,&NumSyms,&hSymDesc); 
				Refno++;
				AddPolyToMap (1,&nPt, &hPt,1,Refno,0,2,arrowLineSym,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
				pPoints = GlobalLock (hPt);
				pPoints[0] = pPoints[1];
				pPoints[1] = DPoint;
				GlobalUnlock (hPt);
				AddToSymList (solidLineSym,&NumSyms,&hSymDesc); 
				Refno++;
				AddPolyToMap (1,&nPt, &hPt,1,Refno,0,2,solidLineSym,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
				GSSiGlobFree (&hPt);
			}
		}
		else
		{
			Refno++;
			if (AddLabel)
				hGRT = hGRText;
			AddPointToMap (DPoint,Refno++,0,SymNum,SymSize,0,0,hGRT,0,"VEHHIST",UDI,Color,-1,-1,FALSE,TRUE,0,0);
		}
		Point1 = DPoint;
		FirstPoint = FALSE;
	}
	if (!pass++)
		goto NextPass;
	GSSiGlobFree (&hGRText);
	GSSiClose (FidTemp);
	AddPointToMap (DPoint,0,0,0,0,0,0,0,0,0,0,0,0,0,TRUE,TRUE,NULL,NULL);
	CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,NULL); 
	DestroySymList (&NumSyms,&hSymDesc); 
	ForceRefIndex = ForceTAGIndex = FALSE;
	return TRUE;
}

BOOL AddVehicleToPMMap (LPSTR VDCONNID,LPSTR VehID,COLORREF Color,BOOL Connect,short PointSym,short LineSym,LPLONG pRefno)
{
	BOOL	rtn=FALSE,Good;
	HANDLE	hSQL=0;
	char	str[256], SQL[64],UDI[66], DBName[MAX_PATH];
	DPOINT	DPoint;
	int		Seq=0;
	int			SymSize=-30;
	HANDLE		hGRText=0;
	LPGRTEXT lpGRText;
	HANDLE	hPoints=0;
	HPDPOINT	PolyPoints;
	int		nPolyPoints=0;

	strcpy (DBName,"VHF=[VEHICLEHISTFILE]");
	sprintf (SQL,"VHCONNID==%s",VDCONNID);
	if (!OpenDataFile (DBName,SQL,BT_READ,&hSQL))
		return FALSE;
	hGRText = GSSiGlobAlloc (0,GHND,sizeof(GRTEXT));
	if (Connect)
	{
		hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX*sizeof(DPOINT));
		PolyPoints = GlobalLock (hPoints);
		while (FetchDBRec (hSQL))
		{
			strcpy (str,"[VHF.VHHAVECOORD]");
			ExpandText (str);
			Good = atob (str);
			if (Good)
			{
				strcpy (str,"[VHF.VHX]");
				ExpandText (str);
				DPoint.x = atof (str);
				strcpy (str,"[VHF.VHY]");
				ExpandText (str);
				DPoint.y = atof (str);
				PolyPoints[nPolyPoints++] = DPoint;
			}
		}
		CloseDataFile (TRUE, &hSQL); 
		if (nPolyPoints)
		{
			AddPolyToMap (1,&nPolyPoints, &hPoints,1,(*pRefno)++,0,-1,LineSym,0,"PMROUTE",VehID,-1,Color,5,0,0,0,0,TRUE,0);
		}
		OpenDataFile (DBName,SQL,BT_READ,&hSQL);
	}
	while (FetchDBRec (hSQL))
	{
		strcpy (str,"[VHF.VHHAVECOORD]");
		ExpandText (str);
		Good = atob (str);
		if (Good)
		{
			strcpy (str,"[VHF.VHX]");
			ExpandText (str);
			DPoint.x = atof (str);
			strcpy (str,"[VHF.VHY]");
			ExpandText (str);
			DPoint.y = atof (str);
			Seq++;
			sprintf (UDI,"%s-%i",VehID,Seq);
			lpGRText = (LPGRTEXT)GlobalLock (hGRText);
			lpGRText->version = 1;    
			lpGRText->length = sizeof(GRTEXT);
			lpGRText->FontNum = 0;
			_fstrcpy (lpGRText->cHeight,"16P"); 
   			lpGRText->hJust = 1;
			lpGRText->vJust = 2;  
			sprintf (lpGRText->Text,"%i",Seq); 
			lpGRText->ltext = _fstrlen (lpGRText->Text)+1;
			lpGRText->ltext += lpGRText->ltext % 2;  
			GlobalUnlock (hGRText);
			AddPointToMap (DPoint,(*pRefno)++,0,PointSym,SymSize,0,0,hGRText,0,"VEHHIST",UDI,Color,-1,-1,FALSE,TRUE,0,0);
		}
	}
	GSSiGlobUlFree (&hPoints);
	GSSiGlobFree (&hGRText);
	CloseDataFile (TRUE, &hSQL); 
	return rtn;
}

BOOL CreateProgressMonitoringMap (LPSTR DataFile,LPSTR MapFile,LPSTR vehid,LPSTR radio,LPSTR SymName,COLORREF Color,LPMNMXCORD pBounds)
{
	LPSTR		pLoc;
	int			SymSize=-30;
	HANDLE		hGRText=0;
	char		UDI[64];
	int			Seq;
	LPSTR		Time;
	HANDLE		hSymDesc=0;
	short		NumSyms=0; 
	long		Refno=1;
	DPOINT		DPoint;
	BOOL		Err;
	short		SymNum=GetDictSymbolNumber (SymName);
	HFILE		FidTemp;
	char		str[256];

	DBoundsInit (pBounds);
	FidTemp = GSSiOpenFile (DataFile,0,OF_READ);
	while (fgetstring (str,250,FidTemp))
	{
		pLoc = strrchr (str,'\t');
		pLoc++;
		DPoint = atopt (pLoc,&Err);
		ConvertCoord(&DPoint,2,1);
		AddDPointToMinMax (&DPoint,pBounds);
	}
	strcpy (PltName,MapFile);
	CreateNewMap (PltName,pBounds,0,0,0,0,0,0,FALSE);
	AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	GSSillseek (FidTemp,0,0);
	hGRText = GSSiGlobAlloc (0,GHND,sizeof(GRTEXT));
	while (fgetstring (str,250,FidTemp))
	{
		LPGRTEXT lpGRText;

		pLoc = strchr (str,'\t');
		*pLoc++ = 0;
		Seq = atoi (str);
		Time = pLoc;
		pLoc = strchr (Time,'\t');
		*pLoc++ = 0;
		pLoc = strrchr (pLoc,'\t');
		pLoc++;
		DPoint = atopt (pLoc,&Err);
		ConvertCoord(&DPoint,2,1);
		sprintf (UDI,"%s-%i %s",vehid,Seq,Time);
		lpGRText = (LPGRTEXT)GlobalLock (hGRText);
		lpGRText->version = 1;    
		lpGRText->length = sizeof(GRTEXT);
		lpGRText->FontNum = 0;
		_fstrcpy (lpGRText->cHeight,"16P"); 
   		lpGRText->hJust = 1;
		lpGRText->vJust = 2;  
	    sprintf (lpGRText->Text,"%i",Seq); 
	    lpGRText->ltext = _fstrlen (lpGRText->Text)+1;
	    lpGRText->ltext += lpGRText->ltext % 2;  
	    GlobalUnlock (hGRText);
	    Refno++;

		AddPointToMap (DPoint,Refno++,0,SymNum,SymSize,0,0,hGRText,0,"VEHHIST",UDI,Color,-1,-1,FALSE,TRUE,0,0);
	}
	GSSiGlobFree (&hGRText);
	GSSiClose (FidTemp);
	AddPointToMap (DPoint,0,0,0,0,0,0,0,0,0,0,0,0,0,TRUE,TRUE,NULL,NULL);
	CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,NULL); 
	DestroySymList (&NumSyms,&hSymDesc); 
	ForceRefIndex = ForceTAGIndex = FALSE;
	return TRUE;
}

BOOL CreateProgMonMap (LPSTR Vehid,LPSTR CPoint)
{
	BOOL	Display,Connect, rc=FALSE;
	COLORREF	color;
	HANDLE	hSQL=0;
	char	str[256];
	char	VDCONNID[32],VDFile[MAX_PATH];

	if (!ValidBounds (&PMBounds))
		return FALSE;
	strcpy (VDFile,"VDF=[VEHICLEDATAFILE]");
	ExpandText (VDFile);
	sprintf (str,"VDVEHID=%s",Vehid);
	if (!OpenDataFile (VDFile,str,BT_READ,&hSQL))
		return FALSE;
	if (FetchDBRec (hSQL))
	{
		strcpy (str,"[VDF.VDHISTSELECT]");
		ExpandText (str);
		rc = atob (str);
	}
	else
		rc = FALSE;
	CloseDataFile (TRUE, &hSQL); 
	if (rc)
	{
		HANDLE		hSymDesc=0;
		HFILE		FidTemp;
		short		NumSyms=0; 
		short		PointSym=GetDictSymbolNumber ("CIRCLE");
		short		LineSym=GetDictSymbolNumber ("PMLINE");
		long		Refno = 109099900;

		if (*CPoint)
		{
			BOOL	err;
			DPOINT	DPoint = atopt (CPoint,&err);
			
			AddDPointToMinMax (&DPoint,&PMBounds);
			sprintf (PltName,"[%%PROGMONDIR]\\processmon.plt");
			CreateNewMap (PltName,&PMBounds,0,0,0,0,0,0,FALSE);
			AddToSymList (PointSym,&NumSyms,&hSymDesc); 
			AddToSymList (LineSym,&NumSyms,&hSymDesc); 
			if (OpenDataFile (VDFile,"VDHISTSELECT=1",BT_READ,&hSQL))
			{
				while (FetchDBRec (hSQL))
				{
					strcpy (str,"[VDF.VDHISTSELECT]");
					ExpandText (str);
					if (atob (str))
					{
						strcpy (VDCONNID,"[VDF.VDCONNID]");
						ExpandText (VDCONNID);
						strcpy (str,"[VDF.VDHISTCOLOR]");
						ExpandText (str);
						color = atoi (str);
						strcpy (str,"[VDF.VDHISTCONNECT]");
						ExpandText (str);
						Connect = atob (str);
						AddVehicleToPMMap (VDCONNID,Vehid,color,Connect,PointSym,LineSym,&Refno);
					}
				}
			}
			CloseDataFile (TRUE, &hSQL); 
			CloseMap(TRUE);  
			AddSymToMap (NumSyms,hSymDesc,0,NULL); 
			DestroySymList (&NumSyms,&hSymDesc); 
			ForceRefIndex = ForceTAGIndex = FALSE;
			FidTemp = GSSiOpenFile ("[%PROGMONDIR]\\filelist.txt",0,OF_CREATE);
			SearchFilesInDir ("[%PROGMONDIR]", ".plt", FidTemp,&TotFiles,"*.plt",1,TRUE,TRUE);     
			GSSiClose (FidTemp);
			rc = TRUE;
		}
	}
	return rc;
}

int	DrawTextInRect(HDC hDC,LPSTR Txt,LPRECT pRect,COLORREF ShadowColor,int MinFontSize,int MaxFontSize)
{
	int		rtn, lText=strlen(Txt), x,y;
	SIZE	txSize;
	double	fac;
	HFONT	hFont,hOldFont;
	int		FontSize = pRect->bottom - pRect->top;
	POINT	MidPoint = RectMid (pRect);

//	rtn = DrawText(hDC,Txt,lText,pRect,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
//	return rtn;
	hFont = CreateFont(FontSize, 0, 0, 0, FW_NORMAL,0, 0, 0, 0, 0, 0, 0, 0,"Arial Rounded MT Bold");   
	hOldFont = SelectObject (hDC,hFont);
	GetTextExtentPoint32 (hDC,Txt,lText,&txSize); 
	fac = ((double)(pRect->right - pRect->left))/txSize.cx;
	SelectObject (hDC,hOldFont);
	DeleteObject (hFont);
	FontSize *= fac;
/*	{
		char	mes[256];

		sprintf (mes,"%i %i %i %i %f %i",pRect->left,pRect->right,pRect->top,pRect->bottom,fac,FontSize);
		MessageBox (0,mes,0,MB_OK);
	}*/
	hFont = CreateFont(min(MaxFontSize,max(FontSize,MinFontSize)), 0, 0, 0, FW_NORMAL,0, 0, 0, 0, 0, 0, 0, 0,"Arial Rounded MT Bold");   
	hOldFont = SelectObject (hDC,hFont);
//	rtn = DrawText(hDC,Txt,lText,pRect,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	GetTextExtentPoint32 (hDC,Txt,lText,&txSize); 
	x = MidPoint.x - txSize.cx / 2;
	y = MidPoint.y - txSize.cy / 2;
	rtn = TextOutWithShadow (hDC,x,y,Txt,lText,1,ShadowColor);
	SelectObject (hDC,hOldFont);
	DeleteObject (hFont);

	return rtn;
}

void DisplayVehicleStatus (int iveh,LPDRAWITEMSTRUCT lpdis,LPRECT pRect)
{
	LPVEHLOCATION	pVehLoc;  
	HDC				hDC=lpdis->hDC;
	int				OldMode;
	COLORREF		OldColor;
	COLORREF		VehTextColor=GetGlobalLVal2("[%VEHTEXTCOLOR]",0);
	char			Desc[128], Txt[128];
	DPOINT			Loc;
	//double			size;
	BOOL			SaveDM=DisplayMarkers;
	HPEN			hBorderPen;
	HBRUSH			hOldBrush;
	static	ii=0;
//	int				Symbol = GetSymbolNum ("CIRCLE");
	COLORREF		Color = RGB(255,0,0), ShadowColor=RGB(255,255,255);
	POINT			LocI;
	RECT			TextRect;
	char			FollowID[66];
	int				FollowSym = GetSymbolNum ("FOLLOWBACKGROUND");
	LPRECT			SavepSymbolRect = pSymbolRect;

	SaveDC (lpdis->hDC);
	GetGlobalCVal ("[%AVLFOLLOW]",FollowID,0);
	pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
	OldMode = SetBkMode (hDC,OPAQUE);
	OldColor  = SetTextColor (hDC,VehTextColor);
	if (GetDictSymDescription (pVehLoc->Symbol,Desc))
	{
		ExpandText (Desc);
		{
			LPSTR	pBar=_fstrchr (Desc,'|');
								
			if (pBar)
			{
				LPSTR pComma;
				
				*pBar++ = 0; 
				pComma = MatchLev (Desc,',');
				if (!atob (pBar))
					SetBkMode (hDC,TRANSPARENT);
				if (pComma)
				{
					*pComma++ = 0;
					ShadowColor = atol (pComma);
				}
				SetTextColor (hDC,atol(Desc));
			} 
		}
	}
//	DisplayMarkers = TRUE;
//	DisplayMarker (Loc,0,Txt,size,0,pVehLoc->Color,FALSE,-pVehLoc->Symbol,0,0);
	LocI = RectMid (&lpdis->rcItem);
	strcpy (Txt,"123");
//	size=30;
	if (pVehLoc->Color == (COLORREF)-1)
		HaveVarFillColor = FALSE;  
	else
	{
		HaveVarFillColor = TRUE;  
		GlobalColors[0] = pVehLoc->Color;
	}
	SetDisplayMode (lpdis->hDC, GF_SCREENMODE);
  	SelectClipRgn (lpdis->hDC,0);
	FillRect (lpdis->hDC,pRect,GetStockObject (WHITE_BRUSH));
	if (!stricmp (FollowID,pVehLoc->ID))
		DisplaySymInRect (lpdis->hDC,FollowSym,*pRect,1,FALSE);
	pSymbolRect = &TextRect;
	RectInit (pSymbolRect);
	DisplaySymInRect (lpdis->hDC,pVehLoc->Symbol,*pRect,1,FALSE);
	pSymbolRect = SavepSymbolRect;
	//sprintf (Txt,"%i",iveh);
	HaveVarFillColor = FALSE;  
	strcpy (Txt,pVehLoc->Desc);
	pVehLoc->MenuRect = *pRect;
	OldMode = SetBkMode (lpdis->hDC,TRANSPARENT);
	//TextRect = *pRect;
	InflateRect (&TextRect,-1,-1);
	DrawTextInRect (lpdis->hDC,Txt,&TextRect,ShadowColor,12,14);
	GlobalUnlock (hVehicle[iveh]);
//	DisplayMarker (PointToDPoint (LocI),0,Txt,size,0,Color,FALSE,-Symbol,0,0);
	DisplayMarkers = SaveDM;  
	SetBkMode (hDC,OldMode); 
	SetTextColor (hDC,OldColor);
	RestoreDC (lpdis->hDC,-1);
//	GlobalUnlock (hVehicle[iveh]); 
	return;
}

BOOL DisplayVehicleInfo (LPSTR VehID,BOOL Force)
{
	HANDLE	htxt=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
	LPSTR	txt = GlobalLock (htxt);
	int		l;
	BOOL	rtn;

	sprintf (txt,"$MACRO([%%DL]macros\\pmvehicledata.txt,%s)",VehID);
	ExpandText (txt);
	l = strlen(txt)- 4;
	if (!strncmp (txt,"|VM|",4))
		memmove (txt,&txt[4],l);
	txt[l] = 0;
	rtn = SetVehicleInfo (txt,VehID,Force);
	GSSiGlobUlFree (&htxt);
	return rtn;
}

LRESULT CALLBACK NewEditProc (HWND hwnd, UINT message, 
                             WPARAM wParam, LPARAM lParam)
{
    int	ii;
	LPSTR	txt,pLoc;
	RECT	TBRect, LastBoxRect;
	int		row,col, iveh;
	char	str[128];
	
	if (VehStatHeight && VehStatWidth && NumVehicles)
	switch (message)
	{
		case WM_PAINT:
			ii=1;
			break;
		case WM_MOUSEMOVE:
		{
			//HDC	hDC = GetDC (hwnd);

    		POINT	MovePoint = POINTStoPOINT(MAKEPOINTS (lParam)); 
 			ii=1;
			row = MovePoint.y / VehStatHeight;
			col = min (maxinrow-1,MovePoint.x / VehStatWidth);
			MovePoint.x = (col + 0.5) * VehStatWidth;
			MovePoint.y = (row + 0.5) * VehStatHeight;
			row += SendDlgItemMessage(hWndVehStatus,IDC_VEHICLE_STAT_LIST,LB_GETTOPINDEX,0,0);
            if (SendMessage(hwnd, LB_GETTEXT,row, (LPARAM) str) == LB_ERR)
				break;
			pLoc = str;
			while (col--)
			{
				if ((pLoc = strchr (pLoc,'\t')))
					pLoc++;
				else
					goto Exit;
			}
			iveh = atoi (pLoc);
			if (iveh != LastVeh)
			{
				int	l;
				LPVEHLOCATION	pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);

				DisplayVehicleInfo (pVehLoc->ID,TRUE);
				GlobalUnlock (hVehicle[iveh]);
			/*	RestoreScreen2 (hDC, hLastBox,0,FALSE);
				{
					RECT	ClientRect,LBRect;
					int		w,h;
					float	move=0.5;
					POINT	MidPt;

					GetClientRect (hWndMain,&ClientRect);
					GetWindowRect (hwnd,&LBRect);
					MidPt = RectMid (&ClientRect);
					MovePoint.x += LBRect.left;
					MovePoint.y += LBRect.top;
					YellowTextBox (hWndMain,txt,MovePoint,&LastBoxRect,0,FALSE);
					w = LastBoxRect.right - LastBoxRect.left;
					h = LastBoxRect.bottom - LastBoxRect.top;
					if (MovePoint.x > MidPt.x)
					{
						LastBoxRect.left -= w * move + 20;
						LastBoxRect.right -= w * move + 20;
					}
					else
					{
						LastBoxRect.left += w * move + 20;
						LastBoxRect.right += w * move + 20;
					}
					if (MovePoint.y < MidPt.y)
					{
						LastBoxRect.top += h * move + 20;
						LastBoxRect.bottom += h * move + 20;
					}
					else
					{
						LastBoxRect.top -= h * move + 20;
						LastBoxRect.bottom -= h * move + 20;
					}
					hLastBox = YellowTextBox (hWndMain,txt,MovePoint,0,&LastBoxRect,FALSE);
					hDCLastBox = GetDC (hWndMain);
					ReleaseDC (hWndMain,hDC);
					LastBoxVP = CurView;
					LastVP = SetLastVP (LastBoxVP);
					GSSiGlobUlFree (&htxt);
				}*/
			}
			LastVeh = iveh;

		//	ReleaseDC (hwnd,hDC);
		}
		break;
	case WM_LBUTTONDOWN:
		if (LastVeh > -1)
		{
			LPVEHLOCATION	pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[LastVeh]);
			char cmd[256]="$MENU([PMDIR]\\vehicles.txt)";
			
			SetGlobalValue ("PICKED_CONNID",pVehLoc->ID);
			GlobalUnlock (hVehicle[LastVeh]);
			ClearVehicleInfoRect ();
			ExpandText (cmd);
			return 0;
		}
		break;

	case WM_CHAR:
//		chCharCode = (TCHAR) wParam;
//		if(chCharCode > 0x20 && !IsCharAlpha(chCharCode))
//			return 0;
		break;
	}
Exit:
	return CallWindowProc (g_OldEdit, hwnd, message, wParam, lParam);
}

BOOL VehicleRecentlyMoved (LPVEHLOCATION	pVehLoc,int Now)
{
	BOOL rtn=FALSE;
	int	 n = pVehLoc->nLoc;

	if (n > 1)		
		while (n--)
		{
			if (Now - pVehLoc->LocTime[n] > 300)
				break;
		if (rtn = (pVehLoc->Speeds[n] > 5))
			break;
		}
	return rtn;
}

BOOL FAR PASCAL VEHICLE_STATUSMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	static	RECT	LastRect, ColorRect,ConnectedRect,DisplayRect;
	int	Choice,iveh,Count;
	LPDRAWITEMSTRUCT lpdis;
	POINT	Point;
	LPSTR	pLoc,pEnd;
	float	Tabpct[4]={0.04,0.4,0.56,0.81};
	static	int		nTab=4,Tab[4];
	char	str[256];
	BOOL	Display,Connect;
	COLORREF	color;
	char	VehID[32],VDCONNID[32];
	char	BoundsC[128];
	short   BRtn;
	struct	tm		tmtime;
	int		nv,n,i,nrow, h, w, iVeh;
	RECT	rect;
	static	int	lastw, lasth;
	int		Now;
	static	UINT	CurType=IDC_VEHDISPLAY_ALL;

 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message) 
 {
    case WM_INITDIALOG:
		hWndVehStatus = hWndDlg;
		VehStatusVP = CurView;
		SendDlgItemMessage (hWndDlg,IDC_VEHDISPLAY_ALL,BM_SETCHECK,FALSE,0L);
		SendDlgItemMessage (hWndDlg,IDC_VEHDISPLAY_INSERVICE,BM_SETCHECK,FALSE,0L);
		SendDlgItemMessage (hWndDlg,IDC_VEHDISPLAY_ACTIVE,BM_SETCHECK,FALSE,0L);
		SendDlgItemMessage (hWndDlg,IDC_VEHDISPLAY_AVAILABLE,BM_SETCHECK,FALSE,0L);
		SendDlgItemMessage (hWndDlg,IDC_VEHDISPLAY_FENCE,BM_SETCHECK,FALSE,0L);
		SendDlgItemMessage (hWndDlg,CurType,BM_SETCHECK,TRUE,0L);
		g_OldEdit = (WNDPROC)SetWindowLong(GetDlgItem(hWndDlg,IDC_VEHICLE_STAT_LIST), GWL_WNDPROC, (LONG)NewEditProc);
		PostMessage(hWndDlg, GSSI_REPOSITION,1, 0); 
		break;
	case GSSI_REPOSITION:
	{
		RECT	Rect=VehStatusVP->ScreenRect, rect;
		time_t	Now=time(0);

		if (Message != GSSI_REPOSITION || wParam == 1)
			lastw = lasth = -1;
//		ClientRectToScreenRect (CurView->hWnd,&Rect);
		SetWindowPos(hWndDlg,HWND_TOP,Rect.left,Rect.top+1,Rect.right-Rect.left,Rect.bottom-Rect.top-2,SWP_SHOWWINDOW|SWP_NOZORDER);
		w = Rect.right-Rect.left;
		maxinrow = 6;
		if (w < 300)
			maxinrow = 5;
		if (w < 210)
			maxinrow = 4;
		if (w < 180)
			maxinrow = 3;
		VehStatHeight = w/maxinrow;
		h = Rect.bottom-Rect.top-2;
		h -= VehInfoHeight+42+4;
		if (!VehStatHeight)
			break;
		h = h/VehStatHeight;
		h *= VehStatHeight;
		GetWindowRect (GetDlgItem(hWndDlg,IDC_VEHDISPLAY_FENCE),&rect);
		ScreenRectToClientRect (hWndDlg,&rect);
		SetWindowPos(GetDlgItem(hWndDlg,IDC_VEHICLE_STAT_LIST),HWND_TOP,0,rect.bottom+2,w-6,h,SWP_SHOWWINDOW|SWP_NOZORDER);
		SendDlgItemMessage (hWndDlg,IDC_VEHICLE_STAT_LIST,LB_SETITEMHEIGHT,-1,MAKELPARAM(w/maxinrow,0));
		SetWindowPos(GetDlgItem(hWndDlg,IDC_VEHICLEINFO),HWND_TOP,3,rect.bottom+2+h+4,w-3,Rect.bottom-(rect.bottom+2+h+4)-4,SWP_SHOWWINDOW|SWP_NOZORDER);
		if (w != lastw || h != lasth)
		{
			int	ninrow=0;
			
			*str = 0;
			Choice=SendDlgItemMessage(hWndDlg,IDC_VEHICLE_STAT_LIST,LB_GETTOPINDEX,0,0);
			SendDlgItemMessage (hWndDlg,IDC_VEHICLE_STAT_LIST,LB_RESETCONTENT,0,0);
			for (iveh=0;iveh<NumVehicles;iveh++)
			{
				LPVEHLOCATION	pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);

				pVehLoc->MenuRect = NullRect;
				GlobalUnlock (hVehicle[iveh]);
			}
			nv = NumVehicles;
			//nv = 113;
			nrow = (nv-1)/maxinrow + 1;
			SendDlgItemMessage (hWndDlg,IDC_VEHICLE_STAT_LIST,LB_RESETCONTENT,0,0);
			n = 0;
			Now=time(0);

			for (iveh=0;iveh<nv;iveh++)
			{
				LPVEHLOCATION	pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[VehOrder[n]]);

				if (VehicleInReplayList (pVehLoc->ID) ||
					(SendDlgItemMessage (hWndDlg,IDC_VEHDISPLAY_ALL,BM_GETCHECK,0,0L) ||
					(SendDlgItemMessage (hWndDlg,IDC_VEHDISPLAY_INSERVICE,BM_GETCHECK,0,0L) && pVehLoc->Status && pVehLoc->nLoc && (Now - pVehLoc->LocTime[pVehLoc->nLoc-1]) < 24*3600) ||
					(SendDlgItemMessage (hWndDlg,IDC_VEHDISPLAY_FENCE,BM_GETCHECK,0,0L) && pVehLoc->FenceStatus) ||
					(SendDlgItemMessage (hWndDlg,IDC_VEHDISPLAY_ACTIVE,BM_GETCHECK,0,0L) && VehicleRecentlyMoved (pVehLoc,Now)) ||
					(SendDlgItemMessage (hWndDlg,IDC_VEHDISPLAY_AVAILABLE,BM_GETCHECK,0,0L) && pVehLoc->Status == 6)))
				{
					sprintf (strchr(str,0),"%i\t",VehOrder[n]);
					ninrow++;
					pVehLoc->Display = TRUE;
				}
				else
					pVehLoc->Display = FALSE;
				GlobalUnlock (hVehicle[VehOrder[n++]]);
				if (ninrow == maxinrow)
				{
					SendDlgItemMessage(hWndDlg,IDC_PMVEHLIST,LB_ADDSTRING,0,(LPARAM)str);
					ninrow = 0;
					*str = 0;
				}
			}
			if (ninrow)
				SendDlgItemMessage(hWndDlg,IDC_PMVEHLIST,LB_ADDSTRING,0,(LPARAM)str);
			SendDlgItemMessage(hWndDlg,IDC_VEHICLE_STAT_LIST,LB_SETTOPINDEX,Choice,0);
			lastw = w;
			lasth = h;
		}
		if (Message != GSSI_REPOSITION || !lParam)
			UpdateAllVehicles (TRUE);
	}
   		break;
	case WM_MOUSEMOVE:
		ClearVehicleInfoRect ();
		break;
	case WM_DESTROY:
		 hWndVehStatus = 0;
		 VehStatusVP = 0;
		 break;

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         DestroyWindow (hWndDlg);
         break; /* End of WM_CLOSE                                      */

		break;

    case WM_MEASUREITEM: 
		{
			LPMEASUREITEMSTRUCT	lpmis = (LPMEASUREITEMSTRUCT) lParam; 
 
        /* Set the height of the list box items. */
			RECT	rect;
			GetWindowRect (GetDlgItem(hWndDlg,IDC_VEHICLE_STAT_LIST),&rect);
		VehStatHeight = (VehStatusVP->ScreenRect.right - VehStatusVP->ScreenRect.left)/maxinrow;
		VehStatHeight = (rect.right - rect.left)/maxinrow;
        lpmis->itemHeight = VehStatHeight;
		}
        return TRUE; 
 
    case WM_DRAWITEM: 
 
        lpdis = (LPDRAWITEMSTRUCT) lParam; 
 
        /* If there are no list box items, skip this message. */ 
 
        if (lpdis->itemID == -1) { 
            break; 
        } 
 
        /* 
         * Draw the bitmap and text for the list box item. Draw a 
         * rectangle around the bitmap if it is selected. 
         */ 
 
        switch (lpdis->itemAction) { 
 
            case ODA_SELECT: 
            case ODA_DRAWENTIRE: 
				if (lpdis->rcItem.top == 0)
					ClearVehicleMenuRects ();
				VehStatWidth = (lpdis->rcItem.right - lpdis->rcItem.left - 10)/maxinrow;
				VehStatHeight = lpdis->rcItem.bottom - lpdis->rcItem.top;
				rect = lpdis->rcItem;
				rect.right = rect.left + VehStatWidth;
                SendMessage(lpdis->hwndItem, LB_GETTEXT,lpdis->itemID, (LPARAM) str); 
				pLoc = str;
				while ((pEnd = strchr (pLoc,'\t')))
				{
					*pEnd++=0;
					iVeh = atoi (pLoc);
					pLoc = pEnd;
					DisplayVehicleStatus (iVeh,lpdis,&rect);
					rect.left += VehStatWidth;
					rect.right += VehStatWidth;
				}

/*            {
				RECT	Rect;
				int		w=lpdis->rcItem.right - lpdis->rcItem.left;
				int		y,i;
				TEXTMETRIC	tm;
				COLORREF	Color;
				LPSTR	pCount;
				BOOL	Display,Connected;

				for (i=0;i<nTab;i++)
					Tab[i] = Tabpct[i] * w;
                SendMessage(lpdis->hwndItem, LB_GETTEXT,lpdis->itemID, (LPARAM) str); 
                GetTextMetrics(lpdis->hDC, &tm); 
 
                y = (lpdis->rcItem.bottom + lpdis->rcItem.top -tm.tmHeight) / 2; 
				if ((pLoc=strchr (str,'\t')))
					*pLoc++ = 0;
				Display = atoi (pLoc);
				if ((pLoc=strchr (pLoc,'\t')))
					*pLoc++ = 0;
				pCount = pLoc;
				if ((pLoc=strchr (pLoc,'\t')))
					*pLoc++ = 0;
				Color = atoi (pLoc);
 				if ((pLoc=strchr (pLoc,'\t')))
					*pLoc++ = 0;
				Connected = atoi(pLoc);
                TextOut(lpdis->hDC,lpdis->rcItem.left,y,str,strlen(str)); 
                Point.x=lpdis->rcItem.left + 20;
                Point.y=(lpdis->rcItem.top + lpdis->rcItem.bottom)/2;
						   
				Rect = lpdis->rcItem;
				Rect.left = Tab[0];
				Rect.right = Tab[1];
				ShowCheck (lpdis->hDC,&Rect,Display,&DisplayRect);
                TextOut(lpdis->hDC,Tab[1],y,pCount,strlen(pCount)); 
				Rect.left = Tab[2];
				Rect.right = Tab[3];
				ColorRect = Rect;
				InflateRect (&ColorRect,-2,-2);
				FillRectColor (lpdis->hDC,&ColorRect,Color);
 				Rect.left = Tab[3];
				Rect.right = lpdis->rcItem.right;
				ShowCheck (lpdis->hDC,&Rect,Connected,&ConnectedRect);

                // Is the item selected? 
 
                if (lpdis->itemState & ODS_SELECTED) { 
                    
                    if (LastRect.top != -1000)
                    	DrawFocusRect(lpdis->hDC, &LastRect);
                    //DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
                    //LastRect = lpdis->rcItem; 
                }
				
				
            } */
                break; 
 
            case ODA_FOCUS: 
                //InvertRect(lpdis->hDC, &lpdis->rcItem); 
 
                break; 
        } 
        return TRUE; 
    
    case WM_COMMAND:

		 switch(LOWORD (wParam))

           {
            case IDCANCEL:
				break;

			case IDC_FLASHVEHICLE:
				{
					LPVEHLOCATION	pVehLoc;
					
					if (MenuDisplayed)
						break;
					iVeh = lParam;
					pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iVeh]);
					if (!IsRectEmpty (&pVehLoc->MenuRect))
					{
						HWND	hWnd = GetDlgItem(hWndDlg,IDC_VEHICLE_STAT_LIST);
						HDC		hDC = GetDC (GetDlgItem(hWndDlg,IDC_VEHICLE_STAT_LIST));
						
						//if (!WindowIsCovered (hWnd,1))
						{
							RECT	rect=pVehLoc->MenuRect;

							InflateRect (&rect,-3,-3);
							SaveDC (hDC);
							SelectClipRgn (hDC,0);
 							InvertRect (hDC,&rect);
							RestoreDC (hDC,-1);
						}
						ReleaseDC (hWnd,hDC);
					}
					GlobalUnlock (hVehicle[iVeh]);
				}
				break;

			case IDC_UPDATESTATUS:
				lastw = -1;
				VehicleStatusChanged = FALSE;
				//SendDlgItemMessage (hWndDlg,IDC_VEHICLE_STAT_LIST,LB_RESETCONTENT,0,0);
				PostMessage(hWndDlg, GSSI_REPOSITION,1, 0);
				break;
			case IDC_VEHDISPLAY_ALL:
			case IDC_VEHDISPLAY_ACTIVE:
			case IDC_VEHDISPLAY_FENCE:
			case IDC_VEHDISPLAY_INSERVICE:
			case IDC_VEHDISPLAY_AVAILABLE:
				lastw = -1;
				CurType = LOWORD (wParam);
				SendDlgItemMessage (hWndDlg,IDC_VEHICLE_STAT_LIST,LB_RESETCONTENT,0,0);
				PostMessage(hWndDlg, GSSI_REPOSITION,0, 0L);
				break;

        	case IDC_VEHICLE_STAT_LIST:
			switch(HIWORD(wParam))
            {
                case LBN_SELCHANGE://not currently used since LBS_NOTIFY not set
					{
						HWND	hWnd=GetDlgItem (hWndDlg,IDC_PMVEHLIST);
						RECT	Rect;
						POINT	Point;
						int		width;
						BOOL	DoChange=FALSE;

						GetWindowRect (hWnd,&Rect);
						width = Rect.right - Rect.left;
						Choice=SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0); 
						SendDlgItemMessage(hWndDlg,IDC_VEHICLE_STAT_LIST, LB_GETTEXT,Choice, (LPARAM) str); 
						GetCursorPos (&Point); 
						ScreenToClient (hWnd,&Point);
						if (Point.x >= DisplayRect.left && Point.x <= DisplayRect.right)
						{
							//Display = !Display;
							DoChange=TRUE;
						}

					}
					break;
            }
              break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}
BOOL FAR PASCAL PROGRESS_MONITORINGMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	static	RECT	LastRect, ColorRect,ConnectedRect,DisplayRect;
	int	Choice,iveh,Count;
	LPDRAWITEMSTRUCT lpdis;
	POINT	Point;
	LPSTR	pLoc,pEnd;
	float	Tabpct[4]={0.04,0.4,0.56,0.81};
	static	int		nTab=4,Tab[4];
	char	str[256];
	BOOL	Display,Connect;
	COLORREF	color;
	char	VehID[32],VDCONNID[32];
	char	BoundsC[128];
	short   BRtn;
	struct	tm		tmtime;

 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message) 
 {
    case WM_INITDIALOG:
	{
		HANDLE	hSQL=0;

		tmtime = *localtime (&SystemStartTime); 

		SetDlgItemInt (hWndDlg,IDC_YEAR,tmtime.tm_year+1900,FALSE);
		SetDlgItemInt (hWndDlg,IDC_MONTH,tmtime.tm_mon,FALSE);
		SetDlgItemInt (hWndDlg,IDC_DAY,tmtime.tm_mday,FALSE);
		SetDlgItemInt (hWndDlg,IDC_HOUR,tmtime.tm_hour,FALSE);
		SetDlgItemInt (hWndDlg,IDC_MINUTE,tmtime.tm_min,FALSE);
		LastRect.top = -1000;
		strcpy (str,"VDF=[VEHICLEDATAFILE]");
		ExpandText (str);
		if (!OpenDataFile (str,"",BT_READ,&hSQL))
			GSSiMsgBox( GetFocus(),"No Vehicles Defined",0, MB_OK,0);
		else
		{
			HANDLE	hSQL2=0;

			strcpy (str,"VHF=[VEHICLEHISTFILE]");
			ExpandText (str);
			if (!OpenDataFile (str,"VHCONNID==@[.CONID]",BT_READ,&hSQL2))
				GSSiMsgBox( GetFocus(),"No Vehicle History",0, MB_OK,0);
			else
			{
				while (FetchDBRec (hSQL))
				{
					int	n=0;
					DPOINT	DPoint;
					BOOL	Good;
					MNMXCORD	Bounds;

					DBoundsInit (&Bounds);
					sprintf (str,"[CONID]=[VDF.VDCONNID]");
					ExpandText (str);
					while (FetchDBRec (hSQL2))
					{
						strcpy (str,"[VHF.VHHAVECOORD]");
						ExpandText (str);
						Good = atob (str);
						if (Good)
						{
							n++;
							strcpy (str,"[VHF.VHX]");
							ExpandText (str);
							DPoint.x = atof (str);
							strcpy (str,"[VHF.VHY]");
							ExpandText (str);
							DPoint.y = atof (str);
							AddDPointToMinMax (&DPoint,&Bounds);
						}
					}
					if (n)
					{
						boundstoa (BoundsC,&Bounds);
						sprintf (str,"[VDF.VDVEHID]\t[VDF.VDHISTSELECT]\t%i\t[VDF.VDHISTCOLOR]\t[VDF.VDHISTCONNECT]\t[VDF.VDCONNID]\t%s",n,BoundsC);
						ExpandText (str);
						Choice=SendDlgItemMessage(hWndDlg,IDENTIFY_DATA,LB_GETCURSEL,0,0); 
						SendDlgItemMessage(hWndDlg,IDC_PMVEHLIST,LB_ADDSTRING,0,(LPARAM)str);
					}
				}
				CloseDataFile (TRUE, &hSQL2); 
			}
			CloseDataFile (TRUE, &hSQL); 
		 } 
	}
         break;         

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_DRAWITEM: 
 
        lpdis = (LPDRAWITEMSTRUCT) lParam; 
 
        /* If there are no list box items, skip this message. */ 
 
        if (lpdis->itemID == -1) { 
            break; 
        } 
 
        /* 
         * Draw the bitmap and text for the list box item. Draw a 
         * rectangle around the bitmap if it is selected. 
         */ 
 
        switch (lpdis->itemAction) { 
 
            case ODA_SELECT: 
            case ODA_DRAWENTIRE:  
            {
				RECT	Rect;
				int		w=lpdis->rcItem.right - lpdis->rcItem.left;
				int		y,i;
				TEXTMETRIC	tm;
				COLORREF	Color;
				LPSTR	pCount;
				BOOL	Display,Connected;

				for (i=0;i<nTab;i++)
					Tab[i] = Tabpct[i] * w;
                SendMessage(lpdis->hwndItem, LB_GETTEXT,lpdis->itemID, (LPARAM) str); 
                GetTextMetrics(lpdis->hDC, &tm); 
 
                y = (lpdis->rcItem.bottom + lpdis->rcItem.top -tm.tmHeight) / 2; 
				if ((pLoc=strchr (str,'\t')))
					*pLoc++ = 0;
				Display = atoi (pLoc);
				if ((pLoc=strchr (pLoc,'\t')))
					*pLoc++ = 0;
				pCount = pLoc;
				if ((pLoc=strchr (pLoc,'\t')))
					*pLoc++ = 0;
				Color = atoi (pLoc);
 				if ((pLoc=strchr (pLoc,'\t')))
					*pLoc++ = 0;
				Connected = atoi(pLoc);
                TextOut(lpdis->hDC,lpdis->rcItem.left,y,str,strlen(str)); 
                Point.x=lpdis->rcItem.left + 20;
                Point.y=(lpdis->rcItem.top + lpdis->rcItem.bottom)/2;
						   
				Rect = lpdis->rcItem;
				Rect.left = Tab[0];
				Rect.right = Tab[1];
				ShowCheck (lpdis->hDC,&Rect,Display,&DisplayRect);
                TextOut(lpdis->hDC,Tab[1],y,pCount,strlen(pCount)); 
				Rect.left = Tab[2];
				Rect.right = Tab[3];
				ColorRect = Rect;
				InflateRect (&ColorRect,-2,-2);
				FillRectColor (lpdis->hDC,&ColorRect,Color);
 				Rect.left = Tab[3];
				Rect.right = lpdis->rcItem.right;
				ShowCheck (lpdis->hDC,&Rect,Connected,&ConnectedRect);

                /* Is the item selected? */ 
 
                if (lpdis->itemState & ODS_SELECTED) { 
 
 
                    /* 
                     * Draw a rectangle around bitmap to indicate 
                     * the selection. 
                     */ 
                    
                    if (LastRect.top != -1000)
                    	DrawFocusRect(lpdis->hDC, &LastRect);
                    //DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
                    //LastRect = lpdis->rcItem; 
                }
            } 
                break; 
 
            case ODA_FOCUS: 
                //InvertRect(lpdis->hDC, &lpdis->rcItem); 
 
                break; 
        } 
        return TRUE; 
    
    case WM_COMMAND:

		 switch(LOWORD (wParam))

           {
            case IDCANCEL:
                EndDialog(hWndDlg, FALSE);
				break;

            case IDOK: 
				{
					MNMXCORD	Bounds,TotBounds;
					BOOL		err;
					HFILE		FidTemp;

					DBoundsInit (&TotBounds);

					Choice = 0;
					while (SendDlgItemMessage(hWndDlg,IDC_PMVEHLIST, LB_GETTEXT,Choice++, (LPARAM) str) != LB_ERR)
					{
						pLoc = strchr (str,'\t');
						*pLoc++ = 0;
						strcpy (VehID,str);
						Display = atoi (pLoc);
						pLoc = strchr (pLoc,'\t');
						*pLoc++ = 0;
						Count = atoi (pLoc);
						pLoc = strchr (pLoc,'\t');
						*pLoc++ = 0;
						color = atoi (pLoc);
						pLoc = strchr (pLoc,'\t');
						*pLoc++ = 0;
						Connect = atoi (pLoc);
						pLoc = strchr (pLoc,'\t');
						*pLoc++ = 0;
						strcpy (VDCONNID,pLoc);
						pLoc = strchr (pLoc,'\t');
						*pLoc++ = 0;
						Bounds = atobounds (pLoc,&err);
						if (Display)
							AddMinMaxD (&TotBounds,&Bounds);  
					}
					{
						HANDLE		hSymDesc=0;
						short		NumSyms=0; 
						short		PointSym=GetDictSymbolNumber ("CIRCLE");
						short		LineSym=GetDictSymbolNumber ("PMLINE");
						long		Refno = 109099900;
						char		KeyString[256],UpdateString[256];

						PMBounds = TotBounds;
						sprintf (PltName,"[%%PROGMONDIR]\\processmon.plt");
						if (ValidBounds (&TotBounds))
						{
							CreateNewMap (PltName,&TotBounds,0,0,0,0,0,0,FALSE);
							AddToSymList (PointSym,&NumSyms,&hSymDesc); 
						}
						Choice = 0;
						while (SendDlgItemMessage(hWndDlg,IDC_PMVEHLIST, LB_GETTEXT,Choice++, (LPARAM) str) != LB_ERR)
						{
							pLoc = strchr (str,'\t');
							*pLoc++ = 0;
							strcpy (VehID,str);
							Display = atoi (pLoc);
							pLoc = strchr (pLoc,'\t');
							*pLoc++ = 0;
							Count = atoi (pLoc);
							pLoc = strchr (pLoc,'\t');
							*pLoc++ = 0;
							color = atoi (pLoc);
							pLoc = strchr (pLoc,'\t');
							*pLoc++ = 0;
							Connect = atoi (pLoc);
							pLoc = strchr (pLoc,'\t');
							*pLoc++ = 0;
							pEnd = strchr (pLoc,'\t');
							*pEnd++ = 0;
							strcpy (VDCONNID,pLoc);
							sprintf (KeyString,"VDCONNID=%s",VDCONNID);
							sprintf (UpdateString,"VDHISTSELECT=%i;VDHISTCOLOR=%i;VDHISTCONNECT=%i",Display,color,Connect);
							UpdateGMDFile ("[VEHICLEDATAFILE]",KeyString,UpdateString,';',1);
							if (Display && Connect)
								AddToSymList (LineSym,&NumSyms,&hSymDesc); 
							if (Display && ValidBounds (&TotBounds))
								AddVehicleToPMMap (VDCONNID,VehID,color,Connect,PointSym,LineSym,&Refno);
						}
						if (ValidBounds (&TotBounds))
						{
							CloseMap(TRUE);  
							AddSymToMap (NumSyms,hSymDesc,0,NULL); 
							DestroySymList (&NumSyms,&hSymDesc); 
							ForceRefIndex = ForceTAGIndex = FALSE;
							FidTemp = GSSiOpenFile ("[%PROGMONDIR]\\filelist.txt",0,OF_CREATE);
							SearchFilesInDir ("[%PROGMONDIR]", ".plt", FidTemp,&TotFiles,"*.plt",1,TRUE,TRUE);     
							GSSiClose (FidTemp);
							ZoomToRect(TotBounds,FALSE);  
						}
					}
				}

                EndDialog(hWndDlg, TRUE);
                break;
        	case IDC_PMVEHLIST:
			switch(HIWORD(wParam))
            {
                case LBN_SELCHANGE:
					{
						HWND	hWnd=GetDlgItem (hWndDlg,IDC_PMVEHLIST);
						RECT	Rect;
						POINT	Point;
						int		width;
						BOOL	DoChange=FALSE;

						GetWindowRect (hWnd,&Rect);
						width = Rect.right - Rect.left;
						Choice=SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0); 
						SendDlgItemMessage(hWndDlg,IDC_PMVEHLIST, LB_GETTEXT,Choice, (LPARAM) str); 
						pLoc = strchr (str,'\t');
						*pLoc++ = 0;
						strcpy (VehID,str);
						Display = atoi (pLoc);
						pLoc = strchr (pLoc,'\t');
						*pLoc++ = 0;
						Count = atoi (pLoc);
						pLoc = strchr (pLoc,'\t');
						*pLoc++ = 0;
						color = atoi (pLoc);
						pLoc = strchr (pLoc,'\t');
						*pLoc++ = 0;
						Connect = atoi (pLoc);
						pLoc = strchr (pLoc,'\t');
						*pLoc++ = 0;
						pEnd = strchr (pLoc,'\t');
						*pEnd++ = 0;
						strcpy (VDCONNID,pLoc);
						strcpy (BoundsC,pEnd);
						GetCursorPos (&Point); 
						ScreenToClient (hWnd,&Point);
						if (Point.x >= DisplayRect.left && Point.x <= DisplayRect.right)
						{
							Display = !Display;
							DoChange=TRUE;
						}
						else if (Point.x >= ConnectedRect.left && Point.x <= ConnectedRect.right)
						{
							Connect = !Connect;
							DoChange=TRUE;
						}
						else if (Point.x >= ColorRect.left && Point.x <= ColorRect.right)
						{

							if (GetColor (hWndDlg,&color))
								DoChange = TRUE;
						}
						if (DoChange)
						{
							SendDlgItemMessage(hWndDlg,IDC_PMVEHLIST,LB_DELETESTRING,Choice,0);
							sprintf (str,"%s\t%i\t%i\t%i\t%i\t%s\t%s",VehID,Display,Count,color,Connect,VDCONNID,BoundsC);
			         		SendDlgItemMessage(hWndDlg,IDC_PMVEHLIST,LB_INSERTSTRING,Choice,(DWORD)str);
						}

					}
					break;
            }
              break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL SetVehicleHistorySelection (int iSel)
{
	if (hWndVehHist)
	{
		SendDlgItemMessage (hWndVehHist,IDC_VEH_HISLIST,LB_SETCURSEL,iSel-1,(LPARAM)0);
		return TRUE;
	}
	return FALSE;
}

BOOL FAR PASCAL VEHICLE_HISTORYMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	char	UserID[64], Password[32];
	static	HANDLE	hSaveBM=0;	
	int		iveh,ihour,imin,Choice,ivehid;
	BOOL	Err;
	LPSTR	pLoc, pEnd;
	LPVEHLOCATION	pVehLoc;
	char	str[256], fromtime[32],totime[32],day[32];
	time_t	systime;
	int		ntab, TabStops[5]={30,130,158,212,1350},ii,w,h;
	static	char	CurrentShape[16]="Circle";
	static	COLORREF	CurrentColor=RGB(255,0,0);
	static	char	vehid[64],radio[32],connid[64];
	static	COLORREF	SaveHighlightColor;
	RECT	ClientRect;
	HFILE	FidTemp;
	static	long	inc=0, move=5;
	LPSTR	pBS, pVehicleHistoryID=VehicleHistoryID;

 switch(Message)
   {
    case WM_INITDIALOG:  
   	     //ntab = loadtabs (TabStops);  
		 hWndVehHist = hWndDlg;
		 SaveHighlightColor = HighlightColor;
		 HighlightColor = 0;
		 if (!*pVehicleHistoryID)
 			nInReplayList = 0;
		 SetGlobalValueLong ("%VHTRACEWINDOW",(long)hWndVehHist);
       	 ii=SendDlgItemMessage (hWndDlg,IDC_VEH_HISLIST,LB_SETTABSTOPS,5,(LPARAM)TabStops); 
		 SetGlobalValueLong ("%VHTRACEWINDOW",(long)GetDlgItem (hWndDlg,IDC_VHTRACE));
		 SetGlobalValueLong ("%VHWINDOW",(long)hWndDlg);
//    	 hSaveBM = EnterBlockingWindow (hWndDlg);
 		 SendDlgItemMessage (hWndDlg,IDC_VHCONNECT,BM_SETCHECK,TRUE,0L);
         cwCenter(hWndDlg, 0);

		 for (iveh=0;iveh<NumVehicles;iveh++)
		 {
			pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
			if (*pVehLoc->ID2)
			{
				sprintf (str,"%s                    (%s",pVehLoc->ID2,pVehLoc->ID);
				SendDlgItemMessage (hWndDlg,IDC_VH_VEHICLELIST,CB_ADDSTRING,0,(LPARAM)((LPSTR)str));
			}
			GlobalUnlock (hVehicle[iveh]);
		 } 
		 SendDlgItemMessage (hWndDlg,IDC_VHSHAPE,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Circle"));
		 SendDlgItemMessage (hWndDlg,IDC_VHSHAPE,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Square"));
		 //SendDlgItemMessage (hWndDlg,IDC_VHSHAPE,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Triangle"));
		 SendDlgItemMessage (hWndDlg,IDC_VHSHAPE,CB_ADDSTRING,0,(LPARAM)((LPSTR)"Star"));
		 SetDlgItemText (hWndDlg,IDC_VHSHAPE,CurrentShape);
		 time (&systime);
		 sprintf (str,"$CAL(%i,3)",systime);
		 ExpandText (str);
		 if ((pLoc = strchr (str,' ')))
		 {
			 *pLoc++ = 0;
			 pEnd = strchr (pLoc,':');
			 *pEnd++ = 0;
			 SetDlgItemText (hWndDlg,IDC_VHENDHOUR,pLoc);
			 SetDlgItemText (hWndDlg,IDC_VHENDMIN,pEnd);
		 }
		 SetDlgItemText (hWndDlg,IDC_VHSTARTDAY,str);
		 SetDlgItemText (hWndDlg,IDC_VHENDDAY,str);
		 SetDlgItemText (hWndDlg,IDC_VHSTARTHOUR,"00");
		 SetDlgItemText (hWndDlg,IDC_VHSTARTMIN,"00");
		 if ((pBS = strrchr (VehicleHistoryID,'\\')))
		 {
			 pVehicleHistoryID = pBS + 1;
			 pBS = strrchr (pVehicleHistoryID,'.');
			 strcpy (pBS,".txt");
			 FidTemp = GSSiOpenFile (VehicleHistoryID,0,OF_READ);
			 if (FidTemp != HFILE_ERROR)
			 {
				 fgetstring (str,16,FidTemp);
				 fgetstring (str,16,FidTemp);
				 SetDlgItemText (hWndDlg,IDC_VHSTARTDAY,str);
				 fgetstring (str,16,FidTemp);
				 SetDlgItemText (hWndDlg,IDC_VHENDDAY,str);
				 fgetstring (str,16,FidTemp);
				 SetDlgItemText (hWndDlg,IDC_VHSTARTHOUR,str);
				 fgetstring (str,16,FidTemp);
				 SetDlgItemText (hWndDlg,IDC_VHENDHOUR,str);
				 fgetstring (str,16,FidTemp);
				 SetDlgItemText (hWndDlg,IDC_VHSTARTMIN,str);
				 fgetstring (str,16,FidTemp);
				 SetDlgItemText (hWndDlg,IDC_VHENDMIN,str);
				 while (fgetstring (str,250,FidTemp))
		 			SendDlgItemMessage (hWndDlg,IDC_VEH_HISLIST,LB_ADDSTRING,0,(LPARAM)str);
				 GSSiClose (FidTemp);
			 }
			 if ((pBS = strchr (pVehicleHistoryID,'_')))
				*pBS = 0;
		 }
		 SendDlgItemMessage (hWndDlg,IDC_VH_VEHICLELIST,CB_SELECTSTRING,-1,(LPARAM)((LPSTR)pVehicleHistoryID));
			
		 //GetClientRect(hWndDlg,&ClientRect);
		 GetWindowRect(hWndDlg,&ClientRect);
		 w = ClientRect.right - ClientRect.left;
		 h = ClientRect.bottom - ClientRect.top;
		 SetViewport(*pCommandViewport);
		 //ClientRect = ConfigDisplayRect;
		 //GetClientRect(CurView->hWnd,&ClientRect);
		// ClientRectToScreenRect (CurView->hWnd,&ClientRect);
		// SetWindowPos(hWndDlg,HWND_TOP,ClientRect.right-w,ClientRect.top+1,0,0,SWP_NOSIZE|SWP_NOZORDER);
		 //MoveWindow(hWndDlg, ClientRect.left+inc, ClientRect.top+inc, w,h, FALSE);
		 SetWindowPos (hWndDlg,0,ClientRect.left+inc, ClientRect.top+inc, w,h,SWP_NOZORDER|SWP_NOOWNERZORDER);
		 move *= -33;
		 inc = move % 75;
		 BlockVehicleDisplay = 1;
		 return 0;
         break; /* End of WM_INITDIALOG                                 */
	
	case WM_DESTROY:
		 hWndVehHist = 0;
		 HighlightColor = SaveHighlightColor;
		 BlockVehicleDisplay = 0;
		 SetGlobalValueLong ("%VHTRACEWINDOW",(long)hWndVehHist);
		 break;

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
		 hWndVehHist = 0;
		 SetGlobalValueLong ("%VHTRACEWINDOW",0);
		 SetGlobalValueLong ("%VHWINDOW",0);
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

	case WM_PAINT:
    	 PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWSHAPE, 0L);
		 return 0;
 
	case WM_COMMAND:
         switch(LOWORD(wParam))
         {
            case IDCANCEL: 
               DestroyWindow (hWndDlg);
			   PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
            break; 
            
			case IDC_EXIT:
				PostMessage(hWndDlg, WM_CLOSE,0, 0L);
			break;

			case 0:
				SetDlgItemText (hWndDlg,IDC_VHTRACE,"History complete - Adding points to map");
				ShowWindow (GetDlgItem(hWndDlg,IDC_VEH_HISLIST),SW_SHOW);
				EnableWindow (GetDlgItem(hWndDlg,IDC_MAPHISTORY),TRUE);
		    	PostMessage(hWndDlg,WM_COMMAND,  IDC_MAPHISTORY, 0L);

			break;
			case 9991:
				GetGlobalCVal ("[%VEHHISDATA]",str,0);
				SendDlgItemMessage (hWndDlg,IDC_VEH_HISLIST,LB_ADDSTRING,0,(LPARAM)str);
			break;
			
			case IDC_SHOWSHAPE:
				DisplaySymInDlg (hWndDlg,IDC_VHSYMBOL,CurrentShape,CurrentColor);
			break;

			case IDC_VHSHAPE:
			{
				switch(HIWORD(wParam))
				{
            		case CBN_SELCHANGE:
						Choice=SendDlgItemMessage(hWndDlg,IDC_VHSHAPE,CB_GETCURSEL,0,0); 
						SendDlgItemMessage(hWndDlg,IDC_VHSHAPE,CB_GETLBTEXT,Choice,(LPARAM)CurrentShape); 
						DisplaySymInDlg (hWndDlg,IDC_VHSYMBOL,CurrentShape,CurrentColor);
						break;
				}
			}
            break;

            case IDC_SELECTVHFILL: 
			{
            	 COLORREF	Color = CurrentColor;

            	 if (GetColor (hWndDlg,&Color))
				 {
            	 	CurrentColor = Color;
					DisplaySymInDlg (hWndDlg,IDC_VHSYMBOL,CurrentShape,CurrentColor);
				 }
			}
            break;
            
			case IDC_REPLAYVEHHISTORY:
				if (GetGlobalCVal ("[VHREPLAYFILE]",HistoryReplayFile,0))
				{
					//char macro[]="$MACRO([%DL]macros\\HistoryReplay.txt,START)";

					//ProcessText (macro);
					//OpenTCPReplay (HistoryReplayFile);
					//StartVehTimeMenu (hWndMain);
					if ((VehReplayFid = GSSiOpenFile (HistoryReplayFile,0,OF_READ)) != HFILE_ERROR)
					{
						int	iveh;

						for (iveh=0;iveh<NumVehicles;iveh++)
						{
							LPVEHLOCATION	pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
							
							if (VehicleInReplayList (pVehLoc->ID))
							{
								pVehLoc->nLoc = 0;
								pVehLoc->Display = TRUE;
							}
							GlobalUnlock (hVehicle[iveh]);
						}

						CreateDialog(hInst, (LPSTR)"VEHICLE_REPLAY", hWndMain, VEHICLE_TIMEMsgProc);
					}
				
				}
				
				break;

            case IDOK:
				{
					LPSTR pconnid;

            	GetDlgItemText (hWndDlg,IDC_VH_VEHICLELIST,str,128);
				ShowWindow (GetDlgItem(hWndDlg,IDC_VEH_HISLIST),SW_HIDE);
			    SendDlgItemMessage (hWndDlg,IDC_VEH_HISLIST,LB_RESETCONTENT,0,0);
				EnableWindow (GetDlgItem(hWndDlg,IDC_MAPHISTORY),FALSE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_REPLAYVEHHISTORY),FALSE);
				strncpy (vehid,str,8);
				Truncate (vehid);
				GetDlgItemText (hWndDlg,IDC_VHSTARTDAY,day,sizeof(day));
				ihour = GetDlgItemInt (hWndDlg,IDC_VHSTARTHOUR,&Err,FALSE);
				imin  = GetDlgItemInt (hWndDlg,IDC_VHSTARTMIN,&Err,FALSE);
				sprintf (fromtime,"%s %2.2i:%2.2i",day,ihour,imin);
				GetDlgItemText (hWndDlg,IDC_VHENDDAY,day,sizeof(day));
				ihour = GetDlgItemInt (hWndDlg,IDC_VHENDHOUR,&Err,FALSE);
				imin  = GetDlgItemInt (hWndDlg,IDC_VHENDMIN,&Err,FALSE);
				sprintf (totime,"%s %2.2i:%2.2i",day,ihour,imin);
				pconnid = strchr (str,'(');
				if (!pconnid)
				{
					MessageBox (hWndDlg,"No vehicle selected",0,MB_ICONEXCLAMATION);
					break;
				}
				pconnid++;
				if (stricmp (fromtime,totime) >= 0)
				{
					MessageBox (hWndDlg,"End time must be greater that start time",0,MB_ICONEXCLAMATION);
					break;
				}
				if (GetGlobalCVal ("[VHREPLAYFILE]",HistoryReplayFile,0))
					GSSiRemove (HistoryReplayFile);
				strcpy (connid,pconnid);
				nInReplayList = 1;
				strcpy (VehicleReplayList[0],pconnid);

				sprintf (str,"$TCPSEND([SOCKET],>GetHistory:%s@,%s@,%s@;)",connid,fromtime,totime);
				ProcessText (str);
				sprintf (str,"Retrieving history for vehicle %s",vehid);
				SetDlgItemText (hWndDlg,IDC_VHTRACE,str);
				}
            break;

			case IDC_VEH_HISLIST:
			{
				LPSTR pLoc;
				DPOINT	DPoint;
				int		inc=12;
				RECT	Rect;
				POINT	Point;
				char		UDI[64];
				int			Seq;
				LPSTR		Time;

                switch(HIWORD(wParam))
                {
					case LBN_SELCHANGE:
					{
						BOOL SavePatternBrush=PatternBrush;
						HDC	hDC=GetDC (hWndMain);

						//PatternBrush = FALSE;
						SaveDC (hDC);
						SetDisplayMode (hDC, GF_SCREENMODE);  
						SelectClipRgn (hDC,0);
			 			SetViewport (*pCommandViewport);
						Choice=SendDlgItemMessage(hWndDlg,IDC_VEH_HISLIST,LB_GETCURSEL,0,0); 
		         		SendDlgItemMessage(hWndDlg,IDC_VEH_HISLIST,LB_GETTEXT,Choice,(DWORD)str);
						pLoc = strchr (str,'\t');
						*pLoc++ = 0;
						Seq = atoi (str);
						Time = pLoc;
						pLoc = strchr (Time,'\t');
						*pLoc++ = 0;
						pLoc = strrchr (pLoc,'\t');
						pLoc++;
						DPoint = atopt (pLoc,&Err);
						ConvertCoord(&DPoint,2,1);
						sprintf (UDI,"%s-%i %s",vehid,Seq,Time);
						Point = BasePtToScreenPt (&DPoint);
						Rect.left = Point.x - inc;
						Rect.right = Point.x + inc;
						Rect.top = Point.y + inc;
						Rect.bottom = Point.y - inc;
						InvertRect (hDC,&Rect);
						GdiFlush ();
						Sleep (500);
						InvertRect (hDC,&Rect);
						RestoreDC (hDC,-1);
						ReleaseDC (hWndMain,hDC);
/*						ClearHighlightList (TRUE); 
						if (PickByRefno (0,"VEHHIST",UDI,FALSE))
						{
							PickList[0].Type = 1;
			    			AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE);
   				    		ShowPickedItem (CurView->hWnd,0); 
						}
						PatternBrush = SavePatternBrush;
						*/
					}
				}
				break;
			}

			case IDC_MAPHISTORY:
				{
					char		DataFile[MAX_PATH];
					MNMXCORD	Bounds;
					int			TotFiles=0;
					char		SymName[32];
					
					if (!SendDlgItemMessage (hWndDlg,IDC_VEH_HISLIST,LB_GETCOUNT,0,0)) 
					{
						MessageBox (hWndDlg,"No history for this vehicle in this time frame","",MB_ICONEXCLAMATION);
						break;
					}
			 		SetViewport (*pCommandViewport);
					Choice = 0;
					//strcpy (TempName,"c:\\map.txt");
					time (&systime);
					sprintf (PltName,"[%%VEHHISTDIR]\\%s_%ld.plt",vehid,systime);
					sprintf (DataFile,"[%%VEHHISTDIR]\\%s_%ld.txt",vehid,systime);
					FidTemp = GSSiOpenFile (DataFile,0,OF_CREATE);
					fputstring (vehid,FidTemp);
					GetDlgItemText (hWndDlg,IDC_VHSTARTDAY,str,16);
					fputstring (str,FidTemp);
					GetDlgItemText (hWndDlg,IDC_VHENDDAY,str,16);
					fputstring (str,FidTemp);
					GetDlgItemText (hWndDlg,IDC_VHSTARTHOUR,str,16);
					fputstring (str,FidTemp);
					GetDlgItemText (hWndDlg,IDC_VHENDHOUR,str,16);
					fputstring (str,FidTemp);
					GetDlgItemText (hWndDlg,IDC_VHSTARTMIN,str,16);
					fputstring (str,FidTemp);
					GetDlgItemText (hWndDlg,IDC_VHENDMIN,str,16);
					fputstring (str,FidTemp);
					while (SendDlgItemMessage(hWndDlg,IDC_VEH_HISLIST,LB_GETTEXT,Choice++,(DWORD)str) != LB_ERR)
						fputstring (str,FidTemp);
					GSSiClose (FidTemp);
					sprintf (SymName,"VH%s",CurrentShape);
					CreateVehHistMap (DataFile,PltName,vehid,radio,SymName,CurrentColor,
									  (BOOL)SendDlgItemMessage (hWndDlg,IDC_VHNUMBER,BM_GETCHECK,0,0L),
									  (BOOL)SendDlgItemMessage (hWndDlg,IDC_VHCONNECT,BM_GETCHECK,0,0L),
									  &Bounds);
					FidTemp = GSSiOpenFile ("[%VEHHISTDIR]\\filelist.txt",0,OF_CREATE);
					SearchFilesInDir ("[%VEHHISTDIR]", ".plt", FidTemp,&TotFiles,"*.plt",1,TRUE,TRUE);     
					GSSiClose (FidTemp);
					EnableWindow (GetDlgItem(hWndDlg,IDC_REPLAYVEHHISTORY),TRUE);
					SetDlgItemText (hWndDlg,IDC_VHTRACE,"Done");
					ZoomToRect(Bounds,FALSE);  
			    	//PostMessage(hWndDlg, WM_CLOSE,0, 0L);
				}
			break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 

int GetVehicleIndex (LPSTR Vehid,HWND hWndDlg)
{
	int	iveh, index=0;
	char	str[256];
	LPSTR	pTab,pTab2;
/*	LPVEHLOCATION	pVehLoc;
	char	str[256];

	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (!stricmp (pVehLoc->ID,Vehid))
		{
			index = pVehLoc->index;
			GlobalUnlock (hVehicle[iveh]);
			return iveh;
		}
		GlobalUnlock (hVehicle[iveh]);
	}*/
	while (SendDlgItemMessage (hWndDlg,IDC_GF_VEHLIST,LB_GETTEXT,index,(LPARAM)str)!=LB_ERR)
	{
		pTab = strchr (str,'\t');
		*pTab++ = 0;
		pTab = strchr (pTab,'\t');
		*pTab++ = 0;
		pTab2 = strchr (pTab,'\t');
		*pTab2++ = 0;
		if (!stricmp (Vehid,pTab))
			return index;
		index++;
	}
	return -1;
}

void CreateVehicleList (HWND hWndDlg)
{
	int	iveh;
	LPVEHLOCATION	pVehLoc;
	char	str[256];

	SendDlgItemMessage (hWndDlg,IDC_GF_VEHLIST,LB_RESETCONTENT,0,0);
	for (iveh=0;iveh<NumVehicles;iveh++)
	{
		pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);
		if (*pVehLoc->ID2)
		{
			sprintf (str,"%s\t%s\t%s\t%i",pVehLoc->ID2,pVehLoc->Desc,pVehLoc->ID,iveh);
			SendDlgItemMessage (hWndDlg,IDC_GF_VEHLIST,LB_ADDSTRING,0,(LPARAM)((LPSTR)str));
		}
		GlobalUnlock (hVehicle[iveh]);
	}
	return;
}

LPSTR FenceOffsetText (int i)
{
	static	char	text[128];
	LPSTR	pchar = (LPSTR)text;

	*pchar = 0;
	if (FenceMethod[i] > 0)
		sprintf (pchar,"%f%c",FenceOffset[i],FenceOffsetUnits[i]);

	return pchar;
}

LPSTR YorBlank (BOOL v)
{
	static char YB[2];
	LPSTR	pYB = (LPSTR)YB;

	if (v)
		strcpy (pYB,"Y");
	else
		strcpy (pYB," ");
	return pYB;
}

BOOL FAR PASCAL GEOFENCEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	LPVEHLOCATION	pVehLoc;  
	char	str[512], str2[256], Ext[8]=".gfb";
	double	Factor[2]={FTM,5280*FTM};
	int		iveh,i,j,index,ntab=6;
	int		TabStops[6]={240,282,320,382,430,2000};
	int		TabStops2[3]={94,134,2000};
	int		TabStops3[3]={100,150,2000};
	char	FType[2][12]={"Exclusive","Inclusive"};
	char	ActInact[2][10]={"Inactive","Active"};
	char	FMethod[3][10]={"Polygon","Circle","Route"};
	char	FLinkVeh[128], FName[64], VID[32];
	LPSTR	pTab;
	HANDLE	hItems;
	LPINT	pItem;

 switch(Message)
   {
    case WM_INITDIALOG:  
   	     //ntab = loadtabs (TabStops3);  
		 hWndGeoFence = hWndDlg;
         cwCenter(hWndDlg, 0);
		 BlockVehicleDisplay = 1;
		 SendDlgItemMessage (hWndDlg,IDC_GF_FENCELIST,LB_SETTABSTOPS,6,(LPARAM)TabStops); 
		 SendDlgItemMessage (hWndDlg,IDC_GF_VEHLIST,LB_SETTABSTOPS,3,(LPARAM)TabStops2); 
		 SendDlgItemMessage (hWndDlg,IDC_GF_STOPS,LB_SETTABSTOPS,3,(LPARAM)TabStops3); 
		 
		 SendDlgItemMessage (hWndDlg,IDC_GF_POLYGON,BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_GF_EXCLUSIVE,BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_GF_ATTACH_ALLVEH,BM_SETCHECK,TRUE,0L);
		 
//		 SendDlgItemMessage (hWndDlg,IDC_GF_ATTACH_ALLVEH,BM_SETCHECK,TRUE,0L);
//		 SendDlgItemMessage (hWndDlg,IDC_GF_NOTIFY_FLASH,BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_GF_OFFSETUNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
	     SendDlgItemMessage (hWndDlg,IDC_GF_OFFSETUNITS,CB_ADDSTRING,0,(LPARAM)"Miles"); 
 		 SendDlgItemMessage (hWndDlg,IDC_GF_OFFSETUNITS,CB_SETCURSEL,(WPARAM)0,(LPARAM)0); 
Reload:
		 SendDlgItemMessage (hWndDlg,IDC_GF_FENCELIST,LB_RESETCONTENT,0,0);
		 SendDlgItemMessage (hWndDlg,IDC_GF_VEHLIST,LB_RESETCONTENT,0,0);
		 SendDlgItemMessage (hWndDlg,IDC_GF_STOPS,LB_RESETCONTENT,0,0);
		 for (i=0;i<NumFencesInList;i++)
		 {
			sprintf (str,"%s\t%s\t%s\t%s\t%s\t%s\t%i",FenceName[i],FMethod[FenceMethod[i]],FType[FenceInclusive[i]],FenceOffsetText(i),ActInact[FenceActive[i]],YorBlank(FenceCritical[i]),i);
			SendDlgItemMessage (hWndDlg,IDC_GF_FENCELIST,LB_ADDSTRING,0,(LPARAM)((LPSTR)str));
		 }

         break; /* End of WM_INITDIALOG                                 */
	
	case WM_DESTROY:
		 BlockVehicleDisplay = 0;
		 hWndGeoFence = 0;
		 return FALSE;

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

	case WM_COMMAND:
         switch(LOWORD(wParam))
         {
			case IDC_UPLOADFENCES:
				{
					char	CAN[64]="[CAN]";
					
					strcpy (str,"$FENCE(EXPORT,[FENCESERVER],[FENCEPORT],[CAN])");	
					ExpandText (str);
					if (*str == '1')
					{
						ProcessText ("[C]=$TCPSEND([SOCKET],>FenceUpdate:[CAN]@;$CHR(13)$CHR(10));");
						MessageBox (hWndDlg,"Upload successful","",MB_OK);
					}
					else
						MessageBox (hWndDlg,"Upload failed","",MB_OK);
					break;
				}
			case IDC_GF_NAME:
            switch(HIWORD(wParam))
            {   
             case EN_CHANGE:
				EnableWindow (GetDlgItem(hWndDlg,IDOK),GetDlgItemText (hWndDlg,IDC_GF_NAME,str,sizeof(str)));
                break;
            }
			break;

			case IDC_SAVEFENCE:
				*str = 0;
            	if (GetSaveName2 (hWndDlg,str,0,Ext,IDS_FILEGFB))  
            	{
					char	CAN[64]="[CAN]";

					ExpandText (CAN);
					HaltMapDisplay(FALSE); 
					CloseAllRequestedFiles (FALSE);
					BlockSocketProcessing (7);
					SaveFencesToFile (str,CAN);
					BlockSocketProcessing (FALSE);
					sprintf (str2,"Fences save to %s",str);
					MessageBox (hWndDlg,str2,"",MB_OK);
				}
				break;

			case IDC_RESTOREFENCE:
				*str = 0;
				if (GetFileName2 (hWndDlg,str,Ext,IDS_FILEGFB))
            	{
					char	CAN[64]="[CAN]";

	                WaitCursor (1);
					ExpandText (CAN);
					HaltMapDisplay(FALSE); 
					CloseAllRequestedFiles (FALSE);
					BlockSocketProcessing (8);
					RecallFencesFromFile (str,CAN);
					BlockSocketProcessing (FALSE);
					sprintf (str2,"Fences stored from %s",str);
					WaitCursor (-1);
					MessageBox (hWndDlg,str2,"",MB_OK);
					goto Reload;
				}
				break;

		    case IDC_GF_POLYGON:
				ShowWindow (GetDlgItem(hWndDlg,IDC_GF_OFFSETHEADER),SW_HIDE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_GF_OFFSET),SW_HIDE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_GF_OFFSETUNITS),SW_HIDE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_GF_EXCLUSIVE),TRUE);
			break;

			case IDC_GF_ROUTE:
				SendDlgItemMessage (hWndDlg,IDC_GF_EXCLUSIVE,BM_SETCHECK,FALSE,0L);
				SendDlgItemMessage (hWndDlg,IDC_GF_INCLUSIVE,BM_SETCHECK,TRUE,0L);
		    case IDC_GF_CIRCLE:
				ShowWindow (GetDlgItem(hWndDlg,IDC_GF_OFFSETHEADER),SW_SHOW);
				ShowWindow (GetDlgItem(hWndDlg,IDC_GF_OFFSET),SW_SHOW);
				ShowWindow (GetDlgItem(hWndDlg,IDC_GF_OFFSETUNITS),SW_SHOW);
				EnableWindow (GetDlgItem(hWndDlg,IDC_GF_EXCLUSIVE),LOWORD(wParam)==IDC_GF_CIRCLE);
			break;

			case IDC_FENCE_CRITICAL:
			case IDC_FENCE_NOTCRITICAL:
				{
					int	nItems=GetLBSelectedItems (hWndDlg,IDC_GF_FENCELIST,&hItems);
					
					if (!hItems)
						break;
					pItem = (LPINT)GlobalLock (hItems);
					for (i=0;i<nItems;i++,pItem++)
					{
						SendDlgItemMessage(hWndDlg,IDC_GF_FENCELIST,LB_GETTEXT,*pItem,(DWORD)str); 
						if ((pTab = TabFromEnd (str,1)))
						{
							int	ifence = atoi(pTab+1);
							FenceCritical[ifence] = (LOWORD(wParam)==IDC_FENCE_CRITICAL);
						}
					}
					GSSiGlobUlFree (&hItems);
					goto Reload;
				}
				break;

			case IDC_FENCEACTIVATE:
			case IDC_FENCEDEACTIVATE:
				{
					int	nItems=GetLBSelectedItems (hWndDlg,IDC_GF_FENCELIST,&hItems);
					
					if (!hItems)
						break;
					pItem = (LPINT)GlobalLock (hItems);
					for (i=0;i<nItems;i++,pItem++)
					{
						SendDlgItemMessage(hWndDlg,IDC_GF_FENCELIST,LB_GETTEXT,*pItem,(DWORD)str); 
						if ((pTab = TabFromEnd (str,1)))
						{
							int	ifence = atoi(pTab+1);
							FenceActive[ifence] = (LOWORD(wParam)==IDC_FENCEACTIVATE);
							ActivateDeactivateFence (FenceName[ifence],FenceActive[ifence]);
						}
					}
					GSSiGlobUlFree (&hItems);
					goto Reload;
				}
				break;

			case IDC_GF_FENCELIST:
               {
                switch(HIWORD(wParam))
                {
                    case LBN_SELCHANGE:
					{
						BOOL Enable = SendDlgItemMessage(hWndDlg,IDC_GF_FENCELIST,LB_GETSELCOUNT,0,0);

						EnableWindow (GetDlgItem(hWndDlg,IDC_FENCEDELETE),Enable);
						EnableWindow (GetDlgItem(hWndDlg,IDC_FENCEACTIVATE),Enable);
						EnableWindow (GetDlgItem(hWndDlg,IDC_FENCEDEACTIVATE),Enable);
						EnableWindow (GetDlgItem(hWndDlg,IDC_FENCE_CRITICAL),Enable);
						EnableWindow (GetDlgItem(hWndDlg,IDC_FENCE_NOTCRITICAL),Enable);
						EnableWindow (GetDlgItem(hWndDlg,IDC_UPDATE_VEHLINK),Enable);
						SendDlgItemMessage (hWndDlg,IDC_GF_VEHLIST,LB_RESETCONTENT,0,0);
						SendDlgItemMessage (hWndDlg,IDC_GF_STOPS,LB_RESETCONTENT,0,0);
						SendDlgItemMessage (hWndDlg,IDC_GF_ATTACH_ALLVEH,BM_SETCHECK,TRUE,0L);
						if (Enable == 1)
						{
							HANDLE	hDB=0, hItems;
							char	SQL[128];
							int		nItems=GetLBSelectedItems (hWndDlg,IDC_GF_FENCELIST,&hItems);
							LPINT	pItem = (LPINT)GlobalLock (hItems);
	
							SendDlgItemMessage(hWndDlg,IDC_GF_FENCELIST,LB_GETTEXT,*pItem,(DWORD)str); 
							GSSiGlobUlFree (&hItems);
							pTab = strchr (str,'\t');
							*pTab++ = 0;
							strcpy (FName,str);
							sprintf (SQL,"FENCENAME=%s",FName);
						    if (OpenDataFile ("FLINK=[%GEOFENCEDIR]\\fencelink.gmd",SQL,BT_READ,&hDB))
							{
								if (FetchDBRec (hDB))
								{
									strcpy (FLinkVeh,"[FLINK.VID]");
									ExpandText (FLinkVeh);
									if (stricmp (FLinkVeh,"ALL"))
									{
										SendDlgItemMessage (hWndDlg,IDC_GF_ATTACH_ALLVEH,BM_SETCHECK,FALSE,0L);
										CreateVehicleList (hWndDlg);
										do
										{
											strcpy (FLinkVeh,"[FLINK.VID]");
											ExpandText (FLinkVeh);
											index = GetVehicleIndex (FLinkVeh,hWndDlg);
											SendDlgItemMessage (hWndDlg,IDC_GF_VEHLIST,LB_SETSEL,TRUE,(LPARAM)index);
										}while (FetchDBRec (hDB)); 
									}
								}
								CloseDataFile (TRUE,&hDB);
							}
							sprintf (SQL,"ROUTE=%s",FName);
						    if (OpenDataFile ("STOPS=[%GEOFENCEDIR]\\routestops.gmd",SQL,BT_READ,&hDB))
							{
								while (FetchDBRec (hDB))
								{
									char	Dist[32]="[STOPS.DIST]";
									int		idist;

									ExpandText (Dist);
									idist = atoi (Dist);
									if (idist >= 0 && idist < 2000000000)
									{
										strcpy (str,"[STOPS.NAME]\t$CAL([STOPS.TIME],11)\t[STOPS.DELAY]");
										ExpandText (str);
										SendDlgItemMessage(hWndDlg,IDC_GF_STOPS,LB_ADDSTRING,0,(LPARAM)str);
									}
								}
								CloseDataFile (TRUE,&hDB);
							}
						}
					}
				}
			}
			break;

			case IDC_UPDATE_VEHLINK:
				{
					int	nItems=GetLBSelectedItems (hWndDlg,IDC_GF_FENCELIST,&hItems);

					if (!hItems)
						break;
					pItem = (LPINT)GlobalLock (hItems);
					for (i=0;i<nItems;i++,pItem++)
					{
						SendDlgItemMessage(hWndDlg,IDC_GF_FENCELIST,LB_GETTEXT,*pItem,(DWORD)str); 
						if ((pTab = strchr (str,'\t')))
						{
							*pTab = 0;
							strcpy (FName,str);
							RemoveLinksForFence (FName);
							if (SendDlgItemMessage (hWndDlg,IDC_GF_ATTACH_ALLVEH,BM_GETCHECK,0,0L))
							{
								sprintf (str,"$GMDUPDATE([%%GEOFENCEDIR]\\fencelink.gmd,VID=ALL;FENCENAME=%s,VID2=ALL)",FName);
								ProcessText(str);
							}
							else
							{
								HANDLE	hItems=0;
								LPINT	pItem;
								int	nItems=GetLBSelectedItems (hWndDlg,IDC_GF_VEHLIST,&hItems);

								pItem = (LPINT)GlobalLock (hItems);
								for (j=0;j<nItems;j++,pItem++)
								{
									SendDlgItemMessage(hWndDlg,IDC_GF_VEHLIST,LB_GETTEXT,*pItem,(DWORD)str2); 
									if ((pTab = strrchr (str2,'\t')))
									{
										*pTab++ = 0;
										pTab = strrchr (str2,'\t');
										*pTab++ = 0;
										strcpy (VID,pTab);
										sprintf (str,"$GMDUPDATE([%%GEOFENCEDIR]\\fencelink.gmd,VID=%s;FENCENAME=%s,VID2=ALL)",VID,FName);
										ProcessText(str);
									}
								}
								GSSiGlobUlFree (&hItems);
							}
 						}
					}
					GSSiGlobUlFree (&hItems);
					break;
				}

				break;

			case IDC_FENCEDELETE:
				{
					HANDLE	hItems=0;
					LPINT	pItem;
					LPSTR	pTab;
					int	nItems=GetLBSelectedItems (hWndDlg,IDC_GF_FENCELIST,&hItems);

					pItem = (LPINT)GlobalLock (hItems);
					for (i=0;i<nItems;i++,pItem++)
					{
						SendDlgItemMessage(hWndDlg,IDC_GF_FENCELIST,LB_GETTEXT,*pItem,(DWORD)str); 
						if ((pTab = strchr (str,'\t')))
						{
							*pTab = 0;
							DeleteFence (str);
						}
					}
					GSSiGlobUlFree (&hItems);
					goto Reload;
					break;
				}

			case IDC_GF_ATTACH_ALLVEH:
				if (SendDlgItemMessage (hWndDlg,IDC_GF_ATTACH_ALLVEH,BM_GETCHECK,0,0L))
					SendDlgItemMessage (hWndDlg,IDC_GF_VEHLIST,LB_RESETCONTENT,0,0);
				else
					CreateVehicleList (hWndDlg);
			break;

			case IDC_IMPORT_STOPS:
				ProcessText ("$MACRO([%DL]\\macros\\loadstops.txt))");
				break;

            case IDCANCEL: 
               EndDialog(hWndDlg, FALSE);
            break; 
            
			case IDC_EXIT:
				PostMessage(hWndDlg, WM_CLOSE,0, 0L);
			break;


            case IDOK:
			{
				double	Offsetdist;
				int		iunits;
				char	cunits[2][2]={"F","M"};

            	GetDlgItemText (hWndDlg,IDC_GF_NAME,str,128);
				if (!*str)
				{
					MessageBox (hWndDlg,"GeoFence name not set",0,MB_ICONEXCLAMATION);
					break;
				}
				SetGlobalValue("%GEOFENCENAME",str);  
            	GetDlgItemText (hWndDlg,IDC_GF_OFFSET,str,128);
				Offsetdist = atof (str);
                iunits=SendDlgItemMessage(hWndDlg,IDC_GF_OFFSETUNITS,CB_GETCURSEL,0,0); 
				SetGlobalValueReal ("%GEOFENCEOFFSET",Offsetdist);
				SetGlobalValue ("%GEOFENCECRITICAL","F");
				SetGlobalValue ("%GEOFENCEOFFSETUNITS",cunits[iunits]);
				if (SendDlgItemMessage (hWndDlg,IDC_GF_POLYGON,BM_GETCHECK,0,0L))
					SetGlobalValue("%GEOFENCETYPE","POLYGON");  
				else if (SendDlgItemMessage (hWndDlg,IDC_GF_CIRCLE,BM_GETCHECK,0,0L))
				{
					if (Offsetdist <= 0)
					{
						MessageBox (hWndDlg,"You must supply an offset distance",0,MB_ICONEXCLAMATION);
						break;
					}
					SetGlobalValue("%GEOFENCETYPE","CIRCLE"); 
				}
				else if (SendDlgItemMessage (hWndDlg,IDC_GF_ROUTE,BM_GETCHECK,0,0L))
				{
					if (Offsetdist <= 0)
					{
						MessageBox (hWndDlg,"You must supply an offset distance",0,MB_ICONEXCLAMATION);
						break;
					}
					SetGlobalValue("%GEOFENCETYPE","ROUTE"); 
				}
				if (SendDlgItemMessage (hWndDlg,IDC_GF_INCLUSIVE,BM_GETCHECK,0,0L))
					SetGlobalValue("%GEOFENCEINCLUSIVE","1"); 
				else
					SetGlobalValue("%GEOFENCEINCLUSIVE","0"); 
				if (SendDlgItemMessage (hWndDlg,IDC_GF_ATTACH_ALLVEH,BM_GETCHECK,0,0L))
					SetGlobalValue("%GEOFENCEVEHLIST",""); 
				else
				{
					HANDLE	hItems=0;
					LPSTR	TempName,pDot;
					LPINT	pItem;
					HFILE	Fid;

					int	nItems=GetLBSelectedItems (hWndDlg,IDC_GF_VEHLIST,&hItems);

					if (!nItems)
					{
						MessageBox (hWndDlg,"No vehicles selected",0,MB_ICONEXCLAMATION);
						break;
					}
					else
					{
						if (!hGFVehList)
						{
							hGFVehList = GSSiGlobAlloc (0,GMEM_MOVEABLE,MAX_PATH);
							TempName = GlobalLock (hGFVehList);
							GSSiGetTempFileName (0,"gm",0,(LPSTR)TempName);
							pDot = strrchr (TempName,'.');
							strcpy (pDot,".txt");
						}
						else
							TempName = GlobalLock (hGFVehList);
						SetGlobalValue("%GEOFENCEVEHLIST",TempName); 
						Fid = GSSiOpenFile (TempName,0,OF_CREATE);
						GlobalUnlock (hGFVehList);
						strcpy (str,"VEHID\tCONNID");
						fputstring (str,Fid);
						pItem = (LPINT)GlobalLock (hItems);
						for (i=0;i<nItems;i++,pItem++)
						{
							SendDlgItemMessage(hWndDlg,IDC_GF_VEHLIST,LB_GETTEXT,*pItem,(DWORD)str); 
							fputstring (str,Fid);
						}
						GSSiClose (Fid);
					}
					GSSiGlobUlFree (&hItems);
				}
				EndDialog(hWndDlg, TRUE);
			}
            break;

         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 



