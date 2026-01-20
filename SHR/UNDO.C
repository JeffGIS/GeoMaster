#include "graphint.h"   

#define	MAXUNDOPOINTS	10
#include "gmextern.h"

static	OFSTRUCTGM	OFStructUndo;
static	char		UndoFileName[MAX_PATH]="";
static	long		LastUndoPointSize[MAXUNDOFILES]; 
static	short		NumUndoPoints = 0; 
static	BOOL		UpdatesSinceLastUndoPoint=FALSE;     
static	char	UpdateFileNames[USHRT_MAX]=""; 
static	char	UndoPointList[USHRT_MAX]="";
static	USHORT	UndoPointListEnd = 0;
static	USHORT	LastUndoPoint=0;
static	short	debugstep;

#define UNDO_WRITE	1 
#define UNDO_DATA	2
#define UNDO_FREESPACE	3  
#define UNDO_OPEN	4  
#define UNDO_NULL	5
#define UNDO_CHECKPOINT	6

static	struct	{
					long	FirstSeg,
							LastSeg, 
							FreeSpaceBeg;
				}UndoHeader;  
				
static	struct	{	BYTE	Type,
							UndoFileID;
					DWORD	Time;
					long	PrevSeg,
							NextSeg,				
							Length,
							SeekLoc,
							FirstPiece;
				}UndoRecordHeader; 
				
typedef	struct	{   
					BYTE	Type;
					long	Length,
							Next;
				}UNDOPIECEHEADER;
			
static	UNDOPIECEHEADER	UndoPieceHeader, UndoPieceHeader2;   
static	HWND	UndoThisWindow=0;

void UndoAddWindow (HWND hWnd)
{
	UndoThisWindow = hWnd;
	return;
}

void UndoRemoveWindow (HWND hWnd)
{
	UndoThisWindow = 0;
	return;
}

void UndoError (LPSTR Mess)
{
	GSSiMsgBox (0,Mess,"Undo Error",MB_ICONEXCLAMATION|MB_TASKMODAL,0);  
	return;
}

/*void checkheader (long loc,UNDOPIECEHEADER UndoPieceHeader)
{   
	short	ii;
	
	if (UndoPieceHeader.Next > loc && UndoPieceHeader.Next < loc + UndoPieceHeader.Length + sizeof(UndoPieceHeader))
		ii=1;
	return;
} 

void CheckUndoHeader (short Type,short check)
{   
	short	ii;
	
	if (Type != check)
		ii=1;
	return;
}

void check1721 (HFILE Fid)
{
	short	ii;
	long	CurLoc, loc=1721;
	UNDOPIECEHEADER	UndoPieceHeader;
	return;   
	
	CurLoc =_llseek (Fid,0,1);
	if (_llseek (Fid,loc,0) != loc)
		ii=1;
	else
	{  
		BigRead (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader));
		CheckUndoHeader (UndoPieceHeader.Type,UNDO_FREESPACE);
	} 
	_llseek (Fid,CurLoc,0);
	return;
}
	
void checkfsb (void)
{   
	short	ii;
	
	if (UndoHeader.FreeSpaceBeg == 16777281)
		ii=1;
    return;
} */

void DisableUndo (BOOL Disable)
{   
	static	BOOL	SaveUndoStatus;
	
	if (Disable)
	{
		SaveUndoStatus = UndoEnabled;
		UndoPointListEnd = 0;  
		LastUndoPoint = 0;
		*UndoPointList = 0; 
		NumUndoPoints = 0;
		SaveDataToUndoFile (0,0,0,NULL,0);
		UndoEnabled = FALSE;
	}
	else
	{
		UndoEnabled = SaveUndoStatus;
		CreateUndoPoint ("Start");
	}	
	return;
}

