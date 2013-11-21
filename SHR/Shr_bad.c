#include "shr.h"   
#include "graphics.h"
#include "bigmemln.h"
#include "bigmem.h"
#include "mapl.h"
#define  p_tol
#include "p_tol.h"
#include "hash.h"    
#include <direct.h>   
#include <sys\types.h>
#include <sys\stat.h>         
#include <fcntl.h>
#include <dibapi.h>  
#include <ctype.h>
#include <math.h> 
#include <float.h>   
#include "winexec.h" 
#include "..\geomastr\resource.h"  
#include "dgn7.h"
/*
 *  The CRC table is alocated and defined at run-time.
 */
#define CHECKTIMESTAMP	4
#define CHECKFOREXIST	3
#define DONTEXIST		2
#define	DOESEXIST		1					
#define W_FAR far

    typedef struct
        {   long    FileNum;
            long    FileLength;
            time_t  LastUsed;
            char    FileName[128];
        }   CACHEBUF; 

extern BOOL CALLBACK EnumCtrlProc(HWND hCtrl,LONG lParam);
#include "gmextern.h"

static	HANDLE	hSavedScreens[MAXSAVEDSCREENS];
static	short	nSavedScreens=0;      
static	long	NextScreenID=1;
static	USHORT	crc_table[128]={0}; 
static  BOOL    FirstCache = TRUE; 
static	short	CurTraceLev=0;
static  char    CacheDir[132]="", CacheIndex[128]; 
static	char	SubstitutePathnameFrom[64]="", SubstitutePathnameTo[64];
static	char	CachePathnameFrom[4][64], CachePathnameTo[64]="";  
static	short	NumCachePathnameFrom=0;
static	HANDLE	hCacheAlreadyChecked=0;
static	BOOL    TraceTrace = FALSE, NeedStep=FALSE, StepOver=FALSE;;  
static  BYTE    Mask[8] = {128, 64, 32, 16, 8, 4, 2, 1}; 
static	short	NumOpenFiles=0;
static	OFSTRUCT	OpenFileStruct[MAXFILEHANDLES];
static	HFILE		OpenFileFid[MAXFILEHANDLES]; 
static	short		OpenFileUndoFileID[MAXFILEHANDLES];
static	UINT		OpenFileMode[MAXFILEHANDLES];
static	HANDLE		OpenFileHandle[MAXFILEHANDLES];
static	long		OpenFileLength[MAXFILEHANDLES];
static	long		OpenFilePosition[MAXFILEHANDLES];
static	BOOL		OpenFileCloseRequested[MAXFILEHANDLES]; 
static	OFSTRUCT	OFStructUndo;
static	BOOL		InOpenFile=FALSE;
static	HWND	hWndTrace=0; 

static HWND	FontListhWndDlg;
static UINT	FontListCntl;
static short	MaxWaitCycles=5;
static short	have_crc_table=0;
static HFILE	TraceFid=HFILE_ERROR;
static char		SaveWinText[144];
static FARPROC	lpfnTRACEWINDOWMsgProc;  

#define	MAXCHECKPOINTS	10
static	char	UndoFileName[128]="";
static	char	UpdateFileNames[UINT_MAX]=""; 
static	char	CheckPointList[UINT_MAX]="";
static	USHORT	CheckPointListEnd = 0;
static	USHORT	LastCheckPoint=0;
static	long	LastCheckpointSize[MAXUNDOFILES]; 
static	HFILE	UndoFileFid[MAXUNDOFILES];
static	short	NumCheckPoints = 0; 
static	BOOL	UpdatesSinceLastCheckpoint=FALSE;  

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

void HaltMapDisplay(BOOL ClearCFGStack);   
void DebugShowLine (LPDPOINT p1,LPDPOINT p2);

void UndoError (LPSTR Mess)
{
	MessageBox (0,Mess,"Undo Error",MB_ICONEXCLAMATION|MB_TASKMODAL);  
	return;
}

void checkheader (long loc,UNDOPIECEHEADER UndoPieceHeader)
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
		_lread (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader));
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
}

void DisableUndo (BOOL Disable)
{   
	static	BOOL	SaveUndoStatus;
	
	if (Disable)
	{
		SaveUndoStatus = UndoEnabled;
		CheckPointListEnd = 0;  
		LastCheckPoint = 0;
		*CheckPointList = 0; 
		NumCheckPoints = 0;
		SaveDataToUndoFile (0,0,0,NULL,0);
	}
	else
	{
		UndoEnabled = SaveUndoStatus;
		CreateCheckPoint ("Start");
	}	
	return;
}

void RemoveCheckPoint (void)
{   
	long	CurLoc, NextLoc; 
	HFILE	FidUndo;
	OFSTRUCT	OFStruct;
	
	FidUndo = OpenFile (UndoFileName,&OFStruct,OF_READWRITE); 
	_lread (FidUndo,&UndoHeader,sizeof(UndoHeader));
	CurLoc = _llseek (FidUndo,UndoHeader.FirstSeg,0);
	_lread (FidUndo,&UndoRecordHeader,sizeof(UndoRecordHeader));
	do
	{ 
		NextLoc = UndoRecordHeader.FirstPiece; 
		UndoHeader.FirstSeg = UndoRecordHeader.NextSeg;
		AddToUndoFreeSpace (FidUndo,CurLoc,sizeof(UndoRecordHeader));
		while (NextLoc > -1)
		{   
			_llseek (FidUndo,NextLoc,0);
			_lread (FidUndo,&UndoPieceHeader,sizeof(UndoPieceHeader));
			AddToUndoFreeSpace (FidUndo,NextLoc,(long)sizeof(UndoPieceHeader)+UndoPieceHeader.Length); 
			NextLoc = UndoPieceHeader.Next;
		}
		if (UndoHeader.FirstSeg < 0) 
		{
			UndoHeader.LastSeg = -1;
			goto Exit;                  
		}
		CurLoc = _llseek (FidUndo,UndoHeader.FirstSeg,0);
		_lread (FidUndo,&UndoRecordHeader,sizeof(UndoRecordHeader)); 
	}
	while (UndoRecordHeader.Type != UNDO_CHECKPOINT);
	_llseek (FidUndo,CurLoc,0);   
	UndoRecordHeader.PrevSeg = -1;
	_lwrite (FidUndo,&UndoRecordHeader,sizeof(UndoRecordHeader)); 
Exit:
	_llseek (FidUndo,0,0);
	_lwrite (FidUndo,&UndoHeader,sizeof(UndoHeader));  
	_lclose (FidUndo); 
	return;
}

BOOL CreateCheckPoint (LPSTR InName)
{   
	short	ii;  
	short	iEnd, l; 
	char	str[128], File1[144], File2[144],Name[128]; 
	USHORT	i;
	
	if (NumOpenFiles)
		ii=1;
	if (!UndoEnabled)
		return TRUE; 
    _fstrcpy (Name,InName);
    
    if (UpdatesSinceLastCheckpoint)
    {
		BT_GETPATHNAME (hHighlight,File1);		
		BT_GETPATHNAME (hHighlight2,File2);		
		CloseHighlightList ();
		if (NumCheckPoints == MAXCHECKPOINTS)
		{
			RemoveCheckPoint ();
			NumCheckPoints--;  
		}
		NumCheckPoints++; 
	}
	else
		CheckPointListEnd = LastCheckPoint;  
	NumCheckPoints = max (NumCheckPoints,1); 
	LastCheckPoint = CheckPointListEnd;
	iEnd = _fstrcspn (Name,"^&"); 
	Name[iEnd] = 0; 
	Strip (Name,'|');
	sprintf (str,"%s|%lu",Name,NextVarTime());
	l = _fstrlen (str);
	_fstrcpy (&CheckPointList[CheckPointListEnd],str);
	CheckPointListEnd += l + 1;  
	CheckPointList[CheckPointListEnd] = 0;
		
	for (i=0;i<MAXUNDOFILES;i++)
		LastCheckpointSize[i] = -1;	

	SaveDataToUndoFile (UNDO_CHECKPOINT,0,l+1,str,0);
	UpdatesSinceLastCheckpoint = FALSE;
	for (i=0;i<MAXFILEHANDLES;i++)
	{   
		if (OpenFileUndoFileID[i] > -1)
		{
			long	CurLoc = _llseek (OpenFileFid[i],0,1);
			
			GetUndoFileNameFromUndoFileID (OpenFileUndoFileID[i],str);
			LastCheckpointSize[OpenFileUndoFileID[i]] = _llseek (OpenFileFid[i],0,2); 
			_llseek (OpenFileFid[i],CurLoc,0);
			SaveDataToUndoFile (UNDO_OPEN,OpenFileUndoFileID[i],_fstrlen(str)+1,str,LastCheckpointSize[OpenFileUndoFileID[i]]);
		}
	}
	if (*File1)
		OpenHighlightList (File1,File2);
	return TRUE;
}

void AddToUndoFreeSpace (HFILE Fid,long loc,long len)
{   
	BYTE	Null=UNDO_NULL; 
	short	ii;
	UNDOPIECEHEADER	UndoPieceHeader,UndoPieceHeader2;
		
	ii=_llseek (Fid,loc,0);
	if (len <= sizeof(UndoPieceHeader)) 
	{
		while (len--)
			_lwrite (Fid,&Null,1);
		return;                   
	}
	UndoPieceHeader.Next = UndoHeader.FreeSpaceBeg;
	UndoHeader.FreeSpaceBeg = loc; 
checkfsb ();
	UndoPieceHeader.Type = UNDO_FREESPACE;
	UndoPieceHeader.Length = len - (long)sizeof(UndoPieceHeader);  
	if (UndoPieceHeader.Next ==  loc + len)
	{
		_llseek (Fid,UndoPieceHeader.Next,0);	
		_lread (Fid,&UndoPieceHeader2,sizeof(UndoPieceHeader2));
		UndoPieceHeader.Length += (long)sizeof(UndoPieceHeader) + UndoPieceHeader2.Length;
		UndoPieceHeader.Next = UndoPieceHeader2.Next;
		_llseek (Fid,loc,0);  
	}
	_lwrite (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader));  
