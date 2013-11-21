#include "graphint.h"
#include "extrndb.h"
#include "std.h"  

#include "gmextern.h"
 
static BOOL	ObeyOneWay=TRUE;
static short	OneWayBothDir=0;
static short	OneWayBeginToEnd=1;
static short	OneWayEndToBegin=-1;



COLORREF GetRoadColor (int n)
{   
	char name[32];
	COLORREF color, defaultcolor[8]={RGB(255,0,0),RGB(255,255,0),RGB(0,0,255),RGB(0,255,0),RGB(255,0,255),RGB(0,255,255)};
	
	sprintf (name,"[%%ADDEDITROADCOLOR%i]",n%6+1);
 	color = GetGlobalLVal2(name,defaultcolor[n%6]);
 	return color; 
}



long GetIntADT (long IntID,LPSHORT pNumStreets)
{
	long	ADT=0;
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData;
    SEGDATAGM	Segdata; 
    long	Offset;
    
    *pNumStreets = 0;
    NetIntRefKey.IntID = IntID;
    NetIntRefKey.Refno = LONG_MIN;
	if (!BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_FIRST,BT_GE,(LPSTR)&NetIntRefData))
	while (NetIntRefKey.IntID == IntID)
	{   
		(*pNumStreets)++;
		if (GetSegDataGM (NetIntRefKey.Refno,&Segdata)) 
		{
			if (!Segdata.OneWay)
				ADT += Segdata.TrafVol/2;
			else if (Segdata.OneWay == NetIntRefData.WhichEnd)
				ADT += Segdata.TrafVol;
		}
		if (BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_NEXT,BT_ANY,(LPSTR)&NetIntRefData))
			NetIntRefKey.IntID = IntID-1;
	}
	
	return ADT;
}

double ComputeCriticalAccidentRate (double k,double Ra, long ADT)
{
	double Rc, M;
	
	M = ((double)ADT * 365)/1000000;
	Rc = Ra + k * sqrt(Ra/M) + 1.0/(2 * M);
	return Rc;
}

BOOL CreateINT_ACCIDTable (LPSTR File)
{
 	int		i;
	BTVARDESC BTVar[2], *pVars;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	long	TotFileLen;
	GWDHEADER16 GWDHead; 
	LPGWDHEADER16	lpGWDHead;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo, lpCTField, lpMCDField, lpMAField;
	HANDLE hBT, hVars, hDB, hBlock, hFldInLen,hBTOld, hDBma, hDBct;
	int	FidOld;
	time_t ltime;
	int		FidData;
	int		ibeg,NumVars;
	OFSTRUCT	OFStruct;
	LPVOID	lpVal;
	LPSTR	pName;
	GWFLDINFO FldInfo;
	char	FileIn1[128]; 
	LPSTR	lpDot;  

	 _fstrcpy (FileIn1,File);
	 lpDot = _fstrrchr (FileIn1,'.');
	 if (lpDot)
	 	*lpDot = 0;
	 _fstrcat (FileIn1,".in1");	
	 lpGWDHead = &GWDHead; 
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER16));

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	ibeg = 0;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"IntersectionID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"NumStreets");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Accidents");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"ADT");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"AccidentRate");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"AveAccidentRate");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"AccidentFactor90");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"AccidentFactor95");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"AccidentFactor995");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"AccidentFactor999");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"AccidentLevel");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = time(NULL);
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
 	 GSSillseek (FidData,0,2);
			     
	 NumVars = 1;
			
	 hVars = LocalAlloc (LHND,NumVars * sizeof(BTVARDESC));
     pVars = (LPBTVARDESC)LocalLock(hVars);
			
	 pVars->BT_VARLEN=4;
	 pVars->BT_VARTYP=BT_INTEGER;
	 pVars->BT_VAROFF=0;
	 BT_CREATE (FileIn1, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
     LocalUnlock(hVars);
     LocalFree(hVars);
 	 GSSiClose (FidData);

     hDB = OpenGWDatabase (File,BT_WRITE);
     if (!hDB) return (FALSE);
     CloseGWDatabase (hDB); 
     return TRUE;


} 

 

double GetSegmentSpeed (LPSEGDATAGM pSegdata,double AtCost)
{   
	double	speed = MaxSpeed;
	 
	if (ShortestRoute)
		return 1;
	speed = pSegdata->Speed; 
	if (!speed)
		speed = 25;    
	if (speed == 35)
		speed = 25;
	else if (speed >= 55)
		speed += 10;
	return speed;
}

double GetIntersectionCost (LPSEGDATAGM pSegdataAt,LPSEGDATAGM pSegdataNext,BOOL ReverseNextSeg,int NumPicked)
/*{   
	double	cost=0;
	 
	if (ShortestRoute)
		return 0;
	if (pSegdataAt->Speed > pSegdataNext->Speed)
		cost = 0.25;
	else if (pSegdataAt->StreetNum[0] != pSegdataNext->StreetNum[0])  
		cost = 0.1;
	return cost;
}*/
{   
	double	cost=0, AtSpeed, Speed;
	SEGDATAGM	SegdataInt;
	
	if (pSegdataNext->OneWay != OneWayBothDir && ObeyOneWay)
	{
		if (pSegdataNext->OneWay == OneWayBeginToEnd*InvertNetDir && ReverseNextSeg ||
			pSegdataNext->OneWay == OneWayEndToBegin*InvertNetDir && !ReverseNextSeg)
		return MAX_COST;
	}
	if (ShortestRoute)
		return 0;
	if (pSegdataAt->Speed < pSegdataNext->Speed)
		cost = 0.05;
//	else if (pSegdataAt->Speed > pSegdataNext->Speed && pSegdataNext->Speed <= MinSpeed)
//		cost = MAX_COST;
	else if (pSegdataAt->StreetNum[0] != pSegdataNext->StreetNum[0])  
		cost = 0.15;
	else if (NumPicked > 1)
	{   
// if all speeds at this int are same add a cost, if any speeds are higher than the one we are
// on add a slightly higher cost 
		if (!_fstrncmp(pSegdataAt->CFCC,"A1",2))
		{
			cost = 0;
			goto Exit;
		}
		AtSpeed = GetSegmentSpeed (pSegdataAt,0);
		while (NumPicked--)
		{   
			if (PickList[NumPicked].Refno != pSegdataAt->TLID)
			{
		   		if (GetSegDataGM (PickList[NumPicked].Refno,&SegdataInt))
		   		{ 
		   			Speed = GetSegmentSpeed (&SegdataInt,0);
		   			if (Speed > AtSpeed)
		   			{
		   				cost = 0.2;
		   				goto Exit;
		   			}
		   			else if (Speed < AtSpeed)
		   			{
		   				cost = 0.1;
		   				goto Exit;
		   			}
		   		}
			}
		}
		cost = 0.35; 
	}
	else
		cost = 0; 
Exit:
	return cost;
} 

double GetTurnCost (long FromRef,short FromEnd,long ToRef,short ToEnd)
{ 
	
	TURNKEY	TurnKey;
	double	TurnCost=0, Cost;
   	BOOL	Opened, rtn=FALSE;

	if (!OpenTurnTable (FALSE,&Opened))
		return 0;
	
	TurnKey.FromRef = FromRef;
	TurnKey.FromEnd = FromEnd;
	TurnKey.ToRef = ToRef;
	TurnKey.ToEnd = ToEnd;
	TurnKey.Time = 0;
	
	if (BT_FIND (hBTTurnTable,(LPSTR)&TurnKey,BT_FIRST,BT_EQ,(LPSTR)&Cost)) 
		goto Exit;
	TurnCost = Cost;	
Exit:
	CloseTurnTable (Opened);
	return TurnCost;
}

BOOL GetSegDataGM (long TLID,LPSEGDATAGM pSegdata)
{   
	long	Offset; 
	BOOL	rtn=FALSE;  
	LPGWDHEADER lpGWDHead;
	LPSEGDATAGM pSegdataDB;  
	
	if (GetSegFromGlobals)
	{   
		LPSTR	str;
		HANDLE	hStr;  
		PICKDATA	SavePickData;
		short	SaveNumPicked=NumPicked;
		
		SavePickData = PickList[0];
		_fmemset (pSegdata,0,sizeof(SEGDATAGM));
		rtn = PickByRefno(TLID,NULL,NULL,-1); 
		NumPicked = SaveNumPicked;
		PickList[0] = SavePickData;
		if (!rtn)
			return FALSE; 
		hStr = GSSiGlobAlloc ( 568,GMEM_MOVEABLE,1024);
		str = GlobalLock (hStr);
		_fstrcpy (str,"[%ONEWAYVAL]");
		ExpandText (str);
		pSegdata->OneWay = atoi (str);
		_fstrcpy (str,"[%SPEEDVAL]");
		ExpandText (str);
		pSegdata->Speed = atoi (str);
		GSSiGlobUlFree (&hStr); 
		return TRUE;
	}
	if (!hDBStreetSegments)
		return FALSE;
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
	pSegdataDB = (LPSEGDATAGM)&lpGWDHead->GWDData;
    if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&TLID,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    {
		FillGWDData (lpGWDHead,Offset);
		*pSegdata = *pSegdataDB;
		rtn = TRUE;
	} 
 	GlobalUnlock (hDBStreetSegments);
 	return rtn;
}

/*BOOL AddSpeed (void)
{   
	long	Offset, TLID; 
	BOOL	rtn=FALSE, OpenedSeg=FALSE;  
	LPGWDHEADER lpGWDHead;
	LPSEGDATAGM pSegdataDB; 
	short	pos=BT_FIRST;
	short	len;
	time_t	systime;     
	
	time (&systime);
	
	OpenStreetSegmentTable (TRUE,&OpenedSeg);
	if (!hDBStreetSegments)
		return FALSE;
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
	pSegdataDB = (LPSEGDATAGM)&lpGWDHead->GWDData;
    while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&TLID,pos,BT_ANY,(LPSTR)&Offset))
    {   
    	pos = BT_NEXT;
		len = FillGWDData (lpGWDHead,Offset);
		pSegdataDB->Speed = IDNINT(GetSegmentSpeed (pSegdataDB,0));  
		pSegdataDB->UpdateTime = systime;
		
		GSSillseek (lpGWDHead->Fid,Offset,0);
		BigWrite (lpGWDHead->Fid,(HPSTR)&len,2,-1);
		BigWrite (lpGWDHead->Fid,(HPSTR)pSegdataDB,len,-1);
	    SetGWDCurrentOffset (lpGWDHead,-1);
	} 
 	GlobalUnlock (hDBStreetSegments);
	CloseStreetSegmentTable (OpenedSeg);
 	return rtn;
}  */

BOOL UnloadStreets (HWND hWnd)
{   
	char	Name[128], TrueName[66], str[256]; 
	OFSTRUCT	OFStruct; 
	long	StreetNum;
	HFILE	Fid;
	BOOL	Opened;
	BOOL	UseSegs;

	
	HCURSOR	hcurSave;   

    SetAddressDir();

	CloseStreetNameTable();
    _fstrcpy (Name,AddMatchDir); 
    _fstrcat (Name, "\\snunload.txt");   
    if (ExistFile (Name)) 
    {   
    	sprintf (str,"File %s already exists - do you wish to overwrite it?",Name);
		if (GSSiMsgBox( hWnd,str,"Verify Overwrite", MB_OKCANCEL,0) == IDCANCEL) return FALSE;  
	}
	UseSegs = (GSSiMsgBox( hWnd,"Remove those not in segment table?","", MB_YESNO,0) == IDYES);
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));
    STNDSN_INIT(TRUE);
    
   OpenStreetSegmentTable (FALSE,&Opened);
   Fid = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
    while ((StreetNum = GetStreetNumFromName (STDNAMv,2,0,TrueName)))
    {  
		int nSegs=1;
		
		if (UseSegs)
			nSegs = GetNumStreetSegs (StreetNum,0,0,0,0,0,0,0);
    	sprintf (str,"%ld,\"%s\",%ld",StreetNum,TrueName,nSegs);  
    	if (nSegs)
			fputstring (str,Fid);
    }
    GSSiClose (Fid);
    CloseStreetSegmentTable(Opened); 
	CloseStreetNameTable();   
    GSSiSetCursor (hcurSave); 
	GSSiMsgBox (hWnd,"Unload Complete",Name,MB_OK,0);
	return TRUE;
}	