void RemoveUndoPoint (void)
{   
	long	CurLoc, NextLoc; 
	HANDLE	FidUndo;
	OFSTRUCTGM	OFStruct = { 0 };
	
	return;
	FidUndo = OpenFileGM (UndoFileName,&OFStruct,OF_READWRITE); 
	BigRead64 (FidUndo,(HPSTR)&UndoHeader,sizeof(UndoHeader));
	CurLoc = llFileSeek (FidUndo,UndoHeader.FirstSeg,0);
	BigRead64 (FidUndo,(HPSTR)&UndoRecordHeader,sizeof(UndoRecordHeader));
	do
	{ 
		NextLoc = UndoRecordHeader.FirstPiece; 
		UndoHeader.FirstSeg = UndoRecordHeader.NextSeg;
		AddToUndoFreeSpace (FidUndo,CurLoc,sizeof(UndoRecordHeader));
		while (NextLoc > -1)
		{   
			llFileSeek (FidUndo,NextLoc,0);
			BigRead64 (FidUndo,(HPSTR)&UndoPieceHeader,sizeof(UndoPieceHeader));
			AddToUndoFreeSpace (FidUndo,NextLoc,(long)sizeof(UndoPieceHeader)+UndoPieceHeader.Length); 
			NextLoc = UndoPieceHeader.Next;
		}
		if (UndoHeader.FirstSeg < 0) 
		{
			UndoHeader.LastSeg = -1;
			goto Exit;                  
		}
		CurLoc = llFileSeek (FidUndo,UndoHeader.FirstSeg,0);
		BigRead64 (FidUndo,(HPSTR)&UndoRecordHeader,sizeof(UndoRecordHeader)); 
	}
	while (UndoRecordHeader.Type != UNDO_CHECKPOINT);
	llFileSeek(FidUndo,CurLoc,0);
	UndoRecordHeader.PrevSeg = -1;
	BigWrite64 (FidUndo,(LPSTR)&UndoRecordHeader,sizeof(UndoRecordHeader),-1); 
Exit:
	llFileSeek(FidUndo,0,0);
	BigWrite64(FidUndo,(LPSTR)&UndoHeader,sizeof(UndoHeader),-1);
	GSSiClose64 (&FidUndo); 
	return;
}

BOOL CreateUndoPoint (LPSTR InName)
{   
	short	ii;  
	short	iEnd, l; 
	char	File1[144]="", File2[144]="";    
	HANDLE	hMem;
	LPSTR	str, Name;
	USHORT	i;
	
	if (NumOpenFiles)
		ii=1;
	if (!UndoEnabled)
		return TRUE; 
	hMem=GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,2048);
	str=GlobalLock (hMem);
	Name=str+1024;
    _fstrcpy (Name,InName);
    
    if (UpdatesSinceLastUndoPoint)
    {
		BT_GETPATHNAME (hHighlight,File1);		
		BT_GETPATHNAME (hHighlight2,File2);		
		CloseHighlightList ();
		if (NumUndoPoints == MAXUNDOPOINTS)
		{
			RemoveUndoPoint ();
			NumUndoPoints--;  
		}
		NumUndoPoints++; 
	}
	else
		UndoPointListEnd = LastUndoPoint;  
	NumUndoPoints = max (NumUndoPoints,1); 
	LastUndoPoint = UndoPointListEnd;
	iEnd = _fstrcspn (Name,"^&"); 
	Name[iEnd] = 0; 
	Strip (Name,'|');
	sprintf (str,"%s|%lu",Name,NextVarTime());
	l = _fstrlen (str);
	_fstrcpy (&UndoPointList[UndoPointListEnd],str);
	UndoPointListEnd += l + 1;  
	UndoPointList[UndoPointListEnd] = 0;
		
	for (i=0;i<MAXUNDOFILES;i++)
		LastUndoPointSize[i] = -1;	

	SaveDataToUndoFile (UNDO_CHECKPOINT,0,l+1,str,0);
	UpdatesSinceLastUndoPoint = FALSE;
	for (i=0;i<MAXFILEHANDLES;i++)
	{   
		if (OpenFileUndoFileID[i] > -1 && OpenFileFid[i] > -1)
		{
			long	CurLoc = _llseek (OpenFileFid[i],0,1);
			
			GetUndoFileNameFromUndoFileID (OpenFileUndoFileID[i],str);
			LastUndoPointSize[OpenFileUndoFileID[i]] = _llseek (OpenFileFid[i],0,2); 
			_llseek (OpenFileFid[i],CurLoc,0);
			SaveDataToUndoFile (UNDO_OPEN,OpenFileUndoFileID[i],_fstrlen(str)+1,str,LastUndoPointSize[OpenFileUndoFileID[i]]);
		}
	}
	if (*File1)
		OpenHighlightList (File1,File2);  
	GSSiGlobUlFree (&hMem);
	return TRUE;
}