checkheader (loc,UndoPieceHeader);
check1721 (Fid);
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
	short	ID=0,l;
	LPSTR	pName = UpdateFileNames;
	
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
	_fstrcpy (pName,Name);
	l = _fstrlen (Name);
	pName[l+1] = 0; 
	LastCheckpointSize[ID] = -1; 
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
		long	HeadLoc = _llseek (Fid,NextPiece,0);
		
		_lread (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader));
		_hread (Fid,pData,UndoPieceHeader.Length); 
		Outlen += UndoPieceHeader.Length;
		pData += UndoPieceHeader.Length;
		NextPiece = UndoPieceHeader.Next; 
		if (Remove)
			AddToUndoFreeSpace (Fid,HeadLoc,UndoPieceHeader.Length + (long)sizeof(UndoPieceHeader));
	}
	if (Outlen != length)
		ii=1;
check1721 (Fid);
	return TRUE;
}

HFILE GetUndoFid (short UndoFileID)
{
	static	HFILE	OpenUndoFid=HFILE_ERROR;
	static	short	OpenUndoFileID = -1;  
	char	Name[128]; 
	short	ii;
	
	if (UndoFileID == -1)
	{
		if (OpenUndoFid != HFILE_ERROR)
			_lclose (OpenUndoFid);
		OpenUndoFid = HFILE_ERROR;
		OpenUndoFileID = -1;
		return HFILE_ERROR;
	}
		
	if (UndoFileID == OpenUndoFileID)
		goto Exit;
	if (OpenUndoFid != HFILE_ERROR)
		_lclose (OpenUndoFid);
	GetUndoFileNameFromUndoFileID (UndoFileID,Name);
	OpenUndoFid = OpenFile (Name,&OFStructUndo,OF_READWRITE); 
	if (OpenUndoFid == HFILE_ERROR && OFStructUndo.nErrCode == 2)
		OpenUndoFid = OpenFile (Name,&OFStructUndo,OF_CREATE);
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
			hMem = GSSiGlobAlloc (  75,GMEM_MOVEABLE,UndoRecordHeader.Length);
			pMem = GlobalLock (hMem); 
			GetUndoData (FidUndo,pMem,UndoRecordHeader.Length,UndoRecordHeader.FirstPiece,TRUE);
			GSSiGlobUlFree (&hMem);
		break;
		
		case UNDO_OPEN: 
			hMem = GSSiGlobAlloc (  76,GMEM_MOVEABLE,UndoRecordHeader.Length);
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
		        	
		        	sprintf (mess,"Change Length(%i):%ld %s (%ld) %ld",errno,(long)Fid,OFStructUndo. szPathName,(long)OFStructUndo.nErrCode,UndoRecordHeader.SeekLoc);
		        	UndoError (mess);  
		        }
		    }
			GSSiGlobUlFree (&hMem);
		break;
		case UNDO_WRITE: 
			hMem = GSSiGlobAlloc (  77,GMEM_MOVEABLE,UndoRecordHeader.Length);
			pMem = GlobalLock (hMem); 
			GetUndoData (FidUndo,pMem,UndoRecordHeader.Length,UndoRecordHeader.FirstPiece,TRUE);
			Fid = GetUndoFid (UndoRecordHeader.UndoFileID);
			if (_llseek (Fid,UndoRecordHeader.SeekLoc,0) == UndoRecordHeader.SeekLoc)
				_hwrite (Fid,pMem,UndoRecordHeader.Length);
			else
				UndoError ("Seek");
			GSSiGlobUlFree (&hMem);
		break;
	}
check1721 (FidUndo);
	return TRUE;
}   

void RemoveLastCheckPoint ()
{   
	USHORT	i;
	
	if (!NumCheckPoints)
		return;
	CheckPointList[LastCheckPoint] = 0;  
	CheckPointListEnd = LastCheckPoint; 
	NumCheckPoints--;
	if (NumCheckPoints)
	{ 
		i = CheckPointListEnd - 2;
		while (CheckPointList[i])
			i--;
		LastCheckPoint = i + 1;   
	}
	else
	{
		CheckPointListEnd = 0;
		*CheckPointList = 0;
		SaveDataToUndoFile (0,0,0,NULL,0);
	}
	return;
}	

BOOL UndoChanges (HWND hWnd)
{
	HFILE	FidUndo;
	OFSTRUCT	OFStruct; 
	long	CurLoc;  
	long	CheckPointTime;
	LPSTR	pTime; 
	USHORT	i,ii;
	char	mess[256];
	BOOL	rtn=FALSE, SaveUSLC;

	if (!UndoEnabled)
		return FALSE;   
	SaveUSLC = UpdatesSinceLastCheckpoint;
	CloseHighlightList (); 
	UpdatesSinceLastCheckpoint = SaveUSLC;
	if (!UpdatesSinceLastCheckpoint)
		RemoveLastCheckPoint ();
	if (!*UndoFileName || !NumCheckPoints)
	{
		MessageBox(hWnd, "Nothing to Undo","", MB_OK);
		goto Exit;  
	} 
	pTime = _fstrchr (&CheckPointList[LastCheckPoint],'|');
	*pTime = 0;
	sprintf (mess,"Undo back to '%s'?",&CheckPointList[LastCheckPoint]);
	*pTime = '|';
    if (MessageBox (hWnd,mess,"Verify Undo",MB_OKCANCEL) == IDCANCEL)
    	goto Exit;
	pTime = _fstrchr (&CheckPointList[LastCheckPoint],'|');
	pTime++;
	CheckPointTime = _atold (pTime);
	FidUndo = OpenFile (UndoFileName,&OFStruct,OF_READWRITE); 
	_lread (FidUndo,&UndoHeader,sizeof(UndoHeader));
	CurLoc = _llseek (FidUndo,UndoHeader.LastSeg,0);  
if (CurLoc == 1721)
	ii=1;
	_lread (FidUndo,&UndoRecordHeader,sizeof(UndoRecordHeader));
	while (UndoRecordHeader.Time >= CheckPointTime)
	{   
check1721 (FidUndo);
		AddToUndoFreeSpace (FidUndo,CurLoc,sizeof(UndoRecordHeader)); 
		UndoAction (FidUndo); 
		UndoHeader.LastSeg = UndoRecordHeader.PrevSeg;
		if (UndoHeader.LastSeg < 0)  
		{
		    UndoHeader.FirstSeg = -1;
			break;
		}
		CurLoc = _llseek (FidUndo,UndoRecordHeader.PrevSeg,0);
		_lread (FidUndo,&UndoRecordHeader,sizeof(UndoRecordHeader));
	}
	_llseek (FidUndo,0,0);
	_lwrite (FidUndo,&UndoHeader,sizeof(UndoHeader));  
check1721 (FidUndo);
	_lclose (FidUndo); 
	GetUndoFid (-1); 
	RemoveLastCheckPoint (); 
	if (NumCheckPoints)
		UpdatesSinceLastCheckpoint = TRUE;
	rtn = TRUE; 
Exit:
	OpenHighlightList (NULL,NULL);
	return rtn;
}

//			GetUndoData (FidUndo,pMem,UndoRecordHeader.Length,UndoRecordHeader.FirstPiece);
//			Fid = GetUndoFid (UndoRecordHeader.UndoFileID);
//			if (_llseek (Fid,UndoRecordHeader.SeekLoc,0) == UndoRecordHeader.SeekLoc)
long WriteToUndoFile (HFILE Fid,HPSTR pData,long len)
{
// len < 0 implies contiguous space and no piece header    
	long	loc,ii;
	BYTE	Null=UNDO_NULL;
	
	if (!len)
		return -1; 
	if (UndoHeader.FreeSpaceBeg == -1) 
	{
WriteAtEOF:
		loc = _llseek (Fid,0,2);
		if (len < 0 )
			_hwrite (Fid,pData,labs(len));
		else
		{
			UndoPieceHeader.Type = UNDO_DATA;
			UndoPieceHeader.Length = len;
			UndoPieceHeader.Next = -1;
			_lwrite (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader)); 
checkheader (loc,UndoPieceHeader);
			_hwrite (Fid,pData,len);
		}
check1721 (Fid);
		return loc;
	}
	else  
	{
		loc = UndoHeader.FreeSpaceBeg;
		ii=_llseek (Fid,loc,0);
		_lread (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader)); 
		CheckUndoHeader (UndoPieceHeader.Type,UNDO_FREESPACE);
		if (len < 0)
		{
			if (UndoPieceHeader.Length > labs (len))
			{   
				_llseek (Fid,loc,0);
				_hwrite (Fid,pData,labs(len)); 
				UndoHeader.FreeSpaceBeg += labs (len);
	checkfsb ();
				UndoPieceHeader.Length -= labs(len);
checkheader (_llseek(Fid,0,1),UndoPieceHeader);
				_lwrite (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader));
check1721 (Fid);
				return loc;
			} 
			else if (UndoPieceHeader.Length + sizeof(UndoPieceHeader) >= labs (len))
			{
				UndoHeader.FreeSpaceBeg = UndoPieceHeader.Next;	
	checkfsb ();
				_llseek (Fid,loc,0);
				_hwrite (Fid,pData,labs(len)); 
	            len = UndoPieceHeader.Length + sizeof(UndoPieceHeader) - labs(len);
				while (len--)
					_lwrite (Fid,&Null,1);
check1721 (Fid);
				return loc;
			}
			else 
			{
				UndoHeader.FreeSpaceBeg = UndoPieceHeader.Next;	
	checkfsb ();
				goto WriteAtEOF; 
			}
		}
		else
		{   
			long	NextLoc = loc;
			
			while (len)
			{
				_llseek (Fid,NextLoc,0);
				if (UndoPieceHeader.Length >= len)
				{   
					UndoPieceHeader2.Type = UNDO_DATA;
					UndoPieceHeader2.Length = len;
					UndoPieceHeader2.Next = -1;
					_lwrite (Fid,&UndoPieceHeader2,sizeof(UndoPieceHeader2));
checkheader (NextLoc,UndoPieceHeader2);
					_hwrite (Fid,pData,labs(len)); 
check1721 (Fid);
					UndoPieceHeader.Length -= len;  
					if (UndoPieceHeader.Length > sizeof(UndoPieceHeader))
					{
						UndoHeader.FreeSpaceBeg += len + sizeof(UndoPieceHeader); 
						UndoPieceHeader.Length -= sizeof(UndoPieceHeader);
	checkfsb ();
checkheader (_llseek(Fid,0,1),UndoPieceHeader);
						_lwrite (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader)); 
check1721 (Fid);
					}
					else 
					{
						UndoHeader.FreeSpaceBeg = UndoPieceHeader.Next; 
						AddToUndoFreeSpace (Fid,_llseek (Fid,0,1),UndoPieceHeader.Length); 
					}
	checkfsb ();
					len = 0;
				}
				else
				{
					UndoHeader.FreeSpaceBeg = UndoPieceHeader.Next;	  
	checkfsb ();
					UndoPieceHeader.Type = UNDO_DATA; 
					if (UndoPieceHeader.Next == -1)
					{
						UndoPieceHeader.Next = _llseek (Fid,0,2);
						_llseek (Fid,NextLoc,0);
					}
					_lwrite (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader));
checkheader (NextLoc,UndoPieceHeader);
					_hwrite (Fid,pData,UndoPieceHeader.Length);  
check1721 (Fid);
					len -= UndoPieceHeader.Length;
					pData += UndoPieceHeader.Length;
					if (UndoHeader.FreeSpaceBeg == -1)
					{   
						long	EndLoc;
						
						UndoPieceHeader.Type = UNDO_DATA;  
						UndoPieceHeader.Length = len; 
						UndoPieceHeader.Next = -1; 
						EndLoc=_llseek (Fid,0,2);
						_lwrite (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader));
checkheader (EndLoc,UndoPieceHeader);
						_hwrite (Fid,pData,UndoPieceHeader.Length);  
check1721 (Fid);
						len = 0;
					}
					else 
					{
						NextLoc = UndoHeader.FreeSpaceBeg;
						ii=_llseek (Fid,NextLoc,0);
						_lread (Fid,&UndoPieceHeader,sizeof(UndoPieceHeader)); 
						CheckUndoHeader (UndoPieceHeader.Type,UNDO_FREESPACE);
					}
				}
			}
