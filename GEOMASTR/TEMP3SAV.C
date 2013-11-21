#include "graphint.h"
#include "gmextern.h"

//DWORD FAR PASCAL LoadLibraryEx32W(LPCSTR lpszLibFile, DWORD hFile, DWORD dwFlags);
//BOOL FAR PASCAL FreeLibrary32W( DWORD );
//DWORD FAR PASCAL GetProcAddress32W( DWORD, LPCSTR );
//DWORD FAR PASCAL CallProc32W( DWORD, DWORD, LPVOID, DWORD, DWORD );
extern	void	MyOpen (void);
static	short  NextFunID=1;
typedef void (FAR PASCAL *MYPROC)(LPSTR);

/*short GetTraceFunID (LPSTR line)
{
	short	id=0; 
	char	str[512];
	static	BOOL	First=TRUE; 
	char	File[]="c:\\gssi\\prog\\geomastr\\funids.txt";
	HFILE	Fid;
	
	if (First)
	{
		Fid=GSSiOpenFile (File,NULL,OF_READ);
		if (Fid != HFILE_ERROR)
		{
			while (fgetstring (str,510,Fid))
				id = max (id,atoi (str));
			GSSiClose (Fid);
		}
		else
			id = 0;
		NextFunID = id+1;
	} 
	First = FALSE;
	
	id = NextFunID++;  
	
	sprintf (str,"%i %s",id,line);
	AppendFile (File,str);
	return id;
}

BOOL AddEnableTrace (LPSTR Name)
{   
	char	line[512], NameNew[128], NameOld[128],lastline[512],str[1024];
	long	lineno=0;
	HFILE	Fid=GSSiOpenFile (Name,NULL,OF_READ);
	HFILE	FidNew;
	LPSTR	pDot, pchr, pLoc;
	short	nbrace=0, opt, funid; 
	BOOL	IsEndProg=FALSE, WriteLine, literal = FALSE;  
	char	tabs[]="\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	
	_fstrcpy (NameNew,Name);
	pDot = _fstrrchr (NameNew,'.');
	_fstrcpy (pDot,".new");
	_fstrcpy (NameOld,Name);
	pDot = _fstrrchr (NameOld,'.');
	_fstrcpy (pDot,".old");
	FidNew = GSSiOpenFile (NameNew,NULL,OF_CREATE);
	
	while (fgetstring (line,510,Fid))
	{    
		WriteLine = TRUE;
		pchr = line;
		while (*pchr)
		{   
			if (literal)
				literal = FALSE;
			else if (*pchr == '\\')
				literal = TRUE;
			else if (*pchr == '{')
			{
				if (!nbrace)
				{
					sprintf (str,"%s\r\n%s",lastline,line);
					opt = MessageBox (NULL,str,"Function",MB_YESNOCANCEL);
					switch (opt)
					{           
						case IDYES:
							funid = GetTraceFunID (lastline);  
			EnterProg:
							sprintf (str,"%s#if ENABLETRACE\r\n%s{GSSiEnterProg (%i);\r\n%s#endif",tabs,tabs,funid,tabs);
							fputstring (str,FidNew); 
							IsEndProg = TRUE;
						break;
						case IDNO:
							funid = GetTraceFunID (line); 
							goto EnterProg;
						break;
						case IDCANCEL:
							IsEndProg = FALSE;
						break;
					}
				}
				nbrace++;
			}
			else if (*pchr == '}') 
			{
				nbrace--;
				if (!nbrace && IsEndProg)
				{
					sprintf (str,"%s#if ENABLETRACE\r\n%s}\r\n%s#endif",tabs,tabs,tabs);
					fputstring (str,FidNew); 
				} 
			}
			pchr++;
		}
		if ((pLoc = _fstrstr (line,"return")))
		{   
			pLoc+=6;
			if (*pLoc == ' ' || *pLoc == ';' || *pLoc == '(' || *pLoc == '\t')
			{
				sprintf (str,"%s\r\n%s",lastline,line);
	//			opt = MessageBox (NULL,str,"Return",MB_YESNOCANCEL);
				opt = IDYES;
				switch (opt)
				{           
					case IDNO:  
						fputstring ("fix next line",FidNew);
					case IDYES:
						sprintf (str,"{\r\n%s#if ENABLETRACE\r\n%sGSSiExitProg (%i);\r\n%s#endif",tabs,tabs,funid,tabs);
						fputstring (str,FidNew); 
						fputstring (line,FidNew);
						fputstring ("}",FidNew);
						WriteLine = FALSE;
					break;
					case IDCANCEL:
					break;
				}
			}
		}
		_fstrcpy (lastline,line);
		if (WriteLine)
			fputstring (line,FidNew);
	}
	GSSiClose (Fid);
	GSSiClose (FidNew);
	GSSiRename (Name,NameOld);
	GSSiRename (NameNew,Name);
	return TRUE;
} */