void AddToUndoFreeSpace (HANDLE Fid,long loc,long len)
{   
	BYTE	Null=UNDO_NULL; 
	short	ii;
	UNDOPIECEHEADER	UndoPieceHeader,UndoPieceHeader2;
		
	ii=llFileSeek (Fid,loc,0);
	if (len <= sizeof(UndoPieceHeader)) 
	{
		while (len--)
			BigWrite64 (Fid,&Null,1,-1);
		return;                   
	}
	UndoPieceHeader.Next = UndoHeader.FreeSpaceBeg;
	UndoHeader.FreeSpaceBeg = loc; 
//checkfsb ();
	UndoPieceHeader.Type = UNDO_FREESPACE;
	UndoPieceHeader.Length = len - (long)sizeof(UndoPieceHeader);  
	if (UndoPieceHeader.Next ==  loc + len)
	{
		llFileSeek (Fid,UndoPieceHeader.Next,0);	
		BigRead64 (Fid,(HPSTR)&UndoPieceHeader2,sizeof(UndoPieceHeader2));
		UndoPieceHeader.Length += (long)sizeof(UndoPieceHeader) + UndoPieceHeader2.Length;
		UndoPieceHeader.Next = UndoPieceHeader2.Next;
		llFileSeek(Fid,loc,0);
	}
	BigWrite64 (Fid,(LPSTR)&UndoPieceHeader,sizeof(UndoPieceHeader),-1);  
//checkheader (loc,UndoPieceHeader);
//check1721 (Fid);
	return;
} 
void AddToUndoFreeSpaceHFILE(HFILE Fid, long loc, long len)
{
	BYTE	Null = UNDO_NULL;
	short	ii;
	UNDOPIECEHEADER	UndoPieceHeader, UndoPieceHeader2;

	ii = GSSillseek(Fid, loc, 0);
	if (len <= sizeof(UndoPieceHeader))
	{
		while (len--)
			BigWrite(Fid, &Null, 1, -1);
		return;
	}
	UndoPieceHeader.Next = UndoHeader.FreeSpaceBeg;
	UndoHeader.FreeSpaceBeg = loc;
	//checkfsb ();
	UndoPieceHeader.Type = UNDO_FREESPACE;
	UndoPieceHeader.Length = len - (long)sizeof(UndoPieceHeader);
	if (UndoPieceHeader.Next == loc + len)
	{
		GSSillseek(Fid, UndoPieceHeader.Next, 0);
		BigRead(Fid, (HPSTR)&UndoPieceHeader2, sizeof(UndoPieceHeader2));
		UndoPieceHeader.Length += (long)sizeof(UndoPieceHeader) + UndoPieceHeader2.Length;
		UndoPieceHeader.Next = UndoPieceHeader2.Next;
		GSSillseek(Fid, loc, 0);
	}
	BigWrite(Fid, &UndoPieceHeader, sizeof(UndoPieceHeader), -1);
	//checkheader (loc,UndoPieceHeader);
	//check1721 (Fid);
	return;
}

short GetUndoFileIDFromFid (HFILE Fid)
{       
	USHORT	i;  
	
	if (Fid == HFILE_ERROR)
		return -1;
	for (i=0;i<MAXFILEHANDLES;i++)
	{   
		if (OpenFileFid[i] == Fid)
			return OpenFileUndoFileID[i];
	}
	
	return -1;
}

short GetUndoFileIDFromName (LPSTR Name,BOOL Add)
{ 
	short	ID=0;
	UINT	l;
	LPSTR	pName = UpdateFileNames;   
	long	pNameBeg = (long)pName;
	
	if (!UndoEnabled)
		return -1;  
	while (*pName)
	{   
		if (!_fstricmp (pName,Name))
			return ID;
		pName = _fstrchr (pName,0);
		pName++;  
		ID++;
	}  
	if (!Add)
		return -1;
	l = _fstrlen (Name);
	if ((long)pName - pNameBeg + l + 1 > (long)USHRT_MAX)
		UndoError ("Undo file name array exceeded");
	_fstrcpy (pName,Name);
	pName[l+1] = 0; 
	LastUndoPointSize[ID] = -1; 
	UndoFileFid[ID] = HFILE_ERROR;
	return ID;
}

BOOL GetUndoFileNameFromUndoFileID (short UndoFileID,LPSTR Name)
{   
	LPSTR	pName = UpdateFileNames;
	
	while (UndoFileID--)
	{   
		if (!*pName)
			return FALSE;
		pName = _fstrchr (pName,0);
		pName++;
	}
	_fstrcpy (Name,pName);
	return TRUE;
}