check1721 (Fid);
			return loc;
		}
	}
}

void AddFileToUndoFile (LPSTR Name,long BeginLoc,HFILE Fid)
{   
	HANDLE	hStr;
	HPSTR	pStr; 
	long	len, LastLoc,IncLen=(long)UINT_MAX;  
	long	CurLoc; 
	short	UndoFileID;    
	OFSTRUCT	OFStruct; 
	
	if (!UndoEnabled || !NumCheckPoints)
		return;
	if (Name)
	{     
		UndoFileID = GetUndoFileIDFromName (Name,FALSE);
		if (UndoFileID < 0)
			return;
		Fid = OpenFile (Name,&OFStruct,OF_READ); 
		BeginLoc = 0;
	}
	else
	{
		UndoFileID = GetUndoFileIDFromFid (Fid);
	}
	if (Fid == HFILE_ERROR)
		return;
	hStr = GSSiGlobAlloc (  78,GMEM_MOVEABLE,UINT_MAX);
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
		_hread (Fid,pStr,len); 
		SaveDataToUndoFile (UNDO_WRITE,UndoFileID,len,pStr,CurLoc);
		CurLoc -= IncLen;
		CurLoc = max (CurLoc,BeginLoc);
		len = LastLoc - CurLoc;
	}
	_lclose (Fid);
	GSSiGlobUlFree (&hStr);
	return;
}

void SaveDataToUndoFile (short Type,short UndoFileID, long len, HPSTR pData,long SeekLoc)
{   
	HFILE		FidUndo;
	OFSTRUCT	OFStruct; 
	long		HeaderLoc, DataLoc;
	
	if (!pData)
	{
		if (*UndoFileName)
			remove (UndoFileName);
		*UndoFileName = 0;
		return;
	} 
	if (Type == UNDO_WRITE)
		UpdatesSinceLastCheckpoint = TRUE;
	if (!*UndoFileName)
	{
		GetTempDir (UndoFileName);
		sprintf (_fstrchr (UndoFileName,0),"\\gmundo.tmp");
		FidUndo = OpenFile (UndoFileName,&OFStruct,OF_CREATE);    
		UndoHeader.FirstSeg = -1;
		UndoHeader.LastSeg = -1;
		UndoHeader.FreeSpaceBeg = -1;
		_lwrite (FidUndo,&UndoHeader,sizeof(UndoHeader));
	}
	else 
	{
		FidUndo = OpenFile (UndoFileName,&OFStruct,OF_READWRITE); 
		_lread (FidUndo,&UndoHeader,sizeof(UndoHeader));
	}
	DataLoc = WriteToUndoFile (FidUndo,pData,len);
	UndoRecordHeader.Type = Type;  
	UndoRecordHeader.UndoFileID = UndoFileID;
	UndoRecordHeader.Length = len;
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
		_llseek (FidUndo,UndoHeader.LastSeg,0);
		_lread (FidUndo,&UndoRecordHeader,sizeof(UndoRecordHeader));  
		UndoRecordHeader.NextSeg = HeaderLoc;
		_llseek (FidUndo,UndoHeader.LastSeg,0);
		_lwrite (FidUndo,&UndoRecordHeader,sizeof(UndoRecordHeader));  
	}
check1721 (FidUndo);
	UndoHeader.LastSeg = HeaderLoc;
	_llseek (FidUndo,0,0);
	_lwrite (FidUndo,&UndoHeader,sizeof(UndoHeader));  
check1721 (FidUndo);
	_lclose (FidUndo);
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

void FileOpenForUpdate (LPSTR Name,HFILE Fid)
{   
	short	ID; 
	
	if (!UndoEnabled || Fid == HFILE_ERROR)
		return;
	ID = GetUndoFileIDFromName (Name,TRUE);
	UndoFileFid[ID] = Fid; 
	if (LastCheckpointSize[ID] < 0)
	{
		long	CurLoc = _llseek (Fid,0,1);
			
		LastCheckpointSize[ID] = _llseek (Fid,0,2); 
		_llseek (Fid,CurLoc,0);
		SaveDataToUndoFile (UNDO_OPEN,ID,_fstrlen(Name)+1,Name,LastCheckpointSize[ID]);
	}

	return;
}

long GetSizeAtLastCheckpoint (short	UndoFileID)
{
	if (UndoFileID < 0)
		return -1;
	return LastCheckpointSize[UndoFileID];
}

void CloseAllRequestedFiles (void)
																							#if ENABLETRACE
																							{GSSiEnterProg (171);
																							#endif
{ 
	UINT	i;
	
	if (KeepFilesOpen)
	{
		for (i=0;i<MAXFILEHANDLES;i++)
		{   
			if (OpenFileFid[i] != HFILE_ERROR)
			{
				if (OpenFileCloseRequested[i])
				{
					_lclose (OpenFileFid[i]); 
					LogOpenFilesClose (OpenFileFid[i]); 
				}
			}
		}
		CloseAllRequestedDGNFiles ();
	}  
																							{
																							#if ENABLETRACE
																							GSSiExitProg (171);
																							#endif
	return;
																							}
																							#if ENABLETRACE
																							}
																							#endif
}

void LogOpenFilesOpen (UINT Mode,HFILE Fid,LPOFSTRUCT pOFStruct)
																							#if ENABLETRACE
																							{GSSiEnterProg (172);
																							#endif
{   
	short	i,j,ii; 
	LPSTR	pDot;   
	static	BOOL	FirstFileOpen=TRUE;
	
	if (FirstFileOpen)
	{ 
		for (i=0;i<MAXFILEHANDLES;i++)
		{
			OpenFileFid[i] = HFILE_ERROR; 
			OpenFileHandle[i] = 0; 
			OpenFileUndoFileID[i] = -1;
		}  
		FirstFileOpen = FALSE;
    }
	if (Fid == HFILE_ERROR)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
		return;
																							}
/*	if ((pDot=_fstrstr (pOFStruct->szPathName,".PLT")))
	{
		char	test[128];  
		
		*pDot = 0;
		_fstrcpy (test,pOFStruct->szPathName);  
		*pDot = '.';  
		_fstrcat (test,".rin");
		for (i=0;i<MAXFILEHANDLES;i++) 
		{
			if (OpenFileFid[i] != HFILE_ERROR)
			{ 
				if (!_fstricmp (test,OpenFileStruct[i].szPathName))
					ii=1;//BlowOut();
			}
		}
	} */
	if (Mode & OF_EXIST)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
		return;
																							}
	if (Mode & OF_DELETE)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
		return; 
																							}
	MaxHFile = max (MaxHFile,Fid);
	for (i=0;i<MAXFILEHANDLES;i++) 
		if (OpenFileFid[i] == HFILE_ERROR)
			goto Gotid;
	i = -1;
	if (KeepFilesOpen) 
	{   
		for (j=0;j<MAXFILEHANDLES;j++)
		{
			if (OpenFileFid[j] != HFILE_ERROR) 
			{
				if (OpenFileCloseRequested[j])
				{
					_lclose (OpenFileFid[j]); 
					LogOpenFilesClose (OpenFileFid[j]);
					i = j; 
					break;
				}
			}
		}
	}
	if (i >= 0)
		goto Gotid;
	MessageBox (GetFocus(),"Open file limit exceeded",NULL,MB_ICONEXCLAMATION);
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
	return;
																							}
Gotid:			
	OpenFileFid[i] = Fid;     
	OpenFileStruct[i] = *pOFStruct;
	OpenFileMode[i] = Mode;
	OpenFileCloseRequested[i] = FALSE; 
	OpenFileUndoFileID[i] = GetUndoFileIDFromName (pOFStruct->szPathName,FALSE);
   
	NumOpenFiles++;
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
	return;
																							}
																							#if ENABLETRACE
																							}
																							#endif
}

