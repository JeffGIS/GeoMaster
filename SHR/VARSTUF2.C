#include "graphint.h"   
#include "extrndb.h"

#include "gmextern.h"     

#define	MAX_BUFFERED_MACROS	32

static	HANDLE	hBufferedMacros[MAX_BUFFERED_MACROS];
static	char	BufferedMacroNames[MAX_BUFFERED_MACROS][256];
static	short	NumBufferedMacros=0;  
static	ULONG	CurrentMacroTime=0;
static	ULONG	LastMacroUse[MAX_BUFFERED_MACROS];
static	HANDLE	MacroVarSpace[MAX_MACRO_STACK] = { 0 };

BOOL LoadInternalGMD (LPGWDHEADER lpGWDHead,long iref)
#if ENABLETRACE
{GSSiEnterProg (598);
#endif
{   
	DPOINT	MP={0,0};
	short	ii, Type;
	char	SymName[34];
	BOOL	rtn=FALSE;
	struct {
			long	refno;
			long	MSLink;
			char	prefix[8], udi[64], hastext, Type, SymName[32];
			short	symnum, Layer;
			double	Length, Area, Perimeter, AZM, Elev;
			long	MinTime, MaxTime;
			double	BPX, BPY, MPX, MPY, EPX, EPY;
			char	text[256];
			char	FileName[256];
			char	Globals[256];
			long	NumPoints;
			short	NumPoly; 
			MNMXCORD	Bounds;} Type1;
	struct {
			long	refno;  
		 	long	MSLink;
			char	prefix[8], udi[64];
			short	symnum, Type,Layer;  
			DPOINT	BeginPoint, EndPoint, PickedPoint,NodePoint;
			double	PCT, Length,Area, OffDist;  
			long	NumPoints;
		 	double	BPAZ,PPAZ,EPAZ;
		 	double	SlopePCT,SlopeAngle; 
		   } Type2;
	
	switch (lpGWDHead->Version)
	{
		case 1001: 
		case 1002: 
		case 1003:
		case 1004:
		case 1005:  
		case 1006:

		if (InGraphicsProcessor)
			ii=1;
		else
			if (!SetPickGlobalsCalled)
				break;	
       
        SetPickGlobalsCalled = FALSE;
		_fmemset (&Type1,0,sizeof(Type1));
		  
		Type1.refno = iref;
		if (InGraphicsProcessor)
			Type1.MSLink = CurMSLink;
		else
		{
			CurrentProcessedPickedItem = 0;
			Type1.MSLink = PickList[0].MSLink;
		}
		_fstrncpy (Type1.prefix,CurrentPrefix,MAX_PREFIX_LEN); 
		_fstrncpy (Type1.udi,CurrentUDI,MAX_UDI_LEN);
		Type1.Layer = FileNum;
		if (InGraphicsProcessor)
			ScanItemForDBVals ();
		GetGlobalVal (hTEXT,Type1.text,0); 
		REPLAC (Type1.text,"\r\n","$CHR(13)",256);
		if (*Type1.text)
			Type1.hastext = 'Y';
		else
			Type1.hastext = 'N';  
		if (CurrentType == GF_TEXT)
			Type = CurrentTextBaseType;
		else
			Type = CurrentType; 
		if (!InGraphicsProcessor && CurrentProcessedPickedItem >=0) 			 
			Type1.Bounds = PickList[CurrentProcessedPickedItem].Rect;
		else
			GetItemMinMax (&CurrentItemMinMax,&Type1.Bounds);
		switch (Type)
		{
			case GF_POINT:
				Type1.Type = 'P';  
				if (!InGraphicsProcessor)
				{  
					if (CurrentProcessedPickedItem >= 0)
					{
						Type1.AZM = PickList[CurrentProcessedPickedItem].BPAZ;
						Type1.BPX = Type1.MPX = Type1.EPX = PickList[CurrentProcessedPickedItem].BeginPoint.x;
						Type1.BPY = Type1.MPY = Type1.EPY = PickList[CurrentProcessedPickedItem].BeginPoint.y;
						Type1.Elev = 0;
					}
				}
				else
				{
					Type1.AZM = CurPointAZ; 
					Type1.BPX = Type1.MPX = Type1.EPX = CurPointLocD.x;
					Type1.BPY = Type1.MPY = Type1.EPY = CurPointLocD.y; 
					Type1.Elev = CurPointZ;
				}
				Type1.NumPoints = 1;
				break;
			case GF_LINE:
			case GF_POLYLINE:
			{   
				HPPOINTS	lpEndPoint;  
				HPDPOINT	lpDEndPoint;
				double	Dist=0;
				double MPAZ;
				DPOINT	MidPoint;

				Type1.Type = 'L'; 
				Type1.MPX = 0;
				Type1.MPX = 0;
GetPolylineEP:
				Type1.Elev = (double)iref/10;
				if (InGraphicsProcessor)
				{    
					Type1.NumPoints = nCurPoints;  
					Type1.Layer = FileNum;
					if (HiPrecis)
					{   
						HPDPOINT	lpPoints=lpDCurPoints;
						
						long		nPnts=nCurPoints;
						
						if (hDynamicSeg)
						{
							lpPoints=(HPDPOINT)GlobalLock (hDynamicSeg);
							nPnts = nDynamicCoord;
						}
						Dist = GetPolyLengthD (lpPoints,nPnts);
						GetPolyBoundsD2 (lpPoints,nPnts,&Type1.Bounds,Type);
						lpDEndPoint = lpPoints + (nPnts-1); 
						Type1.BPX = lpPoints->x;
						Type1.BPY = lpPoints->y;  
						Type1.EPX = lpDEndPoint->x;
						Type1.EPY = lpDEndPoint->y;
						Type1.AZM = getazd (lpPoints,lpDEndPoint); 
						if (CurrentType == GF_LINE || CurrentType == GF_POLYLINE)
						{
							Type1.NumPoly = nPolySegments;
							if (useOnlyOnscreenPoly)
							{
									DPOINT ScreenPoints[5];
									double ScreenAZ[5];
									DPOINT IntPoint;
									double	IntDist[3], InAZ, OutAZ[3];
									short	WhichPoly[3];
									BOOL	OutReverse[3];
									double  SegDist[16];

									int n=1;
									double startDist = 0;
									int nDist = 0;

									Type1.MPX = -1;
									Type1.MPY = -1;

									if (PointInWBounds(lpPoints))
										SegDist[nDist++] = 0;

									BoundsToPoints(&CurView->WBounds, ScreenPoints,ScreenAZ);
									ScreenPoints[4] = ScreenPoints[0];

									while (n)
									{
										n = IntersectPolys2(nPnts, lpPoints,
											5, ScreenPoints,
											startDist, IntDist, &IntPoint, &InAZ,
											OutAZ, OutReverse, WhichPoly, TRUE);
										if (n)
										{
											SegDist[nDist++] = IntDist[0];
											startDist = IntDist[0];
										}
									}
									if (PointInWBounds(&lpPoints[nPnts-1]))
										SegDist[nDist++] = Dist;
									if (nDist > 1)
									{
										double MPDist = (SegDist[0] + SegDist[1]) / 2.0;
										MidPoint = PointAtDistOnPoly(lpPoints, nPnts, MPDist, &MPAZ, 0);
										Type1.MPX = MidPoint.x;
										Type1.MPY = MidPoint.y;
										Type1.AZM = MPAZ;
									}
							}
							else
							{
								MidPoint = PointAtDistOnPoly(lpPoints, nPnts, Dist / 2, &MPAZ, 0);
								Type1.MPX = MidPoint.x;
								Type1.MPY = MidPoint.y;
							}
						}
						if (CurrentType == GF_CURVE)
						{   
							DPOINT	BP,RP; 
							short	rc; 
							double	Radius;
								
							Type1.MPX = lpPoints[1].x;
							Type1.MPX = lpPoints[1].y;
							rc = RCURVE(&Type1.BPX,&Type1.BPY,&Type1.MPX,&Type1.MPY,&Type1.EPX,&Type1.EPY,
										&RP.x,&RP.y,&Dist); 
							BP.x = Type1.BPX;
							BP.y = Type1.BPY;
							Radius = ldistp (BP,RP);
							Type1.Length = Dist; 
							SetGlobalValueReal ("%RADIUS",Radius);
						}
						else if (ComputeArea)
							ComputeProjectedAreaAreaD (lpPoints,-nPnts,&Dist); 
						if (hDynamicSeg)
							GlobalUnlock (hDynamicSeg); 
						CurLength = Dist;
					}
					else
					{   
						DPOINT	BPointW, EPointW;
						
						CurStartPoint = POINTStoPOINT(*lpCurPoints);
						lpEndPoint = lpCurPoints + (nCurPoints-1);   
						BPointW = FilePtToBasePt (CurStartPoint);
						EPointW = FilePtToBasePt (POINTStoPOINT(*lpEndPoint));
						Type1.BPX = BPointW.x;
						Type1.BPY = BPointW.y;  
						Type1.EPX = EPointW.x;
						Type1.EPY = EPointW.y;
						Type1.AZM = getaz (POINTStoPOINT(*lpCurPoints),POINTStoPOINT(*lpEndPoint));  
						if (ComputeArea)
						{
							ComputeProjectedAreaArea (lpCurPoints,-nCurPoints,&Dist);  
							CurLength = Dist;
							Type1.Length = ConvertDist(Dist,OutDistUnits);
						}
					}
				}  
				else if (CurrentProcessedPickedItem >=0) 
				{   
					HANDLE hPnts;
					int nPnts;
					if (GetPolyPoints((LPPICKDATAHEADER)&PickList[CurrentProcessedPickedItem], FALSE, &nPnts, &hPnts))
					{
						HPDPOINT lpPoints = (HPDPOINT)GlobalLock(hPnts);
						double MPAZ;
						DPOINT	MidPoint = PointAtDistOnPoly(lpPoints, nPnts, PickList[CurrentProcessedPickedItem].Length / 2, &MPAZ, 0);
						PickList[CurrentProcessedPickedItem].Length = GetPolyLengthD(lpPoints, nPnts);
						Type1.BPX = BP.x = lpPoints[0].x;
						Type1.BPY = BP.y = lpPoints[0].y;
						Type1.MPX = MP.x = MidPoint.x;
						Type1.MPY = MP.y = MidPoint.y;
						Type1.EPX = EP.x = lpPoints[nPnts - 1].x;
						Type1.EPY = EP.y = lpPoints[nPnts - 1].y;
						PickList[CurrentProcessedPickedItem].BeginPoint = BP;
						PickList[CurrentProcessedPickedItem].EndPoint = EP;
						GSSiGlobUlFree(&hPnts);
					}
					Type1.Length = ConvertDist(PickList[CurrentProcessedPickedItem].Length, OutDistUnits);
					CurLength = PickList[CurrentProcessedPickedItem].Length;

				}
				else
				{
				}
			}
				break;
			case GF_CURVE:		 
				Type1.Type = 'C'; 
				Type1.NumPoints = 3;  
				goto GetPolylineEP;
				break;
			case GF_AREA:
			{
				double	Dist, Area;  
							  
				Type1.Type = 'A';  
				if (InGraphicsProcessor)
				{   
					Type1.NumPoints = nCurPoints; 
					Type1.NumPoly = nPoly;
					if (ComputeArea)
					{
						if (HiPrecis)
							Area = ComputeProjectedAreaAreaD (lpDCurPoints,nCurPoints,&Dist);
						else  
							Area = ComputeProjectedAreaArea (lpCurPoints,nCurPoints,&Dist); 
						CurLength = Dist;
						CurArea = Area; 
						Type1.Area = ConvertArea (Area,OutAreaUnits); 
						Type1.Perimeter = ConvertDist(Dist,OutDistUnits);
					}
				}
				else if (CurrentProcessedPickedItem >=0) 
				{   
					if (hCurPolyPoints)
					{
						PickList[CurrentProcessedPickedItem].Area = ComputeAreaAreaDH (hCurPolyPoints,nCurPolyPoints,&PickList[CurrentProcessedPickedItem].Length);
						MP = ComputeAreaMidpoint (hCurPolyPoints,nCurPolyPoints);
					}
					CurLength = PickList[CurrentProcessedPickedItem].Length;
					CurArea = PickList[CurrentProcessedPickedItem].Area;
					Type1.Area = ConvertArea (PickList[CurrentProcessedPickedItem].Area,OutAreaUnits); 
					Type1.Perimeter = ConvertDist(PickList[CurrentProcessedPickedItem].Length,OutDistUnits); 
				}
			}
				break;
			case GF_TEXT:			 
				Type1.Type = 'T';
				Type1.AZM = TXRot;
				break;
		}
		Type1.MinTime = GRStartTime;
		Type1.MaxTime = GREndTime; 
		_fstrcpy (Type1.Globals,"[%GLOBALS]");
		ExpandText (Type1.Globals);
		Type1.symnum = CurrentDesc;
		if (InGraphicsProcessor)  
		{
			if (!GetOpenFileSymName (CurrentDesc,SymName))
				GetDictSymName (CurrentDesc,SymName);
			_fstrcpy (Type1.FileName,PltName);
			ExpandText (Type1.FileName);   
		}
		else
		{   
			double	Radius;
			
			Type1.BPX = CurBP.x;
			Type1.BPY = CurBP.y;
			Type1.MPX = MP.x;
			Type1.MPY = MP.y;
			Type1.EPX = CurEP.x;
			Type1.EPY = CurEP.y;
			GetSymbolName (CurrentDesc,SymName,NULL,0,NULL);
			_fstrcpy (Type1.FileName,PickName);
			ExpandText (Type1.FileName);   
			if (CurrentType == GF_CURVE)
			{   
				DPOINT	BP,RP; 
				short	rc; 
				double	CLEN;
								
				Type1.MPX = CurMP.x;
				Type1.MPY = CurMP.y;
				rc = RCURVE(&Type1.BPX,&Type1.BPY,&Type1.MPX,&Type1.MPY,&Type1.EPX,&Type1.EPY,
							&RP.x,&RP.y,&CLEN); 
				BP.x = Type1.BPX;
				BP.y = Type1.BPY;
				Type1.Length = CLEN; 
				CurLength = CLEN;
				Radius = ldistp (BP,RP);
			} 
			else
				Radius = 0;
				SetGlobalValueReal ("%RADIUS",Radius);
		}
		_fstrncpy (Type1.SymName,SymName,32);
		_fmemmove (&lpGWDHead->GWDData,&Type1,sizeof(Type1)); 
		rtn = TRUE;
		break;
	
		case 2001:
		case 2002:
		case 2003:
		_fmemset (&Type2,0,sizeof(Type2));
		Type2.refno = iref; 
		if (InGraphicsProcessor) 
		{
			_fstrncpy (Type2.prefix,CurrentPrefix,MAX_PREFIX_LEN); 
			_fstrncpy (Type2.udi,CurrentUDI,MAX_UDI_LEN);
			Type2.Layer = FileNum; 
			Type2.symnum = CurrentDesc;
			Type2.NumPoints = nCurPoints;
		}
		else
		{
			_fstrncpy (Type2.prefix,PickList[0].Prefix,8); 
			_fstrncpy (Type2.udi,PickList[0].UDI,64);     
			Type2.symnum = PickList[0].Desc;
			Type2.Type = PickList[0].Type;  
			Type2.Layer = PickList[0].FileNum; 
			Type2.NumPoints = PickList[0].NumPoints;
		}
		Type2.BeginPoint = PickList[0].BeginPoint;
		Type2.EndPoint = PickList[0].EndPoint;
		Type2.PickedPoint = PickList[0].PickedPoint;
		Type2.NodePoint = PickList[0].NodePoint;
		Type2.PCT = PickList[0].PCT;
		Type2.Length = PickList[0].Length;
		Type2.OffDist = PickList[0].OffDist; 
		Type2.BPAZ = PickList[0].BPAZ;
		Type2.PPAZ = PickList[0].PPAZ;
		Type2.EPAZ = PickList[0].EPAZ;
		Type2.Area = PickList[0].Area;
		Type2.MSLink = PickList[0].MSLink;
		Type2.SlopePCT = CurSlopePCT*100;
		Type2.SlopeAngle = CurSlopeAngle;
		_fmemmove (&lpGWDHead->GWDData,&Type2,sizeof(Type2)); 
		rtn = TRUE;
		break;	

	}  
	CurrentProcessedPickedItem = -1;
{
#if ENABLETRACE
GSSiExitProg (598);
#endif
	return rtn;	
}
#if ENABLETRACE
}
#endif
} 