BOOL GetUndoData (HFILE Fid,HPSTR pData,long length,long NextPiece,BOOL Remove)
{    
	long	Outlen=0; 
	short	ii;
	
	while (NextPiece >= 0)
	{
		long	HeadLoc = GSSillseek (Fid,NextPiece,0);
		
		BigRead (Fid,(HPSTR)&UndoPieceHeader,sizeof(UndoPieceHeader));
		BigRead (Fid,pData,UndoPieceHeader.Length); 
		Outlen += UndoPieceHeader.Length;
		pData += UndoPieceHeader.Length;
		NextPiece = UndoPieceHeader.Next; 
		if (Remove)
			AddToUndoFreeSpaceHFILE (Fid,HeadLoc,UndoPieceHeader.Length + (long)sizeof(UndoPieceHeader));
	}
	if (Outlen != length)
		ii=1;
//check1721 (Fid);
	return TRUE;
}

HFILE GetUndoFid (short UndoFileID)
{
	static	HFILE	OpenUndoFid=HFILE_ERROR;
	static	short	OpenUndoFileID = -1;  
	char	Name[MAX_PATH]; 
	short	ii;
	
	debugstep=1;
	if (UndoFileID == -1)
	{
		if (OpenUndoFid != HFILE_ERROR)
			GSSiClose2 (&OpenUndoFid);
		OpenUndoFileID = -1;
		return HFILE_ERROR;
	}
	debugstep++;	
	if (UndoFileID == OpenUndoFileID)
		goto Exit;
	debugstep++;	
	if (OpenUndoFid != HFILE_ERROR)
		GSSiClose2(&OpenUndoFid);
	GetUndoFileNameFromUndoFileID (UndoFileID,Name);
	//GetShortPathName (Name,128);
	OpenUndoFid = OpenFile (Name,(LPOFSTRUCT)&OFStructUndo,OF_READWRITE); 
	if (OpenUndoFid == HFILE_ERROR)
	{
		if (OFStructUndo.nErrCode == 2) 
		{
			OpenUndoFid = OpenFile (Name, (LPOFSTRUCT)&OFStructUndo,OF_CREATE);
			debugstep=100;	
		}
		else
			debugstep=200;
	}	
	OpenUndoFileID = UndoFileID;
Exit:
	if (OpenUndoFid == HFILE_ERROR)
		ii=1;
	return OpenUndoFid;
}

BOOL UndoAction (HFILE FidUndo)
{   
	HANDLE	hMem;
	HPSTR	pMem;
	short	ii; 
	HFILE	Fid;
	
	switch (UndoRecordHeader.Type)
	{
		case UNDO_CHECKPOINT:
			hMem = GSSiGlobAlloc(GAIDNO  75,GMEM_MOVEABLE,UndoRecordHeader.Length);
			pMem = GlobalLock (hMem); 
			GetUndoData (FidUndo,pMem,UndoRecordHeader.Length,UndoRecordHeader.FirstPiece,TRUE);
			GSSiGlobUlFree (&hMem);
		break;
		
		case UNDO_OPEN: 
			hMem = GSSiGlobAlloc(GAIDNO  76,GMEM_MOVEABLE,UndoRecordHeader.Length);
			pMem = GlobalLock (hMem); 
			GetUndoData (FidUndo,pMem,UndoRecordHeader.Length,UndoRecordHeader.FirstPiece,TRUE);
			if (!UndoRecordHeader.SeekLoc)
				remove (pMem);
			else
			{
				Fid = GetUndoFid (UndoRecordHeader.UndoFileID); 
		        if (!GSSiChangeLength (Fid,-UndoRecordHeader.SeekLoc)) 
		        {
		        	char	mess[512]; 
		        	long	FileLen=_llseek (Fid,0,2);
		        	
			       // DumpOpenFiles ();
		        	sprintf (mess,"Change Length(%i):%ld %i %s (%ld) %ld %ld",errno,(long)Fid,debugstep,OFStructUndo. szPathName,(long)OFStructUndo.nErrCode,UndoRecordHeader.SeekLoc,FileLen);
		        	UndoError (mess);  
		        }
		    }
			GSSiGlobUlFree (&hMem);
		break;
		case UNDO_WRITE: 
			hMem = GSSiGlobAlloc(GAIDNO  77,GMEM_MOVEABLE,UndoRecordHeader.Length);
			pMem = GlobalLock (hMem); 
			GetUndoData (FidUndo,pMem,UndoRecordHeader.Length,UndoRecordHeader.FirstPiece,TRUE);
			Fid = GetUndoFid (UndoRecordHeader.UndoFileID);
			if (GSSillseek (Fid,UndoRecordHeader.SeekLoc,0) == UndoRecordHeader.SeekLoc)
				BigWrite (Fid,pMem,UndoRecordHeader.Length,-1);
			else
				UndoError ("Seek");
			GSSiGlobUlFree (&hMem);
		break;
	}
//check1721 (FidUndo); 
	if (UndoThisWindow)
		PostMessage (UndoThisWindow,GF_UNDOCOMPLETED, 0, 0L);

	return TRUE;
}   

