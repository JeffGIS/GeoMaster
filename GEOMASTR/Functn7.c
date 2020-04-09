#include "graphint.h"   
#include "extrndb.h"   
#include "pnet.h" 
#include "umio.h"
#include <mmsystem.h>
#include "gmextern.h"
#include "RampCompliance.h"
#include "CurbRamps.h"
#include "CRAPI.h"

static char	DOW[7][10] = { "SUNDAY","MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY" };
static HIGHLIGHTDATA	HighlightData;
BOOL	InAtPrint = FALSE;

int	GetFunctionValue7(int FunID, LPSTR Args, LPSTR OutLoc, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen)
#if ENABLETRACE
{
	GSSiEnterProg(1348);
#endif
	{   HANDLE	hMem = 0, hMem2 = 0, hDLT, hSurf;
	LPSTR	Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, ParLoc, lpstr, lpstrb;
	BOOL	FromLimits, Immediate;
	LPSTR	pEnd, pCR, pFile, Arg[20] = { 0 }, pVal, pPar;
	short	nArgs;
	HFILE	Fid1, Fid2, Fid3;
	int		l, CvtDir, year, i;
	DPOINT	Point, Point2;
	POINT	Point16;
	RECT	Rect;
	HWND	hWnd;
	MNMXCORD	Bounds, Bounds2;
	long	hNum, Refno, ii, nlong;
	LPSTR	lpColon, CmdMess;
	time_t	systime;
	BOOL	TORF, rtn = FALSE, Err, PickVis;
	LPVIEWPORT	SaveVP = CurView;
	short	SaveCfg = CurrentConfig;
	short SymNum, Mode, irc;
	short	n, n1, n2, pos, nrem, nRc;
	double	AZ, Dist, Elevation, RVal;
	HFILE	Fid;
	LPOFSTRUCTGM	pOFStruct;
	LPDPOINT	pPoint;
	HANDLE	hDibInfo;
	long	ImageOffset;
	double	Offset;
	HANDLE	hSQL;
	HANDLE	hTran;
	LPOPENFILEDATA  FilePtr;
	LPOPENSQLDATA   SQLPtr;
	LPFIELDINFO	lpFieldInfo;
	LPSQLFIELD	lpSQLField;
	LPFILEPATH	FilePathPtr;
	LPHANDLE	lpFileHandle;
	HANDLE	hPoints;
	short	nPoints;
	short	ndec;
	short	incDbOff;
	HDC		hDC;
	COLORREF	Color;

	if (LinkToVar)
	{
		ExpandText(Args);
		{
#if ENABLETRACE
			GSSiExitProg(1348);
#endif
			return 0;
		}
	}
	if (TraceOn)
	{
		hMem = GSSiGlobAlloc(785, GMEM_MOVEABLE, 4096);
		lpstr = GlobalLock(hMem);
		sprintf(lpstr, "GFV:%s(%s)", LastFunctionName, Args);
		GSSiTraceLev(lpstr, 1, 2);
		GSSiGlobUlFree(&hMem);
	}
	switch (FunID)
	{

	case 701: /* $LOADVIS(visibility_file,Optional VPName) Load visibility file */
	{
		nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);

		SaveVP = CurView;
		SetCurView(SetVPFromName(Arg[2], &Err));
		rtn = LoadVisList(Arg[1]);
		SetCurView(SaveVP);
		if (rtn)
			goto RtnTrue;
		else
			goto RtnFalse;
	}

	case 702: /* $LOADPIK(pickability_file) Load visibility file */
	{
		nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);

		SaveVP = CurView;
		SetConfig(1);

		SetCurView(SetVPFromName(Arg[2], &Err));
		rtn = 1;
		if (!_fstricmp(Arg[1], "SAME"))
			SetPickSame(CurView);
		else
			rtn = LoadPickList(Arg[1]);
		if (CurView->ID != SaveVP->ID)
			SetCurView(SaveVP);
		if (rtn)
			goto RtnTrue;
		else
			goto RtnFalse;
	}

	case 707: /* $LOADRDF (redef file,VPName(opt)) Load viewport redef file */
	{
		nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 1)
			goto RtnFalse;

		if (Pick || !Display)
			goto RtnTrue;
		SetCurView(SetVPFromName(Arg[2], &Err));
		if (!*Arg[1])
		{
			RemoveVPRedef();
			goto RtnTrue;
		}

		if (LoadDisplayRedefFile(Arg[1]))
		{
			ConfigChangesMade = TRUE;
			goto RtnTrue;
		}
		else
			RemoveVPRedef();
		goto RtnTrue;
	}

	case 711: // $LOADCFG (config file) Load config file
			  // $LOADCFG(,pathname,networkdir,personaldir,startindir) brings up new load control
	{
		BOOL 	SaveTrackingStatus = TrackingStatus, ForceOpen;

		HaltMapDisplay(TRUE, TRUE);
		nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
		ForceBounds = FALSE;
		if (!SaveZooms(0))
			DestroySavedZooms();
		if (!*Arg[1])
		{
			BOOL	StartInNew, RetainZoom = FALSE, LinkZoom = FALSE;
			char	modulePath[MAX_PATH];

			_fstrcpy(Arg[1], Arg[2]);
			if ((pEnd = _fstrrchr(Arg[2], '\\')))
				*pEnd = 0;
			if (!*Arg[5])
			{
				if (!GSSiGetGMCName(hWndMain, Arg[1], Arg[2], "Load GeoMaster Configuration", &StartInNew, &RetainZoom, &LinkZoom, Arg[3], Arg[4]))
					goto RtnFalse;
			}
			else
				StartInNew = TRUE;
			ForceBounds = RetainZoom;
			if (StartInNew)
			{
				CreateGMStartupFile(Arg[7], Arg[1], LinkZoom, RetainZoom);
				long len;
				GSSiGetTempFileName(0, "gmc", 0, Arg[7]);
				Fid = GSSiOpenFile(Arg[7], 0, OF_CREATE);
				if (LinkZoom)
					sprintf(strchr(Arg[1], 0), "(%ld)", (ULONG)hWndMain);
				len = strlen(Arg[1]);
				BigWrite(Fid, (HPSTR)&len, 4, -1);
				BigWrite(Fid, (HPSTR)Arg[1], len, -1);
				if (RetainZoom && hSavedZooms)
				{
					len = GlobalSize(hSavedZooms);
					LPSHORT pNumSavedViews = (LPSHORT)GlobalLock(hSavedZooms);

					BigWrite(Fid, (HPSTR)&len, 4, -1);
					BigWrite(Fid, (HPSTR)pNumSavedViews, len, -1);
					GlobalUnlock(hSavedZooms);
				}
				GSSiClose2(&Fid);
				EscapeFunction(TRUE);
				sprintf(Arg[6], "$SESSION(CREATE,GeoMaster %s,,,%s)", Arg[7], Arg[5]);
				ProcessText(Arg[6]);
				goto RtnTrue;
			}
		}
		if (!ExistFile(Arg[1]))
		{
			DestroySavedZooms();
			goto RtnFalse;
		}
		ExpandText(Arg[2]);
		ForceOpen = atob(Arg[2]);
		GPSTracking(0);
		_fstrcpy(CfgName, Arg[1]);
		CFGOpenTrackingStatus = SaveTrackingStatus;
		CheckForContinue(TRUE, 0);
		//	SetWindowText (hWndMain,"Switch"); 
		if (InAccel || ForceOpen)
			SendMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, -99);
		else
			PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, -99);
		goto RtnTrue;
	}

	case 703: /* $MAKEMAP(COORD,lat,long,scale,bmname)
						 (CITY,cityname,scale,bmname)
						 (BOUNDS,bounds,bmname)	*/
		if (MakeMap(Args))
			goto RtnTrue;
		else
			goto RtnFalse;

	case 704: /* $WHEREAT(lat,long) */
		if (WhereAt(Args))
			goto RtnTrue;
		else
			goto RtnFalse;

	case 705: /* $POINTER(x1,y1,x2,y2)*/
	{	double	rval;
	LPSTR	lpEnd;
	DPOINT	p1, p2;

	if (!CurView)
		goto RtnFalse;
	nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
	if (nArgs < 4)
		goto RtnFalse;
	p1.x = atof(Arg[1]);
	p1.y = atof(Arg[2]);
	p2.x = atof(Arg[3]);
	p2.y = atof(Arg[4]);
	SetCurView(SetVPFromName(Arg[6], &Err));
	SimplePointer(CurView->hDC, &p1, &p2, 5, -30, -15, atoi(Arg[5]));
	goto RtnTrue;
	}

	case 706:	/* $HLTAREA(SAVE,name) saves hlt area to file */
				/* $HLTAREA(LOAD,name) loads hlt area from file */
				// $HLTAREA(CLEAR)
				// $HLTAREA(SET,PICKED,n,hTran(opt))
				// $HLTAREA(SET,ITEM,TAGorRefno,use pickability(TorF))
	{	double	rval;
	int		l, len, usePick = -1;
	LPSTR	lpOut;
	char	fillchar;
	LPSTR	pFile;

	nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);

	if (!_fstricmp(Arg[1], "CLEAR"))
	{
		if (!CurrentConfig)
			ii = 1;
		ClearPolyOff(atob(Arg[2]));
		goto RtnTrue;
	}

	if (!_fstricmp(Arg[1], "SET"))
	{
		if (!_fstricmp(Arg[2], "PICKED"))
		{
			SelectAreaToOffsetFile(atoi(Arg[3]) - 1, atof(Arg[3]), (HANDLE)atoi(Arg[4]));
		}
		else if (!_fstricmp(Arg[2], "ITEM"))
		{
			if ((lpColon = _fstrchr(Arg[3], ':')))
			{
				*lpColon++ = 0;
				Refno = 0;
			}
			else
			{
				Refno = atol(Arg[3]);
				Arg[3] = 0;
			}
			if (atob(Arg[4]))
				usePick = -101;

			if (!PickByRefno(Refno, Arg[3], lpColon, usePick))
				goto RtnFalse;
			Dist = atobasedist(Arg[4], &Err);
			if (Dist)
			{
				if (!OffsetPickedArea(0, Dist))
					goto RtnFalse;
			}
			else
				SelectAreaToOffsetFile(0, 0, 0);
		}
		else if (!_fstricmp(Arg[2], "HLTLIST"))
		{
			long	StartRef = LONG_MIN;
			BOOL	SaveAutoClearOffset = AutoClearOffset;

			nlong = BT_NUM_IN_INDEX(hHighlight);
			AutoClearOffset = FALSE;
			while (!BT_FIND(hHighlight, (LPSTR)&StartRef, BT_FIRST, BT_GT, (LPSTR)&HighlightData))
			{
				PickList[0] = HighlightData.PD;
				SelectAreaToOffsetFile(0, atof(Arg[3]), 0);
			}
			AutoClearOffset = SaveAutoClearOffset;
		}
		goto RtnTrue;
	}

	if (!_fstricmp(Arg[1], "BOUNDS"))
	{
		int	AreaNum = 1, nPoints, nPoly;
		double	Offset;
		HANDLE	hArea;
		int	Type;
		MNMXCORD	Bounds;

		DBoundsInit(&Bounds);

		if (!hAreaOffFile)
			goto RtnFalse;
		while (hArea = GetNextHighlightArea(AreaNum++, 0, &Type, &nPoints, &nPoly, 0, &Offset, 0))
		{
			LPMNMXCORD pRect = GlobalLock(hArea);
			InflateBounds(pRect, Offset);
			AddMinMaxD(&Bounds, pRect);
			GSSiGlobUlFree(&hArea);
		}
		boundstoa(OutLoc, &Bounds);
		goto Rtnl;
	}

	if (!_fstricmp(Arg[1], "SAVE"))
	{
		if (!*Arg[2])
			goto RtnFalse;
		GSSiRemove(Arg[2]);
		if (!hAreaOffFile)
			goto RtnTrue;
		pFile = GlobalLock(hAreaOffFile);
		copyfile(Arg[2], pFile, FALSE, 0, 0, 0, 0, 0, 0);
		GlobalUnlock(hAreaOffFile);
		goto RtnTrue;
	}
	else if (!_fstricmp(Arg[1], "LOAD"))
	{
		if (!*Arg[2])
			goto RtnFalse;
		if (!hAreaOffFile)
		{
			hAreaOffFile = GSSiGlobAlloc(882, GMEM_MOVEABLE, 256);
			pFile = GlobalLock(hAreaOffFile);
			GSSiGetTempFileName(0, "gm", 0, pFile);
		}
		else
		{
			pFile = GlobalLock(hAreaOffFile);
			GSSiRemove(pFile);
		}
		copyfile(pFile, Arg[2], FALSE, 0, 0, 0, 0, 0, 0);
		GlobalUnlock(hAreaOffFile);
		HaveAreaOffFile(1);
		if (*Arg[3])
			ChangeAreaOffset(atof(Arg[3]));
		goto RtnTrue;
	}
	goto RtnFalse;

	}

	case 641: // $GETVAL(prompt,initval,type:def=C,cancelval)
	case 728: /* $GETIVAL (prompt,storvar(opt),AutoIncValue) Get integer value*/
	case 709: /* $GETFVAL (prompt,storvar(opt),initval) Get floating point value*/
	case 708: /* $GETCVAL (prompt,storvar(opt),ValueListPathname(opt),dropdownstyle(Y-default)or N for LB style,Sorted(y/n)) Get character value*/
	case 904: /* $GETMLCVAL (prompt,storvar(opt)) Get multiline character value*/
	{
		HANDLE	hMem2;
		LPSTR	lpstr, lpstr2, lpSave;
		BOOL	DropDown = TRUE, Sorted = TRUE;

		nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
		hMem2 = GSSiGlobAlloc(883, GHND, 4096);
		lpstr = GlobalLock(hMem2);
		lpstr2 = lpstr + 2048;
		if (FunID == 641)
			strcpy(lpstr2, Arg[2]);
		else if (nArgs > 1)
		{
			sprintf(lpstr2, "[%s]", Arg[2]);
			ExpandText(lpstr2);
		}
		else
			lpstr2 = 0;
		if (nArgs > 3)
			DropDown = atob(Arg[4]);
		if (nArgs > 4)
			Sorted = atob(Arg[5]);
		if (FunID == 709)
		{
			long	AutoInc = atol(Arg[3]);
			rtn = GetTextString(GetParentFocus(), lpstr, 2048, Arg[1], 0, Arg[3], 0, 0, 0);
		}
		else if (FunID == 728)
		{
			long	AutoInc = atol(Arg[3]);
			rtn = GetTextString(GetParentFocus(), lpstr, 2048, Arg[1], 0, lpstr2, AutoInc, DropDown, Sorted);

		}
		else if (FunID == 904)
			rtn = GetTextStringML(GetParentFocus(), lpstr, 2048, Arg[1], lpstr2);
		else if (FunID == 641)
		{
			rtn = GetTextString(GetParentFocus(), lpstr, 2048, Arg[1], "", lpstr2, 0, 1, 0);
			if (rtn)
				strcpy(OutLoc, lpstr);
			else if (*Arg[4])
				strcpy(OutLoc, Arg[4]);
			else
				strcpy(OutLoc, Arg[2]);
			GSSiGlobUlFree(&hMem2);
			goto Rtnl;
		}
		else
			rtn = GetTextString(GetParentFocus(), lpstr, 2048, Arg[1], Arg[3], lpstr2, 0, DropDown, Sorted);
		if (rtn)
		{
			l = _fstrlen(lpstr);
			_fstrncpy(OutLoc, lpstr, l + 1);
			if (nArgs > 1 && *Arg[2])
			{
				/*	if (lpstr2)
						SetGlobalValue(Arg[2],lpstr2);
					else*/
				SetGlobalValue(Arg[2], lpstr);
			}

		}
		else
		{
			if (lpstr2)
				_fstrcpy(OutLoc, lpstr2);
			else
				*OutLoc = 0;
			SetContinueProcessing(FALSE);
			PostMessage(hWndMain, GF_CLEAR_FUN_STACK, 0, 0L);
		}
		GSSiGlobUlFree(&hMem2);
		goto Rtnl;
	}

	case 710: /* $LINESYM(cursymorpar (default all)) set line symbol variables */
	{
		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		if (SelectSymbol(CurView->hWnd, 2, Arg[1], 1))
			goto RtnTrue;
		else
			goto RtnFalse;
	}

	case 505: // $TRNTO(Transformation file,Point)  transform point from 'from' coords to 'to' coords
	case 720: // $TRNFROM(Transformation file,Point)  transform point from 'to' coords to 'from' coords
		CvtDir = 1;
		if (FunID == 720)
			CvtDir = 2;
		{
			double	X, Y, OutX, OutY;
			short	n, Type;

			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			Type = atoi(Arg[3]);
			if (Type < 1 || Type > 3)
				Type = 1;

			nPoints = GetPointsFromList(Arg[2], &hPoints);
			*OutLoc = 0;
			if (!(hTran = LoadTranFile(Arg[1], CvtDir, Type, 0, 0)))
			{
				GSSiGlobFree(&hPoints);
				_fstrcpy(OutLoc, "Invalid Transformation File");
				goto Rtnl;
			}
			if (!hPoints)
				goto Rtnl;
			pointstoa(OutLoc, nPoints, hPoints, hTran);
			GSSiGlobFree(&hPoints);
			CloseTRANS2(&hTran);
			goto Rtnl;
		}
	case 506: // $CVTTO(convert file,Point) convert point between projections 
	case 721: // $CVTFROM(convert file,Point) convert point between projections  
		CvtDir = 1;
		if (FunID == 506)
			CvtDir = 2;
		{
			short	n;
			HANDLE	hPoints;
			HPDPOINT	pPoint;
			short	nPoints;

			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			if (!stricmp(Arg[1], "DMS"))
			{
				strcpy(OutLoc, "Invalid Projection File");
				if (CvtDir == 1)
				{
					DPOINT point;
					if (PointFromDMS(Arg[2], &point))
						dpointtoa(OutLoc, &point);
				}
				goto Rtnl;
			}

			nPoints = GetPointsFromList(Arg[2], &hPoints);
			*OutLoc = 0;
			if (!hPoints)
				goto Rtnl;
			pPoint = (HPDPOINT)GlobalLock(hPoints);
			for (n = 0; n < nPoints; n++)
			{
				if (n)
					_fstrcat(OutLoc, " ");
				if (ConvertPoint(Arg[1], &pPoint[n], CvtDir))
				{
					_fstrcpy(OutLoc, "Invalid Projection File");
					GSSiGlobUlFree(&hPoints);
					goto Rtnl;
				}
				dpointtoa(_fstrrchr(OutLoc, 0), &pPoint[n]);
			}
			GSSiGlobUlFree(&hPoints);
			goto Rtnl;
		}

	case 722: /* $AREASYM() set area symbol variables */
	{
		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		if (SelectSymbol(CurView->hWnd, 3, Arg[1], 1))
			goto RtnTrue;
		else
			goto RtnFalse;
	}

	case 723: // $GETPATH(R W D or C (creates with no overwrite prompt),Extension,title(opt),storvar(opt),startdir,return status instead of path) Get pathname
			  // $GETPATH(A,path) gets full actual path - if cached displays cache name
	{
		HANDLE	hTemp;
		LPSTR	str, lpSave;
		BOOL	rtn, ResetSubDL;
		BOOL	returnStatus = FALSE;

		nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 2)
			goto RtnFalse;
		returnStatus = atob(Arg[6]);
		_fstrupr(Arg[1]);
		if (*Arg[1] == 'A')
		{
			OFSTRUCTGM	OFStruct;
			HFILE		Fid = GSSiOpenFile(Arg[2], &OFStruct, OF_READ);

			*OutLoc = 0;
			if (Fid != HFILE_ERROR)
				strcpy(OutLoc, OFStruct.szPathName);
			GSSiClose2(&Fid);
			goto Rtnl;
		}
		hTemp = GSSiGlobAlloc(888, GHND, 256);
		str = GlobalLock(hTemp);
		_fstrcpy(str, Arg[5]);
		if (*str && !*LastChr(str) != '\\')
			strcat(str, "\\");

		ResetSubDL = GetGlobalBVal("[%SUBDL]");
		SetGlobalValue("%SUBDL", "N");
		switch (*Arg[1])
		{
		case 'R':
			rtn = GetFileName4(GetFocus(), str, 0, Arg[2], Arg[3], Arg[4]);
			goto TestGPRtn;
		case 'W':
			OverWritePrompt = FALSE;
		case 'C':
			rtn = GetSaveName3(GetFocus(), str, 0, Arg[2], Arg[3], Arg[4]);
			break;
		case 'D':
		{
			LPSTR VarName = Arg[4];
			if (VarName)
			{
				if (*VarName)
				{
					sprintf(str, "[%s]", VarName);
					ExpandText(str);
					if (!*str)
						strcpy(str, Arg[5]);
					strcpy(Arg[6], str);
				}
				else
					VarName = 0;
			}

			rtn = GetFolderName(GetFocus(), Arg[6], str, Arg[3]);

			//rtn = GetSaveName3 (GetFocus(),str,0,Arg[2],Arg[3],Arg[4]);
			if (rtn)
			{
				if (FileType(str) == 1)
					GSSiRemove(str);
				else if (VarName)
				{
					SetGlobalValue(VarName, str);
				}
			}
		}
		break;
		default:
			rtn = FALSE;
		}
	TestGPRtn:	if (ResetSubDL)
		SetGlobalValue("%SUBDL", "Y");
	if (returnStatus)
	{
		if (rtn)
			strcpy(OutLoc, "1");
		else
			strcpy(OutLoc, "0");
		GSSiGlobUlFree(&hTemp);
		goto Rtnl;
	}
	else
	{
		if (rtn)
			_fstrcpy(OutLoc, str);
		else
		{
			SetContinueProcessing(FALSE);
			PostMessage(hWndMain, GF_CLEAR_FUN_STACK, 0, 0L);
			*OutLoc = 0;
		}
	}
	GSSiGlobUlFree(&hTemp);
	goto Rtnl;
	}

	case 724: // $NULLMAP(pathname,CoordOpt,R to replace existing file)
	{

		nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 2)
			goto RtnFalse;
		switch (*Arg[2])
		{
		default:
			Bounds = atobounds(Arg[2], &Err);
			if (Err)
				goto RtnFalse;
			break;
		case 'v':
		case 'V':
		{
			LPSTR pEndPar, pPar = strchr(Arg[2], '(');

			if (pPar)
			{
				pPar++;
				pEndPar = strrchr(pPar, ')');
				if (pEndPar)
					*pEndPar = 0;
				SetCurView(SetVPFromName(pPar, &Err));
			}
			Bounds = CurView->WBounds;
			CurView = SaveVP;
		}
		break;
		case 'R':
		case 'r':
			Bounds = ZoomBoxRect;
			break;
		}
		ExpandText(Arg[3]);
		if (ExistFile(Arg[1]) && _fstricmp("R", Arg[3]))
			goto RtnFalse;
		if (CreateNewMap(Arg[1], &Bounds, 0, 0, 0, 0, 0, 0, FALSE))
			goto RtnTrue;
		else
			goto RtnFalse;
	}

	case 725: // $MAKEDIR(pathname) make directory
	{
		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		if (!makedirectories(Arg[1], TRUE, FALSE))
			goto RtnFalse;
		goto RtnTrue;
	}

	case 726: // $ENLARGE(width,factor) enlarge portion of screen
	{
		short	factor, width;

		nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
		width = atoi(Arg[1]);
		factor = atoi(Arg[2]);
		EnlargeScreen(factor, width);
		goto RtnTrue;
	}

	case 727:	// $INFOBOX(LOAD,pathname)load infobox  
				// $INFOBOX(CREATE,pathname,Point,optional infobox center point,optional move factor)load infobox or
				// $INFOBOX(CREATE,pathname,ITEM,pickeditemnum,optional infobox center point)
				// $INFOBOX(CREATE,pathname,ZOOM,optional infobox center point)
	{
		short	factor, width, item = -1;

		nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 2)
			goto RtnFalse;
		if (!_fstricmp(Arg[1], "LOAD"))
		{
			if (LoadInfoBox(Arg[2], &TAGBox))
				goto RtnTrue;
		}
		if (!_fstricmp(Arg[1], "DELETE"))
		{
			if (DeleteTAGByName(Arg[2]))
				goto RtnTrue;
		}
		if (!_fstricmp(Arg[1], "CREATE"))
		{
			if (!_fstricmp(Arg[3], "ITEM"))
			{
				item = atoi(Arg[4]) - 1;
				Point = PickList[item].PickedPoint;
				SetConfig(PickList[item].ConfigID);
				SetViewport(PickList[item].ViewID);
			}
			else if (!_fstricmp(Arg[3], "POINT"))
			{
				SetCurView(SetVPFromName(Arg[5], &Err));
				Point = atopt(Arg[4], &Err);
			}
			else if (!_fstricmp(Arg[3], "ZOOM"))
				item = -2;
			else if (nArgs > 2)
				Point = atopt(Arg[3], &Err);
			else
				Point = MinMaxMidPointD(&CurView->WBounds);
			if (CreateTAGBoxFromFile(CurView->hDC, Point, Arg[2], item, atof(Arg[6])))
				goto RtnTrue;
		}
		goto RtnFalse;
	}
	case 729: // $OPENAPP(pathname) executes app associated with file
	{
		HINSTANCE	hI;

		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		if (!ExistFile(Arg[1]))
		{
			GSSiMessageBox(0, Arg[1], "Unable to Access Application File", MB_ICONEXCLAMATION, 0);
			goto RtnFalse;
		}
		else
		{
			hI = ShellExecute(hWndMain, 0, Arg[1], 0, 0, SW_SHOWMAXIMIZED);
			if (DisplayShellExError((UINT)hI, Arg[1]))
				goto RtnFalse;
			else
				goto RtnTrue;
		}
	}
	case 730: // $SYMNAME (symnum,from(opt)) from=4 forces to get from file not dict
	{
		nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
		SymNum = atoi(Arg[1]);
		if (*Arg[2])
			GetSymbolName(SymNum, OutLoc, 0, atoi(Arg[2]), 0);
		else
			GetDictSymName(SymNum, OutLoc);
		goto Rtnl;
	}

	case 731: // $SYMDESC (symnum)
	{
		short SymNum;
		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		SymNum = atoi(Arg[1]);
		GetDictSymDescription(SymNum, OutLoc);
		goto Rtnl;
	}

	case 732: // $AUTOINC(prefix,udi) returns autoinc udi or same udi if no autoinc
	{
		short	factor, width;

		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		_fstrcpy(OutLoc, Arg[2]);
		LoadTAGDef();
		if (NumTAGDef)
		{
			LPTAGDEF    lpTAGDef;
			short	i, len;
			LPSTR	pBeg, pEnd;
			double	CurVal;
			char	SaveEnd;

			lpTAGDef = (LPTAGDEF)GlobalLock(hTAGDef);
			for (i = 0; i < NumTAGDef; i++, lpTAGDef++)
			{
				if (!_fstricmp(Arg[1], lpTAGDef->Prefix))
				{
					if (lpTAGDef->IncLen)
					{
						l = _fstrlen(Arg[2]);
						if (lpTAGDef->IncBeg)
						{
							pBeg = Arg[2] + (lpTAGDef->IncBeg - 1);
							pEnd = pBeg + lpTAGDef->IncLen;
							len = lpTAGDef->IncLen;
						}
						else
						{
							len = min(l, lpTAGDef->IncLen);
							pEnd = _fstrchr(Arg[2], 0);
							pBeg = Arg[2] + (l - len);
						}
						SaveEnd = *pEnd;
						*pEnd = 0;
						CurVal = atof(pBeg);
						if (lpTAGDef->IncNDP)
							;
						else
							IWRITEZ(IDNINT(CurVal + 1), pBeg, len);
						*pEnd = SaveEnd;
						_fstrcpy(OutLoc, Arg[2]);
					}
					break;
				}
			}
			GlobalUnlock(hTAGDef);
		}
		goto Rtnl;
	}

	case 733: // $GLOBALS (?)
	{
		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		DisplayFieldList(GetFocus(), 0, 0, 3, 0);
		goto Rtnl;
	}

	case 734:// $FILEFIT (filelist,exclusionlist,fittype,projection,OutMap,symbol)
	{
		short	FitType;

		nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 5)
			goto RtnFalse;
		FitType = atoi(Arg[3]);
		rtn = LinfitFilePoints(Arg[1], Arg[2], FitType, Arg[4], Arg[5], Arg[6]);
		if (rtn)
			goto RtnTrue;
		goto RtnFalse;
	}

	case 735:// $EPMACRO (prepickmacro,bpmacro,epmacro,postpickmacro)
	{
		short	FitType;

		nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
		rtn = EndPointMacros(Arg[1], Arg[2], Arg[3], Arg[4]);
		if (rtn)
			goto RtnTrue;
		goto RtnFalse;
	}

	case 736: // $MESSAGE(message,button opt,pos,cancelNum) Title is blank unless arg1 contains | to sep message from title (i.e $MESSAGE(message|title,opt)
	{
		char space[2] = " ";
		LPSTR VB;
		int cancelNum = 0;

		nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 1)
			goto RtnFalse;
		cancelNum = atoi(Arg[4]);
		rtn = IDOK;
		if ((VB = strrchr(Arg[1], '|')))
			*VB++ = 0;
		else
			VB = space;
		if (InServerMode)
		{
			LogServerActivity(Arg[1]);
		}
		else
			switch (atoi(Arg[2]))
			{
			case 1:
				rtn = GSSiMessageBox(cancelNum, Arg[1], VB, MB_YESNO, Arg[3]);
				break;
			case 2:
				rtn = GSSiMessageBox(cancelNum, Arg[1], VB, MB_ABORTRETRYIGNORE, Arg[3]);
				break;
			default:
				rtn = GSSiMessageBox(cancelNum, Arg[1], VB, MB_OK, Arg[3]);
			}
		SetCurView(SaveVP);
		switch (rtn)
		{
		case IDOK:
			goto RtnTrue;
		case IDYES:
			goto RtnTrue;
		case IDRETRY:
			CloseBufferedMacros();
			CloseAllRequestedFiles(FALSE);
			CacheAlreadyChecked(0, 0, 0);
			goto RtnTrue;
		case IDNO:
			goto RtnFalse;
		case IDABORT:
			SetContinueProcessing(FALSE);
		}
		goto RtnTrue;
	}

	case 737: /* $SAVEPIK(pickability_file,description,optional VP name) Save pickability file */
		PickVis = TRUE;
		goto SaveVis;
	case 738: /* $SAVEVIS(pickability_file,description,optional VP name) Save pickability file */
		PickVis = FALSE;
	SaveVis:
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			SaveVP = CurView;
			SetCurView(SetVPFromName(Arg[3], &Err));
			rtn = SaveVisFile(Arg[1], PickVis, Arg[2]);
			SetCurView(SaveVP);
			if (rtn)
				goto RtnTrue;
			else
				goto RtnFalse;
		}

	case 739: // $ATPRINT(commands) infobox actions at print time
	{

		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		*OutLoc = 0;
		if (Printing && InDrawTAG)
		{
			InAtPrint = TRUE;
			ExpandText(Arg[1]);
			InAtPrint = FALSE;
		}
		goto Rtnl;
	}

	case 740: //$TCPOPEN(ipaddress,port,VARNAMEforsocket,terminator,inputprocesscommand,closeprocesscommand,restartcommand)
	{
		USHORT	port;
		SOCKET	sock;

		nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 6)
			goto RtnFalse;
		port = atol(Arg[2]);
		SetGlobalValueLong(Arg[3], 0);
		if (OpenTCPIPSocket(hWndMain, Arg[1], port, &sock, Arg[4], Arg[5], Arg[6], Arg[7], TRUE) < 0)
			goto RtnFalse;
		SetGlobalValueLong(Arg[3], sock);
		goto RtnTrue;
	}

	case 741: //$TCPSEND(socket,string,sendnullterm,waitseconds)
	{
		SOCKET	socket;

		nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 2)
			goto RtnFalse;
		socket = atol(Arg[1]);
		rtn = SendTCPIPString(socket, Arg[2], atob(Arg[3]), (UINT)atol(Arg[4]));
		ProcessTCPData(socket);
		ltoa((long)rtn, OutLoc, 10);
		goto Rtnl;
	}

	case 742: // $GMDFIND(file,setkey,setval(opt),partialmatchnum(opt),index(opt)) 
			  // ex: $GMDFIND(file.gmd,KEY1=A;KEY2=B)  
			  //	 $GMDFIND(fileid,KEY1=A;KEY2=B,[val]=[fileval];[val2]=[fileval2],3)
	{
		short	index;

		nArgs = GetFunArgs(Args, Arg, -5, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 2)
			goto RtnFalse;
		ExpandText(Arg[1]);
		ExpandText(Arg[4]);
		n = atoi(Arg[4]);
		index = atoi(Arg[5]);
		if (FindGMDRecord(Arg[1], Arg[2], Arg[3], n, index))
			goto RtnTrue;
		else
			goto RtnFalse;

	}
	break;

	case 743: // $UMREFNO(intref) 
	{

		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		Refno = atol(Arg[1]);
		sprintf(OutLoc, "%.2f", (((double)Refno) - ZERO$) / 100);
		goto Rtnl;
	}
	break;

	case 744: // $CONVERT(AREA,from units(system),to units(sqrfeet),value) convert units
	{
		double	InVal, OutVal;

		nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 4)
			goto RtnFalse;
		OutVal = -999999999;
		if (!_fstricmp(Arg[2], "SYSTEM"))
		{
			if (!_fstricmp(Arg[1], "LENGTH"))
			{
				InVal = atof(Arg[4]);
				for (n = 0; n < 5; n++)
					if (!_fstricmp(DistUnitOpts[n], Arg[3]))
						OutVal = ConvertDist(InVal, n + 1);
			}
			else if (!_fstricmp(Arg[1], "AREA"))
			{
				InVal = atof(Arg[4]);
				for (n = 0; n < 6; n++)
					if (!_fstricmp(AreaUnitOpts[n], Arg[3]))
						OutVal = ConvertArea(InVal, n + 1);
			}
			sprintf(OutLoc, "%f", OutVal);
		}
		else
		{
			GetDistAndUnits(Arg[2], &OutVal, &n1, TRUE);
			GetDistAndUnits(Arg[3], &OutVal, &n2, TRUE);
			n1++;
			n2++;
			if (!_fstricmp(Arg[1], "POINT"))
			{
				Point = atopt(Arg[4], &Err);
				Point.x = ConvertDist2(Point.x, n1, n2);
				Point.y = ConvertDist2(Point.y, n1, n2);
				sprintf(OutLoc, "%lf %lf", Point.x, Point.y);
			}
			else
			{
				InVal = atof(Arg[4]);
				OutVal = ConvertDist2(InVal, n1, n2);
				sprintf(OutLoc, "%lf", OutVal);
			}

		}
		goto Rtnl;
	}

	case 745: // $ADJTIME(seconds,ADJUSTKEY) adjust time to first sec of previous month, day or year
			  // ex: $ADJTIME([%SYS_CLOCK],MONDAY) adjusts to first second of prev (or current) Monday
	{

		struct	tm	thistime;
		short	i;

		nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
		systime = max(0, atol(Arg[1]));
		thistime = *localtime(&systime);
		thistime.tm_hour = 0;
		thistime.tm_min = 0;
		thistime.tm_sec = 0;
		thistime.tm_isdst = -1;
		systime = mktime(&thistime);
		if (!_fstricmp(Arg[2], "MONTHBEGIN"))
		{
			thistime.tm_mday = 1;
			systime = mktime(&thistime);
		}
		if (!_fstricmp(Arg[2], "MONTHEND"))
		{
			thistime.tm_mday = 15;
			systime = mktime(&thistime);
			systime += 3600L * 24L * 30L;
			thistime = *localtime(&systime);
			thistime.tm_hour = 0;
			thistime.tm_min = 0;
			thistime.tm_sec = 0;
			thistime.tm_mday = 1;
			thistime.tm_isdst = -1;
			systime = mktime(&thistime);
			systime--;
		}
		else
			//check for day of week
			for (i = 0; i < 7; i++)
			{
				if (!_fstricmp(Arg[2], DOW[i]))
				{
					if (thistime.tm_wday >= i)
						systime -= (long)(thistime.tm_wday - i) * 86400L;
					else
						systime -= (long)(7 - i + thistime.tm_wday) * 86400L;
					break;
				}
			}
		ltoa((long)systime, OutLoc, 10);
		goto Rtnl;
	}
	break;

	case 746: // $COPYMAP(toname,fromname,conversion)
	{
		LPSTR	pEnd2, pFile2;

		nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
		pFile = Arg[4] + 2048;
		pFile2 = pFile + 2048;

		if (!makedirectories(Arg[1], FALSE, FALSE))
			goto RtnFalse;
		if (!copyfile(Arg[1], Arg[2], FALSE, 0, 0, 0, 0, 0, 0))
			goto RtnFalse;
		_fstrcpy(pFile, Arg[2]);
		_fstrcpy(pFile2, Arg[1]);
		{
			_fstrupr(pFile);
			_fstrupr(pFile2);
			if ((pEnd = _fstrstr(pFile, ".PLT")) && (pEnd2 = _fstrstr(pFile2, ".PLT")))
			{
				_fstrcpy(pEnd, ".RIN");
				_fstrcpy(pEnd2, ".RIN");
				if (ExistFile(pFile))
					copyfile(pFile2, pFile, FALSE, 0, 0, 0, 0, 0, 0);
				else
					GSSiRemove(pFile2);
				_fstrcpy(pEnd, ".TIN");
				_fstrcpy(pEnd2, ".TIN");
				if (ExistFile(pFile))
					copyfile(pFile2, pFile, FALSE, 0, 0, 0, 0, 0, 0);
				else
					GSSiRemove(pFile2);
			}
		}
		_fstrcpy(MapCopyProjection, Arg[3]);
		ConvertFileCoordinates(Arg[1], 0, 0, 0);
		*MapCopyProjection = 0;
		goto RtnTrue;
	}
	break;

	case 747: // $SYMCOPY(symdic name,symname,parent(opt),newname(opt),fromsymnum(opt),tosymnum(opt)) copies symbol from another symbol dict
	{

		nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 2)
			goto RtnFalse;
		rtn = CopySymbolFromDict(Arg[1], Arg[2], Arg[3], Arg[4], atoi(Arg[5]), atoi(Arg[6]));
		if (rtn)
			goto RtnTrue;
		goto RtnFalse;
	}

	case 748: // $EXECUTE(command,MIN or MAX(default))
	{
		UINT hI;
		UINT	ShowVal = SW_SHOW;

		nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
		if (!_fstrnicmp(Arg[2], "MIN", 3))
			ShowVal = SW_SHOWMINIMIZED;
		else if (!_fstrnicmp(Arg[2], "MAX", 3))
			ShowVal = SW_SHOWMAXIMIZED;
		else if (!_fstrnicmp(Arg[2], "FIND", 3))
		{
			UINT	hI = (UINT)FindExecutable(Arg[1], CurDir, OutLoc);

			DisplayShellExError(hI, Arg[1]);
			goto Rtnl;
		}
		//MessageBox (0,Arg[1],0,MB_OK);	
		_getcwd(CurDir, MAX_PATH);
		hI = WinExec(Arg[1], ShowVal);
		if (hI < 32)
		{
			DisplayShellExError(hI, Arg[1]);
			goto RtnFalse;
		}
		goto RtnTrue;
	}

	case 749: //$CDUNITS(2,DMS,1)
	{

		nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 2)
			goto RtnFalse;
		*OutLoc = 0;
		for (n = 0; n < *pNumViewports; n++)
		{
			if (pViewports[n]->pTheme)
			{
				if (pViewports[n]->pTheme->ID == PF_COORD_DISPLAY)
				{
					if (!_fstricmp(Arg[2], "DM"))
						((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Units[1] = 3;
					else if (!_fstricmp(Arg[2], "DMS"))
					{
						((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Units[1] = 4;
						((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Precision[1] = atoi(Arg[3]) + 1;
					}
					else if (!_fstricmp(Arg[2], "DD"))
					{
						((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Units[1] = 5;
						((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Precision[1] = atoi(Arg[3]) + 1;
					}
					else if (!_fstricmp(Arg[2], "MGRS"))
					{
						((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Units[1] = 8;
						((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Precision[1] = atoi(Arg[3]) + 1;
					}
					if (*Arg[4])
						_fstrcpy(((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->LineID[1], Arg[4]);
					break;
				}
			}
		}
		goto Rtnl;
	}

	case 750: // $LOADTIN(infile,outfile)
	{
		nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);

		if (LoadTIN(Arg[1], Arg[2]))
			goto RtnTrue;
		goto RtnFalse;
	}

	case 751: // $SYMTYPE(symname) 
	{
		short	itype;
		char	CTypes[4][2] = { "p","P","L","A" };

		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		if (*Arg[1] == '#')
			n = atol(Arg[1] + 1);
		else
			n = GetDictSymbolNumber(Arg[1]);
		itype = GetDictSymbolType(n);
		if (itype > 3 || itype < 0)
			*OutLoc = 0;
		else
			_fstrcpy(OutLoc, CTypes[itype]);
		goto Rtnl;
	}
	break;

	case 752: // $REPLACE(str,oldstr,newstr) 
	{
		nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
		_fstrcpy(OutLoc, Arg[1]);
		REPLAC(OutLoc, Arg[2], Arg[3], 4096);
		goto Rtnl;
	}

	case 753: // $SYMPLOT(Parent,outfile,format) 
	{
		nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
		if (CreateSymbolPlot(Arg[1], Arg[2], Arg[3]))
			goto RtnTrue;
		goto RtnFalse;
	}

	case 754: // $LOADDTM(GRID,infile,outfile)
	{
		nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 2)
			goto RtnFalse;
		if (!_fstricmp(Arg[1], "GRID"))
		{
			if (LoadGRIDDTM(Arg[2], Arg[3]))
				goto RtnTrue;
		}
		else if (!_fstricmp(Arg[1], "AREA"))
		{
			if (LoadAREADTM(Arg[2], Arg[3]))
				goto RtnTrue;
		}
		else if (!_fstricmp(Arg[1], "LIDAR"))
		{
			if (LoadLIDARDTM(Arg[2], Arg[3], Arg[4]))
				goto RtnTrue;
		}
		else if (!_fstricmp(Arg[1], "LIDARLAZ"))//$LOADDTM(LIDARLAZ,InDir,OutFile,pointType)
		{
			if (LoadLIDARDTMfromLAZ(Arg[2], Arg[3], atoi(Arg[4])))
				goto RtnTrue;
		}
		else if (!_fstricmp(Arg[1], "ERDAS"))
		{
			if (LoadERDASDem())
				goto RtnTrue;
		}
		goto RtnFalse;
	}
	case 755: // $SYMDICT(COMPRESS) 
	{
		HANDLE hSymbol;
		int isym;

		nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
		rtn = FALSE;
		if (nArgs < 1)
			goto RtnFalse;
		if (!stricmp(Arg[1], "COMPRESS"))
			CompressSymDict();
		else if (!stricmp(Arg[1], "DELETE"))
		{
			if (!stricmp(Arg[2], "SYMBOL"))//$SYMDICT(DELETE,SYMBOL,symnum,close(boolean))
			{
				n = atoi(Arg[3]);
				if (DeleteSymbol(n, atob(Arg[2])))
					goto RtnTrue;
				goto RtnFalse;
			}
		}
		else if (!stricmp(Arg[1], "COPY"))
		{
			if (!stricmp(Arg[2], "SYMBOL"))
				rtn = CopySymbolFromDict(Arg[3], Arg[4], Arg[5], Arg[6], atoi(Arg[7]), atoi(Arg[8]));
			if (!stricmp(Arg[2], "PARENT"))
				rtn = CopyParentFromDict(Arg[3], Arg[4], Arg[5], Arg[6]);
		}
		else if (!stricmp(Arg[1], "GAPS"))
		{
			HFILE fidOut = GSSiOpenFile(Arg[2], 0, OF_CREATE);
			LPSTR str = Arg[8];
			int fromSym = atoi(Arg[3]);
			int toSym = atoi(Arg[4]);

			n = 0;
			if (fidOut != HFILE_ERROR)
			{
				sprintf(str, "GAPSYMNUM");
				fputstring(str, fidOut);
				for (isym = fromSym; isym < toSym; isym++)
				{
					if ((hSymbol = GetDictSymDesc(isym, 0)))
					{
						DestroySymbol(hSymbol);
					}
					else
					{
						itoa(isym, str, 10);
						fputstring(str, fidOut);
					}
				}
				GSSiClose2(&fidOut);
			}
			itoa(n, OutLoc, 10);
			goto Rtnl;
		}
		else if (!stricmp(Arg[1], "LIST"))//$SYMDICT(LIST,outfile,PARENT,parentname)
										  //$SYMDICT(LIST,outfile,GAPS,fromsymnum,tosymnum
		{
			HFILE fidOut = GSSiOpenFile(Arg[2], 0, OF_CREATE);
			LPSTR str = Arg[8];

			n = 0;
			if (fidOut != HFILE_ERROR)
			{
				sprintf(str, "SYMNUM\tSYMNAME\tSYMTYPE");
				fputstring(str, fidOut);
				if (!stricmp(Arg[3], "PARENT"))
				{
					int iparsym = GetSymbolNum(Arg[4]);
					if (iparsym > 0)
					{
						for (isym = 1; isym <= NumSymbols; isym++)
						{
							if ((hSymbol = GetDictSymDesc(isym, 0)))
							{
								LPSYMBOL pSymbol = (LPSYMBOL)GlobalLock(hSymbol);
								if (pSymbol->Parent == iparsym)
								{
									sprintf(str, "%i\t%s\t%i", pSymbol->Number, pSymbol->Name, pSymbol->Type);
									fputstring(str, fidOut);
								}
								GlobalUnlock(hSymbol);
								DestroySymbol(hSymbol);
							}
						}

					}
				}
				GSSiClose2(&fidOut);
			}
			itoa(n, OutLoc, 10);
			goto Rtnl;
		}
		goto Rtnrtn;
	}
	break;

	case 756: /* $ARCDIST(point1,point2) */
	{	double dist;

	nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
	Point = atopt(Arg[1], &Err);
	Point2 = atopt(Arg[2], &Err);
	dist = ArcDistance(Point, Point2);
	sprintf(OutLoc, "%f", dist);
	goto Rtnl;
	}

	case 757: //$LOADBMP(BMPFile,Infile)
	{
		long	ICmd;

		nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
		if (BMPFileFromEXT(Arg[2], Arg[1], 1))
			goto RtnTrue;
		goto RtnFalse;
	}

	case 758: //$ADDAREA(file.plt,TAG,SYMBOLNAME,points)  
	{
		nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
		if (AddAreaToMap(Arg[1], Arg[2], Arg[3], Arg[4], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0))
			goto RtnTrue;
		goto RtnFalse;
	}

	case 759: //$DOWNAME(downum)  
	{
		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		i = atoi(Arg[1]);
		if (i > 0 && i < 8)
			_fstrcpy(OutLoc, DOW[i - 1]);
		else
			*OutLoc = 0;
		goto Rtnl;
	}

	case 760: //$ADDLINE(file.plt,TAG,SYMBOLNAME,points)  
	{
		nArgs = GetFunArgs(Args, Arg, -4, &hMem, pBrkPt, bpOffset, bpLen);
		hMem2 = GSSiGlobAlloc(913, GMEM_MOVEABLE, USHRT_MAX);
		ExpandText(Arg[1]);
		ExpandText(Arg[2]);
		ExpandText(Arg[3]);
		Arg1 = GlobalLock(hMem2);
		_fstrcpy(Arg1, Arg[4]);
		ExpandText(Arg1);
		rtn = AddAreaToMap(Arg[1], Arg[2], Arg[3], Arg1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		GSSiGlobUlFree(&hMem2);
		if (rtn)
			goto RtnTrue;
		goto RtnFalse;
	}

	case 761: //$NUMROWS(fileid)  
	{
		nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 1)
			goto RtnFalse;

		nlong = 0;
		if (!_fstricmp(Arg[1], "HLTLIST"))
			nlong = BT_NUM_IN_INDEX(hHighlight);
		else if (FilePathHandle)
		{
			FilePathPtr = (LPFILEPATH)GlobalLock(FilePathHandle);
			lpFileHandle = &FilePathPtr->FileHandle;

			for (i = 0; i < FilePathPtr->NumFiles; i++, lpFileHandle++)
			{
				if (*lpFileHandle)
				{
					SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
					if (!_fstricmp(SQLPtr->IDName, Arg[1]))
						nlong = NumSQLRows(*lpFileHandle);
					GlobalUnlock(*lpFileHandle);
				}
			}
			GlobalUnlock(FilePathHandle);
		}
		ltoa(nlong, OutLoc, 10);
		goto Rtnl;
	}

	case 762: //$PICKCPT(point,filelist,ignorename,maxdist)  
		nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 4)
			goto RtnFalse;
		Point = atopt(Arg[1], &Err);
		RVal = atof(Arg[4]);
		PickNearestCPTFile(Point, Arg[2], Arg[3], RVal, OutLoc);
		goto Rtnl;

	case 763: //$DISPLAY(ITEM,refno or TAG,Viewport Name)  
		nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 1)
			goto RtnFalse;
		if ((lpColon = _fstrchr(Arg[2], ':')))
			*lpColon++ = 0;
		else
		{
			Refno = atol(Arg[2]);
			Arg[2] = 0;
		}
		SetCurView(SetVPFromName(Arg[3], &Err));
		if (PickByRefno(Refno, Arg[2], lpColon, -100))
		{
			ProcessPickedItem(NumPicked - 1, TRUE);
			goto RtnTrue;
		}
		goto RtnFalse;

	case 764: //$GMDCOPY(gmdfile,fromkey,tokey)  
		nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 3)
			goto RtnFalse;
		if (GMDCopyRecord(Arg[1], Arg[2], Arg[3]))
			goto RtnTrue;
		else
			goto RtnFalse;

	case 765: //$VEHICLE(DEFINE,id,description,symname,color,size)  
		nArgs = GetFunArgs(Args, Arg, 9, &hMem, pBrkPt, bpOffset, bpLen);

		if (!_fstricmp(Arg[1], "DISPLAYIN"))
		{
			int	iview;

			SetCurView(SetVPFromName(Arg[2], &Err));
			for (iview = 0; iview < nAVLVP; iview++)
			{
				if (CurView == AVLViewports[iview])
					goto HaveVP;
			}
			AVLViewports[nAVLVP++] = CurView;
			DisplayAllVehicles(FALSE, TRUE);
		HaveVP:;
			goto RtnTrue;
		}

		if (!_fstricmp(Arg[1], "BOUNDS"))
		{
			Bounds = GetVehicleBounds(Arg[2]);
			boundstoa(OutLoc, &Bounds);
			goto Rtnl;
		}

		if (!_fstricmp(Arg[1], "UPDATE"))
		{
			if (!_fstrnicmp(Arg[3], "Invalid", 7))
				goto RtnFalse;
			Point = atopt(Arg[3], &Err);
			rtn = SetVehicleLoc(Arg[2], Point, (short)atof(Arg[4]), (short)atof(Arg[5]), atoi(Arg[6]), atob(Arg[7]), TRUE, atoi(Arg[8]), atob(Arg[9]));
			goto Rtnrtn;
		}

		if (!_fstricmp(Arg[1], "SETSTATUS"))
		{
			rtn = SetVehicleStatus(Arg[2], atoi(Arg[3]), FALSE, atob(Arg[4]));
			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "SETFENCESTATUS"))
		{
			rtn = SetVehicleStatus(Arg[2], atoi(Arg[3]), TRUE, atob(Arg[4]));
			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "UPDATEINFO"))
		{
			rtn = DisplayVehicleInfo(Arg[2], FALSE);
			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "SETCOLOR"))
		{
			rtn = SetVehicleColor(Arg[2], atoi(Arg[3]));
			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "SETSPEED"))
		{
			rtn = SetVehicleStatus(Arg[2], atoi(Arg[3]), FALSE, atob(Arg[4]));
			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "FLASH"))
		{
			rtn = FlashVehicle(Arg[2]);
			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "DISPLAY"))
		{
			UpdateAllVehicles(TRUE);
			goto RtnTrue;
		}
		if (!_fstricmp(Arg[1], "TRACK"))
		{
			rtn = SetVehicleTrackColor(Arg[2], atoi(Arg[3]));
			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "REMOVE"))
		{
			rtn = RemoveVehicle(Arg[2]);
			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "DEFINE"))
		{
			if (nArgs < 6)
				goto RtnFalse;
			rtn = DefineVehicle(Arg[2], Arg[8], Arg[3], Arg[4], Arg[5], Arg[6], Arg[7]);
			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "SPEED"))
		{
			if (!stricmp(Arg[3], "GET"))
			{
				RVal = GetVehicleSpeed(Arg[2]);
				ftoa(OutLoc, RVal);
				goto Rtnl;
			}
			goto RtnFalse;
		}
		if (!_fstricmp(Arg[1], "DRIVER"))
		{
			if (!stricmp(Arg[3], "SET"))
			{
				rtn = SetVehicleDriver(Arg[2], Arg[4]);
				goto Rtnrtn;
			}
			if (!stricmp(Arg[3], "GET"))
			{
				GetVehicleDriver(Arg[2], OutLoc);
				goto Rtnl;
			}
			goto RtnFalse;
		}
		if (!_fstricmp(Arg[1], "HEADING"))
		{
			if (!stricmp(Arg[3], "GET"))
			{
				RVal = GetVehicleHeading(Arg[2]);
				ftoa(OutLoc, RVal);
				goto Rtnl;
			}
			goto RtnFalse;
		}
		if (!_fstricmp(Arg[1], "HISTORY"))
		{
			if (!stricmp(Arg[2], "CLEAR"))
			{
				ClearVehicleHistory(atoi(Arg[3]));
				goto RtnTrue;
			}
			else if (!stricmp(Arg[2], "SETMAX"))
			{
				int maxtime = atoi(Arg[3]);

				if (!maxtime)
					maxtime = -USHRT_MAX;
				else
					maxtime = -maxtime;
				ClearVehicleHistory(maxtime);
				goto RtnTrue;
			}
			else if (!stricmp(Arg[2], "SETSEL"))
			{
				rtn = SetVehicleHistorySelection(atoi(Arg[3]));
				goto Rtnrtn;
			}
			else if (!stricmp(Arg[3], "GET"))
			{
				rtn = GetVehicleHistory(Arg[2], Arg[4], atob(Arg[5]));
				goto Rtnrtn;
			}
			else
			{
				HWND hWnd;

				strcpy(VehicleHistoryID, Arg[2]);
				hWnd = CreateDialog(hInst, (LPSTR)"VEHICLE_HISTORY", hWndMain, (DLGPROC)VEHICLE_HISTORYMsgProc);
				ShowWindow(hWnd, SW_SHOW);
				goto RtnTrue;
			}
		}
		if (!_fstricmp(Arg[1], "MONITOR"))
		{
			if (*Arg[2])
				rtn = CreateProgMonMap(Arg[2], Arg[3]);
			else
				rtn = DialogBox(hInst, (LPSTR)"PROGRESS_MONITORING", hWndMain, (DLGPROC)PROGRESS_MONITORINGMsgProc);
			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "TIMERDIALOG"))
		{
			if (!stricmp(Arg[2], "START"))
				rtn = StartVehTimeMenu(CurView->hWnd);
			else if (hWndVehTime && VehReplayFid == HFILE_ERROR)
				SetDlgItemText(hWndVehTime, IDC_VEHTIME, Arg[3]);
			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "STATUS"))
		{
			if (!stricmp(Arg[2], "CREATE"))
			{
				if (hWndVehStatus)
					rtn = TRUE;
				else
				{
					HWND	hWndDlg;
					LPVIEWPORT	SaveVP;

					SetCurView(SetVPFromName(Arg[3], &Err));
					SaveVP = CurView;
					StartVehInfoMenu(SaveVP);
				}
			}
			else if (!stricmp(Arg[2], "SHOW"))
			{
				if (hWndVehStatus)
				{
					int	Control = IDC_VEHDISPLAY_ALL;

					if (!stricmp(Arg[3], "ACTIVE"))
						Control = IDC_VEHDISPLAY_ACTIVE;
					else if (!stricmp(Arg[3], "INSERVICE"))
						Control = IDC_VEHDISPLAY_INSERVICE;
					else if (!stricmp(Arg[3], "AVAILABLE"))
						Control = IDC_VEHDISPLAY_AVAILABLE;
					else if (!stricmp(Arg[3], "FENCE"))
						Control = IDC_VEHDISPLAY_FENCE;
					SendDlgItemMessage(hWndVehStatus, IDC_VEHDISPLAY_ALL, BM_SETCHECK, FALSE, 0L);
					SendDlgItemMessage(hWndVehStatus, IDC_VEHDISPLAY_INSERVICE, BM_SETCHECK, FALSE, 0L);
					SendDlgItemMessage(hWndVehStatus, IDC_VEHDISPLAY_ACTIVE, BM_SETCHECK, FALSE, 0L);
					SendDlgItemMessage(hWndVehStatus, IDC_VEHDISPLAY_AVAILABLE, BM_SETCHECK, FALSE, 0L);
					SendDlgItemMessage(hWndVehStatus, IDC_VEHDISPLAY_FENCE, BM_SETCHECK, FALSE, 0L);
					SendDlgItemMessage(hWndVehStatus, Control, BM_SETCHECK, TRUE, 0L);
					UpdateVehicleStatusDlg();
					rtn = TRUE;
				}
			}
			else if (!stricmp(Arg[2], "REFRESH"))
			{
				UpdateVehicleStatusDlg();
				rtn = TRUE;
			}
			else if (!stricmp(Arg[2], "CLOSE"))
			{
				if (hWndVehStatus)
					SendMessage(hWndVehStatus, WM_CLOSE, 0, 0);
				rtn = TRUE;
			}

			goto Rtnrtn;
		}
		if (!_fstricmp(Arg[1], "CLEAR"))
		{
			SetCurView(SetVPFromName(Arg[2], &Err));
			rtn = ClearVehicles();
			goto Rtnrtn;
		}
		goto RtnFalse;

	case 766: //$ADDITEM(file.plt,Type(P,A,L,Open,Close,TwopointCurve),TAG,SYMBOLNAME,points,size,rot,text,textsize,TextColor,OpaqueText,Shadow,HIPrecis)
	{
		HANDLE	hStuff = 0, hTime = 0;
		LPSHORT	pStuff = 0;

		nArgs = GetFunArgs(Args, Arg, -16, &hMem, pBrkPt, bpOffset, bpLen);
		hMem2 = GSSiGlobAlloc(913, GMEM_MOVEABLE, USHRT_MAX);
		ExpandText(Arg[1]);
		ExpandText(Arg[2]);
		ExpandText(Arg[3]);
		ExpandText(Arg[4]);
		ExpandText(Arg[6]);
		ExpandText(Arg[7]);
		ExpandText(Arg[8]);
		ExpandText(Arg[9]);
		ExpandText(Arg[10]);
		ExpandText(Arg[11]);
		ExpandText(Arg[12]);
		ExpandText(Arg[13]);
		ExpandText(Arg[14]);
		ExpandText(Arg[15]);
		ExpandText(Arg[16]);
		Arg1 = GlobalLock(hMem2);
		_fstrcpy(Arg1, Arg[5]);
		ExpandText(Arg1);
		if (!strnicmp(Arg[2], "CI", 2))
			n = 7;
		else
		{
			Arg[2][1] = 0;
			n = _fstrcspn(" ALPOCT", Arg[2]);
		}
		if (!n)
			goto RtnFalse;
		n--;
		HiPrecis = TRUE;
		if (*Arg[13])
			HiPrecis = atob(Arg[13]);
		if (SetStuffFromText(Arg[14], atoi(Arg[15]), &hStuff))
			pStuff = GlobalLock(hStuff);
		hTime = SetTimeStampHandle(Arg[16]);
		rtn = AddAreaToMap(Arg[1], Arg[3], Arg[4], Arg1, n, atof(Arg[6]), atof(Arg[7]), Arg[8], atof(Arg[9]), atol(Arg[10]), atob(Arg[11]), atob(Arg[12]), pStuff, hTime);
		GSSiGlobUlFree(&hMem2);
		GSSiGlobUlFree(&hStuff);
		GSSiGlobFree(&hTime);
		if (rtn)
			goto RtnTrue;
		goto RtnFalse;
	}

	case 767: //$TCPPARM(socket,TIMER,seconds,timerstring)
	{
		SOCKET	socket;

		nArgs = GetFunArgs(Args, Arg, -4, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 3)
			goto RtnFalse;
		ExpandText(Arg[1]);
		ExpandText(Arg[2]);
		ExpandText(Arg[3]);
		socket = atol(Arg[1]);
		if (!_fstricmp(Arg[2], "TIMER"))
		{
			if (SetupSocketTimer(socket, atol(Arg[3]), Arg[4]))
				goto RtnTrue;
		}
		if (!_fstricmp(Arg[2], "TERM"))
		{
			if (!socket)
				socket = CurrentServerSocket;
			if (SetupSocketTerminator(socket, Arg[3]))
				goto RtnTrue;
		}
		goto RtnFalse;
	}
	case 768: //$DIRPATH(DESKTOP or DESKTOPDIR or MY DOCUMENTS or ALLUSERAPPDATA or APPDATA or LOCALAPPDATA,subdir(opt - will be created))  
		nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
		_fstrcpy(OutLoc, Arg[1]);
		if (!GetSpecialDirectory(OutLoc))
		{
			if (*Arg[2])
			{
				sprintf(strchr(OutLoc, 0), "\\%s", Arg[2]);
				makedirectories(OutLoc, TRUE, FALSE);
			}
			goto Rtnl;
		}
		else
			goto RtnFalse;

	case 769: //$NUMERIC(VAL,start,length) 
	{
		int	starting_at, for_how_long;

		nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
		starting_at = atoi(Arg[2]);
		for_how_long = atoi(Arg[3]);
		if (!starting_at)
			starting_at = 1;
		if (!for_how_long)
			for_how_long = strlen(Arg[1]) - starting_at + 1;
		if (NCHK(Arg[1], starting_at, for_how_long))
			goto RtnTrue;
		goto RtnFalse;
	}
	case 770: //$FILEPOS(GET,FILEID,CURRENT|RECORD|LAST
			  //		(SET,FILEID,FIRST|NEXT|PRIOR|LAST
		nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
		nlong = FilePos(Arg[1], Arg[2], Arg[3], atoi(Arg[4]));
		ltoa(nlong, OutLoc, 10);
		goto Rtnl;
	case 771: //$FILELEN(Pathname)
		nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
		nlong = GSSiLength(Arg[1]);
		ltoa(nlong, OutLoc, 10);
		goto Rtnl;

	case 772: //$TCPFILE(GET,socket,ToFile,FromFile,Delete)
			  //$TCPFILE(SEND,FromFile,Delete)
			  //$TCPFILE(REQUEST,FromFile,BlockSize)
			  //$TCPFILE(SAVE,ToFile,Length)
	{
		SOCKET	socket;

		nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 2)
			goto RtnFalse;
		if (!stricmp(Arg[1], "GET"))
		{
			socket = atol(Arg[2]);
			rtn = TCPGetFile(socket, Arg[3], Arg[4], atob(Arg[5]));
		}
		else if (!stricmp(Arg[1], "SEND"))
		{
			rtn = TCPSendFile(Arg[2], 500, atob(Arg[3]));
		}
		else if (!stricmp(Arg[1], "REQUEST"))
		{
			rtn = TCPSendFile(Arg[2], atoi(Arg[3]), FALSE);
		}
		else if (!stricmp(Arg[1], "SAVE"))
		{
			rtn = TCPSaveFile(Arg[2], atoi(Arg[3]));
		}
		goto Rtnrtn;
	}
	case 773: //$TOOLBAR(LOAD,FLOAT,Pathname,height,nperrowfloating,pos,DPoint,Scale,vpID)
			  //$TOOLBAR(LOAD,DOCK,Pathname,
		//$TOOLBAR(RELOAD,CURRENT(defalut) or ALL)
		//$TOOLBAR(REDISPLAY,CURRENT(defalut) or ALL)
		nArgs = GetFunArgs(Args, Arg, 9, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 1)
			goto RtnFalse;
		if (!stricmp(Arg[1], "LOAD"))
		{
			Point = atopt(Arg[7], &Err);
			RVal = atof(Arg[8]);

			rtn = LoadToolbar(CurView->hWnd, Arg[3], Arg[2], atoi(Arg[4]), atoi(Arg[5]), Arg[6], TRUE, FALSE, &Point, RVal, atoi(Arg[9]));
			itoa(rtn, OutLoc, 10);
			goto Rtnl;
		}
		if (!stricmp(Arg[1], "HOVER"))
		{
			rtn = SetToolbarHoverCmd(atoi(Arg[2]), Arg[3]);
		}
		if (!stricmp(Arg[1], "DESTROY"))
		{
			if (!stricmp(Arg[2], "ALL"))
			{
				DestroyAllToolbars();
				rtn = TRUE;
			}
			else
				rtn = DestroyCurrentToolbar();
		}
		if (!stricmp(Arg[1], "RELOAD"))
		{
			rtn = ReloadToolbar(Arg[2]);
		}
		if (!stricmp(Arg[1], "REDISPLAY"))
		{
			rtn = RedisplayToolbar(Arg[2]);//not working yet
		}
		goto Rtnrtn;
	case 774: //$NETWORK(FALSEINT,STREETLIST
			  //$NETWORK(FALSEINT,LOAD
			  //$NETWORK(FALSEINT,NEXTREF
			  //$NETWORK(FALSEINT,CLEAR
			  //$NETWORK(LOAD,HLTTOSLT,outfile
		nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 2)
			goto RtnFalse;
		if (!stricmp(Arg[1], "FALSEINT"))
			rtn = FalseIntFunctions(Arg[2], Arg[3], OutLoc);
		if (!stricmp(Arg[1], "LOAD"))
			rtn = LoadNetwork(Arg[2], Arg[3], Arg[4]);
		goto Rtnl;

	case 775: //$LOADMAP()
		nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
		FastMapCopy = TRUE;
		if (*Arg[1])
			strcpy(Arg[6], Arg[1]);
		else
			GSSiGetTempFileName(0, "gm", 0, Arg[6]);
		FastMapCopyHltOnly = atob(Arg[2]);
		FastMapCopyFid = GSSiOpenFile(Arg[6], 0, OF_CREATE);
		SetViewport(*pCommandViewport);
		FastMapCopynRecs = 0;
		GetWindowText(CurView->hWnd, Arg[5], 255);
		RedisplayViewport(TRUE, TRUE);
		FastMapCopyHltOnly = FALSE;
		SetWindowText(CurView->hWnd, Arg[5]);
		FastMapCopy = FALSE;
		GSSiClose2(&FastMapCopyFid);
		if (!*Arg[1])
			CopySelectedRecords(Arg[6], TRUE, FALSE, 0, FALSE);
		goto RtnTrue;

	case 776: //$AZDIFF(az1,az2)
		nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
		ftoa(OutLoc, DeltaAZ(atof(Arg[1]), atof(Arg[2])));
		goto Rtnl;

	case 777: //$PROCESS(CREATE,program,commandline,newwindowrect,waitforprocesstoend,maxwait,startindir)
			  //$PROCESS(STOP,
			  //$PROCESS(GETWINDOW
	{
		nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
		*OutLoc = 0;
		if (!stricmp(Arg[1], "CREATE"))
		{
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			DWORD	CRFlags = 0;
			RECT	rect;

			CloseAllRequestedFiles(FALSE);
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			rect = atorect(Arg[4], &Err);
			if (!Err)
			{
				si.dwX = rect.left;
				si.dwY = rect.top;
				si.dwXSize = RECTWIDTH(&rect);
				si.dwYSize = RECTHEIGHT(&rect);
				si.dwFlags = STARTF_USEPOSITION | STARTF_USESIZE;
			}
			if (!atob(Arg[5]))
				CRFlags = DETACHED_PROCESS;
			else
				CRFlags = CREATE_NEW_CONSOLE;
			ExpandText(Arg[3]);
			if (!*Arg[7])
			{
				strcpy(Arg[7], "[%DL]");
				ExpandText(Arg[7]);
				*LastChr(Arg[7]) = 0;
			}
			if (CreateProcess(Arg[2], Arg[3],
				NULL,             // Process handle not inheritable. 
				NULL,             // Thread handle not inheritable. 
				FALSE,            // Set handle inheritance to FALSE. 
				CRFlags,		  // creation flags. 
				NULL,             // Use parent's environment block. 
				Arg[7],             // Use parent's starting directory. 
				&si,              // Pointer to STARTUPINFO structure.
				&pi)             // Pointer to PROCESS_INFORMATION structure.
				)
			{
				BOOL TimedOut;
				int	 MaxWait = atol(Arg[6]);

				if (MaxWait >= 0)
				{
					WaitForInputIdle(pi.hProcess, INFINITE);
					ltoa((long)pi.hProcess, OutLoc, 10);

					if (atob(Arg[5]))
					{
						while (WaitForProcessToEnd(pi.dwProcessId, &MaxWait));
						if (MaxWait)
							goto RtnTrue;
						else
							goto RtnFalse;
					}
				}
			}
			else
			{
				int err = GetLastError();
				ltoa(-err, OutLoc, 10);
			}
		}
		else if (!stricmp(Arg[1], "STOP"))
		{
			//TerminateProcess (pi.hProcess,0);
			//CloseHandle( pi.hProcess );
			//CloseHandle( pi.hThread );
		}
		else if (!stricmp(Arg[1], "GETWINDOW"))
		{
			HANDLE	hProcess = (HANDLE)atol(Arg[2]);
			DWORD	ProcessID = GetProcessId(hProcess);
			HWND	hWnd, hWndPar;
			char	txt[256];

			WaitForInputIdle(hProcess, 2000);
			hWnd = FindWindowByProcessID(ProcessID, Arg[3]);
			hWndPar = GetParent(hWnd);
			GetWindowText(hWnd, txt, 256);
			while (hWndPar)
			{
				hWnd = hWndPar;
				GetWindowText(hWnd, txt, 256);
				hWndPar = GetParent(hWnd);
			}
			ltoa((long)hWnd, OutLoc, 10);
		}
		goto Rtnl;

	}

	case 778: //$SESSION(CREATE,commandline,newwindowrect,startupzoom,startlocation)
			  //$SESSION(STOP,hwnd
			  //$SESSION(COMMAND,hwnd
	{
		char modulePath[MAX_PATH];

		nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
		*OutLoc = 0;
		if (!stricmp(Arg[1], "CREATE"))
		{
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			DWORD	CRFlags = 0;
			RECT	rect;
			char	startIn[MAX_PATH] = "[%DL]";
			LPSTR	pstartIn = startIn;
			MNMXCORD zoomBounds;

			CloseAllRequestedFiles(FALSE);
			GetModuleFileName(NULL, modulePath, MAX_PATH);
			REPLAC(modulePath, "\\.\\", "\\",MAX_PATH);
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			rect = atorect(Arg[3], &Err);
			if (!Err)
			{
				si.dwX = rect.left;
				si.dwY = rect.top;
				si.dwXSize = RECTWIDTH(&rect);
				si.dwYSize = RECTHEIGHT(&rect);
				si.dwFlags = STARTF_USEPOSITION | STARTF_USESIZE | STARTF_USESHOWWINDOW;
			}
			zoomBounds = atobounds(Arg[4], &Err);
			if (!Err)
			{
				char OutFile[MAX_PATH];
				LPSTR pConfigFile = Arg[2];
				if (!strnicmp(pConfigFile, "GeoMaster ", 10))
					pConfigFile += 10;
				SaveZooms(&zoomBounds);
				CreateGMStartupFile(OutFile, pConfigFile, FALSE, TRUE);
				sprintf(Arg[2], "GeoMaster %s", OutFile);
				CloseAllRequestedFiles(FALSE);
			}
			CRFlags = DETACHED_PROCESS;
			// for mapserver addd BELOW_NORMAL_PRIORITY_CLASS
			//ExpandText(Arg[2]);
			if (*TestFileLocation)
				sprintf(strchr(Arg[2], 0), " [%%TESTDL]=%s;", TestFileLocation);
			else if (*Arg[5] && FileType(Arg[5]) == 2)
			{
				strcpy(startIn, Arg[5]);
			}
			ExpandText(startIn);
			if (!*startIn)
				pstartIn = NULL;
			if (strstr(Arg[2], "/GMDoc"))
			{
				char gmdocEx[MAX_PATH];
				LPSTR pDot;
				strcpy(gmdocEx, modulePath);
				pDot = strrchr(gmdocEx, '.');
				strcpy(pDot, "_gmdoc.exe");
				//if (!FileType(gmdocEx))
				CopyFile(modulePath, gmdocEx, FALSE);
				strcpy(modulePath, gmdocEx);
			}
			if (strstr(Arg[2], "/GMCache"))
			{
				char gmdocEx[MAX_PATH];
				LPSTR pDot;
				strcpy(gmdocEx, modulePath);
				pDot = strrchr(gmdocEx, '.');
				strcpy(pDot, "_gmcache.exe");
				//if (!FileType(gmdocEx))
				CopyFile(modulePath, gmdocEx, FALSE);
				strcpy(modulePath, gmdocEx);
			}
			if (CreateProcess(modulePath, Arg[2],
				NULL,             // Process handle not inheritable. 
				NULL,             // Thread handle not inheritable. 
				FALSE,            // Set handle inheritance to FALSE. 
				CRFlags,		  // creation flags. 
				NULL,             // Use parent's environment block. 
				pstartIn,             // Use parent's starting directory. 
				&si,              // Pointer to STARTUPINFO structure.
				&pi)             // Pointer to PROCESS_INFORMATION structure.
				)
			{
				BOOL TimedOut;
				int	 MaxWait = atol(Arg[5]);
				DWORD	ProcessID = GetProcessId(pi.hProcess);

				//Wait (1000);

				WaitForInputIdle(pi.hProcess, INFINITE);
				hWnd = FindWindowByProcessID(ProcessID, "");
				ltoa((long)hWnd, OutLoc, 10);
				if (atob(Arg[5]))
				{
					while (WaitForProcessToEnd(pi.dwProcessId, &MaxWait));
					if (MaxWait)
						goto RtnTrue;
					else
						goto RtnFalse;
				}
			}
			else
			{
				ltoa(-((int)GetLastError()), OutLoc, 10);
			}
		}
		else if (!stricmp(Arg[1], "STOP"))
		{
			HWND hProcessWnd = (HWND)atol(Arg[2]);
			//TerminateProcess (pi.hProcess,0);
			//CloseHandle( pi.hProcess );
			//CloseHandle( pi.hThread );
			SendConnectedProcessMessage(hProcessWnd, GF_END_PROCESS, 0, 0);
			goto RtnTrue;
		}
		else if (!stricmp(Arg[1], "COMMAND"))
		{
			HWND hProcessWnd = (HWND)atol(Arg[2]);
			SendConnectedProcessCommand(hProcessWnd, Arg[2]);
			goto RtnTrue;
		}
		else if (!stricmp(Arg[1], "PATH"))
		{
			GetModuleFileName(NULL, OutLoc, MAX_PATH);
		}
		goto Rtnl;

	}

	case 779: //$COMPOSE(Title,InMessage(opt),SendMacro)
		nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
		if (ComposeMessage(hWndMain, Arg[1], Arg[2], Arg[3]))
			goto RtnTrue;
		goto RtnFalse;

	case 780: //$BATTERY(EXISTS)
			  //$BATTERY(REMAINING)
			  //$BATTERY(CHARGING)
	{
		SYSTEM_POWER_STATUS sps;
		BOOL rtn = GetSystemPowerStatus(&sps);


		nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
		if (!stricmp(Arg[1], "EXISTS"))
		{
			if (sps.BatteryFlag == 128)
				strcpy(OutLoc, "0");
			else
				strcpy(OutLoc, "1");
		}
		else if (!stricmp(Arg[1], "REMAINING"))
		{
			itoa(sps.BatteryLifePercent, OutLoc, 10);
		}
		else
		{
			if (sps.BatteryFlag < 128 && sps.BatteryFlag & 8)
				strcpy(OutLoc, "0");
			else
				strcpy(OutLoc, "1");
		}
		goto Rtnl;
	}

	case 781: //$GEOCODE(FORWARD,)
		//$GEOCODE(REVERSE,point,format)
		nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
		*OutLoc = 0;
		if (!stricmp(Arg[1], "REVERSE"))
			ReverseGeocodeCommand(nArgs - 1, &Arg[1], OutLoc);
		else if (!stricmp(Arg[1], "ALLTYPES"))
			GeocodeAlltypes(hWndMain, OutLoc, Arg[2]);
		goto Rtnl;
	case 782: //$ADDRESS(SET,(I2orI4orR4orR8),address,value)
	{
		nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
		if (!stricmp(Arg[1], "SET"))
		{
			if (!stricmp(Arg[2], "I4"))
			{
				LPINT pI4 = (LPINT)atoi(Arg[3]);
				*pI4 = atoi(Arg[4]);
				goto RtnTrue;
			}
		}
		goto RtnFalse;
	}
	case 783: //$TESTENV(STORE,testdir)
	{
		nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
		if (!stricmp(Arg[1], "STORE"))
		{
			rtn = StoreTestToProduction(Arg[2], 1);
			goto Rtnrtn;
		}
		goto RtnFalse;
	}

	case 784: //$COPYDIR(TODIR,FROMDIR,T or F replace,T or F display status window)
	{
		nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
		*OutLoc = 0;
		if (!FileType(Arg[2]) == 2)
		{
			sprintf(OutLoc, "%s is not a directory", Arg[2]);
			goto Rtnl;
		}
		if (FileType(Arg[1]) && !atob(Arg[3]))
		{
			sprintf(OutLoc, "%s already exists", Arg[1]);
			goto Rtnl;
		}
		rtn = CopyDirectory(Arg[1], Arg[2], atob(Arg[3]), Arg[4]);
		goto Rtnrtn;
	}
	case 785: //$NVMETRO(LOADMULTPROP,LastYear)
	{
		nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
		if (!stricmp(Arg[1], "LOADMULTPROP"))
		{
			itoa(LoadMultPropertyDB(Arg[2], atoi(Arg[3])), OutLoc, 10);
			goto Rtnl;
		}
		else if (!stricmp(Arg[1], "CREATEMULTVALUES"))
		{
			itoa(CreateMultValueFile(Arg[2]), OutLoc, 10);
			goto Rtnl;
		}
		else if (!stricmp(Arg[1], "ASSIGNMULTVALUES"))//$NVMETRO(ASSIGNMULTVALUES, [MDRIVE]\parcelfiles\textfiles, [%DL]attribut\MetroGISPropinfo.gmd)
		{
			rtn = AssignMultValues(Arg[2], Arg[3]);
			goto Rtnrtn;
		}
		else if (!stricmp(Arg[1], "TESTMULTVALUES"))//$NVMETRO(TESTMULTVALUES, indexfile,datafile,pid)
		{
			rtn = TestMultValues(Arg[2], Arg[3],Arg[4]);
			goto Rtnrtn;
		}
		else if (!stricmp(Arg[1], "ASSIGNUSECODE"))//$NVMETRO(ASSIGNUSECODE,[DBHANDLE],[PT])
		{
			BOOL err;
			DPOINT pt = atopt(Arg[3], &err);
			LLPOINT ptll;

			if (err)
				goto RtnFalse;
			ConvertCoord(&pt, 1, 2);
			ptll.lat = pt.y;
			ptll.lon = pt.x;
			int code = AssignLandUseCodeToParcels(ptll, (sqlite3*)atol(Arg[2]));
			itoa(code, OutLoc, 10);
			goto Rtnl;
		}
		goto RtnFalse;
	}
	case 786: //$TAGDUMP(pltfile,prefix,dumpfile)
	{
		int ntags = 0;
		nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
		*OutLoc = 0;
		if (nArgs > 2)
			ntags = DumpTAGsToFile(Arg[1], Arg[2], Arg[3]);
		itoa(ntags, OutLoc, 10);
		goto Rtnl;
	}
	break;
	case 787: // $DIMLINE(x1,y1,x2,y2,opt,color,units,format,ViewPort,txtsize)
	{
		LPSTR	lpEnd;
		DPOINT	p1, p2;
		int opt;
		COLORREF color;
		int units;
		double txtsize;

		if (!CurView)
			goto RtnFalse;
		nArgs = GetFunArgs(Args, Arg, 10, &hMem, pBrkPt, bpOffset, bpLen);
		if (nArgs < 4)
			goto RtnFalse;
		p1.x = atof(Arg[1]);
		p1.y = atof(Arg[2]);
		p2.x = atof(Arg[3]);
		p2.y = atof(Arg[4]);
		opt = atoi(Arg[5]);
		color = (COLORREF)atoi(Arg[6]);
		units = atoi(Arg[7]);
		SetCurView(SetVPFromName(Arg[9], &Err));
		txtsize = atof(Arg[10]);
		DimensionLine(CurView->hDC, &p1, &p2, opt, color, units, Arg[8], txtsize);
		goto RtnTrue;
	}
	case 788: //$MONITOR(COUNT)
			  //$MONITOR(CURRENT)
			  //$MONITOR(MOVE,id)
	{
		nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
		*OutLoc = 0;
		if (!stricmp(Arg[1], "COUNT"))
		{
			itoa(numMonitors, OutLoc, 10);
		}
		else if (!stricmp(Arg[1], "CURRENT"))
		{
			itoa(GetCurrentMonitor(), OutLoc, 10);
		}
		else if (!stricmp(Arg[1], "SWITCH"))
		{
			int fromMon = GetCurrentMonitor();
			int iMon = fromMon;
			if (iMon == 1)
				iMon = 2;
			else
				iMon = 1;
			MoveToMonitor(iMon - 1, fromMon - 1);
			strcpy(OutLoc, "1");
		}
		else if (!stricmp(Arg[1], "MOVETO"))
		{
			int iMon = atoi(Arg[2]);
			int fromMon = GetCurrentMonitor();
			iMon = max(0, iMon - 1);
			MoveToMonitor(iMon, fromMon);
			strcpy(OutLoc, "1");
		}
		goto Rtnl;
	}
	break;
	case 789:  //$INTCHOP(returns integer value (truncates remaining junk))
	{
		hMem = GSSiGlobAlloc(821, GMEM_MOVEABLE, 2048);
		Arg1 = GlobalLock(hMem);
		_fstrcpy(Arg1, Args);
		ExpandText(Arg1);
		nlong = integerValue(Arg1);
		ltoa(nlong, OutLoc, 10);
		goto Rtnl;
	}


		default:
			goto Rtn0;
	}
Rtnrtn:
	if (!rtn)
		goto RtnFalse;
RtnTrue:
	l = 1;
	_fstrcpy(OutLoc, "1");
	goto Exit;
RtnFalse:
	l = 1;
	_fstrcpy(OutLoc, "0");
	goto Exit;
Rtn0:
	l = 0;
	goto Exit;
Rtnl:
	l = _fstrlen(OutLoc);
Exit:
	if (SaveCfg != CurrentConfig)
	{
		SetConfig(SaveCfg);
		if (*pNumViewports)
			SetCurView(SaveVP);
	}
	GSSiGlobUlFree(&hMem);
	if (TraceOn)
	{
		hMem = GSSiGlobAlloc(914, GMEM_MOVEABLE, 4096);
		lpstr = GlobalLock(hMem);
		sprintf(lpstr, "GFV:%s", Args);
		GSSiTraceLev(lpstr, -1, 2);
		GSSiGlobUlFree(&hMem);
	}
	{
#if ENABLETRACE
		GSSiExitProg(1348);
#endif
		return (l);
	}
#if ENABLETRACE
}
#endif
}