BOOL GetDBUniqueFieldValues (HANDLE hDB, LPSTR SQL,LPSTR FldName, LPHANDLE phDBList)
{
	BOOL rtn=FALSE;
	LPOPENSQLDATA	SQLPtr;
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr; 
	short	j, fLen; 

	SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	lpFieldInfo = &FilePtr->FldInfo;
	if ((j=GetFieldIDFromName(SQLPtr->IDName,FilePtr,FldName))>=0) 
	{
		fLen = min (lpFieldInfo[j].length,255);    
		*phDBList = CreateUniqueList (fLen,NULL);  
		switch (FilePtr->Type)
		{   
			case UMIFS_DATAFILE:
			case ORA_DATAFILE:
				rtn = GetGMDUniqueFieldValues (hDB, SQL,j, *phDBList); 
				break;
			case GMTEXT_DATAFILE:
				rtn = GetTXTUniqueFieldValues (hDB,SQL,lpFieldInfo[j].name,fLen,*phDBList);
				break;
			default:
				rtn = GetODBCUniqueFieldValues ((int)FilePtr->FileHandle,"",lpFieldInfo[j].name,fLen,*phDBList);
				break;
		}
    }
	GlobalUnlock (SQLPtr->OFHandle); 
	GlobalUnlock (hDB);
	
	return rtn;
}