BOOL ReloadStreets (HWND hWnd)
{   
	char	Name[128], str[256]; 
	OFSTRUCT	OFStruct;
	HFILE	Fid;   
	LPSTR	lpComma, TrueName;  
	long	StreetNum;
	
	HCURSOR	hcurSave;   

	if (GSSiMsgBox( GetFocus(),"Be sure to have a backup of the ADDRESS subdirectory of your project before continuing",
			"Verify Rebuild", MB_OKCANCEL,0) == IDCANCEL) return FALSE;
    SetAddressDir();
    _fstrcpy (Name,AddMatchDir); 
    _fstrcat (Name, "\\snunload.txt");
    if (!ExistFile (Name))
    {   
    	sprintf (str,"Unload file %s not found",NULL,MB_ICONEXCLAMATION);
    	return FALSE;
    }
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));

	CloseStreetNameTable();
    CreateStreetNameTable (AddMatchDir);
    STNDSN_INIT(TRUE);
    
    Fid = GSSiOpenFile (Name,&OFStruct,OF_READ);  
    while (fgetstring (str,250,Fid))
    { 
    	if ((lpComma = _fstrchr (str,',')))
    	{   
    		*lpComma++=0;
    	    StreetNum = atol (str); 
    	    if (*lpComma == '"')
    	    	lpComma++;
    	    TrueName = lpComma;
	    	if ((lpComma = _fstrchr (TrueName,'"')))
    	    	*lpComma = 0;
			AddStreetName (TrueName,-StreetNum,"","","",""); 
		} 
	}
    
    GSSiClose (Fid);

	{   
		long	Offset, TLID; 
		BOOL	rtn=FALSE;  
		LPGWDHEADER lpGWDHead;
		LPSEGDATAGM pSegdataDB; 
		short	pos=BT_FIRST;
		short	len,i;
		BOOL	MissingStreets=FALSE, OpenedSeg=FALSE; 
		char	TName[64];   
		short	ii;
		
		TrueName = str;
		OpenStreetSegmentTable (TRUE,&OpenedSeg);
		if (hDBStreetSegments) 
		{
			lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
			pSegdataDB = (LPSEGDATAGM)&lpGWDHead->GWDData;
		    while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&TLID,pos,BT_ANY,(LPSTR)&Offset))
		    {   
		    	pos = BT_NEXT;
				len = FillGWDData (lpGWDHead,Offset);
				for (i=0;i<4;i++)
				{   
					if (pSegdataDB->StreetNum[i]==4586)
					    ii=1;
					if (!pSegdataDB->StreetNum[i])
						break; 
					if (!GetTrueStreetName (pSegdataDB->StreetNum[i], TrueName, -1,0))
						MissingStreets = TRUE;
					else
						pSegdataDB->StreetNum[i] = GetStreetNumFromName (TrueName,TRUENAM_INDEX,BT_FIRST,TName);  
				}
				GWDReplaceRecord (lpGWDHead,0,0,Offset);
/*				GSSillseek (lpGWDHead->Fid,Offset,0);
				BigWrite (lpGWDHead->Fid,(HPSTR)&len,2,-1);
				BigWrite (lpGWDHead->Fid,(HPSTR)pSegdataDB,len,-1);
			    SetGWDCurrentOffset (lpGWDHead,-1);*/
			} 
		 	GlobalUnlock (hDBStreetSegments);
			CloseStreetSegmentTable (OpenedSeg); 
		}
	}
	
	CloseStreetNameTable(); 
    GSSiSetCursor (hcurSave); 
	GSSiMsgBox (hWnd,"Reload Complete"," ",MB_OK,0);
	return TRUE;
}	


HANDLE CreateAddLocTable (LPSTR OutDBName,short UDIFieldLen, short AdditionalFieldLen,BOOL New)
{
    LPGWDHEADER16 lpGWDHead;
    char    str[MAX_PATH], PrimeIndex[MAX_PATH]; 
    HANDLE  hDB ,  hVars,   SaveHandle,LastHandle;
    FIELDINFO   KeyFieldDef;
    short    NumHouse,  FidData,ibeg,NumVars, ii,n=0;  
    static  short       NumFields, Reclen, len;
    long        StartHouse,EndHouse, Recno=0;
    GWDHEADER16 GWDHead; 
    HFILE   OutFid;
    OFSTRUCT    OFStruct;
    GWFLDINFO FldInfo;
    BOOL    First;
    LPSTR   lpDot, lpSpace,lpDash;
    BTVARDESC Vars[4] ;
    
    if (!New && ExistFile (OutDBName))
    	goto OpenIt; 
    lpGWDHead = &GWDHead; 
    
    FidData = GSSiOpenFile (OutDBName,&OFStruct,OF_CREATE);   
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER16));
    GWDHead.NumFields=0;
    GWDHead.NumIndex=3;
    GWDHead.Version=2;
    GWDHead.NumIndexFields[0]=4;
    GWDHead.IndexFields[0][0]=0;
    GWDHead.IndexFields[0][1]=1;
    GWDHead.IndexFields[0][2]=2;
    GWDHead.IndexFields[0][3]=3;
    GWDHead.NumIndexFields[1]=3;
    GWDHead.IndexFields[1][0]=2;
    GWDHead.IndexFields[1][1]=4;
    GWDHead.IndexFields[1][2]=1;
	GWDHead.lKeys[1]=12; 
    GWDHead.NumIndexFields[2]=3;
    GWDHead.IndexFields[2][0]=4;
    GWDHead.IndexFields[2][1]=1;
    GWDHead.IndexFields[2][2]=2;
	GWDHead.lKeys[2]=12; 
//    GWDHead.IndexFields[1][3]=3;
    BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
    ibeg = 0;

    FldInfo.Len = 32;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"%STREET_NAME");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"%HOUSE_NUM");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"%ZIPCode");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = UDIFieldLen;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"UDIValue");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;
    
    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"%STREET_NUM");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"%MUNIC_NUM");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"X");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"Y");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    if (AdditionalFieldLen)
    {
	    FldInfo.Len = AdditionalFieldLen;
	    FldInfo.Beg = ibeg;
	    ibeg += FldInfo.Len;
	    FldInfo.Type = BT_CHAR;
	    _fstrcpy (FldInfo.Name,"AdditionalValue");
	    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	    GWDHead.NumFields++;
	 }

     GWDHead.Reclen=ibeg; 
     GWDHead.TimeStamp = time(0);
     GSSillseek (FidData,0,0);
     BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
     GSSillseek (FidData,0,2);
            
     Vars[0].BT_VARLEN= 32;
     Vars[0].BT_VARTYP=BT_CHAR;
     Vars[0].BT_VAROFF=0;
     Vars[1].BT_VARLEN= 4;
     Vars[1].BT_VARTYP=BT_INTEGER;
     Vars[1].BT_VAROFF=32;
     Vars[2].BT_VARLEN= 4;
     Vars[2].BT_VARTYP=BT_INTEGER;
     Vars[2].BT_VAROFF=36;
     Vars[3].BT_VARLEN= UDIFieldLen;
     Vars[3].BT_VARTYP=BT_CHAR;
     Vars[3].BT_VAROFF=40;
     _fstrcpy (PrimeIndex,OutDBName); 
     lpDot = _fstrrchr (PrimeIndex,'.');
     *lpDot = 0;
     _fstrcat (PrimeIndex,".in1");
     BT_CREATE (PrimeIndex, 4, FALSE, 4, 1,Vars,FALSE, 0, GWDHead.TimeStamp, FALSE);
     GSSiClose (FidData);
     lpDot = _fstrrchr (PrimeIndex,'.');
     *lpDot = 0;
     _fstrcat (PrimeIndex,".in2");  
     GSSiRemove (PrimeIndex);
     *lpDot = 0;
     _fstrcat (PrimeIndex,".in3");  
     GSSiRemove (PrimeIndex);

OpenIt:                 
     hDB = OpenGWDatabase (OutDBName,BT_WRITE);
     
     return hDB;
} 




void UpdateSNamesFromHlt (void)
{

	LPGWDHEADER	lpGWDHead;
	SEGDATAGM	StreetTemplate;  
	short	pos=BT_FIRST, len;  
	HIGHLIGHTDATA	HighlightData;
	long	Offset, TLID;  
	BOOL	OpenedSeg;
			
	if (!OpenStreetSegmentTable (TRUE,&OpenedSeg))
		return;
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
	while (!BT_FIND (hHighlight,(LPSTR)&TLID,pos,BT_ANY,(LPSTR)&HighlightData))
	{
		pos = BT_NEXT;
        Offset = GSSillseek (lpGWDHead->Fid,0,2);
        BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&TLID,(LPSTR)&Offset);  
	  	len = sizeof(SEGDATAGM);
	   	BigWrite (lpGWDHead->Fid,(HPSTR)&len,2,-1);
		_fmemset (&StreetTemplate,0,sizeof(SEGDATAGM));
        PickList[0]=HighlightData.PD;  
	    ProcessPickedItem (0,FALSE);
		StreetTemplate.TLID = TLID;
		_fmemcpy (StreetTemplate.StreetNum,CurStreetNumbers,16);
		StreetTemplate.Speed=55;
	   	BigWrite (lpGWDHead->Fid,(HPSTR)&StreetTemplate,len,-1);
	} 
	GlobalUnlock (hDBStreetSegments);
    CloseStreetSegmentTable(OpenedSeg);  
	return;
}