void RemoveLastUndoPoint ()
{   
	USHORT	i;
	
	if (!NumUndoPoints)
		return;
	UndoPointList[LastUndoPoint] = 0;  
	UndoPointListEnd = LastUndoPoint; 
	NumUndoPoints--;
	if (NumUndoPoints)
	{ 
		i = UndoPointListEnd - 2;
		while (UndoPointList[i])
			i--;
		LastUndoPoint = i + 1;   
	}
	else
	{
		UndoPointListEnd = 0;
		*UndoPointList = 0;
		SaveDataToUndoFile (0,0,0,NULL,0);
	}
	return;
}	

BOOL UndoChanges (HWND hWnd)
{
	HFILE	FidUndo;
	OFSTRUCTGM	OFStruct = { 0 };
	long	CurLoc;  
	long	UndoPointTime;
	LPSTR	pTime; 
	USHORT	i,ii;
	char	mess[256];
	BOOL	rtn=FALSE, SaveUSLC;
	HCURSOR hcurSave;  

	if (!UndoEnabled)
		return FALSE;   
    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	SaveUSLC = UpdatesSinceLastUndoPoint;
	CloseHighlightList (); 
	CloseAllRequestedFiles(FALSE);
	UpdatesSinceLastUndoPoint = SaveUSLC;
	if (!UpdatesSinceLastUndoPoint)
		RemoveLastUndoPoint ();
	if (!*UndoFileName || !NumUndoPoints)
	{
		GSSiMsgBox(hWnd, "Nothing to Undo","", MB_OK,0);
		goto Exit;  
	} 
	pTime = _fstrchr (&UndoPointList[LastUndoPoint],'|');
	*pTime = 0;
	sprintf (mess,"Undo back to '%s'?",&UndoPointList[LastUndoPoint]);
	*pTime = '|';
    if (GSSiMsgBox (hWnd,mess,"Verify Undo",MB_OKCANCEL,0) == IDCANCEL)
    	goto Exit;
	pTime = _fstrchr (&UndoPointList[LastUndoPoint],'|');
	pTime++;
	UndoPointTime = atol (pTime);
	FidUndo = OpenFile (UndoFileName,(LPOFSTRUCT)&OFStruct,OF_READWRITE); 
	BigRead (FidUndo,&UndoHeader,sizeof(UndoHeader));
	CurLoc = _llseek (FidUndo,UndoHeader.LastSeg,0);  
if (CurLoc == 1721)
	ii=1;
	BigRead (FidUndo,&UndoRecordHeader,sizeof(UndoRecordHeader));
	while (UndoRecordHeader.Time >= UndoPointTime)
	{   
//check1721 (FidUndo);
		AddToUndoFreeSpaceHFILE (FidUndo,CurLoc,sizeof(UndoRecordHeader)); 
		UndoAction (FidUndo); 
		UndoHeader.LastSeg = UndoRecordHeader.PrevSeg;
		if (UndoHeader.LastSeg < 0)  
		{
		    UndoHeader.FirstSeg = -1;
			break;
		}
		CurLoc = _llseek (FidUndo,UndoRecordHeader.PrevSeg,0);
		BigRead (FidUndo,&UndoRecordHeader,sizeof(UndoRecordHeader));
	}
	_llseek (FidUndo,0,0);
	_lwrite (FidUndo,(LPCSTR)&UndoHeader,sizeof(UndoHeader));  
//check1721 (FidUndo);
	_lclose (FidUndo); 
	GetUndoFid (-1); 
	RemoveLastUndoPoint (); 
	if (NumUndoPoints)
		UpdatesSinceLastUndoPoint = TRUE;
	rtn = TRUE; 
Exit:
	OpenHighlightList (NULL,NULL);
	GSSiSetCursor (hcurSave);
	return rtn;
}