void LogOpenFilesClose (HFILE Fid)
																							#if ENABLETRACE
																							{GSSiEnterProg (173);
																							#endif
{   
	UINT	i,ii;  
	static	UINT	debugi=12;
	
	if (Fid == HFILE_ERROR)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (173);
																							#endif
		return;
																							}
		for (i=0;i<MAXFILEHANDLES;i++)
		if (OpenFileFid[i] == Fid)
		{
			OpenFileFid[i] = HFILE_ERROR;
			if (i == debugi)
				ii=1;
			NumOpenFiles--; 
																							{
																							#if ENABLETRACE
																							GSSiExitProg (173);
																							#endif
			return;
																							}
		} 
	ii=1;
		
																							{
																							#if ENABLETRACE
																							GSSiExitProg (173);
																							#endif
	return;
																							}
																							#if ENABLETRACE
																							}
																							#endif
} 

void CheckOpenFiles (void)
																							#if ENABLETRACE
																							{GSSiEnterProg (173);
																							#endif
{   
	UINT	i;
	
	for (i=0;i<MAXFILEHANDLES;i++)
		if (OpenFileFid[i] != HFILE_ERROR)
		{   
			char	mess[256];
				
			GetOpenFilePathname (OpenFileFid[i],mess);
			_fstrcat (mess," still open");
#if CHECKMEM
			MessageBox (0,mess,NULL,MB_ICONEXCLAMATION);
#endif 
			_lclose (OpenFileFid[i]);
		} 
																							{
																							#if ENABLETRACE
																							GSSiExitProg (173);
																							#endif
	return;
																							}
																							#if ENABLETRACE
																							}
																							#endif
} 

HFILE FileAlreadyOpen (LPSTR InName,UINT Mode,LPOFSTRUCT pOFStruct)
																							#if ENABLETRACE
																							{GSSiEnterProg (174);
																							#endif
{   
	UINT	i;
	
	if (KeepFilesOpen) 
	{   
		LPSTR	pFullPath = _fullpath (NULL,InName,0); 
		
		if (pFullPath)
		{	
			for (i=0;i<MAXFILEHANDLES;i++)
			{
				if (OpenFileFid[i] != HFILE_ERROR)
				{
					if (!_fstricmp (pFullPath,OpenFileStruct[i].szPathName))
					{
						free (pFullPath); 
						if ((Mode == OF_READ || Mode == OF_EXIST) && OpenFileMode[i] == OF_READ)
						{
							if (pOFStruct) 
								*pOFStruct = OpenFileStruct[i]; 
							if (Mode == OF_READ)
							{
								_llseek (OpenFileFid[i],0,0);
								OpenFileCloseRequested[i] = FALSE;
							}
																								{
																								#if ENABLETRACE
																								GSSiExitProg (174);
																								#endif
							return OpenFileFid[i];       
																								}
						}
						_lclose (OpenFileFid[i]); 
						LogOpenFilesClose (OpenFileFid[i]);
																								{
																								#if ENABLETRACE
																								GSSiExitProg (174);
																								#endif
						return HFILE_ERROR;
																								}
					}
				}
			}  
			free (pFullPath);
		}
	}
																							{
																							#if ENABLETRACE
																							GSSiExitProg (174);
																							#endif
	return HFILE_ERROR;
																							}
																							#if ENABLETRACE
																							}
																							#endif
}

BOOL RequestFileClose (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (175);
#endif
{   
	UINT	i;
	
	if (!KeepFilesOpen)
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
		return FALSE;
}
	for (i=0;i<MAXFILEHANDLES;i++)
		if (OpenFileFid[i] == Fid) 
		{   
	    	short	l= _fstrlen(CachePathnameTo);

    		if (l && !_fstrnicmp (OpenFileStruct[i].szPathName,CachePathnameTo,l))
	    	{
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
					return FALSE;
}
	    	}
			if (OpenFileMode[i] == OF_READ)
			{ 
				OpenFileCloseRequested[i] = TRUE;
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
				return TRUE;
}
			} 
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
			return FALSE;
}
		}   
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

short	GetNumOpenFiles (void)
#if ENABLETRACE
{GSSiEnterProg (176);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (176);
#endif
	return NumOpenFiles;
}
#if ENABLETRACE
}
#endif
}

short GetOpenFileID (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (177);
#endif
{   
	short	i;
	
	if (Fid < 0)
{
#if ENABLETRACE
GSSiExitProg (177);
#endif
		return -1;
}
	for (i=0;i<MAXFILEHANDLES;i++)
		if (OpenFileFid[i] == Fid) 
		{   
{
#if ENABLETRACE
GSSiExitProg (177);
#endif
			return i;
}
		}
{
#if ENABLETRACE
GSSiExitProg (177);
#endif
	return -1;
}
#if ENABLETRACE
}
#endif
}  