/*BOOL AddExtern (LPSTR pStr,HFILE Fid,LPSTR FileName,short Lineno)
{   
	char	Type[512], VarDef[512], VarName[512], extline[512], NewVarName[512], mess[256];
	LPSTR	pEnd, pBrack;
	short	ii;
	
	pStr += 7;
	if ((pEnd = _fstrchr (pStr,' ')))
	{
		*pEnd++ = 0;
		_fstrcpy (Type,pStr);
		pStr = pEnd;
	}
	else
	{
		sprintf (mess,"Error in %s at line %i",FileName,Lineno);
		MessageBox (0,mess,NULL,MB_ICONEXCLAMATION);
		return FALSE;
	}
	if (!_fstrcmp (Type,"int"))
		_fstrcpy (Type,"short");
	if (!_fstrcmp (Type,"LONG"))
		_fstrcpy (Type,"long");
	Strip (pStr,' ');
	*LastChr (pStr) = ',';
	while ((pEnd = _fstrchr (pStr,',')))
	{   
		*pEnd++ = 0;
		_fstrcpy (VarDef,pStr);
		pStr = pEnd; 
		_fstrcpy (NewVarName,VarDef);
		if ((pBrack = _fstrchr (NewVarName,'[')))
			*pBrack = 0;
		_llseek (Fid,0,0);
		while (fgetstring (extline,500,Fid))
		{
			LPSTR pType = _fstrchr (extline,' ')+1;
			LPSTR pTypeEnd = _fstrchr (pType,'\t');
			LPSTR pVarDef = _fstrrchr (extline,'\t'); 
			
			*pTypeEnd = 0;
			*pVarDef++ = 0;
			*LastChr (pVarDef) = 0;
			_fstrcpy (VarName,pVarDef);
			if ((pBrack = _fstrchr (VarName,'[')))
				*pBrack = 0;  
			if (!_fstrcmp (VarName,NewVarName))
			{   
				if (_fstrcmp (Type,pType))
				{
					sprintf (mess,"Typedef Error %s:%s:%s in %s at line %i",VarName,Type,pType,FileName,Lineno); 
//					MessageBox (0,mess,NULL,MB_ICONEXCLAMATION);
					return FALSE;
				}
				if (_fstrcmp (VarDef,pVarDef))
				{
					sprintf (mess,"Error in %s at line %i",FileName,Lineno); 
					MessageBox (0,mess,NULL,MB_ICONEXCLAMATION);
					return FALSE;
				}
				goto NextVar;
			}
		}                        
		sprintf (extline,"extern %s\t\t\t%s;",Type,VarDef); 
		if (_fstrchr (extline,'\\'))
			ii=1;
		fputstring (extline,Fid); 
NextVar:;
	}	
	return TRUE;
}   

void CreateSF1FieldFile (void)  
{
	BTVARDESC	BTVar[1];
	short	i,j;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr=0;
    LPFIELDINFO lpFieldInfo; 
	LPGWDHEADER lpGWDHead;
	HANDLE  hDB=0;  
    char    DBName[144], FieldName[16],FieldFile[128]={"[%%DL]attribut\\sf1.fld"};   
    struct	{short	File, Field;} SFFieldData;        
    HANDLE	hFields, hSQL;
    
	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=10;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (FieldFile, sizeof(SFFieldData), FALSE, -1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hFields= BT_OPEN (FieldFile, 0, BT_WRITE, 0);
    hSQL=0;
	
    for (i=0;i<39;i++)
    {   
    	SFFieldData.File = i; 
    	sprintf (DBName,"ODBC|SF1|SF100%2.2i",i+1);
    	if (!OpenDataFile (DBName,"",BT_READ,&hSQL))
    		return;
    	SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
    	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	    lpFieldInfo = &FilePtr->FldInfo;  
	    lpFieldInfo+=4;
	    for (j=0;j<FilePtr->NumFields-4;j++,lpFieldInfo++)
	    {  
	    	SFFieldData.Field = j;
	    	_fstrncpy (FieldName,lpFieldInfo->name,10);
	    	BT_PUT (hFields,FieldName,(LPSTR)&SFFieldData);
	    }
    	GlobalUnlock (SQLPtr->OFHandle);
    	GlobalUnlock (hSQL);
	    CloseDataFile (TRUE, &hSQL);      
    }  
    BT_CLOSE (hFields);
    return;
}

void CreateSF3FieldFile (void)  
{
	BTVARDESC	BTVar[1];
	short	i,j;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr=0;
    LPFIELDINFO lpFieldInfo; 
	LPGWDHEADER lpGWDHead;
	HANDLE  hDB=0;  
    char    DBName[144], FieldName[16],FieldFile[128]={"[%%DL]attribut\\sf3.fld"};   
    struct	{short	File, Field;} SFFieldData;        
    HANDLE	hFields, hSQL;
    
	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=10;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (FieldFile, sizeof(SFFieldData), FALSE, -1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hFields= BT_OPEN (FieldFile, 0, BT_WRITE, 0);
    hSQL=0;
	
    for (i=0;i<76;i++)
    {   
    	SFFieldData.File = i; 
    	sprintf (DBName,"ODBC|SF3|SF300%2.2i",i+1);
    	if (!OpenDataFile (DBName,"",BT_READ,&hSQL))
    		return;
    	SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
    	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	    lpFieldInfo = &FilePtr->FldInfo;  
	    lpFieldInfo+=4;
	    for (j=0;j<FilePtr->NumFields-4;j++,lpFieldInfo++)
	    {  
	    	SFFieldData.Field = j;
	    	_fstrncpy (FieldName,lpFieldInfo->name,10);
	    	BT_PUT (hFields,FieldName,(LPSTR)&SFFieldData);
	    }
    	GlobalUnlock (SQLPtr->OFHandle);
    	GlobalUnlock (hSQL);
	    CloseDataFile (TRUE, &hSQL);      
    }  
    BT_CLOSE (hFields);
    return;
}*/  

/*
BOOL GetGeoLine(HFILE Fid,LPSTR UDI,LPHANDLE phPoints,LPWORD pnPoints,LPMNMXCORD pBounds)
{   
	char	str[130];      
	long	n;
	LPSTR	loc;   
	HPDPOINT	pPoints;
	
	if (!fgetstring (str,128,Fid))
		return FALSE;
	strncpy0 (UDI,&str[13],10);
	*pnPoints = ldread (&str[43],3);
	n = 0; 
	*phPoints = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
	pPoints = GlobalLock (*phPoints);
	while (n<*pnPoints)
	{   
		short	i=0; 
		
		fgetstring (str,128,Fid); 
		loc = str;
		while (n<*pnPoints && i < 4)
		{   
			long	lx,ly;
			
			lx = ldread (loc,10);
			loc += 10;
			ly = ldread (loc,10);   
			pPoints->x = (double)lx/1000000;
			pPoints->y = (double)ly/1000000; 
			AddDPointToMinMax (pPoints,pBounds);
			loc += 10;
			n++; 
			i++; 
			pPoints++;
		}
	} 
	GlobalUnlock (*phPoints);
	return TRUE;
}
*/  

/*void ConvertGSSiAlloc (void)
{
    HFILE Fid1 = GSSiOpenFile ("c:\\gssi\\prog\\geomastr\\ccode2.txt",NULL,OF_READ);
    HFILE Fid2,Fid3;
    char	str[1040], tmp[16], Name[128];  
    short	i,callnum=0;
    LPSTR	pLoc;
    
    while (fgetstring (Name,128,Fid1))
    {   
    	BOOL	First=TRUE; 
		_chdrive (3);
		_chdir ("c:\\gssi\\prog\\geomastr");
	    Fid2 = GSSiOpenFile (Name,NULL,OF_READ); 
	    Fid3 = GSSiOpenFile ("c:\\temp.txt",NULL,OF_CREATE);
	    while (fgetstring (str,1024,Fid2))
	    {   
			REPLAC (str,"GSSiGlobAlloc(","GSSiGlobAlloc (",1024); 
			REPLAC (str,"GSSiGlobAlloc (","GSSiGlobAlloc (****,",1024); 
			if ((pLoc = _fstrstr (str,"GSSiGlobAlloc (")))
			{ 
				callnum++; 
				pLoc+=15;
				sprintf (tmp,"%4i",callnum);
				for (i=0;i<4;i++)
					pLoc[i] = tmp[i];
			}
    		fputstring (str,Fid3);  
	    }
	    GSSiClose (Fid2);
	    Fid2 = GSSiOpenFile (Name,NULL,OF_CREATE);
	    _llseek (Fid3,0,0);
	    while (fgetstring (str,1024,Fid3)) 
	    	fputstring (str,Fid2);
	    GSSiClose (Fid3); 
	    GSSiClose (Fid2); 
    }
    GSSiClose (Fid1); 
    return; 
} */
LPSTR fgetstring2 (LPSTR lpStr, USHORT len, HFILE Fid)
{   short   lrec;
    LPSTR lpEnd; 
    DWORD   loc;
                
    loc = _llseek (Fid,0,1); 
//	LastFGSLoc = loc;    
    lrec = _lread (Fid,lpStr,len+2);
    if (!lrec || lrec == HFILE_ERROR) return 0; 
    lpEnd = lpStr + lrec;
    *lpEnd = 0;
    lpEnd = _fstrchr (lpStr,'\r');
    if (!lpEnd)
	    lpEnd = _fstrchr (lpStr,'\n');
	    if (!lpEnd)
    	    lpEnd = lpStr + lrec;
    *lpEnd++ = 0; 
    if (*lpEnd == '\n') lpEnd++;
    lrec =  lpEnd - lpStr;
//    LastFGSlRec = lrec;
    _llseek (Fid,loc,0);
    _llseek (Fid,lrec,1);
    return lpStr;
}