//			GetUndoData (FidUndo,pMem,UndoRecordHeader.Length,UndoRecordHeader.FirstPiece);
//			Fid = GetUndoFid (UndoRecordHeader.UndoFileID);
//			if (_llseek (Fid,UndoRecordHeader.SeekLoc,0) == UndoRecordHeader.SeekLoc)
long WriteToUndoFile (HANDLE Fid,HPSTR pData,long len)
{
// len < 0 implies contiguous space and no piece header    
	long	loc,ii;
	BYTE	Null=UNDO_NULL;
	
	if (!len)
		return -1; 
	if (UndoHeader.FreeSpaceBeg == -1) 
	{
WriteAtEOF:
		loc = llFileSeek (Fid,0,2);
		if (len < 0 )
			BigWrite64 (Fid,pData,labs(len),-1);
		else
		{
			UndoPieceHeader.Type = UNDO_DATA;
			UndoPieceHeader.Length = len;
			UndoPieceHeader.Next = -1;
			BigWrite64(Fid,&UndoPieceHeader,sizeof(UndoPieceHeader),-1);
//checkheader (loc,UndoPieceHeader);
			BigWrite64(Fid,pData,len,-1);
		}
//check1721 (Fid);
		return loc;
	}
	else  
	{
		loc = UndoHeader.FreeSpaceBeg;
		ii=llFileSeek (Fid,loc,0);
		BigRead64 (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader)); 
//CheckUndoHeader (UndoPieceHeader.Type,UNDO_FREESPACE);
		if (len < 0)
		{
			if (UndoPieceHeader.Length > labs (len))
			{   
				llFileSeek (Fid,loc,0);
				BigWrite64 (Fid,pData,labs(len),-1); 
				UndoHeader.FreeSpaceBeg += labs (len);
//	checkfsb ();
				UndoPieceHeader.Length -= labs(len);
//checkheader (_llseek(Fid,0,1),UndoPieceHeader);
				BigWrite64(Fid,&UndoPieceHeader,sizeof(UndoPieceHeader),-1);
//check1721 (Fid);
				return loc;
			} 
			else if (UndoPieceHeader.Length + sizeof(UndoPieceHeader) >= labs (len))
			{
				UndoHeader.FreeSpaceBeg = UndoPieceHeader.Next;	
//checkfsb ();
				llFileSeek (Fid,loc,0);
				BigWrite64(Fid,pData,labs(len),-1);
	            len = UndoPieceHeader.Length + sizeof(UndoPieceHeader) - labs(len);
				while (len--)
					BigWrite64 (Fid,&Null,1,-1);
//check1721 (Fid);
				return loc;
			}
			else 
			{
				UndoHeader.FreeSpaceBeg = UndoPieceHeader.Next;	
//checkfsb ();
				goto WriteAtEOF; 
			}
		}
		else
		{   
			long	NextLoc = loc;
			
			while (len)
			{
				llFileSeek (Fid,NextLoc,0);
				if (UndoPieceHeader.Length >= len)
				{   
					UndoPieceHeader2.Type = UNDO_DATA;
					UndoPieceHeader2.Length = len;
					UndoPieceHeader2.Next = -1;
					BigWrite64 (Fid,(LPSTR)&UndoPieceHeader2,sizeof(UndoPieceHeader2),-1);
//checkheader (NextLoc,UndoPieceHeader2);
					BigWrite64(Fid,pData,labs(len),-1);
//check1721 (Fid);
					UndoPieceHeader.Length -= len;  
					if (UndoPieceHeader.Length > sizeof(UndoPieceHeader))
					{
						UndoHeader.FreeSpaceBeg += len + sizeof(UndoPieceHeader); 
						UndoPieceHeader.Length -= sizeof(UndoPieceHeader);
//checkfsb ();
//checkheader (_llseek(Fid,0,1),UndoPieceHeader);
						BigWrite64(Fid,(LPSTR)&UndoPieceHeader,sizeof(UndoPieceHeader),-1);
//check1721 (Fid);
					}
					else 
					{
						UndoHeader.FreeSpaceBeg = UndoPieceHeader.Next; 
						AddToUndoFreeSpace (Fid,llFileSeek(Fid,0,1),UndoPieceHeader.Length); 
					}
//checkfsb ();
					len = 0;
				}
				else
				{
					UndoHeader.FreeSpaceBeg = UndoPieceHeader.Next;	  
//checkfsb ();
					UndoPieceHeader.Type = UNDO_DATA; 
					if (UndoPieceHeader.Next == -1)
					{
						UndoPieceHeader.Next = llFileSeek (Fid,0,2);
						llFileSeek (Fid,NextLoc,0);
					}
					BigWrite64(Fid,&UndoPieceHeader,sizeof(UndoPieceHeader),-1);
//checkheader (NextLoc,UndoPieceHeader);
					BigWrite64(Fid,pData,UndoPieceHeader.Length,-1);
//check1721 (Fid);
					len -= UndoPieceHeader.Length;
					pData += UndoPieceHeader.Length;
					if (UndoHeader.FreeSpaceBeg == -1)
					{   
						long	EndLoc;
						
						UndoPieceHeader.Type = UNDO_DATA;  
						UndoPieceHeader.Length = len; 
						UndoPieceHeader.Next = -1; 
						EndLoc=llFileSeek (Fid,0,2);
						BigWrite64(Fid,(LPSTR)&UndoPieceHeader,sizeof(UndoPieceHeader),-1);
//checkheader (EndLoc,UndoPieceHeader);
						BigWrite64(Fid,pData,UndoPieceHeader.Length,-1);
//check1721 (Fid);
						len = 0;
					}
					else 
					{
						NextLoc = UndoHeader.FreeSpaceBeg;
						ii=llFileSeek (Fid,NextLoc,0);
						BigRead64 (Fid,(HPSTR)&UndoPieceHeader,sizeof(UndoPieceHeader)); 
//CheckUndoHeader (UndoPieceHeader.Type,UNDO_FREESPACE);
					}
				}
			}
//check1721 (Fid);
			return loc;
		}
	}
}

