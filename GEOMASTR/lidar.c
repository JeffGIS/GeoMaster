#include <shr.h>
#include "gmextern.h"


#pragma pack(1)

typedef struct {
	char FileSignature[4];
	unsigned short FileSourceID;
	unsigned short GlobalEncoding;
	unsigned long ProjectIDGUIDdata1;
	unsigned short ProjectIDGUIDdata2;
	unsigned short ProjectIDGUIDdata3;
	unsigned char ProjectIDGUIDdata4[8];
	unsigned char VersionMajor;
	unsigned char VersionMinor;
	char SystemIdentifier[32];
	char GeneratingSoftware[32];
	unsigned short FileCreationDayofYear;
	unsigned short FileCreationYear;
	unsigned short HeaderSize;
	unsigned long Offsetopointdata;
	unsigned long NumberofVariableLengthRecords;
	unsigned char PointDataRecordFormat;
	unsigned short PointDataRecordLength;
	unsigned long LegacyNumberofpointrecords;
	unsigned long LegacyNumberofpointsbyreturn[5];
	double Xscalefactor;
	double Yscalefactor;
	double Zscalefactor;
	double Xoffset;
	double Yoffset;
	double Zoffset;
	double MaxX;
	double MinX;
	double MaxY;
	double MinY;
	double MaxZ;
	double MinZ;
	unsigned long long StartofWaveformDataPacketRecord;
	unsigned long long StartoffirstExtendedVariableLengthRecord;
	unsigned long NumberofExtendedVariableLengthRecords;
	unsigned long long Numberofpointrecords;
	unsigned long long Numberofpointsbyreturn[15];
}LIDARHEADER;

typedef struct {
	long X,Y,Z;
	unsigned short	Intensity;
	unsigned char	ReturnNumber:3,
					NumberofReturns:3,
					ScanDirectionFlag:1,
					EdgeofFlightLine:1;
	unsigned char	Classification;
	char			ScanAngleRank;
	unsigned char	UserData; 
	unsigned short	PointSourceID;
	double			GPSTime;
}LIDARPOINTTYPE1;

#pragma pack()

BOOL ReadLidarHeader (LPSTR FileName)
{
	LIDARHEADER lh;
	LIDARPOINTTYPE1 pointrec;
	long	np;
	long	classcount[32]={0};
	long	returnno[32]={0};

	HFILE fid = GSSiOpenFile (FileName,0,OF_READ);

	if (fid != HFILE_ERROR)
	{
		BigRead (fid,&lh,sizeof(LIDARHEADER));
		GSSillseek (fid,lh.Offsetopointdata,0);
		np = lh.LegacyNumberofpointrecords;
		while (np--)
		{
			BigRead (fid,&pointrec,sizeof(LIDARPOINTTYPE1));
			classcount[pointrec.Classification]++;
			returnno[pointrec.ReturnNumber]++;
			/*Xcoordinate = (Xrecord * Xscale) + Xoffset
			Ycoordinate = (Yrecord * Yscale) + Yoffset
			Zcoordinate = (Zrecord * Zscale) + Zoffset*/

		}
	}
	GSSiClose (fid);
	return TRUE;
}