BOOL GetTXTUniqueFieldValues (HANDLE hDB, LPSTR SQL,LPSTR FldName,short FieldLength, HANDLE hDBList)
#if ENABLETRACE
{GSSiEnterProg (612);
#endif
{                             
    int   i, ClassNo, ClassZero=0; 
    HANDLE      hBT;
    long        Offset;
    short       len, pos;
	BOOL		err;
    HCURSOR hcurSave;
	LPOPENSQLDATA	SQLPtr;
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr;
	HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096+2048);
	LPSTR	str=GlobalLock (hMem); 
	LPSTR	Value = str+4096;
	LPSTR	pSQL = Value + 512;
	LPSTR	Value2 = pSQL + 1024;
    
    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
	SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
   	GSSillseek (FilePtr->Fid,0,0);    
	SQLPtr->st =0;
	fgetstring (str,4090,FilePtr->Fid);
	ConvertSQLToLogicP (pSQL,SQL);
	do
	{
		SQLPtr->Offset = GSSillseek (FilePtr->Fid,0,1);  
		if (!fgetstring (str,4090,FilePtr->Fid))
			SQLPtr->st = 1;
		else 
		{
			GetDelimTextData(str,FilePtr->FileHandle,4090); 
			sprintf (Value,"[%s]",FldName);
			ExpandText (Value);
            _fstrncpy (Value2,Value,FieldLength);  
            if (LogicPFile (SQLPtr,pSQL,&err))
            	if (BT_FIND(hDBList,Value2,BT_FIRST,BT_EQ,(LPSTR)&ClassNo))
                	BT_PUT (hDBList,Value2,(LPSTR)&ClassZero); 
        }
	}
	while (!SQLPtr->st);
	GlobalUnlock (SQLPtr->OFHandle); 
    GlobalUnlock (hDB);
    GSSiSetCursor (hcurSave);   
    GSSiGlobUlFree (&hMem);

{
#if ENABLETRACE
GSSiExitProg (612);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetFieldIDsFromNames(LPSTR DBName, LPHANDLE phFieldIDs, LPHANDLE phFieldTypes, LPSTR FieldListIN, LPHANDLE phValues)
{   
	//gets sequential field number from field name list separated by ;
	  
	LPSTR		pSC, pPar;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr=0;
    LPFIELDINFO lpFieldInfo;
    LPGWFLDINFO pFieldTypes=0;    
    BOOL		rtn=FALSE;    
    LPINT		pFieldID,pNumFields;
    short		ifield;  
    HANDLE		hDB=0;
	long		lMem=strlen(FieldListIN)+1;
	HANDLE		hMem;
	LPSTR		FieldList;
	int			numFieldTypes = 0;
	
	*phFieldIDs = 0;
	if (!*FieldListIN)
		return TRUE;
    if (!OpenDataFile (DBName,"",BT_READ,&hDB))
    	return FALSE;
	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,lMem);
	FieldList = GlobalLock (hMem);
	strcpy (FieldList,FieldListIN);
    SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
    *phFieldIDs = GSSiGlobAlloc ( 243,GHND,USHRT_MAX);
    if (phFieldTypes)
    {
    	if (!*phFieldTypes)
    		*phFieldTypes = GSSiGlobAlloc ( 243,GHND,USHRT_MAX); 
    	pFieldTypes = (LPGWFLDINFO)GlobalLock (*phFieldTypes);
		while (*pFieldTypes->Name)
		{
			numFieldTypes++;
			pFieldTypes++;
		}
    }
    pFieldID = (LPINT)GlobalLock (*phFieldIDs);  
    pNumFields = pFieldID++;
    while (FieldList)
    {
		int addedField = 0;
		LPSTR pEq, pValue;
		LPHANDLE pHandle;
		int ln;

		if ((pSC = _fstrchr (FieldList,';')))
			*pSC = 0;
		if ((pPar = _fstrchr (FieldList,'(')))
		{
			*pPar++ = 0;
		    if (phFieldTypes)
		    {
				if (!GetFieldTypeAndLenFromChar (pPar,pFieldTypes,0)) 
					goto Exit; 
				_fstrcpy (pFieldTypes->Name,FieldList);
				if ((pEq = strchr(pPar, '=')))
				{
					pEq++;
					if (phValues)
					{
						int id = 1;
						if (!*phValues)
							*phValues = GSSiGlobAlloc(1791, GHND, USHRT_MAX);
						pHandle = GlobalLock(*phValues);
						while (*pHandle)
						{
							id++;
							pHandle++;
						}
						pFieldTypes->ValueID = id;
						ln = strlen(pEq);
						*pHandle = GSSiGlobAlloc(1792, GMEM_MOVEABLE, ln + 1);
						pValue = GlobalLock(*pHandle);
						strcpy(pValue, pEq);
						GlobalUnlock(*pHandle);
						GlobalUnlock(*phValues);
					}
				}
				pFieldTypes++;
				numFieldTypes++;
				addedField = numFieldTypes;
			}
		}
	    lpFieldInfo = &FilePtr->FldInfo;   
	    for (ifield=0;ifield<FilePtr->NumFields;ifield++,lpFieldInfo++)    
	    {
	    	if (!_fstricmp (lpFieldInfo->name,FieldList))
	    	{
	    		*pFieldID++ = ifield;  
	    		(*pNumFields)++;
	    		goto Next;
	    	}
	    	if (!_fstricmp ("#",FieldList))
	    	{
	    		*pFieldID++ = -1;  
	    		(*pNumFields)++;
	    		goto Next;
	    	}
	    }
		if (addedField)
		{
			*pFieldID++ = -(1+addedField);
			(*pNumFields)++;
		}
		else
			goto Exit;
Next:
	    if (pSC)
	    	*pSC++ = ';';
	    FieldList = pSC;
    }
    rtn = TRUE;
Exit:
	GlobalUnlock (hDB);
	GlobalUnlock (SQLPtr->OFHandle);
    CloseDataFile (TRUE, &hDB);
    GlobalUnlock (*phFieldIDs);
	if (phFieldTypes && *phFieldTypes)
		GlobalUnlock (*phFieldTypes);
    if (!rtn) 
    {
    	GSSiGlobFree (phFieldIDs);
		if (phFieldTypes)
			GSSiGlobFree (phFieldTypes); 
	}
	GSSiGlobUlFree (&hMem);
	return rtn;
}

BOOL InFieldIDList (short FieldID,HANDLE hFieldList)
{
	LPINT pFieldID, pNumFields;
	BOOL	rtn=FALSE;  
	short	i;
	
	if (!hFieldList)
		return FALSE;
	pNumFields = (LPINT)GlobalLock (hFieldList);
	pFieldID = pNumFields + 1;
	for (i=0;i<*pNumFields;i++)
		if (*pFieldID++ == FieldID)
		{
			rtn = TRUE;
			goto Exit;
		}
Exit:
	GlobalUnlock (hFieldList);
	return rtn;
}

void SetStartupGlobalValues (void)
#if ENABLETRACE
{GSSiEnterProg (605);
#endif
{   
	char	str[256];
	
	
	SetCurVal ("@[%DL]gpsfiles\\default.gps",IDS_FILEGPS);
	SetCurVal ("@[%DL]formats\\",IDS_FILEFMT);
	SetCurVal ("@[%DL]menus\\",IDS_FILEMEN);
	SetCurVal ("@[%DL]fundir\\",IDS_FUNDIR);
	SetCurVal ("@[%DL]themes\\",IDS_FILETHM);
	SetCurVal ("@[%DL]reports\\",IDS_FILERPT);
	SetCurVal ("@[%DL]maplib\\",IDS_FILEPLT);
	SetCurVal ("@[%DL]attribut\\",IDS_FILEGMD);
	SetCurVal ("@[%DL]vislists\\",IDS_FILEVIS);
	SetCurVal ("@[%DL]piklists\\",IDS_FILEPIK);
	SetCurVal ("@[%DL]other.txt",IDS_FILEZML);
	SetCurVal ("@[%DL]zoomtype.txt",IDS_FILEZMT);
	SetCurVal ("@[%DL]vpoffset.txt",IDS_FILEVPOFF);
	SetCurVal ("@[%DL]offline.txt",IDS_FILEOFFLINE);
	SetCurVal ("@[%DL]daterang.txt",IDS_FILEDATERANGES);
	SetGlobalValue("%START_DIR",CurDir);
	SetGlobalValue("%DATA_LOC",""); 
	SetCurVal ("@[%DL]",IDS_FILEVPEDIT);
	SetGlobalValue("%SUBDL","Y");
	SetGlobalValue("%PROMPTS","N");
	SetGlobalValue("%IDM","Y");  
	SetGlobalValue("%ALT_PROJECTION","");  
	SetGlobalValue("%ORTHO_BUFFERS","1");
	SetGlobalValue("%ORTHORES","");
	SetGlobalValue("%BASE_UNITS","");    
	SetGlobalValue("%TEXT_EDITOR","NOTEPAD.EXE"); 
	SetGlobalValue("%NEW_POINT_SIZE","5P");
	SetGlobalValue("%NEW_POINT_ROT","0D");
	SetGlobalValue("%NEW_POINT_COLOR","-1");
	SetGlobalValue("%NEW_LINE_COLOR","-1");
	SetGlobalValue("%NEW_AREA_COLOR","-1");
	SetGlobalValue("%NEW_LINE_WIDTH","1P");
	SetGlobalValue("%NEW_LINE_SYM","");
	SetGlobalValue("%NEW_AREA_SYM","");
	SetGlobalValue("%NEW_POINT_SYM","");   
	SetGlobalValue("%CLOSEODBC","N"); 
	SetVarSaveStatus ("%CLOSEODBC",FALSE);
	SetGlobalValue("%ADDRANGEALL","N");  
	SetGlobalValue("%ADDTOL","0");  
	SetGlobalValue("%TRIGGER","3");  
	SetGlobalValue("%CURRENTVIDEOFILE","");
	SetGlobalValueLong ("%AUTO_ORTH_COLOR",RGB(255,0,0));     
	SetGlobalValue("%TIGERFILE1EXT",".rt1");   
	SetGlobalValue("%ConStatFile","constat");
	SetGlobalValue("%PICKAP","4");   
	

{
#if ENABLETRACE
GSSiExitProg (605);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void CloseBufferedMacros (void)
{   
	int	i;
	
	for (i=0;i<NumBufferedMacros;i++)
		GSSiGlobFree (&hBufferedMacros[i]);
	NumBufferedMacros = 0;
	return;
}

BOOL ProcessMacroFile (LPSTR Name,LPSTR RtnVal,LPHANDLE phArgs,short NumArgs)
#if ENABLETRACE
{GSSiEnterProg (608);
#endif
{   
	HANDLE	hSTR=GSSiGlobAlloc ( 244,GMEM_MOVEABLE,4096+USHRT_MAX);
	LPSTR	str = GlobalLock (hSTR);
    LPSTR	RtnValue=&str[4096];   
	HANDLE	hMacro = 0, hTemp=0;
	LPSTR	pMacro, pTemp;
	double	Off; 
	long	Offset, lMacro=0;
	BOOL	rtn=TRUE, AtEnd=FALSE;
    extern	HANDLE	hPIDAddDB,hPIDPid;  
    FIELDINFO Field; 
    int		status;     
    LPSTR	SaveRtnPointer, pEndCmd;
   	LPSTR	pStr; 
   	BOOL	SaveIGFC =InGRFCmd;
	HANDLE	hCmd=0;
	LPSTR	pCmd;
	int	i, OldestID; 
	BOOL	DeleteMacro=TRUE;   
	ULONG	Oldest; 
	LPSTR	Args[MAX_MACRO_ARGS];
	int		macroID;
	HANDLE	hBreakPoints = 0;
	LPBREAKPOINT	pBreakPoints = 0;
	int		lm=0;
	int		atLoc;
	BOOL	loadOnly = FALSE;

	if (editMacro)
	{
		sprintf(str, "$SESSION(CREATE, GMEdit /GMEdit %s)", Name);
		ProcessText(str);
		GSSiGlobUlFree(&hSTR);
		return TRUE;
	}
	if (NumArgs < 0)
	{
		loadOnly = TRUE;
		NumArgs = 0;
	}
    InGRFCmd = FALSE; 
    macroID = AddToMacroStack (1,++CurrentMacro,Name,phArgs,NumArgs);
	MacroVarSpace[CurrentMacro] = CreateVarSpace(VARSPACE_LOCAL);
	SetVarSpace(VARSPACE_LOCAL, MacroVarSpace[CurrentMacro]);
	if (RtnVal)
    	*RtnVal = 0;
    SetGlobalValueLong ("%NUMARGS",NumArgs);
    if (*Name)
    {                                               
    	HFILE	Fid;
		BOOL	saveDoPaint = DoPaint();
    	
		//SetWindowText(hWndMain, Name);
		//Sleep(5000);
	    if (!_fstricmp (Name,"INLINE"))
	    {
			hMacro = GSSiGlobAlloc ( 245,GMEM_MOVEABLE,USHRT_MAX);
	    	pMacro = GlobalLock (hMacro);
	    	*pMacro = 0;
	    	if (phArgs)
	    	{   
	    		HANDLE hNewArgs;
	    		LPSTR	pNewArgs;
	    		long	l=GlobalSize (*phArgs);
	    		
	    		pStr = GlobalLock (*phArgs);
	    		_fstrcpy (pMacro,pStr);
	    		pStr += 4096; 
	    		l -= 4096;
	    		hNewArgs=GSSiGlobAlloc ( 246,GMEM_MOVEABLE,l);
				pNewArgs = GlobalLock (hNewArgs);
                _fmemmove (pNewArgs,pStr,(size_t)l);
	    		GSSiGlobUlFree (phArgs);
	    		GlobalUnlock (hNewArgs);
	    		*phArgs = hNewArgs;
	    	}
	    }
	    else if (!_fstricmp (Name,"USER"))
	    {
			LPSTR	pArg; 
			ULONG	iarg;
			
			rtn = FALSE;
			if (!LoadUserLib (FALSE))
				goto Exit;    
			pArg = GlobalLock (*phArgs);
			NumArgs--;
			for (iarg=0;iarg<NumArgs;iarg++)
				Args[iarg] = pArg + 4096 * (iarg+1);
			rtn = RunUserFunction (pArg,RtnVal,NumArgs,Args);
       		GlobalUnlock (*phArgs);     
       		goto Exit;
        }
    	else
    	{   
    		_fstrcpy (str,Name);
    		ExpandText (str);
	        
	        Oldest = ULONG_MAX;
	        DeleteMacro = FALSE;
			for (i=0;i<NumBufferedMacros;i++)   
			{
				if (!_fstricmp (BufferedMacroNames[i],str))   
				{
					hMacro = hBufferedMacros[i];
			    	pMacro = GlobalLock (hMacro); 
			    	LastMacroUse[i] = CurrentMacroTime++;
			    	goto ProcessMacro;
			    } 
			    if (LastMacroUse[i] < Oldest)
			    {
			    	OldestID = i;
			    	Oldest = LastMacroUse[i];
			    }
			} 
	    	Fid = GSSiOpenFile (Name,NULL,OF_READ);
	    	if (Fid == HFILE_ERROR)
	    	{
	    		rtn = FALSE;
	    		goto Exit; 
	    	}
			lm = GSSifilelength(Fid);
			if (NumBufferedMacros == MAX_BUFFERED_MACROS)
			{
				i = OldestID;
				GSSiGlobFree (&hBufferedMacros[i]);
				hMacro = GSSiGlobAlloc(245, GMEM_MOVEABLE, lm + 1);
				hBufferedMacros[i] = hMacro;
			}
			else 
			{
				hMacro = GSSiGlobAlloc ( 245,GMEM_MOVEABLE,lm+1); 
				if (!GetDebug())
				{
					i = NumBufferedMacros++;
					hBufferedMacros[i] = hMacro;
				}
				else
				{
					DeleteMacro = TRUE;
					hBreakPoints = GSSiGlobAlloc(2451, GHND, sizeof(BREAKPOINT)*(lm + 1));
					pBreakPoints = (LPBREAKPOINT)GlobalLock(hBreakPoints);
					setMacroBrkPtHandle(macroID, hBreakPoints,lm);
				}
			} 
			if (!GetDebug())
			{
				_fstrcpy(BufferedMacroNames[i], str);
				LastMacroUse[i] = CurrentMacroTime++;
			}
	    	pMacro = GlobalLock (hMacro);
    		*pMacro = 0;
	    	while (fgetstring (str,4090,Fid))
	    		if (*str != '#')
	    		{   
	    			Truncate (str);
	    			pStr = FirstNonBlank(str);
					if (pBreakPoints)
						pBreakPoints[lMacro].beginLine = 1;
	    			lMacro += _fstrlen (pStr);   
	    			if (lMacro > USHRT_MAX)
	    			{
	    				GSSiClose2 (&Fid);  
	    				GSSiMessageBox (0,Name,"Macro file exceeds maximum length",MB_ICONEXCLAMATION,0); 
	    				rtn = FALSE;
	    				goto Exit;
	    			}
	    			_fstrcat (pMacro,pStr);   
	    		}
	    	GSSiClose2 (&Fid); 
	    }
ProcessMacro:
		lMacro = strlen (pMacro);
		hTemp = GSSiGlobAlloc (1825,GMEM_MOVEABLE,lMacro+1);
		pTemp = GlobalLock (hTemp);
		strcpy (pTemp,pMacro);
		pMacro = pTemp;
		GlobalUnlock (hMacro);
    	SaveRtnPointer = pMacroReturnValue;
    	pMacroReturnValue = RtnValue;
    	*pMacroReturnValue = 0;
		hCmd = GSSiGlobAlloc ( 247,GMEM_MOVEABLE,USHRT_MAX);
		pCmd = GlobalLock (hCmd);     
		if (!*pMacro)
	   		rtn = FALSE;
    	while (!loadOnly && *pMacro)
    	{   
			HANDLE SaveMacArgs;
			
			//if (pBreakPoints) breakAtPos(pMacro - pTemp, pBreakPoints, 0, lm);
			pEndCmd = MatchLev(pMacro, ';');
    		if (!pEndCmd)
    		{
    			AtEnd = TRUE;
    			pEndCmd = _fstrchr (pMacro,0);
    		}
    		else
    			*pEndCmd = 0;
    		_fstrcpy (pCmd,pMacro); 
			atLoc = (int)(pMacro - pTemp);
    		pMacro = pEndCmd;
    		if (!AtEnd)
    			*pMacro++ = ';';
    		AtEnd = FALSE; 
    		SaveMacArgs = hMacArgs;
    		if (phArgs)
    			hMacArgs = *phArgs;
    		else
    			hMacArgs = 0;
			if (pBreakPoints)
				ExpandTextDB(pCmd, pBreakPoints,atLoc, lm);
			else
				ExpandText(pCmd);
    		hMacArgs = SaveMacArgs;
		    SetGlobalValueLong ("%NUMARGS",NumArgs);
	    	if (!ContinueProcessing)
	    	{
	    		rtn = FALSE;
	    		if (pMacroReturnValue)
	    		{   
	    			rtn = atob (pMacroReturnValue);
	    			if (RtnVal)        
	    				_fstrcpy (RtnVal,pMacroReturnValue);
	    			SetContinueProcessing ( TRUE);
	    		}
	    		*pMacro = 0; 
	    	}
	    }
    	pMacroReturnValue = SaveRtnPointer;
		if (saveDoPaint)
			setDoPaint( saveDoPaint);
    }
Exit:
	GSSiGlobUlFree (&hTemp);
   	GSSiGlobUlFree (&hCmd); 
    GSSiGlobUlFree (&hSTR); 
	GSSiGlobUlFree(&hBreakPoints);
    if (DeleteMacro)
    	GSSiGlobFree (&hMacro);  
    //CloseMacroFiles (ThisMacro);   
    InGRFCmd = InGRFCmd;
	DestroyStatusWindow (CurrentMacro);
	DestroyVarSpace(MacroVarSpace[CurrentMacro]);
    CurrentMacro--;
	SetVarSpace(VARSPACE_LOCAL, MacroVarSpace[CurrentMacro]);
	RemoveFromMacroStack (macroID);
{
#if ENABLETRACE
GSSiExitProg (608);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
} 


void ExpandGlobalRaw (LPSTR Name,BOOL AddEscapes,LPSTR Value, short maxlen)
#if ENABLETRACE
{GSSiEnterProg (602);
#endif
{
	VARPNT	VP;
	HANDLE	handle; 
	
	if ((handle = FindVar (Name)))
	{
		VP = (VARPNT)GlobalLock (handle); 
		_fstrcpy (Value,VP->Value);
		if (VP->ContainsGorF && AddEscapes)
		{
			REPLAC (Value,"[","@[",maxlen);
			REPLAC (Value,"$","@$",maxlen); 
		}
		GlobalUnlock (handle);
	} 
		else *Value=0;
{
#if ENABLETRACE
GSSiExitProg (602);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

long ConvertPoint (LPSTR CvtFile,LPDPOINT Point,int Direction)
#if ENABLETRACE
{GSSiEnterProg (597);
#endif
{
	 LPSTR	lpDot, lpEnd;
	 short	SaveUnits;
	 long	rtn=0;
	 HANDLE hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,1024);
	 LPSTR	SaveProj=GlobalLock (hMem);
	 LPSTR	project=SaveProj+512;
	 BOOL	ProjChanged = FALSE, Err;
		
	 if (!strnicmp (CvtFile,"SCREENTOBASE",12))
	 {
		 LPVIEWPORT	SaveCurView = CurView;
		 char	vpName[64];

		 if (CvtFile[12] == '(')
		 {
			 LPSTR	pEnd;

			 strcpy (vpName,&CvtFile[13]);
			 if ((pEnd = strrchr (vpName,')')))
				 *pEnd = 0;
			 SetCurView ( SetVPFromName (vpName,&Err));
		 }
	 	 *Point = ScreenPtDToBasePt (*Point);
		 CurView = SaveCurView;
		 goto Exit;
	 }
	 if (!strnicmp (CvtFile,"BASETOVP",8))
	 {
		 LPVIEWPORT	SaveCurView = CurView;
		 char	vpName[64];

		 if (CvtFile[8] == '(')
		 {
			 LPSTR	pEnd;

			 strcpy (vpName,&CvtFile[9]);
			 if ((pEnd = strrchr (vpName,')')))
				 *pEnd = 0;
			 SetCurView ( SetVPFromName (vpName,&Err));
		 }
	 	 *Point = BasePtToWinPtD (Point);
		 Point->x -= CurView->DrawRect.left;
		 Point->y -= CurView->DrawRect.top;
		 CurView = SaveCurView;
		 goto Exit;
	 }
	 if (!stricmp(CvtFile, "LATLON"))
	 {
		 if (Direction == 1)
			 rtn = ConvertCoord(Point, 2, 1);
		 else
			 rtn = ConvertCoord(Point, 1, 2);

		 goto Exit;
	 }

	 if (!stricmp(CvtFile, "GOOGLE" ))
	 {
		 if (Direction == 1)
			 rtn = ConvertCoord(Point, GOOGLEMAPSPROJECTION, 1);
		 else
			 rtn = ConvertCoord(Point, 1, GOOGLEMAPSPROJECTION);

		 goto Exit;
	 }
	 if (!strnicmp(CvtFile, "GOOGLE", 6))
	 {
		 int iLev = atoi(&CvtFile[6]);
		 if (iLev > 0 && iLev < 22)
		 {
			 double pixelX, pixelY;

			 if (Direction == 1)
			 {
				 pixelX = Point->x;
				 pixelY = Point->y;
				 PixelXYToLatLong(pixelX, pixelY, iLev, &Point->y, &Point->x);
				 rtn = ConvertCoord(Point, 2, 1);
			 }
			 else
			 {
				 rtn = ConvertCoord(Point, 1, 2);
				 LatLongToPixelXYd(Point->y, Point->x, iLev, &pixelX, &pixelY);
				 Point->x = pixelX;
				 Point->y = pixelY;
			 }
		 }
		 goto Exit;
	 }

	 SaveUnits = PRJ_UNITS[3];
	 _fstrcpy (SaveProj,"[%ALT_PROJECTION]");
	 ExpandText (SaveProj); 
	 if (stricmp (SaveProj,CvtFile))
	 {
		 ProjChanged = TRUE;
		 _fstrcpy (project,CvtFile);
		 ExpandText(project);
		 lpDot = strrchr(project, '.');
		 if (lpDot)
		 {
			 if (stricmp (lpDot,".cvt"))
				*lpDot = 0;//not sure what this is for
		 }
		 SetGlobalValue("%ALT_PROJECTION",project);
		 ConvertCoordClose ();
		 if ((rtn=ConvertCoordInit()))
	 		goto Exit;
	 }
/*     if (*str)
     { 
         if (!_fstrcmp(str,"Feet"))
            PRJ_UNITS[3] = 1;
         else if (!_fstrcmp(str,"Meters"))
            PRJ_UNITS[3] = 2; 
     } */ 
     if (Direction==1)
	 	rtn=ConvertCoord (Point,3,1);  
	 else
	 	rtn=ConvertCoord (Point,1,3); 
Exit: 
	 if (ProjChanged)
	 {
		 PRJ_UNITS[3] = SaveUnits;
		 SetGlobalValue ("%ALT_PROJECTION",SaveProj);
		 ConvertCoordClose ();
	 }
     GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (597);
#endif
     return rtn;
}
#if ENABLETRACE
}
#endif
}
BOOL GetCurVal (LPSTR Val,short maxval, UINT StringID)
#if ENABLETRACE
{GSSiEnterProg (599);
#endif
{   
	char	str[MAX_PATH]="[";
	
	*Val=0;
	if (!StringID)
{
#if ENABLETRACE
GSSiExitProg (599);
#endif
		return FALSE; 
}
	if (!LoadString(hInst, StringID, &str[1], 127))
{
#if ENABLETRACE
GSSiExitProg (599);
#endif
		return FALSE;
}
	_fstrcat (str,"]");	
	ExpandText (str); 
	_fstrcpy (Val,str);
	if (*Val)
{
#if ENABLETRACE
GSSiExitProg (599);
#endif
		return TRUE;
}
	else
{
#if ENABLETRACE
GSSiExitProg (599);
#endif
		return FALSE;
}
#if ENABLETRACE
}
#endif
} 

void SetCurVal (LPSTR Val,UINT StringID)
#if ENABLETRACE
{GSSiEnterProg (600);
#endif
{   
	char	str[MAX_PATH];
	
	if (!StringID)
{
#if ENABLETRACE
GSSiExitProg (600);
#endif
		return;
}
	if (!LoadString(hInst, StringID, str, MAX_PATH))
{
#if ENABLETRACE
GSSiExitProg (600);
#endif
		return;
}
	SetGlobalValue (str,Val);
{
#if ENABLETRACE
GSSiExitProg (600);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}


BOOL GetFieldTypeAndLenFromChar (LPSTR str,LPGWFLDINFO pGWFldInfo,LPHANDLE phSetClause)
{   
	LPSTR	pSetClause, pPar;
	
	switch (*str)
	{
 		case 'C':
			pGWFldInfo->Type = BT_CHAR;
		break;
		case 'B':
			pGWFldInfo->Type = BT_INTEGER; 
		break;
		case 'R':
			pGWFldInfo->Type = BT_REAL;
		break;
		case 'J':
			pGWFldInfo->Type = BT_RIGHT_CHAR;
		break;
		default: 
			return FALSE;
	} 
	str++;
	pGWFldInfo->Len = atoi(str);   
	if (phSetClause && (pPar = _fstrchr (str,')')))
	{
		if (!_fstrncmp (pPar,")=",2))
		{
			LPSTR pEnd;
			pPar+=2;
			pEnd = MatchLev(pPar, ',');
			if (pEnd)
				*pEnd = 0;
			*phSetClause = GSSiGlobAlloc (0,GMEM_MOVEABLE,1024);
			pSetClause = GlobalLock (*phSetClause);
			_fstrcpy (pSetClause,pPar);
			GlobalUnlock (*phSetClause);
			if (pEnd)
				*pEnd = ',';
		}
	}
			
	return TRUE;
}

           

                                       