/*BOOL OpenDGNFile (LPSTR InName,LPMNMXCORD pBounds){return FALSE;};
BOOL CloseDGNFile (void){return FALSE;};
BOOL LoadDGNParm (LPSTR PltName){return FALSE;};
BOOL ReadNextDGNRecord (LPMNMXCORD pBounds){return FALSE;};
BOOL ProcessDGNRecord (HDC hDC){return FALSE;}; */
int SunRiseSet(void);

void MiscFunction (void)
{      	
	HFILE Fid1, Fid2, Fid3, Fid4, Fid;
	OFSTRUCT	OFStruct;
	char	str[4096], Name[132], fullname[128], Mapid[16],munum[64],stcode[64], MCDist[130]; 
	LPSTR	lpDot,pLoc; 
	short	n, symnum=-224, parent = 223, len,ii, pos;  
	double	x1,y1,x2,y2,f=1.0;  
	DPOINT	Point,Point2;    
	extern	long ParcelLoc;
	long	size,tot=0,num=0;   
	long	Offset, Refno;
	HANDLE	hDBPolyID, hDBPolyID2;  
	short	i=-1, lineno;
	long	nFields, nRecs;
	LPUSHORT	pi=&i;  
//	DBFHandle pDBF;  
	short	FieldWidth, FieldDecimals;
//	HANDLE	hLine=GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
//	LPSTR	pLine=GlobalLock (hLine); 
	char	OutFile[128], EdgeFile[128], OrthoFile[128], FieldName[34];
	CITIESDATA4	CitiesData4; 
	HFILE	Cities4FID;
	HANDLE	hCities;
	BTVARDESC	BTVar[2];
	struct
	{	
		DPOINT	LatLong; 
		char	Name[64];
	}CitiesData; 
	struct
	{
		long	pop;
		char	state[4];
		char	Name[64];
	}CitiesKey;
	long	n1=0,n2=0; 
	HANDLE	hSQL = 0; 
	HANDLE	hSymDesc=0;
	short	NumSyms=0; 
	MNMXCORD	Bounds;   
	char	RouteID[64];  
	HANDLE	hPoints;
	WORD	nPoints;   
	short	Symbol; 
   	short   rtn;
  	long	Ref=1;  
  	long	MCPop=0, nMCDist=0;
  	double	MCLat=0, MCLon=0; 
  	DWORD	ghLib;   
  	FARPROC	MyOpenAdd;
  	DWORD	nd=0,ne=1024L*1024L;  
  	long	nw=1024;
  	DWORD	block[256],set=0;
  	USHORT	k;   
  	MYPROC	Proc;
    DWORD temp;
    long	ln=0,STNum;
    char  TestString[256] = "c:\\test.txt";       
    char	STName[64];//testdll[256]="[%DL]test32.dll";      
    double	x;  
    HANDLE	hMem;
    HPSTR	pMem;
    long	l=0,il;  
    DWORD	ld;  
    HANDLE	hbt;
#define JPEG_QUALITYSUPERB  0x80
#define JPEG_QUALITYGOOD    0x100
#define JPEG_QUALITYNORMAL  0x200
#define JPEG_QUALITYAVERAGE 0x400
#define JPEG_QUALITYBAD     0x800    

	Fid1=GSSiOpenFile ("c:\\stname.txt",NULL,OF_READ); 
	Fid2=GSSiOpenFile ("c:\\allstreets.txt",NULL,OF_CREATE);
	fputstring ("STATE\tNUMBER\tLEN\tNAME",Fid2);
	while (fgetstring (str,4090,Fid1)) 
	{   
		short	State = atoi (&str[11]); 
		long	STNum;
		
		hbt = BT_OPEN (str, 0, BT_READ, 0);  
		while (!BT_FIND (hbt,(LPSTR)&STNum,BT_NEXT,BT_ANY,(LPSTR)&STName))
		{ 
			sprintf (str,"%i\t%ld\t%i\t%s",State,STNum,_fstrlen(STName),STName);
			fputstring (str,Fid2);
		}
		BT_CLOSE (hbt);
	}
	GSSiClose (Fid1);  
	GSSiClose (Fid2);
	return;

//LoadMCDS3 (0);
//	hbt = BT_OPEN ("C:\\moxie\\ATTRIBUT\\voters\\voter.in4", 0, BT_WRITE, 0); 
//	BT_CLOSE (hbt);
//	ReadUSR ("C:\\Gssi\\formats\\gps\\testUSRformat\\testingUSR.usr");
/*	Fid1=GSSiOpenFile ("c:\\sanline\\newsanpipe.ctl",NULL,OF_READ); 
	while (fgetstring (str,4090,Fid1))
	{   
		l++;
		if (str[0] == '#' && str[1] != '^')
		{   
			double 	z1;  
			LPSTR	pMSL;
			
			if (sscanf (&str[1],"%Flf|%Flf|%Flf|%Flf|%Flf",&Point.x,&Point.y,&z1,&Point2.x,&Point2.y) == 5)
			{ 
				fgetstring (str,4090,Fid1); 
				while (*str != '#')
					fgetstring (str,4090,Fid1); 
				l++;
				pMSL=_fstrchr (&str[3],'^');
				*pMSL=0;
				if (ldistp (Point,Point2) < 0.0001)    
				{
					AppendFile ("c:\\zerolenpipes.txt",&str[3]);
					num++;                                   
				}
			}
		}
	}
	
	GSSiClose (Fid1); */ 
//SunRiseSet (); 
/*	Fid1=GSSiOpenFile ("F:\\lakscout\\tiffiles\\b0381011\\b0381011.cpt",NULL,OF_READ);
	Fid2=GSSiOpenFile ("F:\\lakscout\\tiffiles\\b0381011\\b038101x.cpt",NULL,OF_CREATE);  
	while (fgetstring (str,128,Fid1))
	{
		if (*str != '#')
		{
			double	ix,iy;
			double	dx,dy;
			
			sscanf (str,"%Flf %Flf %Flf %Flf",&ix,&iy,&dx,&dy);
			ix /=2;
			iy /=2;
			sprintf (str,"%f %f %f %f",ix,iy,dx,dy);
			fputstring (str,Fid2);
		}
	}
	GSSiClose (Fid1);
	GSSiClose (Fid2);
	return; */
/*{
	DPOINT FromPoint,ToPoint,TrannedPoint;
	HANDLE hTran = LoadTranFileWithDandT ("c:\\gmupdate\\travtran.cpt(F,3)"); 
	double	d;
	short	n=0;
	
	Fid = GSSiOpenFile ("c:\\gmupdate\\travtran.cpt",NULL,OF_READ);
	
	while (fgetstring (str,128,Fid))
	{   
		n++;
		sscanf (str,"%Flf %Flf %Flf %Flf",&FromPoint.x,&FromPoint.y,&ToPoint.x,&ToPoint.y);
		TrannedPoint = TranPoint (&FromPoint,hTran);
		d = ldistp (ToPoint,TrannedPoint);
		if (d > 0.5)
			ii=1;	
	}
	GSSiClose (Fid);  
	CloseTRANS2 (&hTran);
	return;  
}*/
	 
/*{
	DWORD Width,Height;
	short ColorSpace,NumBands,DataType,IsLocked;
	double MinMag,MaxMag,ULX,ULY,XRes,YRes,XRot,YRot,Mag=1,x,y;  
	HDIB32 hImage;

	DWORD ImageHandle=MrSidOpen ("F:\\naip\\naip03_dakota\\ortho_e1-1_mn037.sid");
	DWORD st = MrSidGetInfo (ImageHandle,&Width,&Height,&ColorSpace,&NumBands,&DataType,&MinMag,&MaxMag,
					   &IsLocked,&ULX,&ULY,&XRes,&YRes,&XRot,&YRot);
	x = ULX + Width/2;
	y = ULY - Height/2;
	Width = 10;
	Height = 10;	
	hImage = MrSidGetImage (ImageHandle,&ULX,&ULY,&Width,&Height,&Mag); 
	st = MrSidClose (ImageHandle);
}*/ 
//	_fstrcpy (str,"\t");
	return;
/*	Fid1 = GSSiOpenFile ("z:\\tempdtm.bin",NULL,OF_READ);
	l = _llseek (Fid1,0,2);
	ld = (DWORD)_llseek (Fid1,0,2);
	GSSiClose (Fid1);*/
/*	Fid1 = GSSiOpenFile ("c:\\gmnosock.exe",NULL,OF_READWRITE);
	l = _llseek (Fid1,0,2);
	_llseek (Fid1,0,0);
	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,l);
	pMem = GlobalLock (hMem);
	BigRead (Fid1,pMem,l);  
	for (il=0;il<l;il++)
	{
		if (!_fstrcmp (&pMem[il],"2302"))   
		{
			_fstrcpy (&pMem[il],"2314");
			break;
		}
		if (!_fstrcmp (&pMem[il],"LakeMaster MN/WI"))
			_fstrcpy (&pMem[il],"LakeMaster LKOTW");
	}
	_llseek (Fid1,0,0);
	BigWrite (Fid1,pMem,l,-1);
	GSSiGlobUlFree (&hMem);*/
/*	x = atan2 (1,2);  
	x = atan (1.0/2.0);
	x = atan2 (-1,2);  
	x = atan (-1.0/2.0);
	x = atan2 (-1,-2);  
	x = atan (-1.0/-2.0);
	x = atan2 (1,-2);  
	x = atan (1.0/-2.0);
	Fid1 = GSSiOpenFile ("e:\\lidar_s.txt",NULL,OF_READ);
	for (i=0;i<100;i++)
	{
		fgetstring (str,128,Fid1);
		ln+=_fstrlen (str);
	} 
	GSSiClose (Fid1);*/
/*	HDIB hDIB = CopyWindowToDIB(hWndMain,PW_WINDOW);
	rtn = BMPToEXT (hDIB,"c:\\test.jpg",JPEG_QUALITYSUPERB); 
    DestroyDIB (hDIB);
    return; */
/*    {   
    	HANDLE	hMem;
    	double	Extents[6];
    	DWORD	st,hDGN = DGN7Open ("c:\\mpls83\\dgn\\storm.dgn",0);
		LPDGNElementCore pElement;       
        st = DGN7GetExtents (hDGN,Extents);
        hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,2000000L);
        pElement = GlobalLock (hMem);
        while (DGN7ReadElement (hDGN,pElement))
        {
        	ii = pElement->type;  
        }
        DGN7Close (hDGN);  
        GSSiGlobUlFree (&hMem);
    }*/
  	
/*  	Fid = GSSiOpenFile ("f:\\test.bin",NULL,OF_READ); 
  	nd = 3 * 1024L * ne;
  	GSSillseek2 (Fid,nd,0);
  	_hread (Fid,block,1024);
  	return;
  	
  	while (nw == 1024)
  	{   
  		for (k=0;k<256;k++)
  			block[k] = set;
  		set++;
  		nw = _hwrite (Fid,block,1024);
  		nd+=nw; 
  	}
  	_lclose (Fid); */
  	
//    DWORD crd = CRC16 ("M000001052mS4VMSP404    1021997010407        165 453775204", 48);

// Load the Win32-based DLL from the 16-bit code 
/*ExpandText (testdll);
if( NULL == (ghLib = LoadLibraryEx32W(testdll, NULL, 0 )) ) 
{
    MessageBox( NULL, "Cannot load DLL32", "App16", MB_OK );
    return;
}  
   Proc = (MYPROC) GetProcAddress32W( ghLib, "myPrint" );
   if( Proc != NULL ) {
  		temp = (DWORD) hWndMain | 0xffff0000;
   		CallProcEx32W( 2, 2 ,(DWORD)Proc, (DWORD) TestString, (DWORD) temp);
   }
   else MessageBox( hWndMain, "Cannot call DLL function", "App16", MB_OK );
   FreeLibrary32W( ghLib );*/
/*	_fstrcpy (Name,"CHECKSNO:LM999903017:46HIZG-13NPDAF-4-J12EFB");  
	{   
		LPSTR pBuffer = Name;
		LPSTR pSNO = pBuffer + 9;
		LPSTR	SerializationCode = _fstrchr (pSNO,':'); 
		if (SerializationCode)
		{
			*SerializationCode++ = 0;
			{
				short st = CheckLMCode (pSNO,FALSE); 
				return; 
			}
		}
	}*/  

	return;
/*    ghLib = LoadLibrary( "tst32dll.dll" );
    if( ghLib < 32 ) 
    {
      MessageBox( 0,
            "Make sure your PATH contains gctp32.DLL",
            "DLL16",  MB_ICONEXCLAMATION );
      return;
    } */

//    MyOpenAdd = (FARPROC) GetProcAddress( ghLib, "MyOpen" );
//    if(MyOpenAdd == NULL )
//    	ii=1;
//    return;
//				LoadMCDS(0);
    
//    ConvertGSSiAlloc ();
    
/*
    Fid1 = GSSiOpenFile ("c:\\usdata\\mexico\\BTS\\mexhwy\\mexhwy.geo",NULL,OF_READ);
    DBoundsInit (&Bounds);
    while (GetGeoLine(Fid1,RouteID,&hPoints,&nPoints,&Bounds))
    	GSSiGlobFree (&hPoints);
	_fstrcpy (Name,"[%DL]maplib\\mexhwy.plt");
	CreateNewMap (Name,&Bounds,0,NULL,0,NULL,0,0,TRUE); 
	EditBounds = CurView->FileMNMX;
	Symbol = GetDictSymbolNumber ("A01");
	AddToSymList (Symbol,&NumSyms,&hSymDesc); 
	_llseek (Fid1,0,0);
    while (GetGeoLine(Fid1,RouteID,&hPoints,&nPoints,&Bounds))
    {
		        
		rtn=AddPolyToMap (1,&nPoints, &hPoints,1,Ref++,NULL,2,Symbol,NULL,"ROUTEID",RouteID,-1,-1,-1,0,0,0,0,TRUE); 
		GSSiGlobFree (&hPoints);
    }
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,NULL); 
    DestroySymList (&NumSyms,&hSymDesc);   
    GSSiClose (Fid1);
	return;
*/
/*    
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_CHAR;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_CHAR;
	BTVar[2].BT_VARLEN=64;
	BTVar[2].BT_VAROFF=8;
	BT_CREATE ("c:\\temp.btr", sizeof(CitiesData), FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hCities = BT_OPEN ("c:\\temp.btr", 0, BT_WRITE, 0);
	Cities4FID = GSSiOpenFile ("[%DL]cities4.dat",&OFStruct,OF_READ);  
	while (_lread (Cities4FID,&CitiesData4,sizeof(CitiesData4)) == sizeof(CitiesData4))
	{   
		n1++;
		CitiesKey.pop = -CitiesData4.Pop;
		CitiesData.LatLong = CitiesData4.LatLong;
		_fstrncpy (CitiesKey.state,CitiesData4.State,4);
		_fstrncpy (CitiesKey.Name,CitiesData4.Name,64);
		_fstrncpy (CitiesData.Name,CitiesData4.Name,64);
    	BT_PUT (hCities,(LPSTR)&CitiesKey,(LPSTR)&CitiesData); 
	}
	GSSiClose (Cities4FID); 

    if (!OpenDataFile ("C:\\mexcity.TXT","",BT_READ,&hSQL))
        return;      
    
    Fid1 = GSSiOpenFile ("c:\\mexcity2.txt",NULL,OF_CREATE);
    fputstring ("\"MUNUM\",\"MUNAME\",\"STCODE\",LON,LAT,TOTPOP",Fid1);

    while (FetchDBRec (hSQL))
    {
        GetValFromOpenFiles ("MUNAME",CitiesKey.Name);
        UpcaseFirst (CitiesKey.Name);
        GetValFromOpenFiles ("TOTPOP",str);   
        CitiesKey.pop = -atol (str);
        GetValFromOpenFiles ("LAT",str);   
        CitiesData.LatLong.y = atof (str);
        GetValFromOpenFiles ("LON",str);   
        CitiesData.LatLong.x = atof (str);
		_fstrncpy (CitiesData.Name,CitiesKey.Name,64);
		_fstrcpy (CitiesKey.state,"MEX");
    	BT_PUT (hCities,(LPSTR)&CitiesKey,(LPSTR)&CitiesData);
        GetValFromOpenFiles ("MUNUM",munum); 
        GetValFromOpenFiles ("STCODE",stcode);  
        sprintf (str,"\"%s\",\"%s\",\"%s\",%f,%f,%ld",munum,CitiesKey.Name,stcode,CitiesData.LatLong.x,CitiesData.LatLong.y,-CitiesKey.pop); 
        fputstring (str,Fid1);    
	    Fid2 = GSSiOpenFile ("c:\\mexcityd.txt",NULL,OF_READ); 
	    while (fgetstring (MCDist,128,Fid2))
	    {   
	    	if (!_fstricmp (stcode,"D-F"))
	    		ii=1;
	    	if (!_fstricmp (MCDist,CitiesKey.Name) && !_fstricmp (stcode,"D-F"))
	    	{   
	    		MCLat += CitiesData.LatLong.y*CitiesKey.pop;
	    		MCLon += CitiesData.LatLong.x*CitiesKey.pop;
	    		MCPop += CitiesKey.pop;
	    		nMCDist++;
	    		break;
	    	}
	    }
	    GSSiClose (Fid2);
        
    }
    CloseDataFile (TRUE, &hSQL); 
    _fstrcpy (CitiesKey.Name,"Mexico City");
    CitiesKey.pop = MCPop;
    CitiesData.LatLong.y  = MCLat/MCPop;
    CitiesData.LatLong.x  = MCLon/MCPop; 
	_fstrncpy (CitiesData.Name,CitiesKey.Name,64);
	_fstrcpy (CitiesKey.state,"MEX");
   	BT_PUT (hCities,(LPSTR)&CitiesKey,(LPSTR)&CitiesData);
    sprintf (str,"\"%s\",\"%s\",\"%s\",%f,%f,%ld",munum,CitiesKey.Name,stcode,CitiesData.LatLong.x,CitiesData.LatLong.y,-CitiesKey.pop); 
    fputstring (str,Fid1);
    GSSiClose (Fid1);
	Cities4FID = GSSiOpenFile ("[%DL]cities4n.dat",&OFStruct,OF_CREATE);  
	pos = BT_FIRST;
	while (!BT_FIND (hCities,(LPSTR)&CitiesKey,pos,BT_ANY,(LPSTR)&CitiesData))
	{   
		n2++;
		CitiesData4.Pop = -CitiesKey.pop;
		CitiesData4.LatLong = CitiesData.LatLong;
		_fstrncpy (CitiesData4.State,CitiesKey.state,4);
		_fstrncpy (CitiesData4.Name,CitiesData.Name,64);
		_lwrite (Cities4FID,&CitiesData4,sizeof(CitiesData4));
		pos = BT_NEXT;
	}
    BT_CLOSE (hCities);  
    GSSiClose (Cities4FID); 
*/
return;
/*	
	Fid1 = GSSiOpenFile ("c:\\vanzan\\dgn\\maps.txt",NULL,OF_READ);
	while (fgetstring (Mapid,8,Fid1))
	{
		sprintf (OutFile,"c:\\vanzan\\tranfile\\%s.cpt",Mapid);
		sprintf (OrthoFile,"c:\\vanzan\\orthtran\\%s.cpt",Mapid);
		sprintf (EdgeFile,"c:\\vanzan\\edgemat\\%s.txt",Mapid);
		Fid3 = GSSiOpenFile (EdgeFile,NULL,OF_READ); 
		if (Fid3 == HFILE_ERROR) 
		{
			continue;
			MessageBox (NULL,EdgeFile,NULL,MB_OK);
		}
		Fid2 = GSSiOpenFile (OutFile,NULL,OF_CREATE); 
		fputstring ("# from edge match",Fid2);
		while (fgetstring (str,128,Fid3))
			fputstring (str,Fid2);
		GSSiClose (Fid3);
		Fid3 = GSSiOpenFile (OrthoFile,NULL,OF_READ); 
		if (Fid3 != HFILE_ERROR)
		{   
			fputstring ("# from ortho match",Fid2);
			while (fgetstring (str,128,Fid3))
			{	char	newstr[256];
				LPSTR	pSpace = _fstrchr (str,' ');
				pSpace++;
				pSpace = _fstrchr (pSpace,' ');
				*pSpace++ = 0;
				sprintf (newstr,"%s %s",pSpace,str);
				fputstring (newstr,Fid2);
			}
			GSSiClose (Fid3);
		}
		GSSiClose (Fid2);
	}
	GSSiClose (Fid1);
	return;
	
//	CreateSF1FieldFile ();      
//	CreateSF3FieldFile ();      
    return; 
*/
/*    Fid1 = GSSiOpenFile ("c:\\gssi\\prog\\geomastr\\ccode2.txt",NULL,OF_READ);
    Fid4 = GSSiOpenFile ("c:\\gssi\\prog\\include\\gmextern.h",NULL,OF_CREATE);
    while (fgetstring (Name,128,Fid1))
    {   
    	BOOL	First=TRUE; 
		_chdrive (3);
		_chdir ("c:\\gssi\\prog\\geomastr");
	    Fid2 = GSSiOpenFile (Name,NULL,OF_READ);
		_chdrive (5);
		_chdir ("e:\\gssi\\prog\\geomastr");
		_fullpath (fullname,Name,128);
	    Fid3 = GSSiOpenFile (fullname,NULL,OF_CREATE);  
	    lineno = 0;
	    while (fgetstring (str,1024,Fid2))
	    {   
	    	lineno++;
	    	if (!_fstrnicmp (str,"extern",6) && !_fstrchr (str,'(')) 
	    	{   
	    		if (First)
	    		{
	    			First = FALSE;
	    			fputstring ("#include \"gmextern.h\"",Fid3);
	    		}
	    		_fstrcpy (pLine,str); 
	    		ReplaceChar (pLine,'\t',' ');
	    		OneSpace (pLine);
	    		while (*LastChr (pLine) != ';')
	    		{
					fgetstring (str,1024,Fid2);
					lineno++;
		    		ReplaceChar (str,'\t',' ');
		    		OneSpace (str);
		    		_fstrcat (pLine,str);
	    		}
	    		if (!AddExtern (pLine,Fid4,Name,lineno))
	    			ii=1;
	    	}  
	    	else
	    		fputstring (str,Fid3);  
	    }
	    GSSiClose (Fid3); 
	    GSSiClose (Fid2); 
    }
    GSSiClose (Fid1); 
    GSSiClose (Fid4);    
    GSSiGlobUlFree (&hLine);  */
    return; 
/*    Fid1 = GSSiOpenFile ("c:\\gssi\\prog\\geomastr\\files.txt",NULL,OF_READ);
    while (fgetstring (Name,128,Fid1))
    {    
    	AddEnableTrace (Name);
    	break;
    }
    GSSiClose (Fid1); */
//                LPGWDHEADER lpGWDHead,lpGWDHead2;
/*				LPPOLYID	pPI, pPI2;
				short	pos,ii,rw=3,r,g,b;
				HMENU	hMenu;   
				BYTE	bytes[4096];  
	HANDLE	hSurf, hI; 
	LPSTR	pTrace;
    
    hTrace = GSSiGlobAlloc (GHND,4096);
    pTrace = GlobalLock (hTrace);
    _fstrcpy (pTrace,"Hi There Fatty");
    GlobalUnlock (hTrace);
	sprintf (str,"%s %ld","c:\\gssi\\prog\\trace\\basic.exe",(long)hTrace);
	hI = WinExec (str,SW_SHOWNORMAL); */

/*{
	int ick;
	char szMsg[80];
	
	if ((ick = ProfInsChk()) == 0)
	    MessageBox(hWndMain, "Profiler is not installed!",
	        "ProfInsChk", MB_ICONSTOP);
	else {
	    strcpy(szMsg, "Profiler is installed");
	    if (ick == 2) {
	        strcat(szMsg, " in 386 enhanced mode");
	        ProfSetup(128, 0);  
	    }
	    MessageBox(hWndMain, szMsg, "ProfInsChk", MB_OK);
	} 
}*/
// GetDTMHoles ("[%DL]attribut\\city25ft.dtm","c:\\cityvoid.txt");

/*    hSurf = DTMOpen ("[%DL]attribut\\ot1.dtm",DBL_MAX,BT_WRITE);
	DeleteType3SubCell (hSurf);
	DTMClose (&hSurf);  */
    
/*    HANDLE	hTest=GSSiGlobAlloc (GMEM_MOVEABLE,128);
    LPSTR	ptst=GlobalLock (hTest);
    GlobalUnlock (hTest);
    GSSiGlobFree (&hTest);*/ 
    
//    CreateVideoPolys ("c:\\gm2000\\vidlist.txt","s:\\gm2000\\maplib\\video.plt");
//    Fid1 =_lcreat ("c:\\longfilename.txt",0);
//    _lclose (Fid1);

//	RemoveLinkLines ();	
//	("[%DL]attribut\\ot1.dtm","c:\\ngiholes.txt");
/*	LoadTIN ("[%DL]tininput\\sht02dtm.asc","[%DL]maplib\\tin\\sht02tst.plt");
	LoadTIN ("[%DL]tininput\\sht07dtm.asc","[%DL]maplib\\tin\\sht07tst.plt");
	LoadTIN ("[%DL]tininput\\sht08dtm.asc","[%DL]maplib\\tin\\sht08tst.plt");*/
/*            	Fid1 = GSSiOpenFile ("c:\\dgnfiles\\p1202232.dgn",&OFStruct,OF_READ); 
            	_lread (Fid1,bytes,4096);
            	GSSiClose (Fid1);*/ 
				
/*                COLORREF	Color=RGB(255,0,0);  
                HBRUSH	hBrush=GetStockObject(BLACK_BRUSH);
				SetViewport(CommandViewport);            	
		        SetDisplayMode (hDC,GF_TEXTMODE);
		        SelectClipRgn (hDC,0);
                Rect.top = CurView->DrawRect.top;
                do
                {   
                	Rect.bottom = Rect.top + rw;         
                	Rect.left=CurView->DrawRect.left;
                	do
                	{                    
                		r--;g++;
                		Color = RGB(r,g,0);
                		Rect.right = Rect.left + rw;  
                		hBrush = CreateSolidBrush (Color);
                		FillRect (CurView->hDC,&Rect,hBrush);
                		DeleteObject (hBrush);
//		                FillRectPoly (CurView->hDC,&Rect,Color);  
		                Rect.left = Rect.right;
                	}while (Rect.right < CurView->DrawRect.right);
                	Rect.top = Rect.bottom;
                }while (Rect.bottom<CurView->DrawRect.bottom); 
*/
//				DeleteDirAndContents ("f:\\patty1");
//				CreateSportMapCMD ("f:\\patty1","");
				
/*				{ 
					long	n;
					char	str[32];
					
	            	Fid1 = GSSiOpenFile ("c:\\sno.txt",&OFStruct,OF_CREATE);
					for (n = 10003; n<12000; n+=13)
					{
						sprintf (str,"%ld",n);
						fputstring (str,Fid1);
					}
					_lclose (Fid1);
				} */
				
//				double FreeSpace=GetDriveFreeSpace ("c"), mb = FreeSpace/((double)1024*(double)1024);
//				ii=1;
//				createsymbols (0);				
//                LoadERDASDem (); 
//				TestERDASDem (1);
//                LoadPRIMBounds (1);
//                TestGrayScale (hWnd);
//				CopyGWDatabase ("h:\\duluth\\address\\strsegfx.gmd", "h:\\duluth\\address\\strtseg.gmd", TRUE);

//				GetVolumeLabel(3,str);
//            	Fid1 = GSSiOpenFile ("c:\\soilproj\\joinfile\\corners\\filelist.txt",&OFStruct,OF_READ);
/*
            	Fid1 = GSSiOpenFile ("c:\\soilproj\\joinfile\\corners\\filelist.txt",&OFStruct,OF_READ);
            	Fid3 = GSSiOpenFile ("c:\\soilproj\\joinfile\\corners\\missing.txt",&OFStruct,OF_CREATE);
            	tot=1;
            	while (fgetstring (Name,128,Fid1))
            	{   
            		Fid2 = GSSiOpenFile (Name,&OFStruct,OF_READ);
            		num=0;
            		while (fgetstring (str,200,Fid2))
            			num++; 
            		GSSiClose (Fid2);
            		if (num != 4)
            			fputstring (Name,Fid3);
	            }
	            GSSiClose (Fid1);
	            GSSiClose (Fid3);
*/				      
/*            	Fid1 = GSSiOpenFile ("c:\\soilproj\\trs\\filelist.txt",&OFStruct,OF_READ);
            	tot=1;
            	while (fgetstring (Name,128,Fid1))
            	{   
            		if ((lpDot=_fstrchr (Name,'.')))
            		{   
	            		Fid2 = GSSiOpenFile (Name,&OFStruct,OF_READ);
	            		*lpDot = 0;
	            		lpDot = _fstrrchr (Name,'\\');
	            		lpDot++;
            			sprintf (str,"c:\\soilproj\\trs\\tr%s.txt",lpDot);
	            		Fid3 = GSSiOpenFile (str,&OFStruct,OF_CREATE);
	            		fputstring ("BMX,BMY,UTMX,UTMY",Fid3);
	            		while (fgetstring (str,256,Fid2))
	            		{
	            			if (*str != '#')
	            			{ 
	            				REPLAC (str," ",",",250);
	            				fputstring (str,Fid3);
	            			}
	            		}
	            		GSSiClose (Fid2);
	            		GSSiClose (Fid3);
            		}
            	}   
                GSSiClose (Fid1);
*/  
//				LoadMCDS ();
//				DumpDCB ("COM1","c:\\sio1dump.txt");
// 				LoadDTM ();
/*            	Fid1 = GSSiOpenFile ("e:\\filelist.txt",&OFStruct,OF_READ);
            	tot=1;
            	while (fgetstring (Name,128,Fid1))
            	{   
            		if (_fstrchr (Name,'.'))
            		{
            			sprintf (str,"d:\\newdoc\\%ld.pcx",tot++);
            			copyfile (str,Name);
            		}
            	}   
                GSSiClose (Fid1);  */
				
/*                ChangeFileSymName ("RED","REDFLOOD");
/*				typedef struct {long	iref,class;
								char	downum[8];
								short	ver;
								char	name[40];
								short	endbytes;
								} LAKEDATA;
				typedef LAKEDATA	FAR	*LPLAKEDATA;
				LPLAKEDATA	pLake;
				long	downum;
				
				hDBPolyID2 = OpenGWDatabase ("e:\\min83\\lakes\\lakes.gmd",BT_READ); 
				lpGWDHead2 = GlobalLock (hDBPolyID2);   
	    		pLake = &lpGWDHead2->GWDData; 
	    		pos = BT_FIRST;
            	Fid1 = GSSiOpenFile ("c:\\lakes.txt",&OFStruct,OF_CREATE);
	        	while (!BT_FIND (lpGWDHead2->BTHandle[0],&Refno,pos,BT_ANY,(LPSTR)&Offset))
		        {   
		        	pos = BT_NEXT; 
	    			len = FillGWDData (lpGWDHead2,Offset);
	    			pLake->endbytes=0;
	    			if (*pLake->name)
	    			{   
	    				downum = ldread (pLake->downum,8);
	    				sprintf (str,"%10ld %s",downum,pLake->name); 
	    				fputstring (str,Fid1);
	    			}
				} 
				GlobalUnlock (hDBPolyID2);
				CloseGWDatabase (hDBPolyID2); 					
                GSSiClose (Fid1);*/
//                GWD_MergeDBs ("level8.gmd","level8a.gmd",TRUE);

/*            	Fid1 = GSSiOpenFile ("e:\\min83\\wma\\size.txt",&OFStruct,OF_READ);
            	while (fgetstring (str,128,Fid1))
            	{
            		size=atol(str);
            		tot+=size;  
            		num++; 
                } 
                GSSiClose (Fid1); */
				

//				DupDLLs ();
				 
//				LoadGS(1);

//				UpdateSNamesFromHlt ();

//				LoadSYMTable ();
				
/*            	Fid1 = GSSiOpenFile ("c:\\gssi\\prog\\jpeg5\\filelist.txt",&OFStruct,OF_READ);
            	while (fgetstring (Name,128,Fid1))
            	{   
            		Fid2 = GSSiOpenFile (Name,&OFStruct,OF_READ);
            		while (fgetstring (str,256,Fid2)) 
            		{
            			if (_fstrstr (str,"Problem"))
            				ii=1;
            		}
	            	GSSiClose (Fid2);
                } 
                GSSiClose (Fid1);*/
/*                _fstrcpy (Name,"e:\\soilpoly\\mtrista\\polyid.gmd");
				CreatePolyIDFile (Name);
				hDBPolyID = OpenGWDatabase (Name,BT_WRITE);
	    		lpGWDHead = GlobalLock (hDBPolyID);   
	    		pPI = &lpGWDHead->GWDData; 
            	Fid1 = GSSiOpenFile ("e:\\soilpoly\\mtrista\\filelist.txt",&OFStruct,OF_READ);
            	while (fgetstring (Name,128,Fid1))
            	{   
					hDBPolyID2 = OpenGWDatabase (Name,BT_READ); 
	    			lpGWDHead2 = GlobalLock (hDBPolyID2);   
		    		pPI2 = &lpGWDHead2->GWDData; 
		    		pos = BT_FIRST;
		        	while (!BT_FIND (lpGWDHead2->BTHandle[0],&Refno,pos,BT_ANY,(LPSTR)&Offset))
			        {   
			        	pos = BT_NEXT; 
	        			len = FillGWDData (lpGWDHead2,Offset);
						*pPI = *pPI2;
						GWDAddRecord (lpGWDHead,0,NULL);
					} 
					GlobalUnlock (hDBPolyID2);
					CloseGWDatabase (hDBPolyID2); 					
            	}
				GlobalUnlock (hDBPolyID);
				CloseGWDatabase (hDBPolyID); */
				
/*            	Fid1 = GSSiOpenFile ("e:\\min83\\usgsname\\mn.dlm",&OFStruct,OF_READ);
            	Fid2 = GSSiOpenFile ("e:\\min83\\usgsname\\mn.txt",&OFStruct,OF_CREATE);
            	while (fgetstring (str,256,Fid1))
            	{   
            		REPLAC (str,"','","\t",256);
            		fputstring (str,Fid2);
            	}
                GSSiClose (Fid1);
                GSSiClose (Fid2); */
					
								
//            	CreatePickData ();
/*            	Fid1 = GSSiOpenFile ("d:\\soilpoly\\filelist.txt",&OFStruct,OF_READ);
            	while (fgetstring (Name,128,Fid1))
            	{   
            		ParcelLoc = 0;
            		_fstrcpy(PltName,Name);
            		OpenMap (hWnd, hDC);
            		CloseMap (); 
            		if (ParcelLoc)
            		{
	            		Fid2 = GSSiOpenFile (Name,&OFStruct,OF_READWRITE);
	            		_llseek (Fid2,ParcelLoc,0);
	            		_lwrite (Fid2,&symnum,2);  
	            		_lwrite (Fid2,&parent,2);  
	            		_fstrncpy (str,"SOILAREA",32);
	            		_lwrite (Fid2,str,32);
	            		GSSiClose (Fid2);
	            	}
                } */
                
/*            	Fid1 = GSSiOpenFile ("d:\\soilbmps\\filelist.txt",&OFStruct,OF_READ);
            	while (fgetstring (Name,128,Fid1))
            	{
            		Fid2 = GSSiOpenFile (Name,&OFStruct,OF_READ);
            		n=0;
            		while (fgetstring (str,254,Fid2))  
            			n++;
            		_llseek (Fid2,0,0);
            		lpDot = _fstrchr (Name,'.');
            		*lpDot = 0;
            		_fstrcat (Name,".txt");
            		Fid3 = GSSiOpenFile (Name,&OFStruct,OF_READ); 
            		if (Fid3 != HFILE_ERROR)
            		{
	            		*Name='E';
	            		Fid4 = GSSiOpenFile (Name,&OFStruct,OF_CREATE); 
            			fgetstring (str,254,Fid3);  
            			if (!_fstrstr (str,"usgsdoq"))
            				MessageBox (GetFocus(),Name,NULL,MB_ICONEXCLAMATION);
            			fputstring (str,Fid4);
	            		i=4;
	            		while (i--)
	            		{
	            			fgetstring (str,254,Fid3);
	            			fputstring (str,Fid4);
	            		}
            			sprintf (str,"%i",n);
            			fputstring (str,Fid4);
            			while (n--)
            			{
            				fgetstring (str,254,Fid2);
            				fputstring (str,Fid4);
            			}
	            				
	            		GSSiClose (Fid3);  
	            		GSSiClose (Fid4);  
	            	}
	            	GSSiClose (Fid2);
            	}*/
/*            	GSSiClose (Fid1);
            	Fid1 = GSSiOpenFile ("d:\\soilbmps\\filelist.txt",&OFStruct,OF_READ);
            	while (fgetstring (Name,128,Fid1))
            	{
            		Fid2 = GSSiOpenFile (Name,&OFStruct,OF_READWRITE);
            		_llseek (Fid2,0,2);
            		lpDot = _fstrchr (Name,'.');
            		*lpDot = 0;
            		_fstrcat (Name,".trn");
            		Fid3 = GSSiOpenFile (Name,&OFStruct,OF_READ); 
            		if (Fid3 != HFILE_ERROR)
            		{
	            		while (fgetstring (str,254,Fid3))
	            		{
		            		if (sscanf (str,"%Flf %Flf %Flf %Flf",&Point.x,&Point.y,&x2,&y2)==4)
		            		{   
		            			ConvertPoint ("statepln",&Point,1);
		            			sprintf (str,"%f %f %f %f",x2,y2,Point.x,Point.y);
		            			fputstring (str,Fid2);
		            		}
		            	}		
	            		GSSiClose (Fid3);  
	            	}
	            	GSSiClose (Fid2);
            	}
            	GSSiClose (Fid1);
            } */
//StreetSegFromNet (1);             
//            ChangeFileCoords (1);
//UpdateCities(1);
//temprep (1);
//             	FixF61 (1); 
	return;
}