BOOL MatchAddress (int MatchCode, int nList,HANDLE hList,long House, long Munic, long ZIP, BOOL BlockCenter,LPSHORT pNumMatch,LPHANDLE phMatch,BOOL UsePointBased,BOOL UseNetBased,HWND hWndDlg,int nControls,LPINT Controls)
{   
//	return TRUE means match found without changing munic or zip (changing from 0 does not count as changing)
	DPOINT	AddPoint; 
	short	st, n2, LocationCode;  
	LPADDMATCH	pMatch;
	LPLONG	pStreetNum;  
	long	StreetNum;  
	BOOL	ChangedMunic,ChangedZIP, rtn=FALSE;    
	BOOL	GetNext=TRUE;
	UINT	i;
	MSG     msg;
    
    pStreetNum = (LPLONG)GlobalLock (hList);
    while (GetNext && nList--)
    {   
    	if (MatchAddress2 (MatchCode,*pStreetNum,House,Munic,ZIP,BlockCenter,pNumMatch,phMatch,UsePointBased,UseNetBased))
   			rtn = TRUE;   
    	pStreetNum++; 
    	for (i=0;i<nControls;i++)
    	{
			if (GSSiPeekMessage(&msg,GetDlgItem(hWndDlg,Controls[i]),WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
				GetNext = FALSE;
		}	                                                             
    } 
    GlobalUnlock (hList);
	return rtn;
} 

short GetNumZIPsInMunic (long Munic,LPHANDLE phZIPsInMunic)
{
	char	File[MAX_PATH];  
	short	st, Dummy,n=0, pos=BT_FIRST,cond=BT_GT;
	MUNICZIPS	MunZIPs;   
	LPLONG	pZIP;
    
    if (!phZIPsInMunic)
    {   
    	BT_CLOSE (hBTNZINM);
    	hBTNZINM = 0;
    	return 0;
    }
	*phZIPsInMunic = 0;
	if (!hBTNZINM)
	{
	    SetAddressDir();
	    
	    sprintf (File,"%s\\munzips.btr",AddMatchDir);
		hBTNZINM = BT_OPEN (File,0,BT_READ,0);
	} 
    MunZIPs.Munic = Munic;  
    MunZIPs.ZIP = 0;
	while (!BT_FIND (hBTNZINM,(LPSTR)&MunZIPs,pos,cond,(LPSTR)&Dummy))
	{   
		pos = BT_NEXT;
		cond = BT_ANY;
		if (MunZIPs.Munic != Munic)
			return n;
		if (!n)
			*phZIPsInMunic = GSSiGlobAlloc ( 570,GMEM_MOVEABLE,1024);
		pZIP = (LPLONG)GlobalLock (*phZIPsInMunic);
		pZIP += n;
		*pZIP = MunZIPs.ZIP;
		n++;
		GlobalUnlock (*phZIPsInMunic); 
	}
	return n;
}


BOOL MatchAddress2 (int MatchCode,long StreetNum,long House,long Munic,long ZIP, BOOL BlockCenter,LPSHORT pNumMatch, LPHANDLE phMatch,BOOL UsePointBased,BOOL UseNetBased)
{
	SEGMAXKEY	SegMaxKey; 
	HANDLE		hZIPsInMunic;
	LPLONG		pZIPInMunic; 
	short		NumZIPsInMunic, stMax;
	BOOL		rtn=FALSE; 
	long		MinHouseNum, TestZIP;
	
	if (ZIP)
	{   
		if (MatchAddress3 (MatchCode,StreetNum,House,ZIP,ZIP,Munic,BlockCenter,pNumMatch,phMatch,UsePointBased,UseNetBased))
			return TRUE;
		if (MatchCode < 3)
			return FALSE;
	}                  
	if (Munic)
	{
		NumZIPsInMunic = GetNumZIPsInMunic (Munic,&hZIPsInMunic);
		if (hZIPsInMunic)
		{
			pZIPInMunic = (LPLONG)GlobalLock (hZIPsInMunic);
			while (NumZIPsInMunic--)
			{   
				if (*pZIPInMunic != ZIP)
				if ((rtn=MatchAddress3 (MatchCode,StreetNum,House,*pZIPInMunic,ZIP,0/*use 0 munic to match neigboring munic which share zipcode
					(i.e 55432 in Fridley and SLP - all match to street in SLP in 55432 even if FRIDLEY given as city */
					,BlockCenter,pNumMatch,phMatch,UsePointBased,UseNetBased)))
					break; 
				pZIPInMunic++;
			}
			GSSiGlobUlFree (&hZIPsInMunic);
			if (rtn)
				return TRUE;
			if (MatchCode < 3)
				return FALSE;
		}
	} 
	if (UsePointBased && hPIDAddDB)
	{
		if (MatchAddress3 (MatchCode,StreetNum,House,0,ZIP,Munic,BlockCenter,pNumMatch,phMatch,UsePointBased,UseNetBased))
			return TRUE;
	}
	SegMaxKey.StreetNum = StreetNum;     
	SegMaxKey.ZIPCode = -1;
	SegMaxKey.MaxHouseNum = 0;
	SegMaxKey.Segid = LONG_MIN; 
NextZIP:
	SegMaxKey.Side = 9;
	stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_FIRST,BT_GT,(LPSTR)&MinHouseNum);
	if (!stMax && SegMaxKey.StreetNum == StreetNum) 
	{ 
		if (MatchAddress3 (MatchCode,StreetNum,House,SegMaxKey.ZIPCode,ZIP,Munic,BlockCenter,pNumMatch,phMatch,UsePointBased,UseNetBased))
			rtn = TRUE;
		goto NextZIP;
	}
	return rtn;
}

BOOL MatchAddress3 (int MatchCode,long StreetNum,long House,long ZIP,long OrigZIP,long OrigMunic, BOOL BlockCenter,LPSHORT pNumMatch, LPHANDLE phMatch,BOOL UsePointBased,BOOL UseNetBased)
{
	SEGMAXKEY	SegMaxKey, GotSeg, PossibleSeg; 
	long		MinHouseNum, AddRangeOffDist, GotMinHouse, PossibleMinHouse; 
	short		stMax, n,index;
	LPADDMATCH	pMatch, pMatchTest;
    LPSEGDATAGM	pSegdata; 
	LPGWDHEADER	lpGWDHead;
	LPGWFLDINFO lpGWFldInfo;
    long		Offset, Dist1, Dist2, NearHouse, MinAdd, MaxAdd, Dir, LowAdd, HighAdd; 
    short		LocationCode=1;  
    BOOL		rtn=FALSE;
    double		PCT,AZ,Length,OffDist,AddressOffsetDist=15, LowPCT, HighPCT, FromPCT, ToPCT;
    DPOINT		CLPoint; 
    BOOL		HavePossibleSeg=FALSE, FirstSeg;
    long		BlockCenterInc = 0;   
	int			st;
	double		SegLength;
	static		municOpt=0;

	if (!municOpt)
	{
		if (GetMunicName (-1,0,0))
			municOpt = 1;
		else
			municOpt = 2;
	}
	if (PRJ_UNITS[1] == 4)
		AddressOffsetDist=0;
	if (UsePointBased && hPIDAddDB)
	{
		lpGWDHead = (LPGWDHEADER) GlobalLock (hPIDAddDB); 
		if (ZIP)
		{
			SetFieldValFromChar(lpGWDHead,&lpGWDHead->pFldInfo[lpGWDHead->IndexFields[1][0]],(LPSTR)&ZIP,TRUE,FALSE); 
			SetFieldValFromChar(lpGWDHead,&lpGWDHead->pFldInfo[lpGWDHead->IndexFields[1][1]],(LPSTR)&StreetNum,TRUE,FALSE); 
			SetFieldValFromChar(lpGWDHead,&lpGWDHead->pFldInfo[lpGWDHead->IndexFields[1][2]],(LPSTR)&House,TRUE,FALSE); 
//		SetFieldToMinVal(lpGWDHead,lpGWDHead->IndexFields[1][3]);
			GWDFormKey(lpGWDHead,1,TRUE,0,0);
			index=1;
			st = BT_FIND (lpGWDHead->BTHandle[index],lpGWDHead->pKeys[index],BT_FIRST,BT_EQ, (LPSTR)&Offset);
		}
		else
		{
			if (lpGWDHead->NumIndex < 3)
				goto SkipPointBased;
			SetFieldValFromChar(lpGWDHead,&lpGWDHead->pFldInfo[lpGWDHead->IndexFields[2][0]],(LPSTR)&StreetNum,TRUE,FALSE); 
			SetFieldValFromChar(lpGWDHead,&lpGWDHead->pFldInfo[lpGWDHead->IndexFields[2][1]],(LPSTR)&House,TRUE,FALSE); 
			SetFieldValFromChar(lpGWDHead,&lpGWDHead->pFldInfo[lpGWDHead->IndexFields[2][2]],(LPSTR)&ZIP,TRUE,FALSE); 
		    GWDFormKey(lpGWDHead,2,TRUE,0,0);
			index=2;
			st = BT_FIND (lpGWDHead->BTHandle[index],lpGWDHead->pKeys[index],BT_FIRST,BT_GE, (LPSTR)&Offset);
			if (st ||
				*(LPLONG)lpGWDHead->pKeys[index] != StreetNum ||
				*(LPLONG)(lpGWDHead->pKeys[index]+4) != House)
				st = 1;
		}
		if (!st)
		{
			GWFLDINFO FieldInfo;
			short	index;

			FillGWDData (lpGWDHead,Offset);
			if (!*phMatch)
				*phMatch = GSSiGlobAlloc ( 571,GHND,sizeof(ADDMATCH)); 
			else 
				*phMatch = GSSiGlobalReAlloc (0,*phMatch,(*pNumMatch+1)*sizeof(ADDMATCH),GHND);
			pMatch = pMatchTest = (LPADDMATCH)GlobalLock (*phMatch);  
			n = *pNumMatch;
			pMatch+=n;
			pMatch->MatchCode = min (3,MatchCode);
			pMatch->LocationCode = 4;
			pMatch->StreetNum = StreetNum;
			pMatch->HouseNum = House; 
			pMatch->ZIP = ZIP;
            GMDGetCharFieldVal (lpGWDHead,AddUDIVarIndex,pMatch->TAGUDI);
            if (GWDGetFieldInfoFromName (hPIDAddDB,"X",&FieldInfo,&index))
			{
				char	val[64];

				GMDGetCharFieldVal (lpGWDHead,index,val);
				pMatch->Point.x = atof (val);
				GWDGetFieldInfoFromName (hPIDAddDB,"Y",&FieldInfo,&index);
				GMDGetCharFieldVal (lpGWDHead,index,val);
				pMatch->Point.y = atof (val);
			}
			GlobalUnlock (*phMatch);  
			(*pNumMatch)++;  
			rtn = TRUE;
		}
SkipPointBased:
	    GlobalUnlock (hPIDAddDB);
	    if (rtn)
	    	return TRUE;
	} 
	if (!UseNetBased)
		return FALSE;
	if (BlockCenter)
	{
		LocationCode = 3;
		BlockCenterInc = 20;
	}
	SegMaxKey.StreetNum = StreetNum;
	SegMaxKey.ZIPCode = ZIP;
	SegMaxKey.Side = 1;
	SegMaxKey.MaxHouseNum = House;
	SegMaxKey.Segid = LONG_MIN;
	stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_FIRST,BT_GE,(LPSTR)&MinHouseNum); 
	if (!MatchCode)
	{
		if (SegMaxKey.Side != 1) 
		{
			SegMaxKey.StreetNum = StreetNum;
			SegMaxKey.ZIPCode = ZIP;
			SegMaxKey.Side = 2;
			SegMaxKey.MaxHouseNum = House;
			SegMaxKey.Segid = LONG_MIN;
			stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_FIRST,BT_GE,(LPSTR)&MinHouseNum);
		} 
		if (!stMax && SegMaxKey.ZIPCode == ZIP && SegMaxKey.StreetNum == StreetNum) 
		{
			Dist1 = MinHouseNum - House; 
			NearHouse = MinHouseNum;
			GotSeg = SegMaxKey; 
			GotMinHouse = MinHouseNum;
		}
		else
			Dist1 = LONG_MAX;
		stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_PRIOR,BT_ANY,(LPSTR)&MinHouseNum); 
		if (!stMax && SegMaxKey.ZIPCode == ZIP && SegMaxKey.StreetNum == StreetNum)
			Dist2 = House - SegMaxKey.MaxHouseNum;
		else
			Dist2 = LONG_MAX;
		if (Dist2 < Dist1)
		{
			NearHouse = SegMaxKey.MaxHouseNum;
			GotSeg = SegMaxKey;               
			GotMinHouse = MinHouseNum;
		}
		else
			if (Dist1 == LONG_MAX)
				return FALSE;
		House = NearHouse;
		goto GetCoord;
	}   
	FirstSeg = TRUE;
NextSeg:
	if (!stMax && SegMaxKey.ZIPCode == ZIP && SegMaxKey.StreetNum == StreetNum &&
		SegMaxKey.Side == 1 && House+BlockCenterInc >= MinHouseNum)
	{   
		if (SegMaxKey.MaxHouseNum%2 != MinHouseNum%2)  
		{
			HavePossibleSeg = TRUE;
			PossibleSeg = SegMaxKey; 
			PossibleMinHouse = MinHouseNum;
		}
		else if (MinHouseNum%2 == House%2)
		{
			GotSeg = SegMaxKey;
			GotMinHouse = MinHouseNum;
			goto GetCoord;
		}
		else if (FirstSeg)
		{
			FirstSeg = FALSE;
			stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_NEXT,BT_ANY,(LPSTR)&MinHouseNum); 
			goto NextSeg;
		}
	}
	SegMaxKey.StreetNum = StreetNum;
	SegMaxKey.ZIPCode = ZIP;
	SegMaxKey.Side = 2;
	SegMaxKey.MaxHouseNum = House;
	SegMaxKey.Segid = LONG_MIN;
	stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_FIRST,BT_GT,(LPSTR)&MinHouseNum); 
	FirstSeg = TRUE;
NextSeg2:
	if (!stMax && SegMaxKey.ZIPCode == ZIP && SegMaxKey.StreetNum == StreetNum &&
		SegMaxKey.Side == 2 && House+BlockCenterInc >= MinHouseNum)
	{
		GotSeg = SegMaxKey;
		GotMinHouse = MinHouseNum;
		if ((SegMaxKey.MaxHouseNum%2 == MinHouseNum%2) && (MinHouseNum%2 == House%2))
			goto GetCoord;
		else if (FirstSeg)
		{    
			FirstSeg = FALSE;
			stMax = BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_NEXT,BT_ANY,(LPSTR)&MinHouseNum); 
			goto NextSeg2;
		}
	}
	if (HavePossibleSeg)
	{
		GotSeg = PossibleSeg;
		GotMinHouse = PossibleMinHouse;
		goto GetCoord;
	}
	return FALSE;