void DumpOpenFiles (void)
#if ENABLETRACE
{GSSiEnterProg (178);
#endif
{   
	char	str[256];  
	UINT	i;
	   
	if (!TraceOn)  
{
#if ENABLETRACE
GSSiExitProg (178);
#endif
		return; 
}
	sprintf (str,"%i Open Files:");   
	GSSiTrace (str);
	for (i=0;i<MAXFILEHANDLES;i++)
	{
		if (OpenFileFid[i] != HFILE_ERROR)
		{ 
			sprintf (str,"    -%i %s %u",OpenFileFid[i],OpenFileStruct[i].szPathName,OpenFileMode[i]);    
			GSSiTrace (str);
		}
	}
{
#if ENABLETRACE
GSSiExitProg (178);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL BackupFiles (LPSTR FileList,LPSTR BUDir)
#if ENABLETRACE
{GSSiEnterProg (179);
#endif
{
	HFILE	Fid1;
	OFSTRUCT	OFStruct; 
	char	FromFile[260], ToFile[260];    
	BOOL	rtn=FALSE;
	
	Fid1 = GSSiOpenFile (FileList,&OFStruct,OF_READ);
	if (Fid1 == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (179);
#endif
		return FALSE; 
}
	while (fgetstring (FromFile,256,Fid1))
	{ 
		_fstrcpy (ToFile,BUDir);
		_fstrcat (ToFile,"\\");
		_fstrcat (ToFile,FromFile);  
		if (!makedirectories (ToFile,FALSE,FALSE))
			goto Exit;
		if (!copyfile (ToFile,FromFile,FALSE,0,0,0,0,0,0))   
		{
			GSSiMessageBox ("Backup failed on above file",FromFile,MB_ICONEXCLAMATION);
			goto Exit;                                                                 
		}
	} 
	rtn = TRUE;
Exit:
	GSSiClose (Fid1);
{
#if ENABLETRACE
GSSiExitProg (179);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

void GSSiDeleteObject (HGDIOBJ *handle)
#if ENABLETRACE
{GSSiEnterProg (180);
#endif
{   int	i;
    
    if (*handle && *handle == HighlightBrush)
    	i=1;
	if (*handle)
	{
		if (!DeleteObject (*handle))
			i=1;
		*handle = 0;
	}
{
#if ENABLETRACE
GSSiExitProg (180);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

short loadtabs (LPSHORT Tabs)
#if ENABLETRACE
{GSSiEnterProg (181);
#endif
{
	OFSTRUCT OFStruct;
	HFILE	Fid;   
	char	txt[256]; 
	short	n; 
	LPSTR	lpBeg, lpEnd;
	
	Fid = GSSiOpenFile ("tabs.txt",&OFStruct,OF_READ);
	fgetstring (txt,200,Fid);
	n = 0;  
	lpBeg = txt;
	while (lpBeg)
	{
		if ((lpEnd = _fstrchr (lpBeg,' ')))
			*lpEnd++ = 0;
		*Tabs++ = atoi (lpBeg);
		n++;
		lpBeg = lpEnd; 
	}   
	GSSiClose (Fid);
{
#if ENABLETRACE
GSSiExitProg (181);
#endif
	return n; 
}
#if ENABLETRACE
}
#endif
}

BOOL makedirectories (LPSTR Name,BOOL IsDir,BOOL Verify)
#if ENABLETRACE
{GSSiEnterProg (182);
#endif
{   
	char	FullName[128], Dir[130], FName[64], Ext[8], NewName[128]="", drive[4]; 
	LPSTR	StartDir, EndDir;  
	OFSTRUCT	OFStruct;
	BOOL	Replace;
	
	_fmemset (Dir,0,130);
	_fstrcpy (Dir,Name);
	ExpandText (Dir);
	_fullpath (FullName,Dir,sizeof(FullName));
	EndDir = _fstrchr (FullName,0);
	EndDir++;
	*EndDir = 0;
	_splitpath (FullName,drive,Dir,FName,Ext); 
	
	if (!IsDir || *Ext)	
		sprintf (FullName,"%s%s",drive,Dir);
	else
		sprintf (FullName,"%s%s%s",drive,Dir,FName); 
	StartDir = FullName; 
	while (*StartDir)
	{   
		int	DirCreated;
		
		EndDir = _fstrchr ((LPSTR)(StartDir+1),'\\');
		if (!EndDir)
			EndDir = _fstrchr (StartDir,0); 
		*EndDir = 0;
		if (*StartDir != '\\' && *NewName)
			_fstrcat (NewName,"\\");
		_fstrcat (NewName,StartDir); 
		if (*LastChr (NewName) != ':')
		{   
			
			DirCreated = GSSiMakeDir (NewName);
			if (DirCreated == -1 && errno == ENOENT)
{
#if ENABLETRACE
GSSiExitProg (182);
#endif
				return FALSE;
}
		}
		else
			DirCreated = 1;
		if (!DirCreated && Verify)
		{
			HANDLE	hMess=GSSiGlobAlloc (  79,GMEM_MOVEABLE,256);
			LPSTR	pMess = GlobalLock (hMess);
			int DoCreate;
				
			sprintf (pMess,"The following folder does not exist - do you wish to create it?\r\n%s",NewName);
			DoCreate= MessageBox (NULL,pMess,"Verify folder creation",MB_YESNO|MB_ICONQUESTION); 
			GSSiGlobUlFree (&hMess);  
			if (DoCreate == IDNO) 
			{ 
				GSSiRemoveDir (NewName);
{
#if ENABLETRACE
GSSiExitProg (182);
#endif
				return FALSE;
}
			}		
		}
		StartDir = ++EndDir;
	} 
	
{
#if ENABLETRACE
GSSiExitProg (182);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

double	GetDriveFreeSpace (char Drive)
#if ENABLETRACE
{GSSiEnterProg (183);
#endif
{   
	short	st, idrive;
	char	drive = tolower(Drive);
	
	struct  {
	   unsigned total_clusters;          // Clusters per drive
	   unsigned avail_clusters;          // Available clusters
	   unsigned sectors_per_cluster;     // Sectors per cluster
	   unsigned bytes_per_sector;        // Bytes per sector
	
	}diskspace; 
	
	idrive = drive - 'a' + 1;               
    st = _dos_getdiskfree(idrive, (struct _diskfree_t *)&diskspace ); 
    if (st)
{
#if ENABLETRACE
GSSiExitProg (183);
#endif
    	return -1;
}
    else
{
#if ENABLETRACE
GSSiExitProg (183);
#endif
    	return ((double)diskspace.avail_clusters * (double)diskspace.sectors_per_cluster * (double)diskspace.bytes_per_sector); 
}
#if ENABLETRACE
}
#endif
}

double	GetDriveSize (char Drive)
#if ENABLETRACE
{GSSiEnterProg (184);
#endif
{   
	short	st, idrive;
	char	drive = tolower(Drive);
	
	struct  {
	   unsigned total_clusters;          // Clusters per drive
	   unsigned avail_clusters;          // Available clusters
	   unsigned sectors_per_cluster;     // Sectors per cluster
	   unsigned bytes_per_sector;        // Bytes per sector
	
	}diskspace; 
	
	idrive = drive - 'a' + 1;               
    st = _dos_getdiskfree(idrive, (struct _diskfree_t *)&diskspace ); 
    if (st)
{
#if ENABLETRACE
GSSiExitProg (184);
#endif
    	return -1;
}
    else
{
#if ENABLETRACE
GSSiExitProg (184);
#endif
    	return ((double)diskspace.total_clusters * (double)diskspace.sectors_per_cluster * (double)diskspace.bytes_per_sector); 
}
#if ENABLETRACE
}
#endif
}

long	GetProcessorID (void)
#if ENABLETRACE
{GSSiEnterProg (185);
#endif
{
	DWORD End,Start = GetTickCount();
	DWORD	i; 
	double	j;
	
	for (i=0;i<100000;i++)
		j = sqrt ((double)i);
	End = GetTickCount();
{
#if ENABLETRACE
GSSiExitProg (185);
#endif
	return (End-Start);
}
#if ENABLETRACE
}
#endif
} 

BOOL GetVolumeLabel(WORD wDrive, LPSTR lpBuff)
#if ENABLETRACE
{GSSiEnterProg (187);
#endif
{

    struct _find_t  finfo;
    char            sztmp[10];
    
    *lpBuff = 0;
    _fstrcpy(sztmp, "X:\\*.*");
    sztmp[0] = 'A' + wDrive;
    if (_dos_findfirst(sztmp, _A_VOLID, &finfo) != 0)
{
#if ENABLETRACE
GSSiExitProg (187);
#endif
        return FALSE;
}
    _fstrcpy(lpBuff, finfo.name); 
    Strip (lpBuff,'.');
{
#if ENABLETRACE
GSSiExitProg (187);
#endif
    return TRUE;
}
  
  
#if ENABLETRACE
}
#endif
} 

BOOL AppendFile (LPSTR File,LPSTR Line)
#if ENABLETRACE
{GSSiEnterProg (188);
#endif
{
	OFSTRUCT	OFStruct;
	HFILE		Fid;
	
	Fid = GSSiOpenFile (File,&OFStruct,OF_READWRITE);
	if (Fid == HFILE_ERROR)
		Fid = GSSiOpenFile (File,&OFStruct,OF_CREATE_NODELETE);
	if (Fid == HFILE_ERROR) 
		return FALSE;
	_llseek (Fid,0,2);
	fputstring (Line,Fid);
	GSSiClose (Fid);
{
#if ENABLETRACE
GSSiExitProg (188);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

UINT ItemInList (short i,UINT nElements,HANDLE hElements)
#if ENABLETRACE
{GSSiEnterProg (195);
#endif
{   
	LPSHORT	Elements;
	UINT	j;
	
	if (!nElements)
{
#if ENABLETRACE
GSSiExitProg (195);
#endif
		return 0;
}
	Elements = (LPSHORT)GlobalLock (hElements);
	for (j=0;j<nElements;j++)
		if (i == Elements[j])
		{
			GlobalUnlock (hElements);
{
#if ENABLETRACE
GSSiExitProg (195);
#endif
			return (j+1);
}
		}
	GlobalUnlock (hElements);
{
#if ENABLETRACE
GSSiExitProg (195);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
}

short GetLBSelectedItems (HWND hWndDlg,UINT controlid,LPHANDLE phItems)
#if ENABLETRACE
{GSSiEnterProg (196);
#endif
{
	short	nItems=SendDlgItemMessage(hWndDlg,controlid,LB_GETSELCOUNT,NULL,NULL);
	LPSHORT	pItems;
	
	if (nItems)
	{
		*phItems=GSSiGlobAlloc (  80,GHND,nItems*2);
		pItems=  (LPINT) GlobalLock(*phItems);
		SendDlgItemMessage(hWndDlg,controlid,LB_GETSELITEMS,nItems,(LPARAM)pItems); 
		GlobalUnlock (*phItems);
	}
	else
		*phItems = 0;
{
#if ENABLETRACE
GSSiExitProg (196);
#endif
	return nItems;
}
#if ENABLETRACE
}
#endif
}
  

void SetDlgItemTextGlobal (HWND hWndDlg,UINT icntl,LPSTR Global,LPSTR Init)
#if ENABLETRACE
{GSSiEnterProg (197);
#endif
{
	char	str[256];
	
    GetGlobalCVal (Global,str,Init);
    SetDlgItemText (hWndDlg,icntl,str);
{
#if ENABLETRACE
GSSiExitProg (197);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void GetDlgItemTextGlobal (HWND hWndDlg,UINT icntl,LPSTR Global)
#if ENABLETRACE
{GSSiEnterProg (198);
#endif
{
	char	str[300];
	
	_fstrcpy (str,Global);
	_fstrcat (str,"=");
    GetDlgItemText (hWndDlg,icntl,_fstrchr(str,0),256);
    ExpandText (str);
{
#if ENABLETRACE
GSSiExitProg (198);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL StringEndsWith (LPSTR str,LPSTR endstr)
#if ENABLETRACE
{GSSiEnterProg (199);
#endif
{
	short	le=_fstrlen(endstr);
	short	ls=_fstrlen(str);
	
	if (ls<le)
{
#if ENABLETRACE
GSSiExitProg (199);
#endif
		return FALSE;
}
	str += ls-le;
	if (_fstricmp (str,endstr))
{
#if ENABLETRACE
GSSiExitProg (199);
#endif
		return FALSE;
}
{
#if ENABLETRACE
GSSiExitProg (199);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

LPSTR MatchLev (LPSTR InStr, char MatchChar)
#if ENABLETRACE
{GSSiEnterProg (200);
#endif
{
    short lev=0;
    BOOL    Literal=FALSE; 
    char	BegLev,EndLev;
    
    while (*InStr)
    {    
        if (Literal)
            Literal = FALSE;
        else
        {
            if (*InStr == '@')
                Literal = TRUE;
            else if (*InStr == MatchChar && !lev)
{
#if ENABLETRACE
GSSiExitProg (200);
#endif
                return (InStr);
}
            else if (lev)
            {
				if (*InStr == BegLev)
					lev++;            		
				else if (*InStr == EndLev)
					lev--;            		
			}
            else
            	switch (*InStr)
            	{
            		case '(':
            			EndLev = ')'; 
            			BegLev = *InStr;
            			lev++;
            			break;
            		case '[':
            			EndLev = ']';
            			BegLev = *InStr;
            			lev++;
            			break;
            		case '{':
            			EndLev = '}';
            			BegLev = *InStr;
            			lev++;
            			break; 
            		default:
            			break;
            	}
            		
        }
        InStr++;
    }
{
#if ENABLETRACE
GSSiExitProg (200);
#endif
    return (0);
}
#if ENABLETRACE
}
#endif
}   

LPSTR ldelim (LPSTR str, char delim)
#if ENABLETRACE
{GSSiEnterProg (201);
#endif
{
    short lev=0;
    BOOL    Literal=FALSE;   
    LPSTR   ldlm,lpdelim;
    
    ldlm = 0;
    lpdelim = MatchLev (str,delim);
    while (lpdelim)
    {
        ldlm = lpdelim;
        lpdelim++;
        lpdelim = MatchLev (lpdelim,delim);
    }
{
#if ENABLETRACE
GSSiExitProg (201);
#endif
    return (ldlm);
}
#if ENABLETRACE
}
#endif
}   

LPSTR strncpy0(LPSTR Buff,LPSTR str, size_t n)
#if ENABLETRACE
{GSSiEnterProg (202);
#endif
{
    if (n)
    	_fstrncpy (Buff,str,n);
    Buff[n]=0;
{
#if ENABLETRACE
GSSiExitProg (202);
#endif
    return Buff;
}
#if ENABLETRACE
}
#endif
}  

LPSTR UpcaseFirst (LPSTR s)
{
	BOOL LastWasSpace=TRUE;
	
	while (*s)
	{
		if (*s == ' ')
		{
			LastWasSpace = TRUE;
			s++;  
			continue;
		}
		if (LastWasSpace)
			*s = toupper (*s);
		else
			*s = tolower (*s); 
		LastWasSpace = FALSE;
		s++;
	}
	return s;
}			

LPSTR OneSpace (LPSTR Arg1)
#if ENABLETRACE
{GSSiEnterProg (203);
#endif
{
    BOOL LastWasBlank, HaveNonBlank=FALSE; 
    LPSTR   OutLoc, SaveArg1, SaveOutLoc;
    long    l;  
    HANDLE  handle;
    
    Truncate (Arg1);
    SaveArg1 = Arg1;
    l=_fstrlen(Arg1);
    if (!l)
{
#if ENABLETRACE
GSSiExitProg (203);
#endif
    	return (Arg1);
}
    handle = GSSiGlobAlloc (  81,GMEM_MOVEABLE,l+1);
    OutLoc = GlobalLock (handle);
    SaveOutLoc = OutLoc;
    LastWasBlank = FALSE;  
    while (*Arg1)
    {   
        if (*Arg1 == '\t') *Arg1 = ' ';
        if (*Arg1 != ' ')
        {   
            if (LastWasBlank && HaveNonBlank)
                *OutLoc++ = ' '; 
            LastWasBlank = FALSE;  
            HaveNonBlank=TRUE;
            *OutLoc++ = *Arg1;
        }                        
        else
            LastWasBlank = TRUE;
        Arg1++;
    }
    *OutLoc = 0;    
    _fstrcpy (SaveArg1,SaveOutLoc);
    GlobalUnlock (handle);
    GlobalFree (handle); 
{
#if ENABLETRACE
GSSiExitProg (203);
#endif
    return (Arg1);
}
#if ENABLETRACE
}
#endif
}

void BufWrite (HPSTR *pBuf,LPLONG plBuf,HPSTR data,long ldata)
#if ENABLETRACE
{GSSiEnterProg (204);
#endif
{   
	while (ldata--) 
	{
		*(*pBuf)++ = *data++;   
		(*plBuf)++;
	}
{
#if ENABLETRACE
GSSiExitProg (204);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void BoundsToPoints (LPMNMXCORD pBounds, LPDPOINT Points)
#if ENABLETRACE
{GSSiEnterProg (205);
#endif
{   
	Points[0].x = pBounds->xmn;
	Points[0].y = pBounds->ymn;
	Points[1].x = pBounds->xmn;
	Points[1].y = pBounds->ymx;
	Points[2].x = pBounds->xmx;
	Points[2].y = pBounds->ymx;
	Points[3].x = pBounds->xmx;
	Points[3].y = pBounds->ymn;
{
#if ENABLETRACE
GSSiExitProg (205);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
 
BOOL DPointInBounds (LPDPOINT Point,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (206);
#endif
{    
	 BOOL	rtn=TRUE;
	 if (Point->x > pBounds->xmx ||
	     Point->y > pBounds->ymx ||
	     Point->x < pBounds->xmn ||
	     Point->y < pBounds->ymn)
	     	rtn=FALSE;
{
#if ENABLETRACE
GSSiExitProg (206);
#endif
	 return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayTextFileInRect (HDC hDC,LPRECT Rect,LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (210);
#endif
{
    HFILE	Fid;
    long    CurLine=0;
    OFSTRUCT    OFStruct;
    HFILE   FidOut; 
    char    str[260]; 
    short	x,y, l; 
    DWORD	TextExt;
    
    Fid=GSSiOpenFile (Name,&OFStruct,OF_READ);  
    if (Fid==HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (210);
#endif
    	return FALSE; 
}
    x = Rect->left;
    y = Rect->top;
    while (fgetstring (str,256,Fid))  
    {
		ExtTextOut(hDC, x,y, 0, Rect, str, _fstrlen(str),NULL); 
		l = _fstrlen(str);
		if (!l)
		{
			str[0]=' ';  
			str[1]=0;
			l = 1;
		}
		TextExt = GetTextExtent (hDC,str, l);
			
		y += HIWORD (TextExt);
    }
    GSSiClose (Fid);
{
#if ENABLETRACE
GSSiExitProg (210);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL RemoveLine (LPSTR Name,LPSTR line)
#if ENABLETRACE
{GSSiEnterProg (211);
#endif
{
    FILE    *Fid;
    long    CurLine=0;
    OFSTRUCT    OFStruct;
    HFILE   FidOut; 
    char    str[258];
    
    Fid=fopen (Name,"r");  
    if (!Fid)
{
#if ENABLETRACE
GSSiExitProg (211);
#endif
    	return FALSE; 
}
    FidOut = GSSiOpenFile ("gmtemp.txt",&OFStruct,OF_CREATE);
    
    while (fgetss (str,256,Fid))  
    {
        if (_fstrcmp (line,str))
            fputstring(str,FidOut);
        CurLine++;
    }
    fclose (Fid);
    GSSiClose (FidOut);  
    GSSiRemove ("gmtemp2.txt");
    GSSiRename (Name,"gmtemp2.txt");
    GSSiRename ("gmtemp.txt",Name);
{
#if ENABLETRACE
GSSiExitProg (211);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}
BOOL    GetSaveName2 (HWND hWnd,LPSTR Name, short StringID, LPSTR Ext,UINT FileNameID)
#if ENABLETRACE
{GSSiEnterProg (212);
#endif
{
	char	str[128];
	
    GetCurVal (str,sizeof(str),FileNameID); 
    _fstrcpy (Name,str); 
    if (GetSaveName (hWnd,Name, StringID, Ext))
    {
    	SetCurVal (Name,FileNameID); 
    	ExpandText (Name);
{
#if ENABLETRACE
GSSiExitProg (212);
#endif
    	return TRUE;
}
    }
{
#if ENABLETRACE
GSSiExitProg (212);
#endif
    return FALSE;
}
#if ENABLETRACE
}
#endif
}
    
BOOL    GetSaveName3 (HWND hWnd,LPSTR Name, short StringID, LPSTR Ext,LPSTR Title,LPSTR VarName)
#if ENABLETRACE
{GSSiEnterProg (213);
#endif
{
	char	str[128]="";
	                            
	if (VarName)
	{   
		if (*VarName)
		{
			sprintf (str,"[%s]",VarName);
			ExpandText (str);
		}
		else
			VarName = 0;
	}
    if (Title)
    {
    	if (*Title)
    		OFTitle=Title;
    }
    if (GetSaveName (hWnd,Name, StringID, Ext))
    {   
    	ResetOriginalDrive ();
    	if (VarName)
    		SetGlobalValue (VarName,Name);
{
#if ENABLETRACE
GSSiExitProg (213);
#endif
    	return TRUE;
}
    }
{
#if ENABLETRACE
GSSiExitProg (213);
#endif
    return FALSE;
}
#if ENABLETRACE
}
#endif
} 

BOOL    GetSaveName (HWND hWnd,LPSTR Name, short StringID, LPSTR Ext)
#if ENABLETRACE
{GSSiEnterProg (214);
#endif
{ 
    char    CurDir[_MAX_PATH], InitDir[_MAX_PATH], Drive[_MAX_DRIVE], Dir[_MAX_DIR];
    short       SaveDrive; 
    BOOL    rtn=FALSE; 
    short	ln; 
    LPSTR	lpEnd;
    
    if (!StringID)
        sprintf (gszFilter,"%s(*%s)|*%s|","",Ext,_fstrlwr(Ext));
    SetFilterString (StringID);  
    _getcwd (CurDir,128);
    SaveDrive = _getdrive(); 
    _splitpath (Name,Drive,Dir,0,0); 
    sprintf (InitDir,"%s%s",Drive,Dir);
    ln = _fstrlen (InitDir);
    lpEnd = (LPSTR)InitDir + ln;
    lpEnd--;
    if (*lpEnd == '\\') *lpEnd = 0;
    if (GetSaveFileCD (hWnd,Name,InitDir))
    {   
         if (!_fstrstr(Name,"."))
            _fstrcat(Name,Ext);
         else
         {
            char TestName[128],TestExt[16];
            _fstrcpy(TestName,Name);
            _fstrcpy(TestExt,Ext);
            _fstrupr (TestName); 
            _fstrupr (TestExt);
             if (!_fstrstr(TestName,TestExt))
             {  
                char    str[128];
                sprintf (str,"Must have %s extension",Ext);
                MessageBox(hWnd, str,"Invalid Name", MB_OK|MB_ICONEXCLAMATION);
                goto RestoreDir;
             }
         } 
         SubstituteDL (Name);
         rtn=TRUE;
    }
RestoreDir:
    _chdir (CurDir);
    _chdrive (SaveDrive);   
    OFTitle = 0;
{
#if ENABLETRACE
GSSiExitProg (214);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL    GetFileName2 (HWND hWnd,LPSTR Name,LPSTR Ext,UINT FileNameID)
#if ENABLETRACE
{GSSiEnterProg (215);
#endif
{   
	char	str[128], CurExt[8]; 
	short	l;
	
    sprintf (gszFilter,"%s(*%s)|*%s|","",_fstrupr(Ext),_fstrlwr(Ext)); 
    GetCurVal (str,sizeof(str),FileNameID); 
    _fstrcpy (Name,str);
	if (GetFileName (hWnd,Name,0))
	{
		_splitpath (Name,NULL,NULL,NULL,CurExt);
		if (!*CurExt)
			_fstrcat (Name,Ext);
    	SetCurVal (Name,FileNameID);
    	if (*Name == '@')
    	{
    		l=_fstrlen (Name);
    		_fmemmove (Name,(LPSTR)(Name+1),l);
    	}
{
#if ENABLETRACE
GSSiExitProg (215);
#endif
    	return TRUE;
}
    }
{
#if ENABLETRACE
GSSiExitProg (215);
#endif
    return FALSE;
}

#if ENABLETRACE
}
#endif
}
BOOL    GetFileName3 (HWND hWnd,LPSTR Name,short StringID,UINT FileNameID)
#if ENABLETRACE
{GSSiEnterProg (216);
#endif
{
	char	str[128];
	
    GetCurVal (str,sizeof(str),FileNameID); 
    _fstrcpy (Name,str);
	if (GetFileName (hWnd,Name,StringID))
	{
    	SetCurVal (Name,FileNameID);    
    	ExpandText (Name);
{
#if ENABLETRACE
GSSiExitProg (216);
#endif
    	return TRUE;
}
    }
{
#if ENABLETRACE
GSSiExitProg (216);
#endif
    return FALSE;
}
#if ENABLETRACE
}
#endif
} 

void ResetOriginalDrive (void)
{
	_chdir (OriginalDir);
	_chdrive (OriginalDrive);  
	return;
} 

BOOL    GetFileName4 (HWND hWnd,LPSTR Name,short StringID,LPSTR Ext,LPSTR Title,LPSTR VarName)
#if ENABLETRACE
{GSSiEnterProg (217);
#endif
{
	char	str[128]="";

	if (VarName)
	{   
		if (*VarName)
		{
			sprintf (str,"[%s]",VarName);
			ExpandText (str);
		}
		else
			VarName = 0;
	}
    if (Title)
    {
    	if (*Title)
    		OFTitle=Title;
    }
    if (!StringID)
    	sprintf (gszFilter,"%s(*%s)|*%s|","",_fstrupr(Ext),_fstrlwr(Ext)); 
	if (GetFileName (hWnd,Name,StringID))
    {   
    	if (VarName)
    		SetGlobalValue (VarName,Name);  
    	ResetOriginalDrive	();
{
#if ENABLETRACE
GSSiExitProg (217);
#endif
    	return TRUE;
}
    }
{
#if ENABLETRACE
GSSiExitProg (217);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

BOOL    GetFileName (HWND hWnd,LPSTR Name, short StringID)
#if ENABLETRACE
{GSSiEnterProg (218);
#endif
{ 
    char    CurDir[_MAX_DIR+1], InitDir[_MAX_DIR+1], Drive[_MAX_DRIVE+1], Dir[_MAX_DIR+1];
    short       SaveDrive,ii; 
    BOOL    rtn=FALSE;
    LPSTR   lpEnd, lpPrev;
    
    SaveCurView (0);
    SetFilterString (StringID);
    if (!_getcwd (CurDir,_MAX_DIR))
    {   
    	char	str[64];
    	sprintf (str,"getcwd fails %i",errno);
    	GSSiMessageBox (str,NULL,MB_ICONEXCLAMATION);
    }
//    SetWindowText (hWnd,CurDir);
    SaveDrive = _getdrive(); 
    _splitpath (Name,Drive,Dir,0,0); 
    sprintf (InitDir,"%s%s",Drive,Dir);
    lpEnd = LastChr(InitDir);  
    lpPrev = lpEnd;
    if (lpPrev != InitDir)
    	lpPrev--;
    if (*lpEnd == '\\' && *lpPrev != ':')
    	*lpEnd = 0;
    if (GetOpenFileCD (hWnd,Name,InitDir))
    {   
         rtn=TRUE;  
         SubstituteDL (Name);
    }
    _chdrive (SaveDrive);
    if (_chdrive (SaveDrive))
    	ii=1; 
    if (_chdir (CurDir))
    	ii=1;
    SaveCurView (1);
{
#if ENABLETRACE
GSSiExitProg (218);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}   

void SubstituteDL (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (219);
#endif
{   
	if (GetGlobalBVal ("[%SUBDL]"))
	{  
		char	DL[128]="[%DL]";
		short	l=_fstrlen (Name); 
		HANDLE	hMEM=GSSiGlobAlloc (  82,GMEM_MOVEABLE,l+1);
		LPSTR	TempName = GlobalLock (hMEM);         	
		ExpandText (DL);
		_fstrupr (DL); 
		_fstrcpy (TempName,Name);
		_fstrupr (TempName);
		l = _fstrlen (DL);
		         	
		if (!_fstrnicmp (DL,Name,l))
			sprintf (Name,"@[%cDL]%s",'%',&TempName[l]); 
		GSSiGlobUlFree (&hMEM);
	}
{
#if ENABLETRACE
GSSiExitProg (219);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void ClientRectToScreenRect (HWND hWnd,LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (220);
#endif
{   
    POINT   pt;
    
    pt.x = pRect->left;
    pt.y = pRect->bottom;
    ClientToScreen (hWnd,&pt);
    pRect->left = pt.x;
    pRect->bottom = pt.y;
    pt.x = pRect->right;
    pt.y = pRect->top;
    ClientToScreen (hWnd,&pt);
    pRect->right = pt.x;
    pRect->top = pt.y; 
{
#if ENABLETRACE
GSSiExitProg (220);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void ScreenRectToClientRect (HWND hWnd,LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (220);
#endif
{   
    POINT   pt;
    
    pt.x = pRect->left;
    pt.y = pRect->bottom;
    ScreenToClient (hWnd,&pt);
    pRect->left = pt.x;
    pRect->bottom = pt.y;
    pt.x = pRect->right;
    pt.y = pRect->top;
    ScreenToClient (hWnd,&pt);
    pRect->right = pt.x;
    pRect->top = pt.y; 
{
#if ENABLETRACE
GSSiExitProg (220);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

LPSTR ValueConv (double Value, short ValConv,double RoundTo, BOOL Commas)
#if ENABLETRACE
{GSSiEnterProg (224);
#endif
{   static char VCBuff[32];
    char        fmt[32];
    LPSTR   p1, p2, bp, pDot;
    short       n, ndp;  
    double  rndp;
    
    if (RoundTo <= 0) RoundTo = 1; 
    Value = Round (Value,RoundTo);
    if (RoundTo<1)
    {
        rndp = log10(RoundTo); 
        ndp = abs((short)IDNINT(rndp)); 
        sprintf (fmt,"%%.%if",ndp);
        sprintf (VCBuff,fmt,Value);
    }
    else
        sprintf (VCBuff,"%.0f",Value); 
    if (Commas)
    	AddCommas (VCBuff);
{
#if ENABLETRACE
GSSiExitProg (224);
#endif
    return (VCBuff);  
}
#if ENABLETRACE
}
#endif
}

LPSTR AddCommas (LPSTR InBuff)
#if ENABLETRACE
{GSSiEnterProg (225);
#endif
{
	char	Buff[32],VCBuff[32];
    LPSTR   p1, p2, bp, pDot;
    short	n;
	
	if (_fstrlen (InBuff) > 32)
{
#if ENABLETRACE
GSSiExitProg (225);
#endif
		return InBuff;
}
    _fstrcpy (Buff,InBuff);
    bp = Buff; 
    p2=&VCBuff[31]; 
    pDot = _fstrchr (Buff,'.');
    if (pDot)
    {
    	p1 = pDot-1;
    	pDot = _fstrchr(Buff,0);
    	while (pDot > p1)
    		*p2-- = *pDot--;
    }
    else
    {
    	p1 = _fstrchr(Buff,0)-1;
    	*p2-- = 0;
    }
    for (n=0;p1>=bp;p1--,n++)
    {   if (n == 3 && *p1 != '-')
        {   *p2--=',';
            n=0;
        }
        *p2--=*p1;
    }
    ++p2;
    _fstrcpy (InBuff,p2);
{
#if ENABLETRACE
GSSiExitProg (225);
#endif
    return InBuff;
}
#if ENABLETRACE
}
#endif
}

short   Strip(LPSTR str, char chr)
#if ENABLETRACE
{GSSiEnterProg (226);
#endif
{   short   n=0;
    LPSTR strnew;
                  
    strnew = str;                 
    while (*str)
    {
        if (*str != chr)
        {            
            *strnew = *str;
            strnew++;      
        }
        else
            n++;
        str++;
    }
    *strnew='\0';
{
#if ENABLETRACE
GSSiExitProg (226);
#endif
    return n;
}
#if ENABLETRACE
}
#endif
}

void RWRITE (double RVAL, short NDP, LPSTR OutLoc) 
 
#if ENABLETRACE
{GSSiEnterProg (227);
#endif
{
    char    Format[16];
            
    sprintf (Format,"%%.%if",NDP);
    sprintf (OutLoc,Format,RVAL); 
{
#if ENABLETRACE
GSSiExitProg (227);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

void RWRITEZ (double RVAL, short NDP, short len,LPSTR OutLoc) 
 
#if ENABLETRACE
{GSSiEnterProg (227);
#endif
{
    char    Format[16];
            
    sprintf (Format,"%%0%i.%if",len,NDP);
    sprintf (OutLoc,Format,RVAL); 
{
#if ENABLETRACE
GSSiExitProg (227);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

double dread (LPSTR str, short nbytes)
#if ENABLETRACE
{GSSiEnterProg (228);
#endif
{
    char    savechar;
    LPSTR   pSaveChar;
    double  rval;
    
    pSaveChar = str + nbytes;
    savechar = *pSaveChar;
    *pSaveChar = 0;       
    rval = atof(str);
    *pSaveChar = savechar;
{
#if ENABLETRACE
GSSiExitProg (228);
#endif
    return rval;
}
#if ENABLETRACE
}
#endif
}
        

long ldread (LPSTR str, short nbytes)
#if ENABLETRACE
{GSSiEnterProg (229);
#endif
{
    char    savechar;
    LPSTR   pSaveChar;
    long    rval;
    
    pSaveChar = str + nbytes;
    savechar = *pSaveChar;
    *pSaveChar = 0;       
    rval = atol(str);
    *pSaveChar = savechar;
{
#if ENABLETRACE
GSSiExitProg (229);
#endif
    return rval;
}
#if ENABLETRACE
}
#endif
}
        
 
BOOL rread (LPSTR str, LPDOUBLE lpRval, LPINT lpNdp)
#if ENABLETRACE
{GSSiEnterProg (230);
#endif
{
    LPSTR lpEnd, lpDp;
    
    *lpRval = strtod (str,&lpEnd);
    if (*lpEnd)
{
#if ENABLETRACE
GSSiExitProg (230);
#endif
    	return FALSE;
}
    lpDp = _fstrchr (str,'.');
    if (lpDp)
        *lpNdp = lpEnd - lpDp;
    else
        *lpNdp = 0;
{
#if ENABLETRACE
GSSiExitProg (230);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}
    

LPSTR ReplaceChar (LPSTR str, char from, char to)
#if ENABLETRACE
{GSSiEnterProg (231);
#endif
{
    LPSTR Instr;
    
    Instr = str;
    while (*str)
    {
        if (*str == from) *str = to;
        str++;
    }
{
#if ENABLETRACE
GSSiExitProg (231);
#endif
    return Instr; 
}
#if ENABLETRACE
}
#endif
}  

LPSTR RemoveDoubleQuotes (LPSTR str)
#if ENABLETRACE
{GSSiEnterProg (232);
#endif
{        
	LPSTR	EndChar=str;
	short	l;
	
	if (*str != '"')
{
#if ENABLETRACE
GSSiExitProg (232);
#endif
		return str;
}
	l = _fstrlen (str);
	if (l < 2)
{
#if ENABLETRACE
GSSiExitProg (232);
#endif
		return str;
}
	EndChar += l-1;
	if (*EndChar != '"')
{
#if ENABLETRACE
GSSiExitProg (232);
#endif
		return str;
}
	l -= 2;
	if (!l)
		*str = 0;
	else
	{
		_fmemmove (str,(LPSTR)(str+1),l);
		EndChar = str+l;
		*EndChar = 0;
	}
{
#if ENABLETRACE
GSSiExitProg (232);
#endif
	return str;
}
#if ENABLETRACE
}
#endif
}

LPSTR ConvertCharBtwnDoubleQuotes (LPSTR str, char from, char to)
#if ENABLETRACE
{GSSiEnterProg (233);
#endif
{   
	LPSTR	Begin, End, rtn=str;
	
Next:
	Begin = _fstrchr (str,'"'); 
	if (!Begin)
{
#if ENABLETRACE
GSSiExitProg (233);
#endif
		return rtn;
}
	Begin++;
	End = _fstrchr (Begin,'"');
	if (!End)
{
#if ENABLETRACE
GSSiExitProg (233);
#endif
		return str;
}
	while (Begin < End)
	{
		if (*Begin == from)
			*Begin = to;
		Begin++;
	}
	str = Begin + 1;
	goto Next;
#if ENABLETRACE
}
#endif
}

short DrawRectPoly(HDC hDC, LPRECT Rect, HPEN hPen)
#if ENABLETRACE
{GSSiEnterProg (234);
#endif
{
    POINT   Points[5];
    HPEN    CurPen=0;
    short       i;
    if (hPen)
        CurPen = SelectObject (hDC,hPen);
    Points[0].x = Rect->left;
    Points[0].y = Rect->bottom;
    Points[1].x = Rect->left;
    Points[1].y = Rect->top;    
    Points[2].x = Rect->right;
    Points[2].y = Rect->top;
    Points[3].x = Rect->right;
    Points[3].y = Rect->bottom; 
    Points[4] = Points[0];
    i = Polyline (hDC,Points,5);         
    if (hPen)
    {
    	if (IsGDIObject (CurPen))
        	SelectObject (hDC,CurPen);
        else
        	SelectObject (hDC,GetStockObject(BLACK_PEN));
    }
{
#if ENABLETRACE
GSSiExitProg (234);
#endif
    return (i);
}
#if ENABLETRACE
}
#endif
}  
 
void hmemmove (HPSTR out,HPSTR in,DWORD len)
#if ENABLETRACE
{GSSiEnterProg (235);
#endif
{
	while (len--)
		*out++ = *in++;   
{
#if ENABLETRACE
GSSiExitProg (235);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void hmemset (HPSTR out,char in,DWORD len)
#if ENABLETRACE
{GSSiEnterProg (236);
#endif
{
	while (len--)
		*out++ = in;   
{
#if ENABLETRACE
GSSiExitProg (236);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL ReverseBOOL (BOOL In)
#if ENABLETRACE
{GSSiEnterProg (237);
#endif
{
	if (In)
{
#if ENABLETRACE
GSSiExitProg (237);
#endif
		return FALSE;
}
{
#if ENABLETRACE
GSSiExitProg (237);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

HGLOBAL GSSiGlobAlloc (USHORT From,UINT fuAlloc, long cbAlloc)
#if ENABLETRACE
{GSSiEnterProg (238);
#endif
{   HGLOBAL handle;
	char	pMess[24];
    short	ii;
#if CHECKMEM  
	LogMemAlloc (From,cbAlloc);
#endif
    if (cbAlloc <= 0)
   {    
        MessageBox( 0, "Invalid memory allocation",0, MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL);
        BlowOut(NULL,NULL);
   }
    if (cbAlloc == 2) 
    	ii=1;  
//    sprintf (pMess,"Alloc %ld",(long)cbAlloc);
//    ShowTextTrace (pMess);
    handle = GlobalAlloc (fuAlloc, (DWORD)cbAlloc);
//    *pMess=0;
//    ShowTextTrace (pMess);
    if (!handle)
   {    
   		
   		ltoa ((long)cbAlloc,pMess,10);
        MessageBox( 0, "Out of Mem",pMess, MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL);  
        PostMessage(hWndMain, WM_CLOSE, 0, 0L);  
        BlowOut(NULL,NULL);
   }
    
{
#if ENABLETRACE
GSSiExitProg (238);
#endif
    return (handle);
}
#if ENABLETRACE
}
#endif
}  

BOOL IntersectBounds (LPMNMXCORD pBounds1,LPMNMXCORD pBounds2,LPMNMXCORD pBoundsInt)
#if ENABLETRACE
{GSSiEnterProg (239);
#endif
{   
	if (pBounds1->xmn > pBounds2->xmx || 
		pBounds1->ymn > pBounds2->ymx ||
		pBounds1->xmx < pBounds2->xmn ||
		pBounds1->ymx < pBounds2->ymn)
{
#if ENABLETRACE
GSSiExitProg (239);
#endif
		return FALSE;
}
	pBoundsInt->xmn = max (pBounds1->xmn,pBounds2->xmn);
	pBoundsInt->ymn = max (pBounds1->ymn,pBounds2->ymn);
	pBoundsInt->xmx = min (pBounds1->xmx,pBounds2->xmx);
	pBoundsInt->ymx = min (pBounds1->ymx,pBounds2->ymx);
{
#if ENABLETRACE
GSSiExitProg (239);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL RectInRect(LPRECT pRectIn,LPRECT pRectTest)
#if ENABLETRACE
{GSSiEnterProg (240);
#endif
{   
	RECT	IntRect;
{
#if ENABLETRACE
GSSiExitProg (240);
#endif
	return IntersectRect(&IntRect,pRectIn,pRectTest);
}
#if ENABLETRACE
}
#endif
}
     
void InflateBounds (LPMNMXCORD pBounds, double Value)
#if ENABLETRACE
{GSSiEnterProg (241);
#endif
{      
	pBounds->xmn -= Value;
	pBounds->xmx += Value;
	pBounds->ymn -= Value;
	pBounds->ymx += Value;
{
#if ENABLETRACE
GSSiExitProg (241);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

DPOINT FPointToDPoint (FPOINT point)
#if ENABLETRACE
{GSSiEnterProg (242);
#endif
{
	DPOINT	DPoint;
	
	DPoint.x = point.x;
	DPoint.y = point.y;
{
#if ENABLETRACE
GSSiExitProg (242);
#endif
	return DPoint;
}
#if ENABLETRACE
}
#endif
}

FPOINT DPointToFPoint (DPOINT DPoint)
#if ENABLETRACE
{GSSiEnterProg (243);
#endif
{
	FPOINT FPoint ={DPoint.x,DPoint.y};
	
{
#if ENABLETRACE
GSSiExitProg (243);
#endif
	return FPoint;
}
#if ENABLETRACE
}
#endif
}

DPOINT PointToDPoint (POINT Point)
#if ENABLETRACE
{GSSiEnterProg (244);
#endif
{
	DPOINT p;
	
	p.x = Point.x;
	p.y = Point.y;
{
#if ENABLETRACE
GSSiExitProg (244);
#endif
	return p;
}
#if ENABLETRACE
}
#endif
}

POINT DPointToPoint (DPOINT Point)
#if ENABLETRACE
{GSSiEnterProg (245);
#endif
{
	POINT p;
	
	p.x = max ((long)SHRT_MIN,min ((long)SHRT_MAX,IDNINT (Point.x)));
	p.y = max ((long)SHRT_MIN,min ((long)SHRT_MAX,IDNINT (Point.y)));
{
#if ENABLETRACE
GSSiExitProg (245);
#endif
	return p;
}
#if ENABLETRACE
}
#endif
}

void AddPointToRect (POINT Point,LPRECT pBounds)
#if ENABLETRACE
{GSSiEnterProg (246);
#endif
{   
    pBounds->left = min (pBounds->left,Point.x);
    pBounds->right = max (pBounds->right,Point.x);
    pBounds->top = min (pBounds->top,Point.y);
    pBounds->bottom = max (pBounds->bottom,Point.y);
{
#if ENABLETRACE
GSSiExitProg (246);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void MinMaxInit (LPMINMAX pMinMax)
#if ENABLETRACE
{GSSiEnterProg (247);
#endif
{ 
	pMinMax->xmn=SHRT_MAX;
	pMinMax->ymn=SHRT_MAX;
	pMinMax->xmx=SHRT_MIN;
	pMinMax->ymx=SHRT_MIN;    
{
#if ENABLETRACE
GSSiExitProg (247);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void MinMaxInitL (LPMNMXCORL pMinMax)
#if ENABLETRACE
{GSSiEnterProg (248);
#endif
{ 
	pMinMax->xmn=LONG_MAX;
	pMinMax->ymn=LONG_MAX;
	pMinMax->xmx=LONG_MIN;
	pMinMax->ymx=LONG_MIN;    
{
#if                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          