void AddFileToUndoFile (LPSTR Name,long BeginLoc,HFILE Fid)
{   
	HANDLE	hStr;
	HPSTR	pStr; 
	long	len, LastLoc,IncLen=(long)USHRT_MAX;  
	long	CurLoc, ResetLoc; 
	short	UndoFileID;    
	OFSTRUCTGM	OFStruct = { 0 };
	char	Name2[MAX_PATH]; 
	
	if (!UndoEnabled || !NumUndoPoints)
		return;
	if (Name)
	{     
		UndoFileID = GetUndoFileIDFromName (Name,FALSE);
		if (UndoFileID < 0)
			return;   
		_fstrcpy (Name2,Name);
		//GetShortPathName (Name2,128);
		Fid = OpenFile (Name2,(LPOFSTRUCT)&OFStruct,OF_READ); 
		BeginLoc = 0;
	}
	else
	{
		ResetLoc = _llseek (Fid,0,2);  
		UndoFileID = GetUndoFileIDFromFid (Fid);
	}
	if (Fid == HFILE_ERROR)
		return;
	hStr = GSSiGlobAlloc(GAIDNO  78,GMEM_MOVEABLE,USHRT_MAX);
	pStr = GlobalLock (hStr);
	CurLoc = _llseek (Fid,0,2);  
	LastLoc = CurLoc;
	CurLoc -= IncLen;
	CurLoc = max (CurLoc,BeginLoc);
	len = LastLoc - CurLoc;
	while (len > 0)
	{   
		_llseek (Fid,CurLoc,0); 
		LastLoc = CurLoc;
		BigRead (Fid,pStr,len); 
		SaveDataToUndoFile (UNDO_WRITE,UndoFileID,len,pStr,CurLoc);
		CurLoc -= IncLen;
		CurLoc = max (CurLoc,BeginLoc);
		len = LastLoc - CurLoc;
	} 
	if (Name)
		_lclose (Fid); 
	else
		_llseek (Fid,ResetLoc,0);  
	GSSiGlobUlFree (&hStr);
	return;
}