GetCoord:
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
	pSegdata = (LPSEGDATAGM)&lpGWDHead->GWDData;
    if (BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&GotSeg.Segid,BT_FIRST,BT_EQ,(LPSTR)&Offset))  
    {   
    	GlobalUnlock (hDBStreetSegments); 
    	return FALSE;
    }
	FillGWDData (lpGWDHead,Offset);
	if (!*phMatch)
		*phMatch = GSSiGlobAlloc ( 571,GHND,sizeof(ADDMATCH)); 
	else 
		*phMatch = GSSiGlobalReAlloc (0,*phMatch,(*pNumMatch+1)*sizeof(ADDMATCH),GHND);
	pMatch = pMatchTest = (LPADDMATCH)GlobalLock (*phMatch);  
	n = *pNumMatch;
	pMatch+=n;
	pMatch->MatchCode = min (3,MatchCode);
	pMatch->LocationCode = LocationCode;
	pMatch->StreetNum = StreetNum;
	pMatch->HouseNum = House; 
	if ((AddRangeOffDist = GetGlobalLVal2 ("[%ADDRANGEOFFDIST]",0)))
	{   
		if (!GetSegCoorByPct (GotSeg.Segid,0.5,&CLPoint,&AZ,&Length,FALSE))
		{
			rtn = FALSE; 
			goto Exit;
		} 
		FromPCT = AddRangeOffDist/Length;      
		ToPCT = 1.0 - FromPCT;
	} 
	else
	{
		FromPCT = 0;
		ToPCT = 1.0;
	}
		
	SegLength = GetStreetSegLength (GotSeg.Segid);

	if (SegLength <= 0)
		SegLength = 10000;
	if (GotSeg.MaxHouseNum == max (pSegdata->faddl,pSegdata->taddl)) 
	{

		if (BlockCenter)
			OffDist = 0;
		else
			OffDist = -AddressOffsetDist;
		pMatch->ZIP = pSegdata->ZIPL;
		switch (municOpt)
		{
		case 1:
			pMatch->Munic = pSegdata->FMCDL;
			break;
		case 2:
			pMatch->Munic = StandardizedMunic (pSegdata->COUNTYL * 100000 + pSegdata->FMCDL);
			break;
		}

		if (BlockCenter || (pSegdata->faddl == pSegdata->taddl))
			PCT = 0.5;  
		else if (House == pSegdata->faddl)
			PCT = FromPCT;
		else if (House == pSegdata->taddl)
			PCT = ToPCT;
		else if (!pSegdata->LowAddrL && !pSegdata->HighAddrL)
			PCT = FromPCT + (ToPCT - FromPCT) * 
					((double) (House - pSegdata->faddl) / (double) (pSegdata->taddl - pSegdata->faddl));
		else
		{   
			if (House == pSegdata->LowAddrL)
				PCT = ((double)pSegdata->LowOffL)/SegLength;
			else if (House == pSegdata->HighAddrL)
				PCT = 1.0 - ((double)pSegdata->HighOffL)/SegLength; 
			else
			{
				Dir = pSegdata->taddl - pSegdata->faddl;
				MinAdd = min (pSegdata->faddl,pSegdata->taddl) ;
				MaxAdd = GotSeg.MaxHouseNum; 
				if (pSegdata->LowAddrL)
				{
					LowAdd = pSegdata->LowAddrL;
					LowPCT = max (FromPCT,((double)pSegdata->LowOffL)/SegLength);
				}
				else
				{
					LowAdd = MinAdd;
					LowPCT = FromPCT;
				}
				if (pSegdata->HighAddrL)
				{
					HighAdd = pSegdata->HighAddrL;
					HighPCT = min (ToPCT,1.0 - ((double)pSegdata->HighOffL)/SegLength);
				}
				else
				{
					HighAdd = MaxAdd;
					HighPCT = ToPCT;
				}
				
				if (Dir < 0)
				{
					int	LowAddSave = LowAdd;
					double	LowPCTSave = LowPCT;

					LowAdd = HighAdd;
					LowPCT = HighPCT;
					HighAdd = LowAddSave;
					HighPCT = LowPCTSave;
					Dir = 0;
				}
				if (House < LowAdd)
					PCT = FromPCT + 
						 ((double) (House - MinAdd) / (double) (LowAdd - MinAdd)) * (LowPCT - FromPCT);
				else if (House > HighAdd)
					PCT = ((double) (House - HighAdd) / (double) (MaxAdd - HighAdd)) * (ToPCT - HighPCT)
							+ HighPCT;
				else
					PCT = ((double) (House - LowAdd) / (double) (HighAdd - LowAdd)) * (HighPCT - LowPCT)
							+ LowPCT; 
				if (Dir < 0)
					PCT = 1.0 - PCT;
			}
		}
	}  
	else
	{ 
		OffDist = AddressOffsetDist;
		pMatch->ZIP = pSegdata->ZIPR;
		switch (municOpt)
		{
		case 1:
			pMatch->Munic = pSegdata->FMCDR;
			break;
		case 2:
			pMatch->Munic = StandardizedMunic (pSegdata->COUNTYR * 100000 + pSegdata->FMCDR);
			break;
		}
 
		if (pSegdata->faddr == pSegdata->taddr)
			PCT = 0.5;
		else if (House == pSegdata->faddr)
			PCT = FromPCT;
		else if (House == pSegdata->taddr)
			PCT = ToPCT;
		else if (!pSegdata->LowAddrR && !pSegdata->HighAddrR)
			PCT = FromPCT + (ToPCT - FromPCT) * 
					((double) (House - pSegdata->faddr) / (double) (pSegdata->taddr - pSegdata->faddr));
		else
		{   
			if (House == pSegdata->LowAddrR)
				PCT = ((double)pSegdata->LowOffR)/SegLength;
			else if (House == pSegdata->HighAddrR)
				PCT = 1.0 - ((double)pSegdata->HighOffR)/SegLength; 
			else
			{
				Dir = pSegdata->taddr - pSegdata->faddr;
				MinAdd = min (pSegdata->faddr,pSegdata->taddr) ;
				MaxAdd = GotSeg.MaxHouseNum; 
				if (pSegdata->LowAddrR)
				{
					LowAdd = pSegdata->LowAddrR;
					LowPCT = max (FromPCT,((double)pSegdata->LowOffR)/SegLength);
				}
				else
				{
					LowAdd = MinAdd;
					LowPCT = FromPCT;
				}
				if (pSegdata->HighAddrR)
				{
					HighAdd = pSegdata->HighAddrR;
					HighPCT = min (ToPCT,1.0 - ((double)pSegdata->HighOffR)/SegLength);
				}
				else
				{
					HighAdd = MaxAdd;
					HighPCT = ToPCT;
				}
				
				if (Dir < 0)
				{
					int	LowAddSave = LowAdd;
					double	LowPCTSave = LowPCT;

					LowAdd = HighAdd;
					LowPCT = HighPCT;
					HighAdd = LowAddSave;
					HighPCT = LowPCTSave;
					Dir = 0;
				}
				if (House < LowAdd)
					PCT = FromPCT + 
						 ((double) (House - MinAdd) / (double) (LowAdd - MinAdd)) * (LowPCT - FromPCT);
				else if (House > HighAdd)
					PCT = ((double) (House - HighAdd) / (double) (MaxAdd - HighAdd)) * (ToPCT - HighPCT)
							+ HighPCT;
				else
					PCT = ((double) (House - LowAdd) / (double) (HighAdd - LowAdd)) * (HighPCT - LowPCT)
							+ LowPCT; 
				if (Dir < 0)
					PCT = 1.0 - PCT;
			}
		}
	} 
	if (OrigZIP && pMatch->ZIP != OrigZIP)
		pMatch->ChangedZIP = TRUE; 
	else
		pMatch->ChangedZIP = FALSE;
	if (OrigMunic && pMatch->Munic != OrigMunic)
		pMatch->ChangedMunic = TRUE; 
	else
		pMatch->ChangedMunic = FALSE;
	while (n--)
	{
		if (pMatchTest->StreetNum == pMatch->StreetNum && 
			pMatchTest->HouseNum == pMatch->HouseNum &&
			pMatchTest->ZIP == pMatch->ZIP) 
		{
			rtn = FALSE;
			goto Exit;
		}
		pMatchTest++;
	}
	if (!GetSegCoorByPct (GotSeg.Segid,PCT,&CLPoint,&AZ,&Length,FALSE))
	{
//		ContinueProcessing = FALSE;  ?????
		rtn = FALSE; 
		goto Exit;
	}
	else
	{   
		pMatch->Point = dnewpt (CLPoint,AZ+PIHALF,OffDist);
	} 
	if (MatchCode == 4)
	{
		if ((pMatch->ZIP == OrigZIP) || (pMatch->Munic == OrigMunic) || (!OrigZIP && !OrigMunic))
			rtn = TRUE;
		else
			goto Exit;
	}
	else if ((pMatch->ZIP == OrigZIP) || (!OrigZIP && pMatch->Munic == OrigMunic) || (!OrigZIP && !OrigMunic))
		rtn = TRUE;
	else
		goto Exit;
	(*pNumMatch)++;  
Exit:                                              
	GlobalUnlock (*phMatch); 
    GlobalUnlock (hDBStreetSegments);
	return rtn;
		
} 

BOOL GetMunicName (long MCDIn,LPSTR Name,LPSTR Abv)
{ 
	HANDLE hBT; 
	char	File[MAX_PATH];  
	short	st;
	MUNICNAME	MunName;
	long	MCD = MCDIn;
	
	if (Name)
		*Name = 0;
	if (Abv)
		*Abv = 0;
    SetAddressDir();
    
    sprintf (File,"%s\\munics.btr",AddMatchDir);
	if (!(hBT = BT_OPEN (File,0,BT_READ,0)))
		return FALSE;
	st = BT_FIND (hBT,(LPSTR)&MCD,BT_FIRST,BT_EQ,(LPSTR)&MunName);
	BT_CLOSE (hBT);
	if (MCDIn < 0)
		return TRUE; // test for file
	if (st)
		return FALSE;
	if (Name)
		_fstrcpy (Name,MunName.FullName);
	if (Abv)
		_fstrcpy (Abv,MunName.Abv);
	return TRUE;
} 

BOOL GetZIPCenter (long ZIP,LPDPOINT Point)
{
	HFILE	Fid;
	OFSTRUCT OFStruct;
	char	File[128];   
	DPOINT	point;
	long	zip, lon, lat;  
	char	str[70];
	
    SetAddressDir();
    
    sprintf (File,"%s\\zip_cent.txt",AddMatchDir);
	Fid=GSSiOpenFile (File,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
		return FALSE;
	if (!Point) 
	{
		GSSiClose (Fid);
		return TRUE; //used to test for file existance    
	}
	while (fgetstring (str,64,Fid))
	{
		sscanf (str,"%ld %ld %ld",&zip,&lon,&lat);
		if (zip == ZIP)
		{         
			GSSiClose (Fid);   
			point.x = -(double)lon/1000000.0;
			point.y = (double)lat/1000000.0;
			ConvertCoord (&point,2,1);
			*Point = point;
			return TRUE;
		}
	}
	GSSiClose (Fid);
	return FALSE;
}

BOOL ReloadSTNDTables (void)
{   
	char	Name[MAX_PATH], TrueName[66], TempFile[MAX_PATH]; 
	OFSTRUCT	OFStruct;
	HFILE	Fid; 
	long	TotLen,CurLoc;
	
	HCURSOR	hcurSave;   

	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));
	if (GSSiMsgBox( GetFocus(),"Be sure to have a backup of the ADDRESS subdirectory of your project before continuing",
			"Verify Rebuild", MB_OKCANCEL,0) == IDCANCEL) return FALSE;
    SetAddressDir();

	CloseStreetNameTable();
    _fstrcpy (Name,AddMatchDir); 
    _fstrcat (Name, "\\stndsn.hsh");
    GSSiRemove (Name);
    _fstrcpy (Name,AddMatchDir); 
    _fstrcat (Name, "\\stndsn.ary");
    GSSiRemove (Name);
    
    STNDSN_INIT(TRUE);
	CreateStatusWind (hWndMain,1,"Load Street Standardization Table");
	TotLen =  CurLoc = GetStreetNumFromName ("##COUNT##",2,0,TrueName);
	StatusWindowUpdate (NULL,"Unloading street names", TotLen, CurLoc);
   
	GSSiGetTempFileName (0,"gm",0,(LPSTR)TempFile); 
    Fid = GSSiOpenFile (TempFile,&OFStruct,OF_CREATE);
    while ((StreetNum = GetStreetNumFromName (STDNAMv,2,0,TrueName)))
    {
    	BigWrite (Fid,(HPSTR)&StreetNum,4,-1);
    	BigWrite (Fid,(HPSTR)TrueName,sizeof(TrueName),-1);    
		StatusWindowUpdate (NULL,NULL, TotLen, --CurLoc);

    }
    GSSiClose (Fid);
	CloseStreetNameTable();
    sprintf (Name,"%s\\strname.gmd",AddMatchDir);
    GSSiRemove (Name);
    sprintf (Name,"%s\\strname.in1",AddMatchDir);
    GSSiRemove (Name);
    sprintf (Name,"%s\\strname.in2",AddMatchDir);
    GSSiRemove (Name);
    sprintf (Name,"%s\\strname.in3",AddMatchDir);
    GSSiRemove (Name);
    sprintf (Name,"%s\\strname.in4",AddMatchDir);
    GSSiRemove (Name);
    sprintf (Name,"%s\\strname.in5",AddMatchDir);
    GSSiRemove (Name);
    sprintf (Name,"%s\\strname.in6",AddMatchDir);
    GSSiRemove (Name);
    sprintf (Name,"%s\\strname.in7",AddMatchDir);
    GSSiRemove (Name);
    sprintf (Name,"%s\\strname.in8",AddMatchDir);
    GSSiRemove (Name);
    CurLoc = 0;
	StatusWindowUpdate (NULL,"Reloading street names", TotLen, 0);
    Fid = GSSiOpenFile (TempFile,&OFStruct,OF_READ);
    while (BigRead (Fid,(HPSTR)&StreetNum,4) == 4)
    {
    	BigRead (Fid,TrueName,sizeof(TrueName));
		AddStreetName (TrueName,StreetNum,"","","","");
		StatusWindowUpdate (NULL,"Reloading street names", TotLen, ++CurLoc);
	}
	CloseStreetNameTable();
    GSSiClose (Fid);
    GSSiRemove (TempFile);
	DestroyStatusWindow(0);  
    GSSiSetCursor (hcurSave); 
    GSSiMsgBox (GetFocus(),"Street table reload complete"," ",MB_OK,0);
	return TRUE;
}
  