BOOL LoadLIDARDTMFromLAS (LPSTR Infiles,LPSTR OutFile,LPSTR CBounds)
{   
	LPSTR InFile = Infiles;
	LPSTR FileEnd = _fstrchr (InFile,';'),FileEndSave;
	char	TempFile[MAX_PATH]="c:\\tempdtm.bin", str[130], mess[256];  
	HANDLE	TxtHandle=0;
	double	X,Y,Z;    
	long	IX,IY,IZ; 
	long	CurrentCell=-1, Cell, lineno;
	DWORD	FileLength,NumRows,NumCols, loc, TotLen, CurLoc; 
	long	CellRow, CellCol, DataOffset, CellOffset, CellNo=0, IndexSize;
	HFILE	FidIn, FidOut;   
	double	MinX=DBL_MAX, MaxX=-DBL_MAX,MinY=DBL_MAX, MaxY=-DBL_MAX,MinZ=DBL_MAX, MaxZ=-DBL_MAX;   
	double	FileMinX, FileMinY;
	long	LidarDist[MAXLIDARPERREC+1];  
	double	CellMinX, CellMinY; 
	int		i;
     
    LIDARREC	LidarRec;  
    LIDARFILEHEADER	Header;
	
	AllowCache = FALSE;
	UndoEnabled = FALSE; 


/*		char	Name[MAX_PATH]="[%DL]projections"; 
	char	str[66];
	HFILE	Fid,Fid2;  
	long	TotFiles=0, Loc;    
	int		Item;
	LPSTR	pName;
    
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
        		*pCurProj = Item;
				fgetstring (str,64,Fid2);
				*pCurUnits = UnitsFromText (&str[1]);
			}
	        Item = SendDlgItemMessage (hWndDlg,cntl,CB_SETITEMDATA,(WPARAM)Item,(LPARAM)Loc);
        	GSSiClose (Fid2);
        }
        Loc = GSSillseek (Fid,0,1);
	} while (fgetstring (Name,250,Fid));

	if (FileEnd)
		*FileEnd++ = 0;
	FileEndSave = FileEnd;
	if (*CBounds)
	{   
		MNMXCORD Bounds; 
		BOOL	err;
		
		Bounds = atobounds (CBounds,&err);
		if (err) 
		{
			sprintf (mess,"Error in bounds:%s",CBounds);
			MessageBox (0,mess,0,MB_ICONEXCLAMATION);
			return FALSE;
		}
		MinX = Bounds.xmn;
		MinY = Bounds.ymn;
		MaxX = Bounds.xmx;
		MaxY = Bounds.ymx; 
	}
	else
	{       
		FidIn = GSSiOpenFile (InFile,0,OF_READ); 
		TotLen = GSSillseek (FidIn,0,2);
		GSSillseek (FidIn,0,0);
		ProcessDelimTextHeader(str,InFile,FidIn,&TxtHandle,0); 
		CreateStatusWind (hWndMain,1,"Getting Min/Max Values"); 
NextFile:
		lineno = 1;
		while (ContinueProcessing && fgetstring (str,64,FidIn))
		{   
			lineno++;
			GetDelimTextData(str,TxtHandle);
			X = GetGlobalDVal ("[X]")*FTM;
			Y = GetGlobalDVal ("[Y]")*FTM;
			Z = GetGlobalDVal ("[Z]")*FTM;  
			if (X < 10)
			{
				sprintf (mess,"Invalid X at line %ld:%s",lineno,str);
				MessageBox (0,mess,0,MB_ICONEXCLAMATION);
			}
			else
			{
				MinX = min (MinX,X);
				MaxX = max (MaxX,X);
				MinY = min (MinY,Y);
				MaxY = max (MaxY,Y);
			}
			CurLoc = GSSillseek (FidIn,0,1);
			StatusWindowUpdate (0,0, TotLen, CurLoc); 
	    }
	    GSSiClose (FidIn);
	    if (FileEnd)
	    {
	    	
	    	FidIn = GSSiOpenFile (FileEnd,0,OF_READ);
	    	FileEnd = 0; 
			TotLen = GSSillseek (FidIn,0,2); 
			GSSillseek (FidIn,0,0);
			goto NextFile;
		}
	    SetContinueProcessing ( TRUE);
		DestroyStatusWindow (0);
	}
    FileMinX = MinX - fmod (MinX,LIDARCELLSIZE);
    FileMinY = MinY - fmod (MinY,LIDARCELLSIZE); 
    NumCols = 1+(MaxX - FileMinX) / LIDARCELLSIZE;
    NumRows = 1+(MaxY - FileMinY) / LIDARCELLSIZE;
    if ((double)NumRows * (double)NumCols * (double)sizeof(LIDARREC) > (double)LONG_MAX)
    {
    	MessageBox (0,"Lidar file size exceeds maximum",0,MB_ICONEXCLAMATION);
    	return FALSE;
    } 
    FileLength = NumRows * NumCols * sizeof (LIDARREC);
   	CloseFidSmall ();
	FidOut = GSSiOpenFile (TempFile,0,OF_CREATE);
    GSSiChangeLength (FidOut,FileLength);
    GSSiClose (FidOut);
    CreateFidSmall ();
	FidOut = GSSiOpenFile (TempFile,0,OF_READWRITE);
	FidIn = GSSiOpenFile (InFile,0,OF_READ); 
	CreateStatusWind (hWndMain,1,"Loading Data");
	TotLen = (DWORD)GSSillseek (FidIn,0,2);
	GSSillseek (FidIn,0,0);
	GSSiGlobFree (&TxtHandle);  
	ProcessDelimTextHeader(str,InFile,FidIn,&TxtHandle,0);    
	FileEnd = FileEndSave;
NextFile2:
	while (ContinueProcessing && fgetstring (str,64,FidIn))
	{
		GetDelimTextData(str,TxtHandle);
		X = GetGlobalDVal ("[X]")*FTM;
		Y = GetGlobalDVal ("[Y]")*FTM;
		Z = GetGlobalDVal ("[Z]")*FTM; 
		MinZ = min (MinZ,Z);
		MaxZ = max (MaxZ,Z);
		CellCol = (X - FileMinX)/LIDARCELLSIZE;
		CellRow = (Y - FileMinY)/LIDARCELLSIZE; 
		if (CellCol >= 0 && CellCol < NumCols && CellRow >= 0 && CellRow < NumRows)
		{
			Cell = NumCols * CellRow + CellCol;  
			if (Cell != CurrentCell)
			{
				if (CurrentCell > -1)
				{
					loc = (DWORD)CurrentCell * (DWORD)sizeof(LIDARREC);
					GSSillseek2 (FidOut,loc,0);
					BigWrite (FidOut,(HPSTR)&LidarRec,sizeof(LIDARREC),-1);
				}
				loc = (DWORD)Cell * (DWORD)sizeof(LIDARREC);
				GSSillseek2 (FidOut,loc,0);
				BigRead (FidOut,(HPSTR)&LidarRec,sizeof(LIDARREC));
				CurrentCell = Cell;  
				CellMinX = FileMinX + CellCol * LIDARCELLSIZE;
				CellMinY = FileMinY + CellRow * LIDARCELLSIZE;
			}
			if (LidarRec.NumPoints < MAXLIDARPERREC)
			{
				LidarRec.LidarPnt[LidarRec.NumPoints].xoff = IDNINT (1000*(X - CellMinX));
				LidarRec.LidarPnt[LidarRec.NumPoints].yoff = IDNINT (1000*(Y - CellMinY));
				LidarRec.LidarPnt[LidarRec.NumPoints++].Elevation = Z;
			} 
		}
		CurLoc = (DWORD)GSSillseek (FidIn,0,1);
		StatusWindowUpdate (0,0, TotLen, CurLoc); 
    }  
    GSSiClose (FidIn); 
    if (FileEnd)
    {
	    	
    	FidIn = GSSiOpenFile (FileEnd,0,OF_READ);
    	FileEnd = 0; 
		TotLen = GSSillseek (FidIn,0,2); 
		GSSillseek (FidIn,0,0);
		goto NextFile2;
	}
	loc = (DWORD)CurrentCell * (DWORD)sizeof(LIDARREC);
	GSSillseek2 (FidOut,loc,0);
	BigWrite (FidOut,(HPSTR)&LidarRec,sizeof(LIDARREC),-1);
    SetContinueProcessing ( TRUE);
	DestroyStatusWindow (0);
    GSSiClose (FidOut);
	FidOut = GSSiOpenFile (TempFile,0,OF_READ);
    _fmemset (LidarDist,0,sizeof(LidarDist));
	CreateStatusWind (hWndMain,1,"Building Distribution"); 
	TotLen = (DWORD)GSSillseek (FidOut,0,2);
	GSSillseek (FidOut,0,0);
	while (ContinueProcessing && BigRead (FidOut,(HPSTR)&LidarRec,sizeof(LIDARREC)))
	{
		LidarDist[LidarRec.NumPoints]++;
		CurLoc = (DWORD)GSSillseek (FidOut,0,1);
		StatusWindowUpdate (0,0, TotLen, CurLoc); 
    }  
    SetContinueProcessing ( TRUE);
	DestroyStatusWindow (0);
    GSSiClose (FidOut);
	GSSiGlobFree (&TxtHandle);  
	for (i=0;i<MAXLIDARPERREC+1;i++)
	{ 
		sprintf (str,"%i\t%ld",i,LidarDist[i]);
		AppendFile ("c:\\lidardist.txt",str);
	} 
	FidIn = GSSiOpenFile (TempFile,0,OF_READ);
	DataOffset = sizeof (LIDARFILEHEADER) + NumRows * NumCols * 4;
	FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
	Header.Version = 1;  
	Header.Bounds.xmn = FileMinX;
	Header.Bounds.xmx = MaxX;
	Header.Bounds.ymn = FileMinY;
	Header.Bounds.ymx = MaxY;  
	Header.MinElev = MinZ;
	Header.MaxElev = MaxZ;
	Header.NumRows = NumRows;
	Header.NumCols = NumCols; 
	Header.CellSpacing = LIDARCELLSIZE; 
	BigWrite (FidOut,(HPSTR)&Header,sizeof(LIDARFILEHEADER),-1);  
	IndexSize = NumRows * NumCols;
	CellOffset = 0;
	while (IndexSize--)
		BigWrite (FidOut,(HPSTR)&CellOffset,4,-1);
	CreateStatusWind (hWndMain,1,"Creating Output File"); 
	TotLen = (DWORD)GSSillseek (FidIn,0,2);
	GSSillseek (FidIn,0,0);	
	while (ContinueProcessing && BigRead (FidIn,(HPSTR)&LidarRec,sizeof(LIDARREC)))
	{
		if (LidarRec.NumPoints)
		{
			CellOffset = GSSillseek (FidOut,0,2); 
			BigWrite (FidOut,(HPSTR)&LidarRec,2+LidarRec.NumPoints*sizeof(LIDARPNT),-1);
			GSSillseek (FidOut,(long)sizeof (LIDARFILEHEADER)+CellNo*4,0);
			BigWrite (FidOut,(HPSTR)&CellOffset,4,-1);
		}
		CellNo++;
		CurLoc = (DWORD)GSSillseek (FidIn,0,1);
		StatusWindowUpdate (0,0, TotLen, CurLoc); 
    }  
    SetContinueProcessing ( TRUE);
    GSSiClose (FidIn);
    GSSiClose (FidOut);
	DestroyStatusWindow (0);  
	GSSiRemove (TempFile);*/
	return TRUE;
}