void SaveDataToUndoFile (short Type,short UndoFileID, long len, HPSTR pData,long SeekLoc)
{   
	HANDLE		FidUndo;
	OFSTRUCTGM	OFStruct = { 0 };
	long		HeaderLoc, DataLoc;   
	short		ii;
	
	if (!pData)
	{
		if (*UndoFileName)
			remove (UndoFileName);
		*UndoFileName = 0;
		return;
	} 
	if (Type == UNDO_WRITE)
		UpdatesSinceLastUndoPoint = TRUE;
	if (!*UndoFileName)
	{
		GetTempDir (UndoFileName);
		sprintf (_fstrchr (UndoFileName,0),"\\gmundo.tmp"); 
		//GetShortPathName (UndoFileName,128);
		FidUndo = OpenFileGM (UndoFileName,&OFStruct,OF_CREATE);    
		UndoHeader.FirstSeg = -1;
		UndoHeader.LastSeg = -1;
		UndoHeader.FreeSpaceBeg = -1;
		BigWrite64 (FidUndo,(LPSTR)&UndoHeader,sizeof(UndoHeader),-1);
	}
	else 
	{
		FidUndo = OpenFileGM (UndoFileName,&OFStruct,OF_READWRITE); 
		BigRead64 (FidUndo,(LPSTR)&UndoHeader,sizeof(UndoHeader));
	}
	DataLoc = WriteToUndoFile (FidUndo,pData,len);
	UndoRecordHeader.Type = Type;  
	UndoRecordHeader.UndoFileID = UndoFileID;
	UndoRecordHeader.Length = len;   
	if (SeekLoc < 0)
		ii=1;
	UndoRecordHeader.SeekLoc = SeekLoc;
	UndoRecordHeader.Time = NextVarTime();
	UndoRecordHeader.PrevSeg = UndoHeader.LastSeg;
	UndoRecordHeader.NextSeg = -1;	
	UndoRecordHeader.FirstPiece = DataLoc;
	HeaderLoc = WriteToUndoFile (FidUndo,(HPSTR)&UndoRecordHeader,-(long)sizeof(UndoRecordHeader)); 
	if (UndoHeader.FirstSeg == -1)
		UndoHeader.FirstSeg = HeaderLoc;
	if (UndoHeader.LastSeg > -1)
	{
		llFileSeek (FidUndo,UndoHeader.LastSeg,0);
		BigRead64 (FidUndo,(HPSTR)&UndoRecordHeader,sizeof(UndoRecordHeader));  
		UndoRecordHeader.NextSeg = HeaderLoc;
		llFileSeek(FidUndo,UndoHeader.LastSeg,0);
		BigWrite64 (FidUndo,(LPSTR)&UndoRecordHeader,sizeof(UndoRecordHeader),-1);  
	}
//check1721 (FidUndo);
	UndoHeader.LastSeg = HeaderLoc;
	llFileSeek (FidUndo,0,0);
	BigWrite64(FidUndo,(LPSTR)&UndoHeader,sizeof(UndoHeader),-1);
//check1721 (FidUndo);
	GSSiClose64(&FidUndo);
	return;
} 

void FileWillBeCreated (LPSTR Name)
{   
	short	ID;
	
	if (!UndoEnabled)
		return;
	ID = GetUndoFileIDFromName (Name,TRUE);     
//	UndoFileFid[ID] = Fid; this Fid is the being set to current graphics Fid ???
	AddFileToUndoFile (Name,0,0);
//	SaveDataToUndoFile (UNDO_OPEN,ID,_fstrlen(Name)+1,Name,LastCheckpointSize[ID]);
	return;
}

void FileOpenForUpdate (LPSTR Name,HFILE Fid,UINT Mode)
{   
	short	ID; 
	
	if (Fid == HFILE_ERROR)
		return;
	if (AllowJournal)
	{
		OpenJournal (Name,Fid,Mode);
		return;
	}
	if (!UndoEnabled)
		return;
	ID = GetUndoFileIDFromName (Name,TRUE);
	UndoFileFid[ID] = Fid; 
	if (LastUndoPointSize[ID] < 0)
	{
		long	CurLoc = _llseek (Fid,0,1);
			
		LastUndoPointSize[ID] = _llseek (Fid,0,2); 
		_llseek (Fid,CurLoc,0);
		SaveDataToUndoFile (UNDO_OPEN,ID,_fstrlen(Name)+1,Name,LastUndoPointSize[ID]);
	}

	return;
}

long GetSizeAtLastUndoPoint (short	UndoFileID)
{
	if (UndoFileID < 0)
		return -1;
	return LastUndoPointSize[UndoFileID];
}

void CheckPointBegin (void)
{
	CheckPointEnd ();
	CurrentCheckPointID = time (0);
	return;
}

void CheckPointEnd (void)
{
	int	len=0, loc;

	CloseAllRequestedFiles(FALSE); 
	if (hFilesWithJournals)
	{
		LPSTR pFile, pFilesWithJournals;

		pFile = GlobalLock (hFilesWithJournals);
		loc = 0;
		while (loc < lFilesWithJournals)
		{
			char	File[MAX_PATH];

			strcpy (File,&pFile[loc]);
			loc += strlen (File) + 1;
			GlobalUnlock (hFilesWithJournals);
			GMDUpdateCheckPointLog (File);
			pFile = GlobalLock (hFilesWithJournals);
		}
		GlobalUnlock (hFilesWithJournals);
		pFile = pFilesWithJournals = GlobalLock (hFilesWithJournals);

		while (pFile < &pFilesWithJournals[lFilesWithJournals])
		{
			ApplyJournal (pFile);
			pFile = strchr (pFile,0) + 1;
		}
		GSSiGlobUlFree (&hFilesWithJournals);
		lFilesWithJournals = 0;
	}
	return;
}