BOOL FindNonNetworkedStreets (void)
{   
	char	str[128]="", TrueName[66]; 
	OFSTRUCT	OFStruct;
	HFILE	Fid;  
	long	NumNotNet=0, TotNum=0; 
	
	HCURSOR	hcurSave;   

	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));
    SetAddressDir();
	CloseStreetNameTable();

    Fid = GSSiOpenFile ("nonnet.txt",&OFStruct,OF_CREATE);   
    fputstring ("\"STREETNUM\",\"STREETNAME\"",Fid);
    while ((StreetNum = GetStreetNumFromName (str,2,0,TrueName)))
    {   
    	if (!NumStreetSegs (StreetNum))
    	{   
    		sprintf (str,"%ld,\"%s\"",StreetNum,TrueName);
    		fputstring (str,Fid); 
    		NumNotNet++;
    	}              
    	TotNum++;
    }
    GSSiClose (Fid);
    GSSiSetCursor (hcurSave); 
    sprintf (str,"%ld non-networked streets of %ld total\r\nWritten to: %s",NumNotNet,TotNum,OFStruct.szPathName);
    GSSiMsgBox (GetFocus(),str,"Finished",MB_OK,0);
	return TRUE;
} 

BOOL RemoveNonNetworkedStreets (void)
{   
	LPGWDHEADER lpGWDHead;
    LPGWFLDINFO lpGWFldInfo;      
	char	str[132]; 
	OFSTRUCT	OFStruct;
	HFILE	Fid;
	HCURSOR	hcurSave; 
	long	Offset;
	
	if (GSSiMsgBox( GetFocus(),"Be sure to have a backup of the ADDRESS subdirectory of your project before continuing",
			"Verify Rebuild", MB_OKCANCEL,0) == IDCANCEL) return FALSE;

    Fid = GSSiOpenFile ("nonnet.txt",&OFStruct,OF_READ);
    if (Fid == HFILE_ERROR)
    { 
    	GSSiMsgBox (GetFocus(),"File nonnet.txt not found",NULL,MB_ICONEXCLAMATION,0);
    	return FALSE;
    }
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));
	OpenStreetNameTable (TRUE);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetNames); 
    lpGWFldInfo=lpGWDHead->pFldInfo; 
	while (fgetstring (str,128,Fid))
    {
	    SetFieldValFromChar(lpGWDHead,lpGWFldInfo,str,FALSE,FALSE); 
	    GWDFormKey(lpGWDHead,0,TRUE,0,0);
		if (!BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_EQ, (LPSTR)&Offset))
			GWDDeleteRecord (lpGWDHead,Offset);
	}
	CloseStreetNameTable();  
    GSSiSetCursor (hcurSave); 
	GSSiMsgBox (GetFocus(),"Process Complete"," ",MB_OK,0);
	return TRUE;
}

/*short LoadMCDS2 (short dummy)
{   
	char	InName[128]="c:\\mnmcds.txt";
	char	Name[128]="[%DL]address\\munics.btr";
	char	str[256];
	BTVARDESC BTVar[2];  
	HANDLE	hBT, hBT2, hBT3;
	HFILE	Fid;
	OFSTRUCT	OFStruct;  
	long	FIPSCode, FIPSCodeCity, FIPSCodeCounty;      
	short	Dummy=0;

    HANDLE hDB;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hSQL;
    long        Offset; 
    double      rtn;
    LPVOID      lpVal; 
    short         st, i, len,ifield;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    LPFILEPATH  FilePathPtr; 
    BOOL        More;
    short         rc;  
	MUNICZIPS	MunZIPs;
	char	MunName[72];
	ALTMUNICNAME	ALTMunName;
    
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (Name, sizeof(MUNICNAME), FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    hBT = BT_OPEN (Name,0,BT_WRITE,0);  
    if (!OpenDataFile (InName,"",BT_READ,&hSQL))
        return 0;      
    
    while (FetchDBRec (hSQL))
    {
        GetValFromOpenFiles ("MCD",str); 
        FIPSCode = atol (&str[2]);
        GetValFromOpenFiles ("MCD_NAME",MunName); 
    	BT_PUT (hBT,(LPSTR)&FIPSCode,(LPSTR)&MunName);
    }   
    BT_CLOSE (hBT);
    CloseDataFile (TRUE, &hSQL);   
	return 0;
}*/
	
/*short LoadMCDS3 (short dummy)
{   
	char	InName[128]="C:\\moxie\\ATTRIBUT\\voters\\full\\voters.gmd";
	char	Name[128]="[%DL]address\\zipcities.btr";
	char	Name2[128]="[%DL]address\\zipprecincts.btr";
	char	str[256];
	BTVARDESC BTVar[2];  
	HANDLE	hBT, hBT2, hBT3;
	HFILE	Fid;
	OFSTRUCT	OFStruct;  
	long	Count;      
	short	Dummy=0;   
	struct	{
			long	ZipCode;
			char	Name[64];
			}	Key;

    HANDLE hDB;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hSQL;
    long        Offset; 
    double      rtn;
    LPVOID      lpVal; 
    short         st, i, len,ifield;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    LPFILEPATH  FilePathPtr; 
    BOOL        More;
    short         rc;  
	MUNICZIPS	MunZIPs;
	char	MunName[72];
	ALTMUNICNAME	ALTMunName;
    
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_CHAR;
	BTVar[1].BT_VARLEN=64;
	BTVar[1].BT_VAROFF=4;
	BT_CREATE (Name, 4, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    hBT = BT_OPEN (Name,0,BT_WRITE,0);  
	BT_CREATE (Name2, 4, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    hBT2 = BT_OPEN (Name2,0,BT_WRITE,0);  
    if (!OpenDataFile (InName,"",BT_READ,&hSQL))
        return 0;      
    
    while (FetchDBRec (hSQL))
    {
        GetValFromOpenFiles ("ZIPCODE",str); 
        Key.ZipCode = atol (str);
        GetValFromOpenFiles ("City",str);
        _fstrncpy (Key.Name,str,64);
        if (!BT_FIND (hBT,(LPSTR)&Key,BT_FIRST,BT_EQ,(LPSTR)&Count))
        	Count++;
        else
        	Count = 1; 
    	BT_PUT (hBT,(LPSTR)&Key,(LPSTR)&Count);
        GetValFromOpenFiles ("PrecinctName",str);
        _fstrncpy (Key.Name,str,64);
        if (!BT_FIND (hBT2,(LPSTR)&Key,BT_FIRST,BT_EQ,(LPSTR)&Count))
        	Count++;
        else
        	Count = 1; 
    	BT_PUT (hBT2,(LPSTR)&Key,(LPSTR)&Count);
    }   
    BT_CLOSE (hBT);
    BT_CLOSE (hBT2);
    CloseDataFile (TRUE, &hSQL);   
	return 0;
} */
	
/*short LoadMCDS (short dummy)
{   
	char	InName[128];
	char	Name[128]="[%DL]address\\munics.btr";
	char	Name2[128]="[%DL]address\\munnames.btr";
	char	Name3[128]="[%DL]address\\munzips.btr";
	char	str[256];
	BTVARDESC BTVar[2];  
	HANDLE	hBT, hBT2, hBT3;
	HFILE	Fid;
	OFSTRUCT	OFStruct;  
	long	FIPSCode, FIPSCodeCity, FIPSCodeCounty;      
	short	Dummy=0;

    HANDLE hDB;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hSQL;
    long        Offset; 
    double      rtn;
    LPVOID      lpVal; 
    short         st, i, len,ifield;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    LPFILEPATH  FilePathPtr; 
    BOOL        More;
    short         rc;  
	MUNICZIPS	MunZIPs;
	MUNICNAME	MunName;
	ALTMUNICNAME	ALTMunName;
    
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (Name, sizeof(MUNICNAME), FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    hBT = BT_OPEN (Name,0,BT_WRITE,0);  
	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=64;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=64;
	BT_CREATE (Name2, 2, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    hBT2 = BT_OPEN (Name2,0,BT_WRITE,0);  
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BT_CREATE (Name3, 2, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    hBT3 = BT_OPEN (Name3,0,BT_WRITE,0);  
    hSQL = 0;  
    if (!GetFileName2 (hWndMain,InName,"",IDS_FILEDBF))
    	return 0;
    if (!OpenDataFile (InName,"",BT_READ,&hSQL))
        return 0;      
    
    while (FetchDBRec (hSQL))
    {
        GetValFromOpenFiles ("FIPS5_L",str);
        FIPSCodeCity = atol (str);
        GetValFromOpenFiles ("CENCTY_L",str);
        FIPSCodeCounty = atol (str);  
        FIPSCode = FIPSCodeCounty * 100000 + FIPSCodeCity;
    	ALTMunName.Munic = FIPSCode;
        GetValFromOpenFiles ("CITYLEFT",str); 
        Truncate (str);  
        _fstrupr (str);
        _fstrncpy (MunName.FullName,str,sizeof(MunName.FullName)); 
        _fstrncpy (ALTMunName.ALTName,str,sizeof(ALTMunName.ALTName)); 
    	BT_PUT (hBT2,(LPSTR)&ALTMunName,(LPSTR)&Dummy); 
        GetValFromOpenFiles ("CTYLABRV",str); 
        Truncate (str);
        _fstrupr (str);
        _fstrncpy (MunName.Abv,str,sizeof(MunName.Abv)); 
        _fstrncpy (ALTMunName.ALTName,str,sizeof(ALTMunName.ALTName)); 
    	BT_PUT (hBT2,(LPSTR)&ALTMunName,(LPSTR)&Dummy); 
    	BT_PUT (hBT,(LPSTR)&FIPSCode,(LPSTR)&MunName); 
    	MunZIPs.Munic = FIPSCode;
        GetValFromOpenFiles ("ZIP5_L",str);
        MunZIPs.ZIP = atol (str);
    	BT_PUT (hBT3,(LPSTR)&MunZIPs,(LPSTR)&Dummy);
        GetValFromOpenFiles ("FIPS5_R",str);
        FIPSCodeCity = atol (str);
        GetValFromOpenFiles ("CENCTY_R",str);
        FIPSCodeCounty = atol (str);  
        FIPSCode = FIPSCodeCounty * 100000 + FIPSCodeCity;
    	MunZIPs.Munic = FIPSCode;
        GetValFromOpenFiles ("ZIP5_R",str);
        MunZIPs.ZIP = atol (str);
    	BT_PUT (hBT3,(LPSTR)&MunZIPs,(LPSTR)&Dummy);
    }
    CloseDataFile (TRUE, &hSQL);   
    BT_CLOSE (hBT);
    BT_CLOSE (hBT2);
    BT_CLOSE (hBT3);
	return 0;
}*/

int LoadMCDS4 (int dummy)
{   
	char	InName[128]="C:\\FIPSCodes\\NationalFedCodes_20110214.txt";
	char	Name[128]="[%DL]address\\munics.btr";
	char	str[256];
	BTVARDESC BTVar[2];  
	HANDLE	hBT, hBT2, hBT3;
	HFILE	Fid;
	OFSTRUCT	OFStruct;  
	long	FIPSCode, FIPSCodeCity, FIPSCodeCounty;      
	short	Dummy=0;

    HANDLE hDB;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hSQL=0;
    long        Offset; 
    double      rtn;
    LPVOID      lpVal; 
    short         st, i, len,ifield;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    LPFILEPATH  FilePathPtr; 
    BOOL        More;
    short         rc;  
	MUNICZIPS	MunZIPs;
	ALTMUNICNAME	ALTMunName;
	MUNICNAME	MunName;
    
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (Name, sizeof(MUNICNAME), FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
    hBT = BT_OPEN (Name,0,BT_WRITE,0);  
    if (!OpenDataFile (InName,"",BT_READ,&hSQL))
        return 0;      
    
    while (FetchDBRec (hSQL))
    {
        GetValFromOpenFiles ("CENSUS_CODE",str,32); 
        FIPSCode = atol (str);
        GetValFromOpenFiles ("FEATURE_NAME",MunName.FullName,sizeof(MunName.FullName)); 
		GetValFromOpenFiles ("COUNTY_NAME",MunName.CountyName,sizeof(MunName.CountyName));
		GetValFromOpenFiles ("STATE_ALPHA",MunName.StateCode,sizeof(MunName.StateCode));
		strncpy (MunName.Abv,"",sizeof(MunName.Abv));
    	if (!strnicmp (MunName.StateCode,"MN",2))
			BT_PUT (hBT,(LPSTR)&FIPSCode,(LPSTR)&MunName);
    }   
    BT_CLOSE (hBT);
    CloseDataFile (TRUE, &hSQL);   
	return 0;
}
BOOL SeparateIntStreets (LPSTR Street,LPSTR Street1,LPSTR Street2)
{   
	LPSTR	SepLoc, OneHalfLoc;
	char	SepString[32];
	int		ii;

	if (!stricmp (Street,"I 94 & WEAVER LAKE RD N"))
		ii=1;
	GetGlobalCVal ("[%ADDSEPSTRING]",SepString,"/");
	_fstrcpy (Street1,Street);
	OneHalfLoc = _fstrstr (Street1,"1/2 ");
	if (OneHalfLoc)
		_fstrncpy (OneHalfLoc,"***",3);
	SepLoc  = _fstrstr (Street1,SepString);
	if (OneHalfLoc)
		_fstrncpy (OneHalfLoc,"1/2",3);
	if (!SepLoc)
	{
		SepLoc  = _fstrstr (Street1,"&");
		if (!SepLoc)
			return FALSE;
	}
	*SepLoc++=0;
	_fstrcpy (Street2,SepLoc);
	OneSpace (Street1);
	OneSpace (Street2);	
	return TRUE;
}

BOOL SeparateOnFromToStreets (LPSTR StreetIN,LPSTR OnStreet,LPSTR FromStreet,LPSTR ToStreet)
{   
	char	Street[1024];
	int		ln = strlen (StreetIN);
	LPSTR	pFrom, pTo;

	if (ln > 1024)
		return FALSE;

	strcpy (Street,StreetIN);
	strupr (Street);


	if (strnicmp (Street,"ON ",3))
		return FALSE;
	if (!(pFrom = strstr (Street," FROM ")))
		return FALSE;
	if (!(pTo = strstr (Street," TO ")))
		return FALSE;
	*pFrom = 0;
	pFrom += 6;
	*pTo = 0;
	pTo += 4;
	strcpy (OnStreet,(Street + 3));
	strcpy (FromStreet,pFrom);
	strcpy (ToStreet,pTo);
	return TRUE;
}

short GetMatchingNames (short NameType, LPSTR Name,LPSHORT nList,HANDLE hList)
{
	short	pos, nAdded=0, n;  
	LPLONG	pList, pListBeg, pList2;
	char	TrueName[66];
	
    pos = BT_FIRST;
    pList = (LPLONG)GlobalLock (hList);
    pListBeg = pList;
    pList += *nList;
    while ((*pList = GetStreetNumFromName (Name,NameType,pos,TrueName)))
    {
    	pos = BT_NEXT; 
    	pList2 = pListBeg;
    	n = *nList;
    	while (n--)
    		if (*pList2++ == *pList)
    			goto NextName; 
    	(*nList)++; 
    	pList++;
    	nAdded++;  
NextName:;
    } 
    GlobalUnlock (hList);
	return nAdded;
}

short MatchIntLists (short MatchCode, short nList1,short nList2,HANDLE hList1,HANDLE hList2,long Munic,LPHANDLE hMatch,HWND hwnddlg1,HWND hwnddlg2)
{ 
	NETINTPATHSKEY	NetIntPathsKey; 
	INTPATHSDATA	IPD;
	DPOINT	IntPoint;   
	BOOL	MunicMatch, HaveMunics;
	short	st, nMatch=0, n2, i;  
	LPADDMATCH	pMatch;
	LPLONG	pStreetNum1, pStreetNum2;  
	long	StreetNum1, StreetNum2;   
	BOOL	GetNext=TRUE;
	MSG		msg;
    
    pStreetNum1 = (LPLONG)GlobalLock (hList1);
    while (GetNext && nList1--)
    {
    	n2 = nList2;
    	pStreetNum2 = (LPLONG)GlobalLock (hList2);
    	while (GetNext && n2--)
    	{
			NetIntPathsKey.Path1 = min(*pStreetNum1,*pStreetNum2);
			NetIntPathsKey.Path2 = max(*pStreetNum1,*pStreetNum2); 
			StreetNum1 = NetIntPathsKey.Path1;
			StreetNum2 = NetIntPathsKey.Path2; 
			NetIntPathsKey.IntID = LONG_MIN;
			st = BT_FIND (hBTNetIntPaths,(LPSTR)&NetIntPathsKey,BT_FIRST,BT_GE,(LPSTR)&IPD); 
			while (GetNext && !st && NetIntPathsKey.Path1 == StreetNum1 && NetIntPathsKey.Path2 == StreetNum2)
			{                             
				MunicMatch = FALSE;
				HaveMunics = FALSE;
				for (i=0;i<5;i++)
					if (IPD.Munics[i])
					{
						HaveMunics = TRUE;
						if (Munic == StandardizedMunic (IPD.Munics[i]))
							MunicMatch = TRUE;
					}
				if (!Munic || MunicMatch || !HaveMunics || MatchCode > 2) 
				{
					if (!*hMatch)
						*hMatch = GSSiGlobAlloc ( 572,GHND,sizeof(ADDMATCH)); 
					else 
						*hMatch = GSSiGlobalReAlloc (0,*hMatch,(nMatch+1)*sizeof(ADDMATCH),GHND);
					pMatch = (LPADDMATCH)GlobalLock (*hMatch); 
					pMatch+=nMatch;
					pMatch->Point = IPD.Point;
					pMatch->IntID = NetIntPathsKey.IntID;
					pMatch->MatchCode = MatchCode;
					pMatch->LocationCode = 2;  
					if (NetIntPathsKey.Path1 == *pStreetNum1)
					{
						pMatch->StreetNum1 = NetIntPathsKey.Path1;
						pMatch->StreetNum2 = NetIntPathsKey.Path2; 
					}
					else
					{
						pMatch->StreetNum1 = NetIntPathsKey.Path2;
						pMatch->StreetNum2 = NetIntPathsKey.Path1; 
					}
					pMatch->Street1MP = GetIntPathMP (pMatch->IntID,pMatch->StreetNum1);   
					pMatch->Street2MP = GetIntPathMP (pMatch->IntID,pMatch->StreetNum2);   
					pMatch->Munic = StandardizedMunic (IPD.Munics[0]);
					pMatch->ZIP = 0;
					GlobalUnlock (*hMatch); 
					nMatch++;
				}                                                
				st = BT_FIND (hBTNetIntPaths,(LPSTR)&NetIntPathsKey,BT_NEXT,BT_ANY,(LPSTR)&IPD); 
 				if (hwnddlg1)
 				{
	 				if (GSSiPeekMessage(&msg,hwnddlg1,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
	 					GetNext = FALSE;	                                                             
	 				if (GSSiPeekMessage(&msg,hwnddlg2,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
	 					GetNext = FALSE;
	 			}	                                                             
			} 
			pStreetNum2++;
    	}
    	pStreetNum1++;
    	GlobalUnlock (hList2);
    } 
    GlobalUnlock (hList1);
	return nMatch;
}

short GetAllInts (short MatchCode, short nList1,HANDLE hList1,long Munic,LPHANDLE hMatch,HWND hwnddlg1,HWND hwnddlg2)
{ 
	NETINTPATHSKEY	NetIntPathsKey; 
	INTPATHSDATA	IPD;
	DPOINT	IntPoint;   
	BOOL	MunicMatch;
	short	st, nMatch=0, n2, i;  
	LPADDMATCH	pMatch;
	LPLONG	pStreetNum1;  
	long	StreetNum1, StreetNum2;   
	BOOL	GetNext=TRUE;
	MSG		msg;
    
    pStreetNum1 = (LPLONG)GlobalLock (hList1);
    while (GetNext && nList1--)
    {
			st = BT_FIND (hBTNetIntPaths,(LPSTR)&NetIntPathsKey,BT_FIRST,BT_ANY,(LPSTR)&IPD); 
			while (GetNext && !st)
			{   
				if (NetIntPathsKey.Path1 == *pStreetNum1 || NetIntPathsKey.Path2 == *pStreetNum1)
				{
					MunicMatch = FALSE;
					for (i=0;i<5;i++)
						if (IPD.Munics[i])
						{
							if (Munic == StandardizedMunic (IPD.Munics[i]))
								MunicMatch = TRUE;
						}
					if (!Munic || MunicMatch || MatchCode > 2) 
					{
						if (!*hMatch)
							*hMatch = GSSiGlobAlloc ( 572,GHND,sizeof(ADDMATCH)); 
						else 
							*hMatch = GSSiGlobalReAlloc (0,*hMatch,(nMatch+1)*sizeof(ADDMATCH),GHND);
						pMatch = (LPADDMATCH)GlobalLock (*hMatch); 
						pMatch+=nMatch;
						pMatch->Point = IPD.Point;
						pMatch->IntID = NetIntPathsKey.IntID;
						pMatch->MatchCode = MatchCode;
						pMatch->LocationCode = 2;  
						if (NetIntPathsKey.Path1 == *pStreetNum1)
						{
							pMatch->StreetNum1 = NetIntPathsKey.Path1;
							pMatch->StreetNum2 = NetIntPathsKey.Path2; 
						}
						else
						{
							pMatch->StreetNum1 = NetIntPathsKey.Path2;
							pMatch->StreetNum2 = NetIntPathsKey.Path1; 
						}
						pMatch->Street1MP = GetIntPathMP (pMatch->IntID,pMatch->StreetNum1);   
						pMatch->Street2MP = GetIntPathMP (pMatch->IntID,pMatch->StreetNum2);   
						pMatch->Munic = StandardizedMunic (IPD.Munics[0]);
						pMatch->ZIP = 0;
						GlobalUnlock (*hMatch); 
						nMatch++;
					} 
				}
				st = BT_FIND (hBTNetIntPaths,(LPSTR)&NetIntPathsKey,BT_NEXT,BT_ANY,(LPSTR)&IPD); 
 				if (hwnddlg1)
 				{
	 				if (GSSiPeekMessage(&msg,hwnddlg1,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
	 					GetNext = FALSE;	                                                             
	 				if (GSSiPeekMessage(&msg,hwnddlg2,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
	 					GetNext = FALSE;
	 			}	                                                             
    	}
    	pStreetNum1++;
     } 
    GlobalUnlock (hList1);
	return nMatch;
}

short ADD_MATCH (LPSTR Street, LPSTR House, LPSTR Munic, LPSTR ZIP, short MOPT,LPHANDLE phMatch,
				 LPLONG	pStreetNum, LPLONG pMunicNum,BOOL UsePointBased,BOOL UseNetBased,HWND hWndDlg,short nControls,LPINT Controls)
{   
	HANDLE	hList=0; 
	LPLONG	pList; 
	BOOL	OpenedUAA=FALSE, OpenedSeg=FALSE, OpenedSP=FALSE, BlockCenter, OpenedSM=FALSE;
	long	IHouse, IMunic, IZIP;
	short	nList=0, rtn=0, nAdded, NumMatch=0;  
	HANDLE	hMem=GSSiGlobAlloc ( 573,GMEM_MOVEABLE,512);
	LPSTR	STDNAM1=GlobalLock (hMem);
	LPSTR	NRONAM1=STDNAM1+50;
	LPSTR	NMONLY1=NRONAM1+50;
	LPSTR	SANSCH1=NMONLY1+50;
	LPSTR	NANDCH1=SANSCH1+50;
	LPSTR	NCMPNM1=NANDCH1+50;
	LPSTR	ORIGNM1=NCMPNM1+50;
	LPSTR	SANSCP1=ORIGNM1+50;
	LPSTR	SANSCS1=SANSCP1+50;	 
	
	*phMatch = 0;
STNDSN_INIT(FALSE);
	if (!OpenStreetSegmentTable (FALSE,&OpenedSeg))
		goto Exit;
	if (!OpenSegMaxIndex (&OpenedSM))
		goto Exit;
	OpenStreetPolys (&OpenedSP);
	*pStreetNum = 0; 
//	if (!*Street)
//		return -1;
	if (!CurrentConfig)
		SetConfig (1);
	SetViewport(*pCommandViewport);
	IHouse = atol (House);
	if (_fstrstr (House,"BLOCK") || _fstrstr (House,"BLK"))
		BlockCenter = TRUE;
	else
		BlockCenter = FALSE;
	IZIP = atol (ZIP);
	IMunic = GetMunicFromName (Munic); 
	*pMunicNum = IMunic; 
	if (FoundUserAssignedAddress (IHouse,Street,IMunic,BlockCenter,phMatch))
	{
		NumMatch = 1;
		goto Exit;
	}
	hList = GSSiGlobAlloc ( 574,GMEM_MOVEABLE,USHRT_MAX);
    STNDST(Street, (short)_fstrlen(Street),STDNAM1,NRONAM1,NMONLY1,
                             			   SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1,NULL,NULL,NULL,NULL); 
//	Find all type 1 matches (full standardized name), if any found return.
    nAdded = GetNameTypeList (1,&nList,hList,STDNAM1,NRONAM1,NMONLY1,
                             				 SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1); 
	if (nList == 1)
	{
		pList = (LPLONG)GlobalLock (hList);
		*pStreetNum = *pList;
		GlobalUnlock (hList);
	}
    if (nAdded)
    	rtn = MatchAddress (min(MOPT,1),nList,hList,IHouse,IMunic,IZIP,BlockCenter,
    						&NumMatch,phMatch,UsePointBased,UseNetBased,hWndDlg,nControls,Controls);
    if (rtn || MOPT <2)
    	goto Exit;

//	Next try all type 2 changes (Add direction if none present, remove if it is;
//	Add type if none present, remove if it is
    nAdded = GetNameTypeList (2,&nList,hList,STDNAM1,NRONAM1,NMONLY1,
                             				 SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1); 
    if (nAdded)
    	rtn = MatchAddress (2,nList,hList,IHouse,IMunic,IZIP,BlockCenter,
    						&NumMatch,phMatch,UsePointBased,UseNetBased,hWndDlg,nControls,Controls);
    if (rtn)
    	goto Exit; 
//	Try the type 1 and 2 names with type 3 zip and munic changes   
	if (MOPT < 3) 	
   		rtn = MatchAddress (4,nList,hList,IHouse,IMunic,IZIP,BlockCenter,
   							&NumMatch,phMatch,UsePointBased,UseNetBased,hWndDlg,nControls,Controls);
   	else
   		rtn = MatchAddress (3,nList,hList,IHouse,IMunic,IZIP,BlockCenter,
   							&NumMatch,phMatch,UsePointBased,UseNetBased,hWndDlg,nControls,Controls);
    if (rtn || MOPT <3)
    	goto Exit;
//	Next try all type 3 changes (Change direction if present, change type if none present, change ZIP if present
    nAdded = GetNameTypeList (3,&nList,hList,STDNAM1,NRONAM1,NMONLY1,
                             				 SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1); 
    if (nAdded)
    	rtn = MatchAddress (3,nList,hList,IHouse,IMunic,IZIP,BlockCenter,
    						&NumMatch,phMatch,UsePointBased,UseNetBased,hWndDlg,nControls,Controls);
Exit:                        
	GSSiGlobFree (&hList); 
	GSSiGlobUlFree (&hMem); 
	CloseSegMaxIndex (OpenedSM);  
	CloseStreetSegmentTable (OpenedSeg);
	CloseStreetPolys (OpenedSP);
	return NumMatch;
}  
 
BOOL CreateUserDefinedAddress (LPSTR AddressDir,LPSTR Ver)
{   char    File[128];
    BTVARDESC   BTVar[3];

    BTVar[0].BT_VARTYP=BT_CHAR;
    BTVar[0].BT_VARLEN=64;
    BTVar[0].BT_VAROFF=0;
    BTVar[1].BT_VARTYP=BT_INTEGER;
    BTVar[1].BT_VARLEN=4;
    BTVar[1].BT_VAROFF=64;
    BTVar[2].BT_VARTYP=BT_INTEGER;
    BTVar[2].BT_VARLEN=4;
    BTVar[2].BT_VAROFF=68;
    sprintf (File,"%s\\useradd%s.btr",AddressDir,Ver);
	BT_SET_VERSION (2);
    BT_CREATE (File, sizeof(ADDMATCH), FALSE, 3, 1,(LPBTVARDESC) BTVar,FALSE, 0, 0, FALSE);
	BT_SET_VERSION (1);
    return TRUE;
}  

BOOL ConvertUserAddressVer1to2 (HANDLE hBT1,LPSTR AddressDir) 
{
	HANDLE hBT2; 
	char	Name1[128], Name2[128];  
	short	pos=BT_FIRST;
	USERLOCATEDADDRESSKEY	ULAddKey;
	ADDMATCH_ver1	Match1;   
	ADDMATCH	Match2;  
	LPSTR	pDot; 
	
	BT_GETPATHNAME (hBT1,Name1);  
	_fstrcpy (Name2,Name1);
	pDot = _fstrrchr (Name2,'.');
	_fstrcpy (pDot,"2.btr");
	CreateUserDefinedAddress (AddressDir,"2");  
	hBT2 = BT_OPEN (Name2,0,BT_WRITE,0);
	while (!BT_FIND (hBT1,(LPSTR)&ULAddKey,pos,BT_ANY,(LPSTR)&Match1))  
	{ 
		pos = BT_NEXT;  
		_fmemset (&Match2,0,sizeof(Match2));
		Match2.MatchCode = Match1.MatchCode;
		Match2.LocationCode = Match1.LocationCode;
		Match2.StreetNum = Match1.StreetNum;
		Match2.HouseNum = Match1.HouseNum;
		Match2.IntID = Match1.IntID;
		Match2.StreetNum1 = Match1.StreetNum1;
		Match2.StreetNum2 = Match1.StreetNum2; 
		Match2.Munic = Match1.Munic;
		Match2.ZIP = Match1.ZIP;
		Match2.Point = Match1.Point;
		Match2.ChangedMunic = Match1.ChangedMunic;
		Match2.ChangedZIP = Match1.ChangedZIP;
		BT_PUT (hBT2,(LPSTR)&ULAddKey,(LPSTR)&Match2);
	}
	BT_CLOSE (hBT2);
	BT_CLOSEANDDELETE (&hBT1);  
	GSSiRename (Name2,Name1);
	return TRUE;
}

BOOL AddUserAddress (LPSTR Address,LPSTR Munic,DPOINT Point,BOOL Unmatchable)
{   
	USERLOCATEDADDRESSKEY	ULAddKey;
	ADDMATCH	AM;  
	long	IHouse, IMunic;
	char	House[32], Street1[256], Street2[128];
	BOOL	BlockCenter;  
	HANDLE	hMatch=0;  
	LPSTR	Street;
	BOOL	Opened, rtn=FALSE;
					 	 
	if (!OpenUserDefinedAddress (TRUE,&Opened))
		return FALSE;
	if (SeparateIntStreets (Address,Street1,Street2))
	{
		Truncate (Street1);
		strcat (Street1,"/");
		strcat (Street1,Street2);
		Truncate (Street1);
		IHouse = 0;
		Street = Street1;
	}
	else
	{
		Street = GetHouseAndStreet (Address,House);
		IHouse = atol (House);
	}
	if (_fstrstr (House,"BLOCK") || _fstrstr (House,"BLK"))
		BlockCenter = TRUE;
	else
		BlockCenter = FALSE;
	IMunic = GetMunicFromName (Munic); 
	if (FoundUserAssignedAddress (IHouse,Street,IMunic,BlockCenter,&hMatch)) 
		goto Exit;
	_fmemset (&AM,0,sizeof(AM));
	AM.MatchCode=1;
	if (Unmatchable)
		AM.LocationCode=10;
	else
		AM.LocationCode=7;
	AM.Point=Point;
	ULAddKey.Munic = IMunic; 
	ULAddKey.HouseNum = IHouse; 
	_fstrncpy (ULAddKey.Street,Street,sizeof(ULAddKey.Street));
	AddToUserDefinedAddress (&ULAddKey,&AM);  
	rtn = TRUE;
Exit:
	CloseUserDefinedAddress (Opened);
	GSSiGlobFree (&hMatch);
	return TRUE;
}

BOOL OpenUserDefinedAddress (BOOL Update,LPBOOL Opened)
{   
    char    File[MAX_PATH],FileSub[MAX_PATH];
    static  BOOL    OpenMode;
    short       Mode; 
    DWORD	Err;
    
	*Opened = 0;
    SetAddressDir();
    if (hBTUserDefinedAddress && (OpenMode==Update || OpenMode==BT_WRITE))
        return TRUE; 
    CloseUserDefinedAddress (TRUE);                   
    
	sprintf (File,"%s\\useradd.btr",AddMatchDir);
    if (!ExistFile(File))
    {   
        GSSiMakeDir (AddMatchDir,&Err);
        if (!CreateUserDefinedAddress (AddMatchDir,""))
        {
            return FALSE;
        } 
    }
	if (*UserAddressSubDir)
	{
		sprintf (FileSub,"%s\\%s\\useradd.btr",AddMatchDir,UserAddressSubDir);
	    if (!ExistFile(FileSub))
			GSSiCopyFile (File,FileSub,TRUE);
		strcpy (File,FileSub);
	}
Top:
    if (Update)
        Mode = BT_WRITE;
    else
        Mode = BT_READ;
    if (!(hBTUserDefinedAddress = BT_OPEN (File,0,Mode,0)))
    {
        return FALSE;
    }  
	if (BT_GET_VERSION (hBTUserDefinedAddress) != 2)
	{
		ConvertUserAddressVer1to2 (hBTUserDefinedAddress,AddMatchDir);
		goto Top;
	}
     OpenMode = Update; 
    *Opened = 1;
    
    return TRUE;
}
  
void CloseUserDefinedAddress (BOOL Opened)
{  
    if (!Opened)
    	return;
    BT_CLOSE (hBTUserDefinedAddress);
    hBTUserDefinedAddress = 0;
    return;
}
 
BOOL AddToUserDefinedAddressFromFile (LPSTR File)
{
	USERLOCATEDADDRESSKEY ULAddKey;
	ADDMATCH	AM;
	BOOL	OpenedUAA;
	HFILE	Fid;
	BOOL	rtn=FALSE;

	if (!OpenUserDefinedAddress (TRUE,&OpenedUAA))
		return FALSE;  
	Fid = GSSiOpenFile (File,0,OF_READ);
	if (Fid != HFILE_ERROR)
	{
		long	ln;

		BigRead (Fid,&ln,4);
		BigRead (Fid,&ULAddKey,ln);
		BigRead (Fid,&ln,4);
		BigRead (Fid,&AM,ln);
		GSSiClose (Fid);
		strupr (ULAddKey.Street);

		if (BT_PUT (hBTUserDefinedAddress,(LPSTR)&ULAddKey,(LPSTR)&AM)>=0)
			rtn = TRUE;
	}
	CloseUserDefinedAddress (OpenedUAA);
	return rtn; 
}   
BOOL AddToUserDefinedAddress (LPUSERLOCATEDADDRESSKEY pULAddKey, LPADDMATCH	pAM)
{
	LPGWDHEADER lpGWDHead;
	UINT	len; 
	BOOL	IndexArray[2];	  
	long	Offset;
	HANDLE	hDB;  
	BOOL	OpenedUAA;

	if (!OpenUserDefinedAddress (TRUE,&OpenedUAA))
	{   
Errmes:
		CloseUserDefinedAddress (OpenedUAA);
		GSSiMsgBox (0,"Unable to store user defined address",NULL,MB_ICONEXCLAMATION,0);
		return FALSE;  
	}
	strupr (pULAddKey->Street);
	if (BT_PUT (hBTUserDefinedAddress,(LPSTR)pULAddKey,(LPSTR)pAM) <0)
		goto Errmes;
	if (*UserAddressSubDir)
	{
		char	UpdatesDir[MAX_PATH];
		LPSTR	pBS;
		time_t	iTime = time(0);
		long	ln;
		HFILE	Fid;

		BT_GETPATHNAME (hBTUserDefinedAddress,UpdatesDir);
		pBS = strrchr (UpdatesDir,'\\');
		pBS++;
		sprintf (pBS,"%ld.bin",iTime);
		Fid = GSSiOpenFile (UpdatesDir,0,OF_CREATE);
		ln = sizeof (USERLOCATEDADDRESSKEY);
		BigWrite (Fid,&ln,4,-1);
		BigWrite (Fid,pULAddKey,ln,-1);
		ln = sizeof (ADDMATCH);
		BigWrite (Fid,&ln,4,-1);
		BigWrite (Fid,pAM,ln,-1);
		GSSiClose (Fid);
	}
	CloseUserDefinedAddress (OpenedUAA);
	return TRUE; 
}   


BOOL FoundUserAssignedAddress (long IHouse,LPSTR Street,long IMunic,BOOL BlockCenter,LPHANDLE phMatch)
{   
	BOOL	OpenedUAA, rtn=FALSE;
	USERLOCATEDADDRESSKEY	ULAddKey;
	LPADDMATCH	pMatch; 
	
	if (!OpenUserDefinedAddress (FALSE,&OpenedUAA))
		return FALSE; 
	
	_fstrncpy (ULAddKey.Street,Street,sizeof(ULAddKey.Street));
	strupr (ULAddKey.Street);
	ULAddKey.HouseNum = IHouse;
	ULAddKey.Munic = IMunic; 
	*phMatch = GSSiGlobAlloc ( 575,GHND,sizeof(ADDMATCH));
	pMatch = (LPADDMATCH)GlobalLock (*phMatch); 
	if (!BT_FIND (hBTUserDefinedAddress,(LPSTR)&ULAddKey,BT_FIRST,BT_EQ,(LPSTR)pMatch))
	{  
		rtn = TRUE; 
		GlobalUnlock (*phMatch);
	}
	else if (IMunic)
	{  
		ULAddKey.Munic = 0; 
		if (!BT_FIND (hBTUserDefinedAddress,(LPSTR)&ULAddKey,BT_FIRST,BT_EQ,(LPSTR)pMatch))
		{  
			rtn = TRUE; 
			GlobalUnlock (*phMatch);
		}
	}
	else
	{  
		if (!BT_FIND (hBTUserDefinedAddress,(LPSTR)&ULAddKey,BT_FIRST,BT_GE,(LPSTR)pMatch))
		{  
			if (!_fstricmp (Street,ULAddKey.Street) && IHouse == ULAddKey.HouseNum)
			{
				rtn = TRUE; 
				GlobalUnlock (*phMatch);
			}
		}
	}
	if (!rtn)
		GSSiGlobUlFree (phMatch);
	CloseUserDefinedAddress (OpenedUAA);
	return rtn;
} 

BOOL AddMapQuestMatch (HWND hWndDlg,UINT icntl,LPSTR FullAddress)
{   
	int	item;
	char	str[256], Quality[32];
	DPOINT	Point;
	BOOL	rtn;
	
	if ((rtn=GetMapQuestLocation (FullAddress,Quality,&Point,3)))
	{
		sprintf (str,"MapQuest location\tI%10ld%20.8f%20.8f",0,Point.x,Point.y);
		item=SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);  
		//SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETITEMDATA,item,(LPARAM)imatch); 
	}
	return rtn;
}

BOOL AddZipCenterMatch (HWND hWndDlg,UINT icntl,long ZipCode)
{   
	int	item;
	long	imatch=-9;     
	char	DBName[256]="ZIPCENTER=";
	char	SQL[64];
	char	str[256];
	char	CenterPoint[128]="[ZIPCENTER.X] [ZIPCENTER.Y]";
	HANDLE	hDB=0;
	
	if (!GetGlobalCVal ("[%ZIPCENTERDB]",_fstrchr(DBName,0),NULL))
		return FALSE;
    sprintf (SQL,"ZIPCODE=%ld",ZipCode);
    if (!OpenDataFile (DBName,SQL,BT_READ,&hDB))
    	return FALSE;
	ExpandText (CenterPoint);
    CloseDataFile (TRUE,&hDB); 
	sprintf (str,"Match to ZipCode %ld\tZ%10ld%s",ZipCode,ZipCode,CenterPoint);
    item=SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);  
    SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETITEMDATA,item,(LPARAM)imatch);  
	return TRUE;
}

short AddPrecinctCenterMatch (HWND hWndDlg,UINT icntl,long ZipCode)
{
	short nPcts=0;
	
	return nPcts;
}




  
   
BOOL GetZIPBounds (long Zip,LPMNMXCORD pBounds)
{   
	if (!hZIPBounds)
		return FALSE;
	if (BT_FIND (hZIPBounds,(LPSTR)&Zip,BT_FIRST,BT_EQ,(LPSTR)pBounds))
		return FALSE;
	return TRUE;
}

BOOL AddPointToZIPBounds (long Zip,LPDPOINT pPoint)
{   
	MNMXCORD	Bounds;
	
	if (BT_FIND (hZIPBounds,(LPSTR)&Zip,BT_FIRST,BT_EQ,(LPSTR)&Bounds))
		DBoundsInit (&Bounds); 
	AddDPointToMinMax (pPoint,&Bounds);
	BT_PUT (hZIPBounds,(LPSTR)&Zip,(LPSTR)&Bounds);
	return TRUE;
}

void CloseZIPBounds (BOOL Opened)
{  
    if (!Opened) return;
	if (hZIPBounds)
		BT_CLOSE (hZIPBounds);
	hZIPBounds = 0;
	return;
}

BOOL OpenZIPBounds (LPBOOL pOpened)
{ 
    char	File[128];
    BTVARDESC   BTVar[2];
    BOOL	Opened, OpenedSeg;
	NETINTREFKEY	NetIntRefKey; 
	NETINTREFDATA	NetIntRefData;
	SEGDATAGM		Segdata;
	short	pos = BT_FIRST; 
	long	TotLen,CurLoc=0;
    
    *pOpened = 0;
    if (hZIPBounds)
    	return TRUE;
    SetAddressDir();
    sprintf (File,"%s\\zipbound.btr",AddMatchDir);
    hZIPBounds = BT_OPEN (File,0,BT_READ,0);
    if (hZIPBounds)
    {	
    	*pOpened = 1;
    	return TRUE; 
    }
    if (!OpenNetIntersect (NetworkID,FALSE,&Opened))
    	return FALSE; 
    if (!OpenStreetSegmentTable (FALSE,&OpenedSeg))
    	return FALSE;
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (File, sizeof(MNMXCORD), FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	hZIPBounds = BT_OPEN (File, 0, BT_WRITE, 0); 
	TotLen = BT_NUM_IN_INDEX (hBTNetIntRef);
	CreateStatusWind (hWndMain,1,"Create ZIPCode Bounds Table");
	while (ContinueProcessing && !BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,pos,BT_ANY,(LPSTR)&NetIntRefData))
	{
		long	LastZIP = 0; 

		pos = BT_NEXT; 
		if (GetSegDataGM (NetIntRefKey.Refno,&Segdata))
		{
			if (Segdata.ZIPL) 
				AddPointToZIPBounds (Segdata.ZIPL,&NetIntRefData.Coord);
			if (Segdata.ZIPR && Segdata.ZIPR != Segdata.ZIPL)
				AddPointToZIPBounds (Segdata.ZIPR,&NetIntRefData.Coord); 
		}
		StatusWindowUpdate (NULL,NULL, TotLen, ++CurLoc);
    } 
    ContinueProcessing = TRUE;
	CloseNetIntersect (Opened); 
	CloseStreetSegmentTable(OpenedSeg); 
	BT_CLOSE (hZIPBounds);
    hZIPBounds = BT_OPEN (File,0,BT_READ,0);  
	DestroyStatusWindow(0);  
    *pOpened = 1;
	return TRUE;    
}

void AddIntMunic (long Munic,LPSHORT pnMunics,LPLONG Munics,LPSHORT MunicCounts)
{   
	short	i;
	
	if (Munic)
	{
		for (i=0;i<*pnMunics;i++)
		{
			if (Munic == Munics[i]) 
			{
				MunicCounts[i]++;
				return; 
			}
		}
		if (*pnMunics > 4)
			return;	
		Munics[*pnMunics] = Munic;
		MunicCounts[(*pnMunics)++] = 1;
	}
	return;
}

HANDLE AddMunicsToInts (HANDLE hBTNetIntPaths,LPSTR File)
{
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData;   
	NETINTPATHSKEY	NetIntPathsKey;     
	SEGDATAGM		Segdata;
	DPOINT	IntPoint;    
	INTPATHSDATA	IPD;
	//BTHEAD BTHead;  
    BTVARDESC   BTVar[5];
    char	NewFile[144];   
    LPSTR	lpName;   
    HANDLE	hBTNew;   
    short	pos=BT_FIRST, nMunics, st, MaxMC,i,j,n, MunicCounts[8]; 
    long	TotLen,CurLoc=0, Munics[8], FIPSCode;  
    BOOL	OpenSeg;
	
	//GetBTHeader (hBTNetIntPaths,&BTHead);
	if (GetBT_DATLEN (hBTNetIntPaths) == sizeof(INTPATHSDATA))
		return hBTNetIntPaths;
    
	OpenStreetSegmentTable (FALSE,&OpenSeg);
    TotLen = BT_NUM_IN_INDEX (hBTNetIntPaths);
	CreateStatusWind (hWndMain,1,"Add Munics to Intersections");
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=4;
	BTVar[2].BT_VAROFF=8; 
	_fstrcpy (NewFile,File);
	lpName = _fstrstr (NewFile,"paths.btr");
	_fstrcpy (lpName,"path2.btr");
	BT_CREATE (NewFile, sizeof(INTPATHSDATA), FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hBTNew = BT_OPEN (NewFile,0,BT_WRITE,0);
	while (!BT_FIND (hBTNetIntPaths,(LPSTR)&NetIntPathsKey,pos,BT_ANY,(LPSTR)&IntPoint))
	{
		pos = BT_NEXT;  
		IPD.Point = IntPoint; 
		_fmemset (IPD.Munics,0,sizeof(IPD.Munics));   
		NetIntRefKey.IntID = NetIntPathsKey.IntID;
		NetIntRefKey.Refno = LONG_MIN;  
		nMunics = 0;
		st = BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_FIRST,BT_GE,(LPSTR)&NetIntRefData);
		while (nMunics < 5 && !st && (NetIntRefKey.IntID == NetIntPathsKey.IntID))
		{
			if (GetSegDataGM (NetIntRefKey.Refno,&Segdata))
			{   
        		FIPSCode = Segdata.COUNTYL * 100000 + Segdata.FMCDL;
				if (Segdata.COUNTYL && Segdata.FMCDL)
					AddIntMunic (FIPSCode,&nMunics,Munics,MunicCounts);
        		FIPSCode = Segdata.COUNTYR * 100000 + Segdata.FMCDR;
				if (Segdata.COUNTYR && Segdata.FMCDR)
					AddIntMunic (FIPSCode,&nMunics,Munics,MunicCounts); 
			} 
			st = BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_NEXT,BT_ANY,(LPSTR)&NetIntRefData);
		} 
		n = 0;
		do
		{   
			MaxMC = 0;
			j = -1;
			for (i=0;i<nMunics;i++)
			{
				if (MunicCounts[i] > MaxMC)
				{
					j=i;
					MaxMC = MunicCounts[i];
				}
			} 
			if (j > -1)
			{
				IPD.Munics[n++] = Munics[j];
				MunicCounts[j] = 0; 
			}
		}	while (j > -1);
		BT_PUT (hBTNew,(LPSTR)&NetIntPathsKey,(LPSTR)&IPD);
		StatusWindowUpdate (NULL,"", TotLen, ++CurLoc);
	}
	BT_CLOSEANDDELETE (&hBTNetIntPaths);
	BT_CLOSE (hBTNew);
	GSSiRename  (NewFile,File);
	hBTNew = BT_OPEN (File,0,BT_READ,0);   
	CloseStreetSegmentTable (OpenSeg);
	DestroyStatusWindow(0);  
	return hBTNew;
} 

short GetNearHouse (long StreetNum,long WantHouse,long Munic,long ZIP,LPLONG pNearHouse,LPDPOINT NearPoint)
{
	SEGMAXKEY	SegMaxKey; 
	long		MinHouseNum; 
	short		stMax, n;
    long		MinAdd, MaxAdd, Dir, LowAdd, HighAdd; 
    short		LocationCode=1;  
    short		nHouse=0, Side; 
    BOOL		OpenedSeg, OpenedSM; 
    LPLONG		pZIPs;
    short		NumZIPs;
    HANDLE		hZIPs=0;
	
	if (!OpenStreetSegmentTable (FALSE,&OpenedSeg))
		return 0;
	if (!OpenSegMaxIndex (&OpenedSM))
		goto Exit; 
	if (ZIP || !Munic)
	{
		hZIPs = GSSiGlobAlloc (0,GMEM_MOVEABLE,4);
		pZIPs = (LPLONG)GlobalLock (hZIPs);
		*pZIPs = ZIP;
		GlobalUnlock (hZIPs);
		NumZIPs = 1;         
	}
	else
		NumZIPs = GetNumZIPsInMunic (Munic,&hZIPs);
	
	if (hZIPs)
	{
		pZIPs = (LPLONG)GlobalLock (hZIPs);
		while (NumZIPs--)
		{   
			ZIP = *pZIPs++;
			for (Side=1;Side<3;Side++)  
			{
				SegMaxKey.StreetNum = StreetNum;
				SegMaxKey.ZIPCode = ZIP;
				SegMaxKey.Side = Side;
				SegMaxKey.MaxHouseNum = WantHouse;
				SegMaxKey.Segid = LONG_MIN;  
				if (!BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_FIRST,BT_GE,(LPSTR)&MinHouseNum))
				{
					if (SegMaxKey.StreetNum == StreetNum && SegMaxKey.ZIPCode == ZIP && SegMaxKey.Side == Side) 
					{
						short	NumMatch=0;
						HANDLE	hMatch=0;   
						
						pNearHouse[nHouse] = MinHouseNum;	
						if (MatchAddress3 (0,StreetNum,MinHouseNum,SegMaxKey.ZIPCode,SegMaxKey.ZIPCode,0, FALSE,&NumMatch,&hMatch,FALSE,TRUE))
						{   
							LPADDMATCH	pMatch = (LPADDMATCH)GlobalLock (hMatch);
							NearPoint[nHouse++] = pMatch->Point;
							GSSiGlobUlFree (&hMatch);
						}
					}
					if (!BT_FIND (hSegMax,(LPSTR)&SegMaxKey,BT_PRIOR,BT_ANY,(LPSTR)&MinHouseNum))
					{
						if (SegMaxKey.StreetNum == StreetNum && SegMaxKey.ZIPCode == ZIP && SegMaxKey.Side == Side) 
						{   
							short	NumMatch=0;
							HANDLE	hMatch=0;
							
							pNearHouse[nHouse] = SegMaxKey.MaxHouseNum;
							if (MatchAddress3 (0,StreetNum,SegMaxKey.MaxHouseNum,SegMaxKey.ZIPCode,SegMaxKey.ZIPCode,0, FALSE,&NumMatch,&hMatch,FALSE,TRUE))
							{   
								LPADDMATCH	pMatch = (LPADDMATCH)GlobalLock (hMatch);
								NearPoint[nHouse++] = pMatch->Point;
								GSSiGlobUlFree (&hMatch);
							}
			
						}
					}
				}
			}
		}
		GSSiGlobUlFree (&hZIPs);
	}		
Exit:
	CloseStreetSegmentTable (OpenedSeg); 
	CloseSegMaxIndex (OpenedSM);
	return nHouse;
}
 
