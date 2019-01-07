#include "graphint.h"    
#include "dibapi.h"
#include <shlobj.h>
#include <sys\types.h>
#include <sys\stat.h>    
#include <tlhelp32.h>   
#include	"minilzo.h"
#pragma optimize( "", off )
/*
 *  The CRC table is alocated and defined at run-time.
 */

#define	MAXNOTFOUND		128				
#define W_FAR far

    typedef struct
        {   long    FileNum;
            long    FileLength;
            time_t  LastUsed;
            char    FileName[128];
        }   CACHEBUF; 

extern BOOL CALLBACK EnumCtrlProc(HWND hCtrl,LONG lParam);
#include "gmextern.h"

static	char	LastTextFile[MAX_PATH]="";
static	HANDLE	hSavedScreens[MAXSAVEDSCREENS];
static	short	nSavedScreens=0;      
static	long	NextScreenID=1;
static	USHORT	crc_table[256]={0}; 
static  BOOL    FirstCache = TRUE; 
static	short	CurTraceLev=0;
static	HANDLE	hCacheAlreadyChecked=0;
static	BOOL    TraceTrace = FALSE, NeedStep=FALSE, StepOver=FALSE;;  
static  BYTE    Mask[8] = {128, 64, 32, 16, 8, 4, 2, 1}; 
static	OFSTRUCTGM	OpenFileStruct[MAXFILEHANDLES];
static	UINT		OpenFileMode[MAXFILEHANDLES];
static	HANDLE		OpenFileHandle[MAXFILEHANDLES];
static	long		OpenFilePosition[MAXFILEHANDLES];
static	BOOL		OpenFileCloseRequested[MAXFILEHANDLES]; 
static	BOOL		FidIsMapped[MAXFILEHANDLES];
static	LPVOID		FidPtr[MAXFILEHANDLES];
//static	FILE		*FidStream[MAXFILEHANDLES];
static	HANDLE		FidHandle[MAXFILEHANDLES];
static	BOOL		InOpenFile=FALSE;
static	HWND	hWndTrace=0;

static char	OpenFileName[MAXFILEHANDLES][MAX_PATH];
static char	LastPathName[MAX_PATH];
static int	LastAccessedFid=0;
static HWND	FontListhWndDlg;
static UINT	FontListCntl;
static short	MaxWaitCycles=5;
static short	have_crc_table=0;
static HFILE	TraceFid=HFILE_ERROR;
static char		SaveWinText[144];
static DLGPROC	lpfnTRACEWINDOWMsgProc;  
static int		NumTries=0,NumSuccess=0;
static char		NotFoundList[MAXNOTFOUND][MAX_PATH];
static int		NotFoundCode[MAXNOTFOUND];
static BYTE		BlockPadding[JOURNAL_BLOCK_SIZE];

void HaltMapDisplay(BOOL ClearCFGStack, BOOL saveScreen);
void DebugShowLine (LPDPOINT p1,LPDPOINT p2);
int ActuallyCloseFile (HFILE Fid);

extern HWND	TraceWnd2;

char	CacheTitle[256];


double square(double val)
{
	return val * val;
}

__int64 FileTimeToint64(FILETIME ft)
{
    ULARGE_INTEGER    lv_Large ;

    lv_Large.LowPart  = ft.dwLowDateTime   ;
    lv_Large.HighPart = ft.dwHighDateTime  ;

    return lv_Large.QuadPart ;
}  

__int64 HighLowToint64(DWORD HighPart,DWORD LowPart)
{
    ULARGE_INTEGER    lv_Large ;

    lv_Large.LowPart  = LowPart;
    lv_Large.HighPart = HighPart;

    return lv_Large.QuadPart ;
}  

long Time64toTime32 (time_t time64)
{
	long	rtn;
	//converts 64-bit integer specifying the number of 100-nanosecond
    //intervals which have passed since January 1, 1601.
    //This 64-bit value is split into the
    //two 32 bits  stored in the structure.
    __int64	d=116444736000000000L; //difference between 1601 and 1970
    //we divide by 10million to convert to seconds
    //return (((long(h)<< 32) + long(l))-d)/10000000  ;
	rtn = (time64 - d) / 10000000;
	return rtn;
}

HFILE OpenFileGM(
	_In_    LPCSTR lpFileName,
	_Inout_ LPOFSTRUCTGM lpReOpenBuff,
	_In_    UINT uStyle
	)
{
	HFILE fid = (int)INVALID_HANDLE_VALUE;
	char *fullPath;
	int ln;
	char	Name[MAX_PATH];
	int		rtn;

	strcpy(Name, lpFileName);
	ExpandText(Name);

	if (uStyle == OF_CREATE && !makedirectories(Name, FALSE, TRUE))
		return HFILE_ERROR;

	memset(lpReOpenBuff, 0, sizeof(OFSTRUCTGM));
	fullPath = _fullpath(lpReOpenBuff->szPathName, Name, OFS_MAXPATHNAMEGM);
	if (!fullPath)
		return HFILE_ERROR;
	ln = strlen(fullPath);
	if (ln < OFS_MAXPATHNAME)
	{
		fid = OpenFile(fullPath,(LPOFSTRUCT) lpReOpenBuff, uStyle);
		return fid;
	}
	else switch (uStyle)
	{
	case OF_READ:
		fid = (int)CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		break;
	case OF_READWRITE:
		fid = (int)CreateFile(lpFileName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		break;
	case OF_CREATE:
		fid = (int)CreateFile(lpFileName, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		break;
	case OF_EXIST:
		if (GetPathType2((LPSTR)lpFileName) == 1)
			fid = 1;
		break;
	case OF_DELETE:
		if (!remove(lpFileName))
			fid = 1;
	default:
		MessageBox(0, "Invalid Style in OpenFileGM", 0, MB_ICONEXCLAMATION);
		break;
	}
	if (fid == HFILE_ERROR)
		lpReOpenBuff->nErrCode = GetLastError();
	return fid;
}

BOOL FileOpenForWrite (HFILE Fid)
{
	if (Fid != HFILE_ERROR)
	{
		if (OpenFileMode[Fid] == OF_CREATE || OpenFileMode[Fid] == OF_READWRITE)
			return TRUE;
	}
	return FALSE;
}

BOOL WaitForProcessToEnd (DWORD pID,LPINT pMaxWait)
{
	BOOL rtn=FALSE;
	PROCESSENTRY32 pe;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

	pe.dwSize = sizeof (pe);
	 
	if (Process32First (hSnapshot,&pe))
		do
		{
			if (pe.th32ProcessID == pID)
			{
				rtn = TRUE;
				Sleep (100);
				*pMaxWait = max (0,*pMaxWait - 100);
				if (!*pMaxWait)
					rtn = FALSE;
				break;
			}

		}while (Process32Next (hSnapshot,&pe));
	
	CloseHandle (hSnapshot);
	return rtn;
}

void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
 // Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
	return;
}

void UnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst)
{
     FILETIME ft;

     UnixTimeToFileTime(t, &ft);
     FileTimeToSystemTime(&ft, pst);
	 return;
}


int GetAvailableFid (void)
{
	int	i,j;

	for (i=1;i<MAXFILEHANDLES;i++) 
		if (OpenFileFid[i] == HFILE_ERROR)
			return i;

	i = -1;
	for (j=1;j<MAXFILEHANDLES;j++)
	{
		if (OpenFileFid[j] != HFILE_ERROR) 
		{
			if (OpenFileCloseRequested[j])
			{
				ActuallyCloseFile (j);
				i = j; 
				break;
			}
		}
	}
	if (i >= 0)
		return i;  
   // DumpOpenFiles ();
	GSSiMsgBox (GetFocus(),"Open file limit exceeded",0,MB_ICONEXCLAMATION,0); 
 	 BlowOut(0,0);
	return i;
}

int OpenJournal (LPSTR Name,HFILE Fid,UINT Mode)
//return -1 if journal does not exist, 0 if file deleted in journal or 1 if journal exists
#if ENABLETRACE
{GSSiEnterProg (1387);
#endif
{
	char JournalFileName[MAX_PATH];
	LPSTR	pDot;
	HFILE	FidJnl;
	OFSTRUCTGM	OFStruct;
	JOURNALHEADER	JournalHeader;
	LPLONG	pNumIndexBlocks;
	BOOL	st;
	int		rtn=0;

	strcpy (JournalFileName,Name);
	if ((pDot = strrchr (JournalFileName,'.')))
		*pDot = '_';
	strcat (JournalFileName,".jnl");
	if (Mode == OF_CREATE)
		goto Create;
/*	{
		FidJnl = OpenFileGM (JournalFileName,&OFStruct,OF_CREATE);
		OpenFileFid[Fid] = FidJnl;
		OpenFilePosition[Fid] = 0;
		OpenFileLength[Fid] = OriginalFileLength[Fid] = 0;
		JournalFileFid[Fid] = FidJnl;
		JournalFileIndex[Fid] = 0;
		JournalIsCompleteFile[Fid] = 1;
		strcpy (OpenFileName[Fid],Name);
		return 1;
	}*/
	FidJnl = OpenFileGM (JournalFileName,&OFStruct,Mode);
	if (FidJnl != HFILE_ERROR)
	{
		_lread (FidJnl,&JournalHeader,sizeof(JOURNALHEADER));
		if (JournalHeader.CheckPointID != CurrentCheckPointID)
		{
			int	st;

			_lclose (FidJnl);
			st = remove (JournalFileName);
			st = errno;
		}
		else
		{
			LPJOURNALINDEXRECORD pIndexRecord;
			int	iFid,ii;

			OpenFilePosition[Fid] = 0;
			OpenFileLength[Fid] = JournalHeader.FileLength;
			OriginalFileLength[Fid] = GSSifilelength (Fid);
			if (JournalHeader.NumBlocks > 0)
			{
				JournalFileIndex[Fid] = GSSiGlobAlloc (1599,GHND,sizeof(int)+(JournalHeader.NumBlocks / 1024 + 1)*1024*sizeof(JOURNALINDEXRECORD));
				pNumIndexBlocks = GlobalLock (JournalFileIndex[Fid]);
				*pNumIndexBlocks = JournalHeader.NumBlocks;
				pIndexRecord = (LPJOURNALINDEXRECORD)(pNumIndexBlocks+1);
				_llseek (FidJnl,JournalHeader.BlockIndexLoc,0);
				_lread (FidJnl,pIndexRecord,JournalHeader.NumBlocks*sizeof(JOURNALINDEXRECORD));
				if (Mode != OF_READ)
				{
					_lclose (FidJnl);
					iFid = _open (JournalFileName,_O_RDWR);
					ii=_chsize(iFid,JournalHeader.BlockIndexLoc);
					_close (iFid);
					FidJnl = OpenFileGM (JournalFileName,&OFStruct,Mode);
				}
				GlobalUnlock (JournalFileIndex[Fid]);
			}
			else if (JournalHeader.HasBeenDeleted && Mode == OF_READ)
			{
				_lclose (FidJnl);
				goto Exit;
			}
			else
			{
				if (JournalHeader.OrigFileLength < 0)
					JournalFileIndex[Fid] = 0;
				else
					JournalFileIndex[Fid] = GSSiGlobAlloc (1599,GHND,sizeof(int)+1024*sizeof(JOURNALINDEXRECORD));
				OriginalFileLength[Fid] = JournalHeader.OrigFileLength;
			}
			if (JournalHeader.OrigFileLength < 0)
				JournalIsCompleteFile[Fid] = 1;
			else
				JournalIsCompleteFile[Fid] = 0;

			JournalFileFid[Fid] = FidJnl;
			rtn = 1;
			goto Exit;
		}
	}
	if (Mode == OF_READ)
	{
		rtn = -1;
		goto Exit;
	}
Create:
	FidJnl = OpenFileGM (JournalFileName,&OFStruct,OF_CREATE);
	memset (&JournalHeader,0,sizeof(JOURNALHEADER));
	JournalHeader.CheckPointID = CurrentCheckPointID;
	JournalHeader.BlockIndexLoc = -1;
	JournalHeader.BlockSize = JOURNAL_BLOCK_SIZE;
	OpenFilePosition[Fid] = 0;
	if (Mode == OF_CREATE)
	{
		JournalFileIndex[Fid] = 0;
		OriginalFileLength[Fid] = -1;
		JournalHeader.OrigFileLength = -1;
		OpenFileLength[Fid] = 0;
		JournalIsCompleteFile[Fid] = 1;
	}
	else
	{
		JournalHeader.OrigFileLength = OpenFileLength[Fid] = OriginalFileLength[Fid] = GSSifilelength (Fid);
		JournalFileIndex[Fid] = GSSiGlobAlloc (1599,GHND,sizeof(int)+1024*sizeof(JOURNALINDEXRECORD));
		JournalIsCompleteFile[Fid] = 0;
	}
	_lwrite (FidJnl,(LPSTR)&JournalHeader,sizeof(JOURNALHEADER));
	st=FlushFileBuffers ((HANDLE)FidJnl);
	JournalFileFid[Fid] = FidJnl;
	rtn = 1;
Exit:
{
#if ENABLETRACE
GSSiExitProg (1387);
#endif
		return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL CloseJournal (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (1388);
#endif
{
	BOOL	rtn=FALSE;

	if (JournalFileFid[Fid] != HFILE_ERROR)
	{
		LPSTR pFile, pFilesWithJournals;
		int	len,ii;
		JOURNALHEADER	JournalHeader;

		_llseek (JournalFileFid[Fid],0,0); 
		_lread (JournalFileFid[Fid],&JournalHeader,sizeof(JOURNALHEADER));
		JournalHeader.FileLength = OpenFileLength[Fid];
		if (!JournalIsCompleteFile[Fid])
		{
			LPLONG	pNumIndexBlocks = GlobalLock (JournalFileIndex[Fid]);
			LPJOURNALINDEXRECORD pIndexRecord = (LPJOURNALINDEXRECORD)(pNumIndexBlocks+1);

			JournalHeader.NumBlocks = *pNumIndexBlocks;
			JournalHeader.BlockIndexLoc = _llseek (JournalFileFid[Fid],0,2); 
			ii=_lwrite (JournalFileFid[Fid],(LPSTR)pIndexRecord,JournalHeader.NumBlocks*sizeof(JOURNALINDEXRECORD));
			GSSiGlobUlFree (&JournalFileIndex[Fid]);
		}
		JournalFileIndex[Fid] = 0;
		ii=_llseek (JournalFileFid[Fid],0,0); 
		ii=_lwrite (JournalFileFid[Fid],(LPSTR)&JournalHeader,sizeof(JOURNALHEADER));
		if (!_lclose (JournalFileFid[Fid]))
			rtn = TRUE;
		JournalFileFid[Fid] = HFILE_ERROR;
		len = strlen (OpenFileName[Fid]);
		if (!len)
			ii=1;
		if (!hFilesWithJournals)
		{
			lFilesWithJournals = len+1;
			hFilesWithJournals = GSSiGlobAlloc (1600,GMEM_MOVEABLE,lFilesWithJournals);
			pFilesWithJournals = GlobalLock (hFilesWithJournals);
			strcpy (pFilesWithJournals,OpenFileName[Fid]);
		}
		else
		{
			pFile = pFilesWithJournals = GlobalLock (hFilesWithJournals);

			while (pFile < &pFilesWithJournals[lFilesWithJournals])
			{
				if (!stricmp (pFile,OpenFileName[Fid]))
					goto Exit;
				pFile = strchr (pFile,0) + 1;
			}
			GlobalUnlock (hFilesWithJournals);
			if (!(hFilesWithJournals = GSSiGlobalReAlloc (0,hFilesWithJournals,len+lFilesWithJournals+1,GMEM_MOVEABLE)))
				BlowOut (OpenFileName[Fid],"Failed to reallocate memory for journal file list");
			pFilesWithJournals = GlobalLock (hFilesWithJournals);
			strcpy (&pFilesWithJournals[lFilesWithJournals],OpenFileName[Fid]);
			lFilesWithJournals += (len+1);
		}
Exit:
		JournalIsCompleteFile[Fid] = 0;
		GlobalUnlock (hFilesWithJournals);
	}
{
#if ENABLETRACE
GSSiExitProg (1388);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL ApplyJournal (LPSTR FileName)
#if ENABLETRACE
{GSSiEnterProg (1389);
#endif
{
	BOOL rtn=FALSE;
	char JournalFileName[MAX_PATH];
	LPSTR	pDot;
	HFILE	FidJnl;
	OFSTRUCTGM	OFStruct;
	JOURNALHEADER	JournalHeader;
	LPLONG	pNumIndexBlocks;
	BOOL	st;
	int		ii;

	strcpy (JournalFileName,FileName);
	if ((pDot = strrchr (JournalFileName,'.')))
		*pDot = '_';
	strcat (JournalFileName,".jnl");
	FidJnl = OpenFileGM (JournalFileName,&OFStruct,OF_READ);
	if (FidJnl != HFILE_ERROR)
	{
		_lread (FidJnl,&JournalHeader,sizeof(JOURNALHEADER));
		if (JournalHeader.CheckPointID == CurrentCheckPointID)
		{
			if (JournalHeader.HasBeenDeleted) //file deleted
			{
				ii=remove (FileName);
				ii=errno;
				rtn = TRUE;
			}
			else if (JournalHeader.OrigFileLength < 0) //file created
			{
				HFILE	FidFile = OpenFileGM(FileName,&OFStruct,OF_CREATE);
				HANDLE	hJournalRecord = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
				LPBYTE	pJournalRecord = GlobalLock (hJournalRecord);
				int		nread;

				ii = _llseek(FidJnl,0,2);
				ii = _llseek(FidJnl, sizeof(JOURNALHEADER), 0);
				while ((nread = _lread(FidJnl, pJournalRecord, USHRT_MAX)) > 0)
					_lwrite (FidFile,pJournalRecord,nread);
				GSSiGlobUlFree (&hJournalRecord);
				rtn = TRUE;
				_lclose (FidFile);
			}
			else
			{
				LPJOURNALINDEXRECORD pIndexRecord;
				HANDLE hIndex = GSSiGlobAlloc (1599,GHND,sizeof(int)+JournalHeader.NumBlocks*sizeof(JOURNALINDEXRECORD));
				int	i;
				HFILE	FidFile = OpenFileGM(FileName,&OFStruct,OF_READWRITE);

				char	JLogFile[MAX_PATH];
				HFILE	JLog=HFILE_ERROR;
				short	ln;

				pNumIndexBlocks = GlobalLock (hIndex);
				pIndexRecord = (LPJOURNALINDEXRECORD)(pNumIndexBlocks+1);
				ii=_llseek (FidJnl,JournalHeader.BlockIndexLoc,0);
				ii=_lread (FidJnl,pIndexRecord,JournalHeader.NumBlocks*sizeof(JOURNALINDEXRECORD));
				if (GetGlobalCVal ("[%JLOG]",JLogFile,0))
				{
					JLog = OpenFileGM (JLogFile,&OFStruct,OF_READWRITE);
					if (JLog == HFILE_ERROR)
							JLog = OpenFileGM (JLogFile,&OFStruct,OF_CREATE);
					_llseek (JLog,0,2);
					ln = strlen (JournalFileName);
					_lwrite (JLog,(LPSTR)&ln,2);
					_lwrite (JLog,(LPSTR)JournalFileName,ln);
					_lwrite (JLog,(LPSTR)&JournalHeader,sizeof(JOURNALHEADER));
					_lwrite (JLog,(LPSTR)pIndexRecord,JournalHeader.NumBlocks*sizeof(JOURNALINDEXRECORD));
					_lclose (JLog);
				}
				for (i=0;i<JournalHeader.NumBlocks;i++,pIndexRecord++)
				{
					int	StartLoc = pIndexRecord->StartBlock*JournalHeader.BlockSize;
					int	BytesToRead = JournalHeader.BlockSize*pIndexRecord->NumBlocks;
					if (BytesToRead > 0)
					{
						HANDLE	hJournalRecord = GSSiGlobAlloc (0,GMEM_MOVEABLE,BytesToRead);
						LPBYTE	pJournalRecord = GlobalLock (hJournalRecord);

						_llseek (FidJnl,pIndexRecord->JournalFileOffset,0);
						_llseek (FidFile,StartLoc,0);
						BytesToRead = min (BytesToRead,JournalHeader.FileLength - StartLoc);
						_lread  (FidJnl,pJournalRecord,BytesToRead);
						_lwrite (FidFile,pJournalRecord,BytesToRead);
						GSSiGlobUlFree (&hJournalRecord);
						rtn = TRUE;
					}
					else
						ii=1;
				}
				GSSiGlobUlFree (&hIndex);
				_lclose (FidFile);
			}
		}
		_lclose (FidJnl);
		if (!DeleteFile(JournalFileName))
		{
			ii = GetLastError();
		}
	}

{
#if ENABLETRACE
GSSiExitProg (1389);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL DeleteFileInJournal (LPSTR FileName)
#if ENABLETRACE
{GSSiEnterProg (1390);
#endif
{
	BOOL rtn=FALSE;
	char JournalFileName[MAX_PATH];
	LPSTR	pDot;
	HFILE	FidJnl;
	OFSTRUCTGM	OFStruct;
	JOURNALHEADER	JournalHeader;
	LPLONG	pNumIndexBlocks;
	BOOL	st;

	if (!ExistFile(FileName))
		return rtn;
	strcpy (JournalFileName,FileName);
	if ((pDot = strrchr (JournalFileName,'.')))
		*pDot = '_';
	strcat (JournalFileName,".jnl");
	FidJnl = OpenFileGM (JournalFileName,&OFStruct,OF_CREATE);
	if (FidJnl != HFILE_ERROR)
	{
		memset (&JournalHeader,0,sizeof(JOURNALHEADER));
		JournalHeader.CheckPointID = CurrentCheckPointID;
		JournalHeader.HasBeenDeleted = TRUE;
		_lwrite (FidJnl,(LPSTR)&JournalHeader,sizeof(JOURNALHEADER));
		_lclose (FidJnl);
		rtn = TRUE;
	}

{
#if ENABLETRACE
GSSiExitProg (1390);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL ChangeSizeOfFileInJournal (HFILE Fid,int NewLength)
#if ENABLETRACE
{GSSiEnterProg (1391);
#endif
{
	BOOL	rtn=FALSE;
	int		i;

	if (JournalFileFid[Fid] != HFILE_ERROR)
	{
		JOURNALHEADER	JournalHeader;
		LPLONG	pNumIndexBlocks = GlobalLock (JournalFileIndex[Fid]);
		LPJOURNALINDEXRECORD pIndexRecord = (LPJOURNALINDEXRECORD)(pNumIndexBlocks+1);
		LPSTR pFile, pFilesWithJournals;
		int	len,ii;

		_llseek (JournalFileFid[Fid],0,0); 
		_lread (JournalFileFid[Fid],&JournalHeader,sizeof(JOURNALHEADER));
		for (i=0;i<*pNumIndexBlocks;i++,pIndexRecord++)
		{
			if (pIndexRecord->StartBlock*JournalHeader.BlockSize > NewLength)
				break;
		}
		JournalHeader.NumBlocks = *pNumIndexBlocks = i;
		JournalHeader.FileLength = NewLength;
		_llseek (JournalFileFid[Fid],0,0); 
		_lwrite (JournalFileFid[Fid],(LPSTR)&JournalHeader,sizeof(JOURNALHEADER));
		rtn = TRUE;
		OpenFileLength[Fid] = NewLength;
		GlobalUnlock (JournalFileIndex[Fid]);
	}
{
#if ENABLETRACE
GSSiExitProg (1391);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

int GetJournalBlock (HFILE Fid,long StartBlock,LPLONG pnBlocksInJournal,LPLONG pJournalBlockLoc,LPLONG pNextPos)
#if ENABLETRACE
{GSSiEnterProg (1392);
#endif
{
	long	BlockID=-1;
	long	BeginBlock, EndBlock, iBlock;
	LPLONG	pNumIndexBlocks = GlobalLock (JournalFileIndex[Fid]);
	LPJOURNALINDEXRECORD pIndexRecord = (LPJOURNALINDEXRECORD)(pNumIndexBlocks+1);
	int	ii;

if (Fid >= MAXFILEHANDLES)
	BlowOut ("Fid > max","ERROR");
if (!pNumIndexBlocks)
{
	char	mess[512];

	sprintf (mess,"%ld:%ld %s",(int)Fid,(int)JournalFileIndex[Fid],OpenFileName[Fid]);
	MessageBox (0,mess,0,MB_OK);
	BlowOut ("mess","ERROR");
}
	*pNextPos = 0;
	if (*pNumIndexBlocks)
	{
		BeginBlock = 0;
		EndBlock = *pNumIndexBlocks - 1;
Next:
		if (StartBlock < pIndexRecord[BeginBlock].StartBlock)
		{
			BlockID = pIndexRecord[BeginBlock].StartBlock;
			*pnBlocksInJournal = pIndexRecord[BeginBlock].NumBlocks;
			*pJournalBlockLoc = pIndexRecord[BeginBlock].JournalFileOffset;
			*pNextPos = BeginBlock;
			goto Exit;
		}
		if (StartBlock < pIndexRecord[BeginBlock].StartBlock + pIndexRecord[BeginBlock].NumBlocks)
		{
			BlockID = StartBlock;
			*pnBlocksInJournal = pIndexRecord[BeginBlock].NumBlocks - (StartBlock - pIndexRecord[BeginBlock].StartBlock);
			*pJournalBlockLoc = pIndexRecord[BeginBlock].JournalFileOffset + (StartBlock - pIndexRecord[BeginBlock].StartBlock) * JOURNAL_BLOCK_SIZE;
			goto Exit;
		}
		if (StartBlock >= pIndexRecord[EndBlock].StartBlock)
		{
			if (StartBlock < pIndexRecord[EndBlock].StartBlock + pIndexRecord[EndBlock].NumBlocks)
			{
				BlockID = StartBlock;
				*pnBlocksInJournal = pIndexRecord[EndBlock].NumBlocks - (StartBlock - pIndexRecord[EndBlock].StartBlock);
				*pJournalBlockLoc = pIndexRecord[EndBlock].JournalFileOffset + (StartBlock - pIndexRecord[EndBlock].StartBlock) * JOURNAL_BLOCK_SIZE;
				goto Exit;
			}
			if (EndBlock < *pNumIndexBlocks - 1)
			{
				EndBlock++;
				BlockID = pIndexRecord[EndBlock].StartBlock;
				*pnBlocksInJournal = pIndexRecord[EndBlock].NumBlocks;
				*pJournalBlockLoc = pIndexRecord[EndBlock].JournalFileOffset;
				*pNextPos = EndBlock;
			}
			else
			{
				*pNextPos = EndBlock + 1;
				*pJournalBlockLoc = _llseek (JournalFileFid[Fid],0,2);
			}
			goto Exit;
		}
		if (EndBlock - BeginBlock > 1)
		{
			int MidBlock = BeginBlock + (EndBlock - BeginBlock) / 2;

			if (StartBlock < pIndexRecord[MidBlock].StartBlock)
				EndBlock = MidBlock;
			else
				BeginBlock = MidBlock;
			goto Next;
		}
		else
		{
			BlockID = pIndexRecord[EndBlock].StartBlock;
			*pnBlocksInJournal = pIndexRecord[EndBlock].NumBlocks;
			*pJournalBlockLoc = pIndexRecord[EndBlock].JournalFileOffset;
			*pNextPos = EndBlock;
			goto Exit;
		}
	}
Exit:
	GlobalUnlock (JournalFileIndex[Fid]);
{
#if ENABLETRACE
GSSiExitProg (1392);
#endif
	return BlockID;
}
#if ENABLETRACE
}
#endif
}

void AddBlockToJournalIndex (HFILE Fid,int StartBlock,int nBlocks,int Offset)
#if ENABLETRACE
{GSSiEnterProg (1393);
#endif
{
	long	nBlocksInJournal,JournalBlockLoc,NextPos;
	LPLONG	pNumIndexBlocks = GlobalLock (JournalFileIndex[Fid]);
	long	nrecs = *pNumIndexBlocks + 1;
	LPJOURNALINDEXRECORD pIndexRecord;

	if (nrecs < 0)
		BlowOut (OpenFileName[Fid],"nrecs < 0");
	GlobalUnlock (JournalFileIndex[Fid]);
	if (nrecs % 1024 == 0)
	{
		if (!(JournalFileIndex[Fid] = GSSiGlobalReAlloc (0,JournalFileIndex[Fid],sizeof(long)+(nrecs+1024)*sizeof(JOURNALINDEXRECORD),GMEM_MOVEABLE)))
		{
			char	Mess[128];

			sprintf (Mess,"Failed to reallocate memory for journal:%i",nrecs);
			BlowOut (OpenFileName[Fid],Mess);
		}
	}
	pNumIndexBlocks = GlobalLock (JournalFileIndex[Fid]);
	pIndexRecord = (LPJOURNALINDEXRECORD)(pNumIndexBlocks+1);
	if (*pNumIndexBlocks)
	{
		GetJournalBlock (Fid,StartBlock,&nBlocksInJournal,&JournalBlockLoc,&NextPos);
		if (NextPos < nrecs-1)
			memmove (&pIndexRecord[NextPos+1],&pIndexRecord[NextPos],sizeof(JOURNALINDEXRECORD)*(nrecs-NextPos));
	}
	else
		NextPos = 0;
	pIndexRecord[NextPos].StartBlock = StartBlock;
	pIndexRecord[NextPos].NumBlocks  = nBlocks;
	pIndexRecord[NextPos].JournalFileOffset = Offset;
	*pNumIndexBlocks = nrecs;
	GlobalUnlock (JournalFileIndex[Fid]);
{
#if ENABLETRACE
GSSiExitProg (1393);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

long WriteWithJournal2 (HFILE Fid,LPBYTE pMF,DWORD isize)
#if ENABLETRACE
{GSSiEnterProg (1394);
#endif
{
	long	written=0;
	long	BlockID;
	long	StartLoc = GSSillseek (Fid,0,1), EndLoc = StartLoc + isize - 1;
	long	StartBlock = StartLoc / JOURNAL_BLOCK_SIZE;
	long	BytesFromStartOfBlock = StartLoc % JOURNAL_BLOCK_SIZE;
	long	EndBlock   = (StartLoc + isize -1) / JOURNAL_BLOCK_SIZE;
	long	nBlocksInJournal,JournalBlockLoc,NextPos;
	long	BytesToEndOfBlock;
	
	while (isize > 0)
	{
		long	FirstBlockInJournal = GetJournalBlock (Fid,StartBlock,&nBlocksInJournal,&JournalBlockLoc,&NextPos);
		long	StartLoc2, EndLoc2;
		int		BytesToWrite;

		if (FirstBlockInJournal == -1)
		{
			BytesToEndOfBlock = JOURNAL_BLOCK_SIZE - ((StartLoc + isize -1) % JOURNAL_BLOCK_SIZE) -1;
			BytesToWrite = isize;
			JournalBlockLoc = _llseek (JournalFileFid[Fid],0,2);
			if (BytesFromStartOfBlock)
			{
				_lseek (OpenFileFid[Fid],StartBlock * JOURNAL_BLOCK_SIZE,0);
				_read (OpenFileFid[Fid],BlockPadding,BytesFromStartOfBlock);
				_lwrite (JournalFileFid[Fid],BlockPadding,BytesFromStartOfBlock);
			}
			written += _lwrite (JournalFileFid[Fid],pMF,BytesToWrite);
			if (BytesToEndOfBlock)
			{
				if (EndLoc < OriginalFileLength[Fid])
				{
					_lseek (OpenFileFid[Fid],EndLoc+1,0);
					_read (OpenFileFid[Fid],BlockPadding,BytesToEndOfBlock);
				}
				_lwrite (JournalFileFid[Fid],BlockPadding,BytesToEndOfBlock);
			}
			AddBlockToJournalIndex (Fid,StartBlock,EndBlock-StartBlock+1,JournalBlockLoc);
		}
		else if (StartBlock == FirstBlockInJournal)
		{
			BytesToWrite = min (isize,JOURNAL_BLOCK_SIZE - BytesFromStartOfBlock + (nBlocksInJournal - 1) * JOURNAL_BLOCK_SIZE);
			_llseek (JournalFileFid[Fid],JournalBlockLoc+BytesFromStartOfBlock,0);
			written += _lwrite (JournalFileFid[Fid],pMF,BytesToWrite);
			StartBlock += nBlocksInJournal;
			BytesFromStartOfBlock = 0;
		}
		else
		{
			StartLoc2 = StartBlock * JOURNAL_BLOCK_SIZE + BytesFromStartOfBlock;
			BytesToWrite = min (isize,JOURNAL_BLOCK_SIZE - BytesFromStartOfBlock + (FirstBlockInJournal - StartBlock - 1) * JOURNAL_BLOCK_SIZE);
			EndLoc2 = StartLoc2 + BytesToWrite - 1;
			BytesToEndOfBlock = JOURNAL_BLOCK_SIZE - (EndLoc2 % JOURNAL_BLOCK_SIZE) - 1;
			EndBlock   = (StartLoc2 + BytesToWrite -1) / JOURNAL_BLOCK_SIZE;
			JournalBlockLoc = _llseek (JournalFileFid[Fid],0,2);
			if (BytesFromStartOfBlock)
			{
				_lseek (OpenFileFid[Fid],StartBlock * JOURNAL_BLOCK_SIZE,0);
				_read (OpenFileFid[Fid],BlockPadding,BytesFromStartOfBlock);
				_lwrite (JournalFileFid[Fid],BlockPadding,BytesFromStartOfBlock);
			}
			written += _lwrite (JournalFileFid[Fid],pMF,BytesToWrite);
			if (BytesToEndOfBlock)
			{
				if (EndLoc2 < OriginalFileLength[Fid])
				{
					_lseek (OpenFileFid[Fid],EndLoc2+1,0);
					_read (OpenFileFid[Fid],BlockPadding,BytesToEndOfBlock);
				}
				_lwrite (JournalFileFid[Fid],BlockPadding,BytesToEndOfBlock);
			}
			AddBlockToJournalIndex (Fid,StartBlock,EndBlock-StartBlock+1,JournalBlockLoc);
			StartBlock = FirstBlockInJournal;
			BytesFromStartOfBlock = 0;
		}
		if (BytesToWrite <= 0)
			BlowOut ("Bytes to write le 0","ERROR");
		pMF += BytesToWrite;
		isize -= BytesToWrite;
		StartLoc += BytesToWrite;
	}
	OpenFilePosition[Fid] = EndLoc + 1;
	OpenFileLength[Fid] = max (OpenFileLength[Fid],OpenFilePosition[Fid]);
{
#if ENABLETRACE
GSSiExitProg (1394);
#endif
	return written;
}
#if ENABLETRACE
}
#endif
}

long WriteWithJournal (HFILE Fid,LPBYTE pMF,DWORD isize)
#if ENABLETRACE
{GSSiEnterProg (1395);
#endif
{
	long	written;

	if (JournalIsCompleteFile[Fid])
	{
		int	Curpos = GSSillseek (Fid,0,1);
		
		_llseek (JournalFileFid[Fid],Curpos + sizeof(JOURNALHEADER),0);
		written = _lwrite (JournalFileFid[Fid],pMF,isize);
		OpenFilePosition[Fid] = Curpos + isize;
		OpenFileLength[Fid] = max (OpenFileLength[Fid],OpenFilePosition[Fid]);
	}
	else
		written = WriteWithJournal2 (Fid,pMF,isize);
{
#if ENABLETRACE
GSSiExitProg (1395);
#endif
	return written;
}
#if ENABLETRACE
}
#endif
}


long ReadWithJournal2 (HFILE Fid,LPBYTE pMF,DWORD isize)
#if ENABLETRACE
{GSSiEnterProg (1396);
#endif
{
	long	nread=0;
	long	BlockID;
	long	StartLoc = GSSillseek (Fid,0,1), EndLoc = StartLoc + isize;
	long	StartBlock = StartLoc / JOURNAL_BLOCK_SIZE;
	long	BytesFromStartOfBlock = StartLoc % JOURNAL_BLOCK_SIZE;
	long	EndBlock   = (StartLoc + isize) / JOURNAL_BLOCK_SIZE;
	long	nBlocksInJournal,JournalBlockLoc,NextPos;
	long	BytesToEndOfBlock;

	BYTE	dba[128];
	int		ii;
	
	while (isize > 0)
	{
		int		BytesToRead=isize;
		long	FirstBlockInJournal = GetJournalBlock (Fid,StartBlock,&nBlocksInJournal,&JournalBlockLoc,&NextPos);

		if (FirstBlockInJournal == -1)
		{
			_lseek (OpenFileFid[Fid],StartLoc,0);
			nread += _read (OpenFileFid[Fid],pMF,BytesToRead);
		}
		else if (StartBlock == FirstBlockInJournal)
		{
			BytesToRead = min (isize,JOURNAL_BLOCK_SIZE - BytesFromStartOfBlock + (nBlocksInJournal - 1) * JOURNAL_BLOCK_SIZE);
			ii=_llseek (JournalFileFid[Fid],JournalBlockLoc+BytesFromStartOfBlock,0);
			nread += _lread (JournalFileFid[Fid],pMF,BytesToRead);
			StartBlock += nBlocksInJournal;
			BytesFromStartOfBlock = 0;
		}
		else
		{
			BytesToRead = min (isize,JOURNAL_BLOCK_SIZE - BytesFromStartOfBlock + (FirstBlockInJournal - StartBlock - 1) * JOURNAL_BLOCK_SIZE);
			_lseek (OpenFileFid[Fid],StartLoc,0);
			nread += _read (OpenFileFid[Fid],pMF,BytesToRead);
			StartBlock = FirstBlockInJournal;
			BytesFromStartOfBlock = 0;
		}
		memmove (dba,pMF,min(BytesToRead,128));
		OpenFilePosition[Fid] += BytesToRead;
		pMF += BytesToRead;
		StartLoc += BytesToRead;
		isize -= BytesToRead;
		if (BytesToRead <= 0)
			BlowOut ("Bytes to read le 0","ERROR");
	}
{
#if ENABLETRACE
GSSiExitProg (1396);
#endif
	return nread;
}
#if ENABLETRACE
}
#endif
}

long ReadWithJournal (HFILE Fid,LPBYTE pMF,DWORD isize)
#if ENABLETRACE
{GSSiEnterProg (1397);
#endif
{
	long	nread;

	if (JournalIsCompleteFile[Fid])
	{
		int	Curpos = GSSillseek (Fid,0,1);
		
		_llseek (JournalFileFid[Fid],Curpos + sizeof(JOURNALHEADER),0);
		nread = _lread (JournalFileFid[Fid],pMF,isize);
		OpenFilePosition[Fid] += nread;
	}
	else
		nread = ReadWithJournal2 (Fid,pMF,isize);
{
#if ENABLETRACE
GSSiExitProg (1397);
#endif
	return nread;
}
#if ENABLETRACE
}
#endif
}

long BigWrite (HFILE Fid,LPVOID pMF,DWORD isize,long loc)
#if ENABLETRACE
{GSSiEnterProg (389);
#endif
{   
	long	written=0;
	long	ii;
	
	if (Fid == HFILE_ERROR)
		goto Exit;
	if (FidMemLen[Fid])
	{
		LPBYTE pMem = GlobalLock (OpenFileHandle[Fid]);

		memcpy(&pMem[OpenFilePosition[Fid]],pMF,isize);
		OpenFilePosition[Fid] += isize;
		OpenFileLength[Fid] = max (OpenFileLength[Fid],OpenFilePosition[Fid]);
		GlobalUnlock (OpenFileHandle[Fid]);
		written = isize;
		goto Exit;
	}
	if (FidIsMapped[Fid])   
	{
		WriteErrorHandler (Fid,isize,loc);
		goto Exit;
	}
	if (!isize)
		goto Exit;
	LastAccessedFid = Fid;
	if (JournalFileFid[Fid] != HFILE_ERROR)
		written = WriteWithJournal (Fid,pMF,isize);
	else
	{
/*	if (UndoEnabled && NumCheckPoints)
	{   
		short	UndoFileID = GetUndoFileIDFromFid (Fid);
		long	SizeAtLastCheckpoint = GetSizeAtLastCheckpoint (UndoFileID);
		
		if (SizeAtLastCheckpoint > 0)
		{
			if (loc < 0)
				loc = GSSillseek (Fid,0,1);   
			if (loc < SizeAtLastCheckpoint)
			{
				HANDLE	hSave = GSSiGlobAlloc (  99,GMEM_MOVEABLE,isize);
				HPSTR	pSave = GlobalLock (hSave);
				
				ii=BigRead (Fid,pSave,isize);
				GSSillseek (Fid,loc,0); 
				if (UndoFileID == -1)
					ii=1;
				SaveDataToUndoFile (UNDO_WRITE,UndoFileID,isize,pSave,loc);
				GSSiGlobUlFree (&hSave);
			}
		}
	}*/
	/*	if (JournalFileIndex[Fid])
		{
			DWORD loc = GSSillseek2 (OpenFileFid[Fid],0,1);
			int	StartBlock = loc / 1024;
			int	EndBlock   = (loc + isize-1) / 1024;
			int	iBlock;
			LPLONG pNumBlocks = GlobalLock (JournalFileIndex[Fid]);
			LPBYTE	pUpdateMask = (LPBYTE)(pNumBlocks+1);

			for (iBlock = StartBlock;iBlock <= EndBlock;iBlock++)
			{
				if (iBlock <= *pNumBlocks)
					SetBitH (iBlock,pUpdateMask,TRUE);
			}
			GlobalUnlock (JournalFileIndex[Fid]);
		}*/
		if (OpenFileFid[Fid] < 0)
			WriteErrorHandler (Fid,isize,loc);
		else
		if ((written = _write (OpenFileFid[Fid],pMF,isize)) != isize) 
		{
			int ier;

			if (written == -1)
				ier = errno;
			WriteErrorHandler (Fid,isize,loc);
		}
	}
Exit:
{
#if ENABLETRACE
GSSiExitProg (389);
#endif
		return written;
}
#if ENABLETRACE
}
#endif
}

int	GetParmLoc (int MaxParm,char delim,LPSTR str,LPSTR *pLoc)
{
	int nFound=1;
	int	i;

	for (i=1;i<MaxParm;i++)
		pLoc[i] = strchr (str,0);

	pLoc[0] = str;
	MaxParm--;
	while (MaxParm--)
	{
		if ((pLoc[nFound] = MatchLev (pLoc[nFound-1],delim)))
			*pLoc[nFound++]++ = 0;
		else
			break;
	}
	return nFound;
}

LPSTR GetLastAccessedFile (void)
{
	return OpenFileName[LastAccessedFid];
}

LPSTR GetLastPathname (void)
{
	return LastPathName;
}

int GetLongPathFromBuffer (LPSTR Name,short MaxLen)
{
	static	HANDLE hGLPBuffer=0;
	static	int		lGLPBuffer;
	LPSTR	pGLPBuffer, pName1, pName2;
	short	l;
	int		rtn=0, loc=0;
	typedef struct {short ln1,ln2;} HEADER;
	HEADER	*pHeader;

	if (!Name)
	{
		GSSiGlobFree (&hGLPBuffer);
		return 0;
	}
	if (!hGLPBuffer)
	{
		if (!MaxLen)
			return 0;
		hGLPBuffer = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
		pGLPBuffer = GlobalLock (hGLPBuffer);
Top:
		lGLPBuffer = 4;
		pHeader = (HEADER*)pGLPBuffer;
		strcpy (&pGLPBuffer[lGLPBuffer],Name);
		pHeader->ln1 = strlen (Name)+1;
		lGLPBuffer += pHeader->ln1;
		pHeader->ln2 = GetLongPathName (Name,&pGLPBuffer[lGLPBuffer],MaxLen);
		if (pHeader->ln2)
		{
			rtn = pHeader->ln2;
			pHeader->ln2++;
			strcpy (Name,&pGLPBuffer[lGLPBuffer]);
		}
		else
			rtn = 0;
		lGLPBuffer += pHeader->ln2;
		lGLPBuffer += lGLPBuffer % 2;
		GlobalUnlock (hGLPBuffer);
		return rtn;
	}
	pGLPBuffer = GlobalLock (hGLPBuffer);
	while (loc < lGLPBuffer)
	{
		pHeader = (HEADER*)&pGLPBuffer[loc];
		pName1 = &pGLPBuffer[loc+4];
		pName2 = &pGLPBuffer[loc+4+pHeader->ln1];
		if (!stricmp (Name,pName1))
		{
			if (MaxLen)
			{
				if (pHeader->ln2)
				{
					rtn = pHeader->ln2-1;
					strcpy (Name,pName2);
				}
				else
					rtn = 0;
			}
			else
			{
				l = 4 + pHeader->ln1 + pHeader->ln2;
				l += l % 2;
				if (lGLPBuffer - (loc+l)>0)
					memmove (&pGLPBuffer[loc],&pGLPBuffer[loc+l],lGLPBuffer - (loc+l));
				lGLPBuffer -= l;
			}
			GlobalUnlock (hGLPBuffer);
			return rtn;
		}
		l = 4 + pHeader->ln1 + pHeader->ln2;
		loc += l + l % 2;
	}
	if (!MaxLen)
	{
		GlobalUnlock (hGLPBuffer);
		return 0 ;
	}
	if (lGLPBuffer > (USHRT_MAX - 520))
		goto Top;
	pHeader = (HEADER*)&pGLPBuffer[loc];
	lGLPBuffer += 4;
	strcpy (&pGLPBuffer[lGLPBuffer],Name);
	pHeader->ln1 = strlen (Name)+1;
	lGLPBuffer += pHeader->ln1;
	pHeader->ln2 = GetLongPathName (Name,&pGLPBuffer[lGLPBuffer],MaxLen);
	if (pHeader->ln2)
	{
		pHeader->ln2++;
		strcpy (Name,&pGLPBuffer[lGLPBuffer]);
	}
	lGLPBuffer += pHeader->ln2;
	lGLPBuffer += lGLPBuffer % 2;
	rtn = pHeader->ln2;
	GlobalUnlock (hGLPBuffer);
	return rtn;
}

int GetLongPathName2 (LPSTR Name,short MaxLen) //maxlen 0 is from GSSiRemove2
{
	int icpf, l;
	char	CacheFromName[MAX_PATH];

    if (AllowCache)
	{
		for (icpf = 0;icpf < NumCachePathnameFrom; icpf++)
		{   
    		_fstrcpy (CacheFromName,CachePathnameFrom[icpf]); 
    		ExpandText (CacheFromName);
			l = _fstrlen(CacheFromName);
			if (l)
			{
				if (CacheFromName[l-1] != '\\')
				{
	    			CacheFromName[l++] = '\\';
	    			CacheFromName[l] = 0;
				}
				if (!_fstrnicmp (Name,CacheFromName,l))
					return GetLongPathFromBuffer (Name,MaxLen);
			}
		}
	}
	if (MaxLen)
		return GetLongPathName3 (Name,MaxLen);
	return 0;
}
BOOL NameContainsDL (LPSTR Name)
{
	char	DL[MAX_PATH]="[%DL]";
	char	TestName[MAX_PATH];

	ExpandText (DL);
	strcpy (TestName,Name);
	ExpandText (TestName);
	if (strnicmp (DL,TestName,strlen(DL)))
		return FALSE;
	return TRUE;
}

int	FileAlreadyNotFound (LPSTR Name,int opt,int ierrno)
{
	int		rc=0;
	static	int	NumNotFound;
	UINT	i;
	
	if (KeepFilesOpen)
	switch (opt)
	{
		case 0://clear list
			memset (NotFoundList,0,sizeof(NotFoundList));
			NumNotFound = 0;
			break;
		case 1://check list
			for (i=0;i<NumNotFound;i++)
				if (!stricmp (NotFoundList[i],Name))
					return NotFoundCode[i];
			break;
		case 2://add to list
			for (i=0;i<NumNotFound;i++)
				if (!stricmp (NotFoundList[i],Name))
					return NotFoundCode[i];
			if (NumNotFound < MAXNOTFOUND)
			{
				NotFoundCode[NumNotFound]=ierrno;
				strcpy (NotFoundList[NumNotFound++],Name);
			}
			break;
		case 3://remove from list
			for (i=0;i<NumNotFound;i++)
				if (!stricmp (NotFoundList[i],Name))
				{
					if (i < NumNotFound-1)
					{
						memmove (&NotFoundList[i],&NotFoundList[i+1],(NumNotFound-i-1)*MAX_PATH);
						memmove (&NotFoundCode[i],&NotFoundCode[i+1],(NumNotFound-i-1)*sizeof(int));
					}
					NumNotFound--;
					CacheAlreadyChecked (Name,-strlen(CachePathnameTo),0);
				}
			break;
	}
	return rc;
}

HFILE OpenFileGSSi (LPSTR Name,LPOFSTRUCTGM pOFStruct,UINT opt,UINT ShareOpt)
{
	HFILE	Fid;
	UINT	opt2=_O_RDONLY|_O_BINARY, pmode=0,i,ii;
	static	First=TRUE;
	int		Err;

	if (First)
	{
		_umask (0);
		First=FALSE;
		memset (FidIsMapped,0,sizeof(FidIsMapped));
		memset (OpenFileName,0,sizeof(OpenFileName));
		FileAlreadyNotFound (0,0,0);
		memset (FidHandle,0,sizeof(FidHandle));
		memset (JournalFileIndex,0,sizeof(JournalFileIndex));
		memset (JournalIsCompleteFile,0,sizeof(JournalIsCompleteFile));
	}
	if (opt == OF_READ)
	{
		if (!InOpenFile && (Err=FileAlreadyNotFound (Name,1,0)))
		{
			pOFStruct->nErrCode = Err;
			return HFILE_ERROR;
		}
		if (UseMappedFiles)
		{
				HANDLE	handle;

				Fid = _open (Name,opt2,0);
				NumActualOpen++;
				_fullpath (pOFStruct->szPathName,Name,OFS_MAXPATHNAMEGM);
				if (Fid == HFILE_ERROR)
				{
					if (errno == EACCES)
						pOFStruct->nErrCode = 5;
					else
						pOFStruct->nErrCode = errno;
					FileAlreadyNotFound (Name,2,pOFStruct->nErrCode);
					return Fid;
				}
				pOFStruct->nErrCode = 0;
				Fid = LogOpenFilesOpen (opt,Fid,pOFStruct);
				strcpy (OpenFileName[Fid],Name);
			   /* Change handle access to stream access. 
			   if( (stream = _fdopen( Fid, "rbR" )) == NULL )
			   {
				   _close (Fid);
				  return HFILE_ERROR;
			   }*/
			   
			   if (!(OpenFileLength[Fid] = GSSifilelength (Fid)))
				   return Fid;
			   OpenFilePosition[Fid] = 0;
			   OpenFileHandle[Fid] = 
				   CreateFile (Name,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
			   if (OpenFileHandle[Fid] == (HANDLE)INVALID_HANDLE_VALUE)
			   {
				   OpenFileLength[Fid] = -2;
				   return Fid;
			   }
			   FidHandle[Fid] = CreateFileMapping (OpenFileHandle[Fid],0,PAGE_READONLY,0,0,0);
			   if (!FidHandle[Fid])
			   {
				   CloseHandle (OpenFileHandle[Fid]);
				   OpenFileLength[Fid] = -3;
				   return Fid;
			   }
			   FidPtr[Fid] = MapViewOfFile (FidHandle[Fid],FILE_MAP_READ,0,0,0);
			   NumTries++;
			   if (FidPtr[Fid] == NULL)
			   {
				   CloseHandle (FidHandle[Fid]);
				   CloseHandle (OpenFileHandle[Fid]);
				   OpenFileLength[Fid] = -1;
				   return Fid;
			   }
			   NumSuccess++;
			   FidIsMapped[Fid]=TRUE;
			   return Fid;
		}
		else
			opt2 = _O_RDONLY|_O_BINARY;
	}
	if (opt == OF_EXIST)
	{
		if ((Err=FileAlreadyNotFound (Name,1,0)))
		{
			pOFStruct->nErrCode = Err;
			return HFILE_ERROR;
		}
		opt2 = _O_RDONLY;
		if (AllowJournal && CurrentCheckPointID)
		{
			Fid = GetAvailableFid ();
			switch (OpenJournal (Name,Fid,OF_READ))
			{
			case 1: 
				CloseJournal (Fid);
				OpenFileFid[Fid] = HFILE_ERROR;
				return 1;
			case 0:
				CloseJournal (Fid);
				OpenFileFid[Fid] = HFILE_ERROR;
				return HFILE_ERROR;
			default:
				OpenFileFid[Fid] = HFILE_ERROR;
				break;
			}
		}
	}
	if (opt == OF_WRITE)
		opt2 = _O_WRONLY|_O_BINARY;
	if (opt == OF_READWRITE)
		opt2 = _O_RDWR|_O_BINARY;
	if (opt == OF_READWRITE && AllowJournal && CurrentCheckPointID && !ExistFile(Name))
	{
		_fullpath(pOFStruct->szPathName, Name, OFS_MAXPATHNAMEGM);
		pOFStruct->nErrCode = 0;
		Fid = LogOpenFilesOpen(opt, (HFILE)-2, pOFStruct);
		if (OpenJournal(Name, Fid, OF_CREATE) == 1)
			CloseJournal(Fid);
		strcpy(OpenFileName[Fid], Name);
		NumActualOpen++;
		return Fid;
	}
	if (opt == OF_CREATE)
	{
		if (AllowJournal && CurrentCheckPointID)
		{
			_fullpath(pOFStruct->szPathName, Name, OFS_MAXPATHNAMEGM);
			pOFStruct->nErrCode = 0;
			Fid = LogOpenFilesOpen(opt, (HFILE)-2, pOFStruct);
			OpenJournal(Name, Fid, OF_CREATE);
			strcpy(OpenFileName[Fid], Name);
			NumActualOpen++;
			return Fid;
		}
		else
		{
			GSSiRemove2(Name);
			FileAlreadyNotFound (Name,3,0);
			opt2 = _O_CREAT|_O_RDWR|_O_BINARY;
			pmode = _S_IREAD | _S_IWRITE;
		}
	}
/*#define OF_WRITE            0x00000001
#define OF_READWRITE        0x00000002
#define OF_SHARE_COMPAT     0x00000000
#define OF_SHARE_EXCLUSIVE  0x00000010
#define OF_SHARE_DENY_WRITE 0x00000020
#define OF_SHARE_DENY_READ  0x00000030
#define OF_SHARE_DENY_NONE  0x00000040
#define OF_PARSE            0x00000100
#define OF_DELETE           0x00000200
#define OF_VERIFY           0x00000400
#define OF_CANCEL           0x00000800
#define OF_CREATE           0x00001000
#define OF_PROMPT           0x00002000
#define OF_EXIST            0x00004000
#define OF_REOPEN           0x00008000
*/	
	Fid = _open (Name,opt2,pmode);
	if (Fid == HFILE_ERROR && AllowJournal && CurrentCheckPointID)
	{
		HFILE	Fid2 = GetAvailableFid ();
		switch (OpenJournal (Name,Fid2,OF_READ))
		{
		case 1: 
		case 2:
			strcpy (OpenFileName[Fid2],Name);
			CloseJournal (Fid2);
			OpenFileFid[Fid2] = HFILE_ERROR;
			Fid = (HFILE)-2;
			break;
		default:
			OpenFileFid[Fid2] = HFILE_ERROR;
			break;
		}
	}
	NumActualOpen++;
	_fullpath (pOFStruct->szPathName,Name,OFS_MAXPATHNAMEGM);
	if (Fid == HFILE_ERROR)
	{
		if (errno == EACCES)
			pOFStruct->nErrCode = 5;
		else
			pOFStruct->nErrCode = errno;
		FileAlreadyNotFound (Name,2,pOFStruct->nErrCode);
	}
	else if (opt == OF_EXIST)
		_close (Fid);
	else
	{
		for (i=0;i<MAXFILEHANDLES;i++)
			if (OpenFileFid[i] != HFILE_ERROR && !stricmp (OpenFileName[i],Name))
				ii=1;
		pOFStruct->nErrCode = 0;
		Fid = LogOpenFilesOpen (opt,Fid,pOFStruct);
		if (Fid < MAXFILEHANDLES)
		{
			strcpy (OpenFileName[Fid],Name);
			OpenFileLength[Fid] = OriginalFileLength[Fid] = GSSifilelength (Fid);
		}
	}
	return Fid;
}

POINT POINTStoPOINT (POINTS p)
{
	POINT pl={p.x,p.y};

	return pl;
}

POINTS POINTtoPOINTS (POINT p)
{
	POINTS pl={min(max(p.x,SHRT_MIN),SHRT_MAX),min(max(p.y,SHRT_MIN),SHRT_MAX)};

	return pl;
}

BPOINT POINTtoBPOINT (POINT p)
{
	BPOINT pl={min(max(p.x,0),255),min(max(p.y,0),255)};

	return pl;
}

POINT BPOINTtoPOINT (BPOINT p)
{
	POINT pl={p.x,p.y};

	return pl;
}

void DumpOpenFiles (LPSTR File)
{   
	char	str[512];  
	UINT	i; 
	   
	sprintf (str,"%i Open Files:",NumOpenFiles);
	AppendFile2 (File,str);
	for (i=0;i<MAXFILEHANDLES;i++)
	{
		if (OpenFileFid[i] != HFILE_ERROR)
		{ 
			sprintf (str,"    %i-%i %s %u %i",i,OpenFileFid[i],OpenFileStruct[i].szPathName,OpenFileMode[i],(int)OpenFileCloseRequested[i]);    
			AppendFile2 (File,str);
		}
	}
	return;
}


void CloseAllRequestedFiles (BOOL FirstOnly)
																							#if ENABLETRACE
																							{GSSiEnterProg (171);
																							#endif
{ 
	UINT	i;
	
	if (KeepFilesOpen)
	{
		for (i=1;i<MAXFILEHANDLES;i++)
		{   
			if (OpenFileFid[i] != HFILE_ERROR)
			{
				if (OpenFileCloseRequested[i])
				{
					ActuallyCloseFile (i);
					if (FirstOnly)
						return;
				}
			}
		}
		CloseAllRequestedDGNFiles ();
		FileAlreadyNotFound (0,0,0);
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

HFILE LogOpenFilesOpen (UINT Mode,HFILE Fid,LPOFSTRUCTGM pOFStruct)
																							#if ENABLETRACE
																							{GSSiEnterProg (172);
																							#endif
{   
	short	i,j,ii; 
	LPSTR	pDot;   
	static	BOOL	FirstFileOpen=TRUE;
	static	UINT	CallID=0;
	
	if (FirstFileOpen)
	{ 
		for (i=0;i<MAXFILEHANDLES;i++)
		{
			OpenFileFid[i] = HFILE_ERROR; 
			JournalFileFid[i] = HFILE_ERROR;
			JournalIsCompleteFile[i] =0;
			OpenFileHandle[i] = 0; 
			FidIsMapped[i] = FALSE;
			OpenFileLength[i] = 0;
			OriginalFileLength[i] = 0;
			FidMemLen[i] = 0;
			OpenFileUndoFileID[i] = -1;
		}  
		FirstFileOpen = FALSE;
    }
	if (Fid == HFILE_ERROR)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
		return HFILE_ERROR;
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
		return MAXFILEHANDLES;
																							}
	if (Mode & OF_DELETE)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
		return MAXFILEHANDLES; 
																							}
	MaxHFile = max (MaxHFile,Fid);
	
	i = GetAvailableFid ();
Gotid:			
	OpenFileFid[i] = Fid;   
	if (Fid == HFILE_ERROR && FidIsMapped[i])
		ii=1;
	OpenFileStruct[i] = *pOFStruct;  
/*_fstrlwr (pOFStruct->szPathName);
	if (_fstrstr (pOFStruct->szPathName,"minnhenn.plt"))
		ii=1;*/
#if CHECKMEM
	{
		UINT j;
		
		for (j=0;j<0/*MAXFILEHANDLES*/;j++)
			if (i!=j && OpenFileFid[j]!=HFILE_ERROR && !_fstricmp (OpenFileStruct[i].szPathName,OpenFileStruct[j].szPathName))
				MessageBox (0,OpenFileStruct[i].szPathName,"Dup File Open",MB_OK);
	}
#endif
	OpenFileMode[i] = Mode;
	OpenFileCallID[i] = CallID++;
	OpenFileCloseRequested[i] = FALSE; 
	OpenFileUndoFileID[i] = GetUndoFileIDFromName (pOFStruct->szPathName,FALSE);
   
	NumOpenFiles++;
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
	return i;
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
	static	HFILE	debugid=35;

	
	if (Fid == HFILE_ERROR)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (173);
																							#endif
		return;
																							}
	if (Fid == debugid)
		ii=1;
	if (!IsMemFile(Fid))
		NumOpenFiles--; 
	else
		FidMemLen[Fid] = 0;
	OpenFileFid[Fid] = HFILE_ERROR;
	*OpenFileStruct[Fid].szPathName = 0;
	if (FidIsMapped[Fid])
		ii=1;

	if (NumOpenFiles < 0)
		ii=1;
																							{
																							#if ENABLETRACE
																							GSSiExitProg (173);
																							#endif
			return;
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
	UINT	i,ii;
	
	for (i=1;i<MAXFILEHANDLES;i++)
		if (OpenFileFid[i] != HFILE_ERROR)
		{   
			char	mess[300];
				
			GetOpenFilePathname (i,mess);
			sprintf (strchr (mess,0)," still open - %i",OpenFileCallID[i]);
#if CHECKMEM
			GSSiMsgBox (0,mess,0,MB_ICONEXCLAMATION,0);
#endif 
			ActuallyCloseFile (i); 
			if (OpenFileFid[i]==42)
				ii=1;
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

HFILE FileAlreadyOpen (LPSTR InName,UINT Mode,LPOFSTRUCTGM pOFStruct)
																							#if ENABLETRACE
																							{GSSiEnterProg (174);
																							#endif
{   
	UINT	i;
	
	if (KeepFilesOpen) 
	{   
		LPSTR	pFullPath = _fullpath (0,InName,0);    
		HANDLE	hMem = GSSiGlobAlloc (1546,GMEM_MOVEABLE,512);
		LPSTR	pMem = GlobalLock (hMem);
		
		if (pFullPath)
		{	
			_fstrcpy (pMem,pFullPath);
			free (pFullPath);
			//GetShortPathName2 (pMem,512);
			pFullPath = pMem;
			for (i=1;i<MAXFILEHANDLES;i++)
			{
				if (OpenFileFid[i] != HFILE_ERROR)
				{
					if (!_fstricmp (pFullPath,OpenFileStruct[i].szPathName))
					{
						if ((Mode == OF_READ || Mode == OF_EXIST) && OpenFileMode[i] == OF_READ)
						{   
							if (!OpenFileCloseRequested[i] && Mode != OF_EXIST)
								continue;
							if (pOFStruct) 
								*pOFStruct = OpenFileStruct[i]; 
							if (Mode == OF_READ)
							{
								int ii=GSSillseek (i,0,0);
								OpenFileCloseRequested[i] = FALSE;
							}
							GSSiGlobUlFree (&hMem);
							 
																								{
																								#if ENABLETRACE
																								GSSiExitProg (174);
																								#endif
							return i;       
																								}
						}
						GSSiGlobUlFree (&hMem);
						ActuallyCloseFile (i);
																								{
																								#if ENABLETRACE
																								GSSiExitProg (174);
																								#endif
						return HFILE_ERROR;
																								}
					}
				}
			}  
		}
		GSSiGlobUlFree (&hMem);
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
	
	if (!KeepFilesOpen || Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
		return FALSE;
}
	i = Fid;
//	for (i=0;i<MAXFILEHANDLES;i++)
//		if (OpenFileFid[i] == Fid) 
		{   
/*	    	short	l= _fstrlen(CachePathnameTo);

    		if (l && !_fstrnicmp (OpenFileStruct[i].szPathName,CachePathnameTo,l))
	    	{
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
					return FALSE;
}
	    	}*/
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
{
#if ENABLETRACE
GSSiExitProg (177);
#endif
	return Fid;
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
	OFSTRUCTGM	OFStruct; 
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
			GSSiMessageBox (0,"Backup failed on above file",FromFile,MB_ICONEXCLAMATION,0);
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
#if CHECKMEM        
	HPEN	hnp=GetStockObject(NULL_PEN);
    
    if (*handle && *handle == HighlightBrush)
    	i=1;
	if (*handle)
	{
		if (*handle != hnp && !DeleteObject (*handle))
			i=1;
		*handle = 0;
	}
#else
if (*handle)
{
	if (!DeleteObject (*handle))
		ii=1;
}
	*handle = 0;
#endif
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

short loadtabs (LPINT Tabs)
#if ENABLETRACE
{GSSiEnterProg (181);
#endif
{
	OFSTRUCTGM OFStruct;
	HFILE	Fid;   
	char	txt[256]; 
	short	n; 
	LPSTR	lpBeg, lpEnd;
	
	Fid = GSSiOpenFile ("c:\\temp\\tabs.txt",&OFStruct,OF_READ);
	fgetstring (txt,200,Fid);
	n = 0;  
	lpBeg = txt;
	while (lpBeg)
	{
		if ((lpEnd = _fstrchr (lpBeg,',')))
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


BOOL makedirectories2 (LPSTR Name,BOOL IsDir,BOOL Verify)
#if ENABLETRACE
{GSSiEnterProg (182);
#endif
{   
	char	FullName[MAX_PATH], Dir[MAX_PATH], FName[MAX_PATH], Ext[64], NewName[MAX_PATH]="", drive[32]; 
	LPSTR	StartDir, EndDir,pFull;  
	OFSTRUCTGM	OFStruct;
	BOOL	Replace; 
	DWORD	Err;
	
	_fmemset (Dir,0,MAX_PATH);
	_fstrcpy (Dir,Name);
	ExpandText (Dir);
	pFull = _fullpath (FullName,Dir,sizeof(FullName));
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
			
			DirCreated = GSSiMakeDir (NewName,&Err);
			if (!DirCreated && Err)
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
			DoCreate= GSSiMsgBox (0,pMess,"Verify folder creation",MB_YESNO|MB_ICONQUESTION,0); 
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

BOOL makedirectories (LPSTR InName,BOOL IsDir,BOOL Verify)
{
	int		st = GetPathType (InName);
	LPSTR	pBS;
	char	Name[MAX_PATH];

	if (IsDir && st == 2)
		return TRUE;
	if (IsDir && st == 1)
		return FALSE;
	if (!IsDir)
	{
		if (st == 1)
			return TRUE;
		if (st == 2)
			return FALSE;
		strcpy (Name,InName);
		ExpandText (Name);
		if ((pBS = strrchr (Name,'\\')))
			*pBS = 0;
		if (GetPathType (Name) == 2)
			return TRUE;
	}
	return makedirectories2 (InName,IsDir,Verify);
}

double	GetDriveFreeSpace (LPSTR Dir)
#if ENABLETRACE
{GSSiEnterProg (183);
#endif
{   
	BOOL	st;
	ULARGE_INTEGER MySpace,TotSpace,FreeSpace;
	char	Drive[MAX_PATH];
	LPSTR	pBS;
	double	FrSpace;

	strcpy (Drive,Dir);
	ExpandText(Drive);
	if ((pBS = strstr (Drive,":\\")))
	{
		pBS+=2;
		*pBS = 0;
	}
	st = GetDiskFreeSpaceEx (Drive,&MySpace,&TotSpace,&FreeSpace);

    if (!st)
{
#if ENABLETRACE
GSSiExitProg (183);
#endif
    	return -1;
}
    else
	{
		FrSpace = (double)FreeSpace.LowPart + (double)FreeSpace.HighPart * (double)ULONG_MAX;
{
#if ENABLETRACE
GSSiExitProg (183);
#endif
    	return FrSpace;
}
	}
#if ENABLETRACE
}
#endif
}

double	GetDriveSize (char Drive)
{   
	char	DriveC[2]={Drive,0};
	
	return GetDriveFreeSpace (DriveC);
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

BOOL GetVolumeLabel(LPSTR wDrive, LPSTR lpBuff)
#if ENABLETRACE
{GSSiEnterProg (187);
#endif
{

/*    struct _find_t  finfo;
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
}*/
	return FALSE;
  
  
#if ENABLETRACE
}
#endif
} 

int AppendFile (LPSTR InFile,LPSTR Line)
#if ENABLETRACE
{GSSiEnterProg (188);
#endif
{
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;
	int			rtn=0;
	BOOL		SaveAllowJournal = AllowJournal;
	char		File2[MAX_PATH];
	LPSTR		File = File2;
	BOOL		fileIsFID = FALSE;

	strcpy (File2,InFile);
	if (*File == '*')
	{
		AllowJournal = FALSE;
		File++;
	}
	ExpandText (File);
	if (!*File)
		goto Exit;
	if (IsInteger(File))
	{
		Fid = atoi(File);
		fileIsFID = TRUE;
	}
	else
	{
		Fid = GSSiOpenFile(File, &OFStruct, OF_READWRITE);
		if (Fid == HFILE_ERROR)
			Fid = GSSiOpenFile(File, &OFStruct, OF_CREATE_NODELETE);
	}
	if (Fid == HFILE_ERROR) 
		goto Exit;
	if (fileIsFID)
	{
		fputstring(Line, Fid);
		rtn = 1;
	}
	else
	{
		rtn = GSSillseek(Fid, 0, 2) + 1;
		fputstring(Line, Fid);
		GSSiClose(Fid);
	}
Exit:
	AllowJournal = SaveAllowJournal;
{
#if ENABLETRACE
GSSiExitProg (188);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}
int AppendFile2 (LPSTR InFile,LPSTR Line)
#if ENABLETRACE
{GSSiEnterProg (188);
#endif
{
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;
	int			rtn=0;
	BOOL		SaveAllowJournal = AllowJournal;
	char		File2[MAX_PATH];
	LPSTR		File=File2;
	int			len;

	strcpy (File2,InFile);
	if (*File == '*')
	{
		AllowJournal = FALSE;
		File++;
	}
	ExpandText (File);
	if (!*File)
		goto Exit;
	Fid = OpenFileGM (File,&OFStruct,OF_READWRITE);
	if (Fid == HFILE_ERROR)
		Fid = OpenFileGM (File,&OFStruct,OF_CREATE);
	if (Fid == HFILE_ERROR) 
		goto Exit;
	rtn = _llseek (Fid,0,2)+1;
    len=strlen(Line);
    if (len)
        _lwrite (Fid,(char *)Line,len); 
    _lwrite (Fid,"\r\n",2);
	FlushFileBuffers ((HANDLE)Fid);
	_lclose (Fid);
Exit:
	AllowJournal = SaveAllowJournal;
{
#if ENABLETRACE
GSSiExitProg (188);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

UINT ItemInList (int i,UINT nElements,HANDLE hElements)
#if ENABLETRACE
{GSSiEnterProg (195);
#endif
{   
	LPINT	Elements;
	UINT	j;
	
	if (!nElements)
{
#if ENABLETRACE
GSSiExitProg (195);
#endif
		return 0;
}
	Elements = (LPINT)GlobalLock (hElements);
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

LPSTR TabFromEnd (LPSTR str,int n)
{
	LPSTR pTab = strchr (str,0);
	int	 i=0;

	while (pTab != str)
	{
		pTab--;
		if (*pTab == '\t')
			i++;
		if (i == n)
			return pTab;
	}
	return 0;
}

int GetLBSelectedItems (HWND hWndDlg,UINT controlid,LPHANDLE phItems)
#if ENABLETRACE
{GSSiEnterProg (196);
#endif
{
	int	nItems=SendDlgItemMessage(hWndDlg,controlid,LB_GETSELCOUNT,0,0);
	LPINT	pItems;
	
	if (nItems)
	{
		*phItems=GSSiGlobAlloc (  80,GHND,nItems*4);
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

void SetDlgItemTextFromField (HWND hWndDlg,UINT icntl,LPSTR FieldName,LPSTR Init)
#if ENABLETRACE
{GSSiEnterProg (197);
#endif
{
	char	str[256]="";
	
	GetValFromOpenFiles (FieldName,str,256);

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

BOOL SetDlgCheckboxFromField (HWND hWndDlg,UINT icntl,LPSTR FieldName,LPSTR Init)
#if ENABLETRACE
{GSSiEnterProg (197);
#endif
{
	char	str[256]="";
	BOOL	rtn;
	
	GetValFromOpenFiles (FieldName,str,256);
	rtn = atob (str);
    SendDlgItemMessage (hWndDlg,icntl,BM_SETCHECK,rtn,0L);
{
#if ENABLETRACE
GSSiExitProg (197);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL SetDlgComboboxFromField (HWND hWndDlg,UINT icntl,LPSTR FieldName,LPSTR Init)
#if ENABLETRACE
{GSSiEnterProg (197);
#endif
{
	char	str[256]="";
	BOOL	rtn=TRUE;
	
	GetValFromOpenFiles (FieldName,str,256);
    if (SendDlgItemMessage (hWndDlg,icntl,CB_SELECTSTRING,-1,(LPARAM)str) == CB_ERR)
    	rtn = FALSE;
{
#if ENABLETRACE
GSSiExitProg (197);
#endif
    return rtn;
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

short NumMatchChar (LPSTR str1,LPSTR str2,int maxchar)
{  
	short	n=0;
	
	while (maxchar--)
	{ 
		if (*str1++ != *str2++)
			break;
		n++;
	}
	return n;
}
int charLevel(LPSTR pchar, LPSTR inStr)
{
	int lev = 0;
	BOOL    Literal = FALSE;
	char	BegLev, EndLev;

	if (pchar && inStr)
	{
		while (*inStr && pchar > inStr)
		{
			if (Literal)
				Literal = FALSE;
			else
			{
				if (*inStr == literalChar)
					Literal = TRUE;
				else if (lev)
				{
					if (*inStr == BegLev)
						lev++;
					else if (*inStr == EndLev)
						lev--;
				}
				else
					switch (*inStr)
				{
					case '\'':
						EndLev = '\'';
						BegLev = *inStr;
						lev++;
						break;
					case '(':
						EndLev = ')';
						BegLev = *inStr;
						lev++;
						break;
					case '[':
						EndLev = ']';
						BegLev = *inStr;
						lev++;
						break;
					case '{':
						EndLev = '}';
						BegLev = *inStr;
						lev++;
						break;
					default:
						break;
				}

			}
			inStr++;
		}
	}
	return lev;
}

LPSTR MatchLev (LPSTR InStr, char MatchChar)
#if ENABLETRACE
{GSSiEnterProg (200);
#endif
{
    short lev=0;
    BOOL    Literal=FALSE; 
    char	BegLev=0,EndLev=0;
    
    while (*InStr)
    {    
        if (Literal)
            Literal = FALSE;
        else
        {
            if (*InStr == literalChar)
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
    GSSiGlobUlFree (&handle); 
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

void BoundsLToPoints (LPMNMXCORL pBounds, LPDPOINT Points,LPDOUBLE pAZ)
{
	MNMXCORD	DBounds;

	DBounds.xmn = pBounds->xmn;
	DBounds.xmx = pBounds->xmx;
	DBounds.ymn = pBounds->ymn;
	DBounds.ymx = pBounds->ymx;
	BoundsToPoints (&DBounds,Points,pAZ);
	return;
}

void BoundsToPoints (LPMNMXCORD pBounds, LPDPOINT Points,LPDOUBLE pAZ)
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
	if (pAZ)
	{
		pAZ[0] = HALFPI;
		pAZ[1] = 0;
		pAZ[2] = PIHALF;
		pAZ[3] = PY;
	}
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
    OFSTRUCTGM    OFStruct;
    HFILE   FidOut; 
    char    str[260]; 
    short	x,y, l; 
    SIZE	txSize;
    
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
		ExtTextOut(hDC, x,y, 0, Rect, str, _fstrlen(str),0); 
		l = _fstrlen(str);
		if (!l)
		{
			str[0]=' ';  
			str[1]=0;
			l = 1;
		}
		GetTextExtentPoint32 (hDC,str, l,&txSize);
			
		y += txSize.cy;
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
    OFSTRUCTGM    OFStruct;
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
BOOL    GetSaveName2 (HWND hWnd,LPSTR Name, UINT StringID, LPSTR Ext,UINT FileNameID)
#if ENABLETRACE
{GSSiEnterProg (212);
#endif
{
	char	str[MAX_PATH];
	char	ext[64]={0};

	if (Ext)
		strcpy (ext,Ext);
	
    GetCurVal (str,sizeof(str),FileNameID); 
    _fstrcpy (Name,str); 
    if (GetSaveName (hWnd,Name, StringID, ext))
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
    
BOOL    GetSaveName3 (HWND hWnd,LPSTR Name, UINT StringID, LPSTR Ext,LPSTR Title,LPSTR VarName)
#if ENABLETRACE
{GSSiEnterProg (213);
#endif
{
	char	str[MAX_PATH]="";
	LPSTR	pName=Name;
	                            
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
    		strcpy (OFTitle,Title);
    }
	if (*str)
		pName = str;
    if (GetSaveName (hWnd,pName, StringID, Ext))
    { 
		if (pName != Name)
			strcpy (Name,pName);
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

BOOL    GetSaveName (HWND hWnd,LPSTR Name, UINT StringID, LPSTR Ext)
#if ENABLETRACE
{GSSiEnterProg (214);
#endif
{ 
    char    CurDir[_MAX_PATH+4], InitDir[_MAX_PATH+4], Drive[_MAX_DRIVE+4], Dir[_MAX_DIR+4];
    int   SaveDrive; 
    BOOL    rtn=FALSE; 
    int	ln; 
    LPSTR	lpEnd;
	char	ext[256];

	strcpy (ext,Ext);
    
    if (!StringID) 
    {
    	if (*ext == '|')
	        sprintf (gszFilter,"%s(%s)| |","",&ext[1]);
    	else
	        sprintf (gszFilter,"%s(*%s)|*%s|","",ext,_fstrlwr(ext));
	}
    SetFilterString (StringID);  
    _getcwd (CurDir,MAX_PATH);
    SaveDrive = _getdrive(); 
    _splitpath (Name,Drive,Dir,0,0); 
    sprintf (InitDir,"%s%s",Drive,Dir);
    ln = _fstrlen (InitDir);
    lpEnd = (LPSTR)InitDir + ln;
    lpEnd--;
    if (ln > 3 && *lpEnd == '\\') *lpEnd = 0;
    if (GetSaveFileCD (hWnd,Name,InitDir))
    {   
         if (ext)
         {
	         if (!_fstrstr(Name,"."))   
	         {
	         	if (*ext != '|')
	            	_fstrcat(Name,ext);  
	         }
	         else
	         {
	            char TestName[MAX_PATH],TestExt[16]; 
	            
	            _fstrcpy(TestName,Name);
	            _fstrcpy(TestExt,ext);
	            _fstrupr (TestName); 
	            _fstrupr (TestExt);
	             if (!_fstrstr(TestName,TestExt))
	             {  
	                char    str[128];
	                sprintf (str,"Must have %s extension",ext);
	                GSSiMsgBox(hWnd, str,"Invalid Name", MB_OK|MB_ICONEXCLAMATION,0);
	                goto RestoreDir;
	             }
	         } 
	     }
         SubstituteDL (Name,TRUE);
         rtn=TRUE;
    }
RestoreDir:
    _chdir (CurDir);
    _chdrive (SaveDrive);   
    *OFTitle = 0;
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

BOOL    GetFileName2 (HWND hWnd,LPSTR Name,LPSTR ext,UINT FileNameID)
#if ENABLETRACE
{GSSiEnterProg (215);
#endif
{   
	char	str[128], CurExt[8], Ext[8]; 
	short	l;
	
	strcpy (Ext,ext);
    sprintf (gszFilter,"%s(*%s)|*%s|","",_fstrupr(Ext),_fstrlwr(Ext)); 
    GetCurVal (str,sizeof(str),FileNameID); 
    _fstrcpy (Name,str);
	if (GetFileName (hWnd,Name,0,0))
	{
		_splitpath (Name,0,0,0,CurExt);
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
BOOL    GetFileName3 (HWND hWnd,LPSTR Name,UINT StringID,UINT FileNameID)
#if ENABLETRACE
{GSSiEnterProg (216);
#endif
{
	char	str[MAX_PATH];
	
    GetCurVal (str,sizeof(str),FileNameID); 
    _fstrcpy (Name,str);
	if (GetFileName (hWnd,Name,0,StringID))
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

BOOL    GetMultFiles (HWND hWnd,LPSTR Name,int lname,UINT StringID,UINT FileNameID)
#if ENABLETRACE
{GSSiEnterProg (216);
#endif
{
	char	str[MAX_PATH];
	
	SetOpenFlags (OFN_ALLOWMULTISELECT|OFN_ENABLEHOOK|OFN_ENABLETEMPLATE);
    GetCurVal (str,sizeof(str),FileNameID); 
	if (FileType (str) == 2)
		strcat (str,"\\");
    _fstrcpy (Name,str);
	if (GetFileName (hWnd,Name,lname,StringID))
	{
    	SetCurVal (Name,FileNameID);    
 //   	ExpandText (Name);
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

BOOL    GetFileName4 (HWND hWnd,LPSTR Name,UINT StringID,LPSTR ext,LPSTR Title,LPSTR VarName)
#if ENABLETRACE
{GSSiEnterProg (217);
#endif
{
	char	str[256]="";
	char	Ext[64];

	strcpy (Ext,ext);
    
    _fstrcpy (str,Name);
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
	if (!*str)
	    _fstrcpy (str,Name);
    if (Title)
    {
    	if (*Title)
    		strcpy (OFTitle,Title);
    }
    if (!StringID)
    	sprintf (gszFilter,"%s(*%s)|*%s|","",_fstrupr(Ext),_fstrlwr(Ext)); 
	if (GetFileName (hWnd,str,0,StringID))
    {   
    	_fstrcpy (Name,str);
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

BOOL    GetFileName (HWND hWnd,LPSTR Name, int lname, UINT StringID)
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
    	GSSiMessageBox (0,str,0,MB_ICONEXCLAMATION,0);
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
	ConvertToNewLocation (InitDir,FALSE);
    if (GetOpenFileCD (hWnd,Name,lname,InitDir))
    {   
         rtn=TRUE;  
         SubstituteDL (Name,TRUE);
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

void SubstituteDL (LPSTR Name,BOOL WantAt)
#if ENABLETRACE
{GSSiEnterProg (219);
#endif
{   
	if (GetGlobalBVal ("[%SUBDL]"))
	{  
		char	DL[MAX_PATH]="[%DL]";
		short	l=_fstrlen (Name); 
		HANDLE	hMEM=GSSiGlobAlloc (  82,GMEM_MOVEABLE,l+1);
		LPSTR	TempName = GlobalLock (hMEM);         	
		ExpandText (DL);
		_fstrupr (DL); 
		_fstrcpy (TempName,Name);
		_fstrupr (TempName);
		l = _fstrlen (DL);
		         	
		if (!_fstrnicmp (DL,Name,l))
		{
			if (WantAt)
				sprintf (Name,"@[%cDL]%s",'%',&TempName[l]); 
			else
				sprintf (Name,"[%cDL]%s",'%',&TempName[l]); 
		}
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

RECT Rect16ToRect32 (RECT16 rect16)
{
	RECT	r={rect16.left,rect16.top,rect16.right,rect16.bottom};

	return r;
}
RECT16 Rect32ToRect16 (RECT rect16)
{
	RECT16	r={rect16.left,rect16.top,rect16.right,rect16.bottom};

	return r;
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
	char	Buff[64],VCBuff[64];
    LPSTR   p1, p2, bp, pDot;
    short	n;
	
	if (_fstrlen (InBuff) > 63)
{
#if ENABLETRACE
GSSiExitProg (225);
#endif
		return InBuff;
}
    _fstrcpy (Buff,InBuff);
    bp = Buff; 
    p2=&VCBuff[63]; 
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

void RWRITE (double RVAL, int NDP, LPSTR OutLoc) 
 
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

void RWRITEZ (double RVAL, int NDP, int len,LPSTR OutLoc) 
 
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

double dread (LPSTR str, int nbytes)
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
        

long ldread (LPSTR str, int nbytes)
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

LPSTR RemoveDoubleQuotes(LPSTR str)
#if ENABLETRACE
{GSSiEnterProg (232);
#endif
{
	LPSTR	EndChar = str;
	short	l;

	if (*str != '"')
	{
#if ENABLETRACE
		GSSiExitProg (232);
#endif
		return str;
	}
	l = _fstrlen(str);
	if (l < 2)
	{
#if ENABLETRACE
		GSSiExitProg (232);
#endif
		return str;
	}
	EndChar += l - 1;
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
		_fmemmove(str, (LPSTR)(str + 1), l);
		EndChar = str + l;
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
LPSTR RemoveQuotes(LPSTR str)
#if ENABLETRACE
{GSSiEnterProg (232);
#endif
{
	LPSTR	EndChar = str;
	short	l;

	if (*str != '\'')
	{
#if ENABLETRACE
		GSSiExitProg(232);
#endif
		return str;
	}
	l = _fstrlen(str);
	if (l < 2)
	{
#if ENABLETRACE
		GSSiExitProg(232);
#endif
		return str;
	}
	EndChar += l - 1;
	if (*EndChar != '\'')
	{
#if ENABLETRACE
		GSSiExitProg(232);
#endif
		return str;
	}
	l -= 2;
	if (!l)
		*str = 0;
	else
	{
		_fmemmove(str, (LPSTR)(str + 1), l);
		EndChar = str + l;
		*EndChar = 0;
	}
	{
#if ENABLETRACE
		GSSiExitProg(232);
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
/*    	if (IsGDIObject (CurPen))
        	SelectObject (hDC,CurPen);
        else*/
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

HGLOBAL GSSiGlobalCopy(USHORT From, HGLOBAL hGlob)
{
	long len = GlobalSize(hGlob);
	HGLOBAL rtn = GSSiGlobAlloc(From, GMEM_MOVEABLE, len);
	LPBYTE pFrom = GlobalLock(hGlob);
	LPBYTE pTo = GlobalLock(rtn);
	memmove(pTo, pFrom, len);
	GlobalUnlock(hGlob);
	GlobalUnlock(rtn);
	return rtn;
}

HGLOBAL GSSiGlobalReAlloc (USHORT From,HGLOBAL hGlob, long cbAlloc,UINT fuAlloc)
#if ENABLETRACE
{GSSiEnterProg (1398);
#endif
{
	HGLOBAL handle;
	long	PrevLen;
	char	pMess[64];

    if (cbAlloc <= 0)
    {    
		sprintf(pMess, "%i %i", (long)cbAlloc,From);
        BlowOut("Invalid memory allocation in realloc",pMess);
    }
	PrevLen = GlobalSize (hGlob);
	TotMemAlloc -= PrevLen;
	TotMemAlloc += cbAlloc;
	MaxMemAlloc = max (MaxMemAlloc,TotMemAlloc);
	handle = GlobalReAlloc (hGlob,cbAlloc,fuAlloc);
#if CHECKMEM 
	if (PrevLen < cbAlloc && (fuAlloc & GMEM_ZEROINIT))
	{
		LPBYTE pMem = GlobalLock (handle);
		pMem += PrevLen;
		memset (pMem,0,cbAlloc-PrevLen);
		GlobalUnlock (handle);
	}
#endif
{
#if ENABLETRACE
GSSiExitProg (1398);
#endif
    return (handle);
}
#if ENABLETRACE
}
#endif
}  

HGLOBAL GSSiGlobAlloc (int From,UINT fuAlloc, long cbAlloc)
#if ENABLETRACE
{GSSiEnterProg (238);
#endif
{
	HGLOBAL handle;
	char	pMess[64];
    short	ii;

/*	if (fuAlloc == GMEM_MOVEABLE)
		fuAlloc = GMEM_FIXED;
	if (fuAlloc == GHND)
		fuAlloc = GPTR;*/
#if CHECKMEM  
	LogMemAlloc (From,cbAlloc);
#endif
    if (cbAlloc <= 0)
   {    
//        GSSiMsgBox( 0, "Invalid memory allocation",0, MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL);
		sprintf(pMess, "%i %i", (long)cbAlloc, From);
		BlowOut("Invalid memory allocation in GSSiGlobAlloc", pMess);
	}
 //   if (cbAlloc == 2) 
 //   	ii=1;  
//    sprintf (pMess,"Alloc %ld",(long)cbAlloc);
//    ShowTextTrace (pMess);
    handle = GlobalAlloc (fuAlloc, (DWORD)cbAlloc);
//    *pMess=0;
//    ShowTextTrace (pMess);
    if (!handle)
   {    
   		
   		sprintf (pMess,"%i:%i",(long)From,(long)cbAlloc);
 //       GSSiMsgBox( 0, "Out of Mem",pMess, MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL);  
        BlowOut("Out of Mem",pMess);
   }
	TotMemAlloc += cbAlloc;
	MaxMemAlloc = max (MaxMemAlloc,TotMemAlloc);
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
	if (pBoundsInt)
	{
		pBoundsInt->xmn = max (pBounds1->xmn,pBounds2->xmn);
		pBoundsInt->ymn = max (pBounds1->ymn,pBounds2->ymn);
		pBoundsInt->xmx = min (pBounds1->xmx,pBounds2->xmx);
		pBoundsInt->ymx = min (pBounds1->ymx,pBounds2->ymx);
	}
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
     
BOOL RectCompletelyInRect(LPRECT pRectIn,LPRECT pRectTest)
#if ENABLETRACE
{GSSiEnterProg (240);
#endif
{   
	RECT	IntRect;
	BOOL	rtn = FALSE;

	if (IntersectRect(&IntRect,pRectIn,pRectTest))
		rtn = EqualRect (&IntRect,pRectTest);
{
#if ENABLETRACE
GSSiExitProg (240);
#endif
	return rtn;

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

POINT FPointToPoint (FPOINT point)
#if ENABLETRACE
{GSSiEnterProg (242);
#endif
{
	POINT	Point;
	
	Point.x = IDNINT(point.x);
	Point.y = IDNINT(point.y);
{
#if ENABLETRACE
GSSiExitProg (242);
#endif
	return Point;
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

FPOINT PointToFPoint (POINT Point)
{
	FPOINT FPoint ={Point.x,Point.y};

	return FPoint;
}


DPOINT3D DPointToDPoint3D (DPOINT DPoint)
#if ENABLETRACE
{GSSiEnterProg (243);
#endif
{
	DPOINT3D DPoint3D ={DPoint.x,DPoint.y,0};
	
{
#if ENABLETRACE
GSSiExitProg (243);
#endif
	return DPoint3D;
}
#if ENABLETRACE
}
#endif
}

DPOINT DPoint3DToDPoint (DPOINT3D DPoint3D)
#if ENABLETRACE
{GSSiEnterProg (243);
#endif
{
	DPOINT DPoint ={DPoint3D.x,DPoint3D.y};
	
{
#if ENABLETRACE
GSSiExitProg (243);
#endif
	return DPoint;
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

DPOINT SPointToDPoint (POINTS Point)
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
LPOINT PointToLPoint (POINT Point)
#if ENABLETRACE
{GSSiEnterProg (244);
#endif
{
	LPOINT p;
	
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
BOOL DPointInRect(LPDPOINT pt, LPRECT rect)
{
	BOOL rtn = TRUE;

	if (pt->x < rect->left || pt->x > rect->right ||
		pt->y < rect->top || pt->y > rect->bottom)
		rtn = FALSE;
	return rtn;
}
BOOL FPointInRect(LPFPOINT pt, LPRECT rect)
{
	BOOL rtn = TRUE;

	if (pt->x < rect->left || pt->x > rect->right ||
		pt->y < rect->top || pt->y > rect->bottom)
		rtn = FALSE;
	return rtn;
}
HANDLE DPointsToPoints(HANDLE hDPoints, int np)
{
	HANDLE hP = GSSiGlobAlloc(1803, GMEM_MOVEABLE, np*sizeof(POINT)+4);
	LPPOINT p = GlobalLock(hP);
	HPDPOINT dp = GlobalLock(hDPoints);
	for (int i = 0; i < np; i++)
	{
		p[i].x = IDNINT(dp[i].x);
		p[i].y = IDNINT(dp[i].y);
	}
	GlobalUnlock(hDPoints);
	GlobalUnlock(hP);
	return hP;
}

HANDLE DPointsToHFPoints(LPDPOINT DPoints, int np)
{
	HANDLE hP = GSSiGlobAlloc(1803, GMEM_MOVEABLE, np*sizeof(FPOINT)+4);
	LPFPOINT p = GlobalLock(hP);
	HPDPOINT dp = DPoints;
	for (int i = 0; i < np; i++)
	{
		p[i].x = dp[i].x;
		p[i].y = dp[i].y;
	}
	GlobalUnlock(hP);
	return hP;
}

HANDLE HDPointsToHFPoints(HANDLE hDPoints, int np)
{
	HANDLE hP = GSSiGlobAlloc(1803, GMEM_MOVEABLE, np*sizeof(FPOINT) + 4);
	LPFPOINT p = GlobalLock(hP);
	HPDPOINT dp = GlobalLock(hDPoints);
	for (int i = 0; i < np; i++)
	{
		p[i].x = dp[i].x;
		p[i].y = dp[i].y;
	}
	GlobalUnlock(hDPoints);
	GlobalUnlock(hP);
	return hP;
}

POINT DPointToPoint(DPOINT Point)
#if ENABLETRACE
{GSSiEnterProg (245);
#endif
{
	POINT p;
	
	p.x = max ((long)INT_MIN,min ((long)INT_MAX,IDNINT (Point.x)));
	p.y = max ((long)INT_MIN,min ((long)INT_MAX,IDNINT (Point.y)));
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

LPOINT DPointToFilePointL (LPDPOINT pDPoint,HANDLE hTran)
{
	DPOINT NewPoint;  
	LPOINT OutPoint;
	
	NewPoint = TranPoint (pDPoint,hTran);
	OutPoint = DPointToLPoint (&NewPoint);
	return OutPoint;
}
 
LPOINT DPointToLPoint (LPDPOINT pPoint)
#if ENABLETRACE
{GSSiEnterProg (245);
#endif
{
	LPOINT p;
	
	p.x = max ((long)LONG_MIN,min ((long)LONG_MAX,IDNINT (pPoint->x)));
	p.y = max ((long)LONG_MIN,min ((long)LONG_MAX,IDNINT (pPoint->y)));
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

DPOINT LPointToDPoint (LPOINT Point)
#if ENABLETRACE
{GSSiEnterProg (245);
#endif
{
	DPOINT p;
	
	p.x = Point.x;
	p.y = Point.y;
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

void AddFPointToRect (FPOINT Point,LPRECT pBounds)
#if ENABLETRACE
{GSSiEnterProg (246);
#endif
{   
	int x = IDNINT(Point.x);
	int y = IDNINT(Point.y);
    pBounds->left = min (pBounds->left,x);
    pBounds->right = max (pBounds->right,x);
    pBounds->top = min (pBounds->top,y);
    pBounds->bottom = max (pBounds->bottom,y);
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
void AddPointToRect(POINT Point, LPRECT pBounds)
#if ENABLETRACE
{GSSiEnterProg (246);
#endif
{
	pBounds->left = min(pBounds->left, Point.x);
	pBounds->right = max(pBounds->right, Point.x);
	pBounds->top = min(pBounds->top, Point.y);
	pBounds->bottom = max(pBounds->bottom, Point.y);
	{
#if ENABLETRACE
		GSSiExitProg(246);
#endif
		return;
	}
#if ENABLETRACE
}
#endif
}

void AddDPointToRect(DPOINT Point, LPRECT pBounds)
#if ENABLETRACE
{
	GSSiEnterProg(246);
#endif
	{
		int x = IDNINT(Point.x);
		int y = IDNINT(Point.y);
		pBounds->left = min(pBounds->left, x);
		pBounds->right = max(pBounds->right, x);
		pBounds->top = min (pBounds->top,y);
		pBounds->bottom = max (pBounds->bottom,y);
		{
#if ENABLETRACE
			GSSiExitProg(246);
#endif
			return;
		}
#if ENABLETRACE
	}
#endif
}

void AddPointToRect16(POINT Point, LPRECT16 pBounds)
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
#if ENABLETRACE
GSSiExitProg (248);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void RectInit (LPRECT pMinMax)
#if ENABLETRACE
{GSSiEnterProg (249);
#endif
{ 
	pMinMax->left=INT_MAX;
	pMinMax->top=INT_MAX;
	pMinMax->bottom=INT_MIN;
	pMinMax->right=INT_MIN;    
{
#if ENABLETRACE
GSSiExitProg (249);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void RectInit16 (LPRECT16 pMinMax)
#if ENABLETRACE
{GSSiEnterProg (249);
#endif
{ 
	pMinMax->left=SHRT_MAX;
	pMinMax->top=SHRT_MAX;
	pMinMax->bottom=SHRT_MIN;
	pMinMax->right=SHRT_MIN;    
{
#if ENABLETRACE
GSSiExitProg (249);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void InflateMinMax (LPMINMAX pBounds, int Value)
#if ENABLETRACE
{GSSiEnterProg (250);
#endif
{      
	pBounds->xmn -= Value;
	pBounds->xmx += Value;
	pBounds->ymn -= Value;
	pBounds->ymx += Value;
{
#if ENABLETRACE
GSSiExitProg (250);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void InflateMinMaxL (LPMNMXCORL pBounds, int Value)
#if ENABLETRACE
{GSSiEnterProg (250);
#endif
{      
	pBounds->xmn -= Value;
	pBounds->xmx += Value;
	pBounds->ymn -= Value;
	pBounds->ymx += Value;
{
#if ENABLETRACE
GSSiExitProg (250);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 
void AddPointToMinMax (POINT Point,LPMINMAX pBounds)
{
	POINTS	PointS=POINTtoPOINTS (Point);

    pBounds->xmn = min (pBounds->xmn,PointS.x);
    pBounds->ymn = min (pBounds->ymn,PointS.y);
    pBounds->xmx = max (pBounds->xmx,PointS.x);
    pBounds->ymx = max (pBounds->ymx,PointS.y);
    return;
}

void AddPointToMinMaxL (POINT Point,LPMNMXCORL pBounds)
#if ENABLETRACE
{GSSiEnterProg (252);
#endif
{   
    pBounds->xmn = min (pBounds->xmn,Point.x);
    pBounds->ymn = min (pBounds->ymn,Point.y);
    pBounds->xmx = max (pBounds->xmx,Point.x);
    pBounds->ymx = max (pBounds->ymx,Point.y);
{
#if ENABLETRACE
GSSiExitProg (252);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

double BoundsWidth (LPMNMXCORD pBounds)
{
	return pBounds->xmx - pBounds->xmn;
}

double BoundsHeight (LPMNMXCORD pBounds)
{
	return pBounds->ymx - pBounds->ymn;
}

double BoundsHeight (LPMNMXCORD pBounds);


void AddDPointToMinMax (HPDPOINT Point,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (253);
#endif
{   
    pBounds->xmn = min (pBounds->xmn,Point->x);
    pBounds->ymn = min (pBounds->ymn,Point->y);
    pBounds->xmx = max (pBounds->xmx,Point->x);
    pBounds->ymx = max (pBounds->ymx,Point->y);
{
#if ENABLETRACE
GSSiExitProg (253);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

DPOINT SubtractPoint (LPDPOINT pPoint1,LPDPOINT pPoint2)
{
	DPOINT OutPoint;
	
	OutPoint.x = pPoint1->x - pPoint2->x;
	OutPoint.y = pPoint1->y - pPoint2->y;
	return OutPoint;
}

POINT MinMaxMidPoint (LPMINMAX pBounds)
#if ENABLETRACE
{GSSiEnterProg (255);
#endif
{
	POINT point;
	
	point.x = ((long)pBounds->xmn + (long)pBounds->xmx) / 2;
	point.y = ((long)pBounds->ymn + (long)pBounds->ymx) / 2;

{
#if ENABLETRACE
GSSiExitProg (255);
#endif
	return point;
}
#if ENABLETRACE
}
#endif
}

POINT MinMaxMidPointL (LPMNMXCORL pBounds)
#if ENABLETRACE
{GSSiEnterProg (255);
#endif
{
	POINT point;
	
	point.x = ((long)pBounds->xmn + (long)pBounds->xmx) / 2;
	point.y = ((long)pBounds->ymn + (long)pBounds->ymx) / 2;

{
#if ENABLETRACE
GSSiExitProg (255);
#endif
	return point;
}
#if ENABLETRACE
}
#endif
}

DPOINT MinMaxMidPointD (LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (256);
#endif
{
	DPOINT point;
	
	point.x = (pBounds->xmn + pBounds->xmx) / 2;
	point.y = (pBounds->ymn + pBounds->ymx) / 2;

{
#if ENABLETRACE
GSSiExitProg (256);
#endif
	return point;
}
#if ENABLETRACE
}
#endif
}

void DBoundsInit(LPMNMXCORD lpRect)
#if ENABLETRACE
{GSSiEnterProg (257);
#endif
{
	lpRect->xmn = DBL_MAX;
	lpRect->ymn = DBL_MAX;
	lpRect->xmx = -DBL_MAX;
	lpRect->ymx = -DBL_MAX;
	{
#if ENABLETRACE
		GSSiExitProg (257);
#endif
		return;
	}
#if ENABLETRACE
}
#endif
}
void DBoundsInit3D(LPMNMXCORD3D lpRect)
#if ENABLETRACE
{
	GSSiEnterProg(257);
#endif
	{
		lpRect->xmn = DBL_MAX;
		lpRect->ymn = DBL_MAX;
		lpRect->zmn = DBL_MAX;
		lpRect->xmx = -DBL_MAX;
		lpRect->ymx = -DBL_MAX;
		lpRect->zmx = -DBL_MAX;
		{
#if ENABLETRACE
			GSSiExitProg(257);
#endif
			return;
		}
#if ENABLETRACE
	}
#endif
}

BOOL PointInBoundsL (DPOINT Point,LPMNMXCORL pBounds)
{
	MNMXCORD	DBounds;

	DBounds.xmn = pBounds->xmn;
	DBounds.xmx = pBounds->xmx;
	DBounds.ymn = pBounds->ymn;
	DBounds.ymx = pBounds->ymx;
	return PointInBounds (Point, &DBounds);
}

BOOL PointInBounds (DPOINT Point,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (258);
#endif
{     
	 BOOL InBounds=TRUE;
     if (Point.x > pBounds->xmx ||
         Point.y > pBounds->ymx ||
         Point.x < pBounds->xmn ||
         Point.y < pBounds->ymn)
         	InBounds=FALSE;

{
#if ENABLETRACE
GSSiExitProg (258);
#endif
     return InBounds;
}
#if ENABLETRACE
}
#endif
}

BOOL BoundsInBounds (LPMNMXCORD pBounds1,LPMNMXCORD pBounds2,short Opt)
#if ENABLETRACE
{GSSiEnterProg (259);
#endif
{   
	BOOL InBounds=TRUE;
	
	switch (Opt)
	{
		case 0: //bounds 1 completely in bounds2
	    if (pBounds1->xmn < pBounds2->xmn ||   
	        pBounds1->xmx > pBounds2->xmx ||
	        pBounds1->ymn < pBounds2->ymn ||
	        pBounds1->ymx > pBounds2->ymx)
	        InBounds=FALSE; 
	    break;
	    
	    case 1: //bounds 1 at least partially in bounds2   
	    if (pBounds1->xmx < pBounds2->xmn ||   
	        pBounds1->xmn > pBounds2->xmx ||
	        pBounds1->ymx < pBounds2->ymn ||
	        pBounds1->ymn > pBounds2->ymx)
	        InBounds=FALSE; 
	    break;
	}
{
#if ENABLETRACE
GSSiExitProg (259);
#endif
    return InBounds;
}
#if ENABLETRACE
}
#endif
}

BOOL Bounds4InBounds4 (LPMNMXCORL pBounds1,LPMNMXCORL pBounds2,short Opt)
#if ENABLETRACE
{GSSiEnterProg (259);
#endif
{   
	BOOL InBounds=TRUE;
	
	switch (Opt)
	{
		case 0: //bounds 1 completely in bounds2
	    if (pBounds1->xmn < pBounds2->xmn ||   
	        pBounds1->xmx > pBounds2->xmx ||
	        pBounds1->ymn < pBounds2->ymn ||
	        pBounds1->ymx > pBounds2->ymx)
	        InBounds=FALSE; 
	    break;
	    
	    case 1: //bounds 1 at least partially in bounds2   
	    if (pBounds1->xmx < pBounds2->xmn ||   
	        pBounds1->xmn > pBounds2->xmx ||
	        pBounds1->ymx < pBounds2->ymn ||
	        pBounds1->ymn > pBounds2->ymx)
	        InBounds=FALSE; 
	    break;
	}
{
#if ENABLETRACE
GSSiExitProg (259);
#endif
    return InBounds;
}
#if ENABLETRACE
}
#endif
}

void AdjustRectToRect(LPRECT pRectToAdjust, LPRECT pRect)
{
	double	wf, hf, f;
	POINT	mp = RectMid(pRectToAdjust);

	wf = (double)RECTWIDTH(pRectToAdjust) / (double)RECTWIDTH(pRect);
	hf = (double)RECTHEIGHT(pRectToAdjust) / (double)RECTHEIGHT(pRect);
	f = (double)RECTWIDTH(pRect) / (double)RECTHEIGHT(pRect);

	if (RECTWIDTH(pRectToAdjust) * hf > RECTWIDTH(pRect))
	{
		pRectToAdjust->top = mp.y - (f * RECTHEIGHT(pRectToAdjust)) / 2;
		pRectToAdjust->bottom = mp.y + (f * RECTHEIGHT(pRectToAdjust)) / 2;
	}
	else
	{
		pRectToAdjust->left = mp.x - (RECTWIDTH(pRectToAdjust) / f) / 2;
		pRectToAdjust->right = mp.x + (RECTWIDTH(pRectToAdjust) / f) / 2;
	}

	return;
}
double AdjustRectToRectFactor(LPRECT pRectToAdjust, LPRECT pRect)
{
	double rtn = 1;
	double	wf, hf, f;
	POINT	mp = RectMid(pRectToAdjust);

	wf = (double)RECTWIDTH(pRectToAdjust) / (double)RECTWIDTH(pRect);
	hf = (double)RECTHEIGHT(pRectToAdjust) / (double)RECTHEIGHT(pRect);
	f = (double)RECTWIDTH(pRect) / (double)RECTHEIGHT(pRect);

	if (RECTWIDTH(pRect) * hf > RECTWIDTH(pRectToAdjust))
	{
		rtn = wf;
	}
	else
	{
		rtn = hf;
	}

	return rtn;
}

void AdjustBounds (LPMNMXCORD Bounds, double AdjustX, double AdjustY)
#if ENABLETRACE
{GSSiEnterProg (260);
#endif
{ 
    Bounds->xmn += AdjustX;
    Bounds->xmx += AdjustX;
    Bounds->ymn += AdjustY;
    Bounds->ymx += AdjustY;
{
#if ENABLETRACE
GSSiExitProg (260);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void ExpandBounds (LPMNMXCORD Bounds, double Adjust)
#if ENABLETRACE
{GSSiEnterProg (261);
#endif
{ 
    Bounds->xmn -= Adjust;
    Bounds->xmx += Adjust;
    Bounds->ymn -= Adjust;
    Bounds->ymx += Adjust;
{
#if ENABLETRACE
GSSiExitProg (261);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void ExpandMinMax (LPMINMAX Bounds, int Adjust)
#if ENABLETRACE
{GSSiEnterProg (262);
#endif
{   
	if ((long)Bounds->xmn - (long) Adjust >= SHRT_MIN)
    	Bounds->xmn -= Adjust; 
    else
    	Bounds->xmn = SHRT_MIN;
	if ((long)Bounds->xmx + (long) Adjust <= SHRT_MAX)
    	Bounds->xmx += Adjust;
    else
    	Bounds->xmx = SHRT_MAX;
	if ((long)Bounds->ymn - (long) Adjust >= SHRT_MIN)
    	Bounds->ymn -= Adjust;
    else
    	Bounds->ymn = SHRT_MIN;
	if ((long)Bounds->ymx + (long) Adjust <= SHRT_MAX)
    	Bounds->ymx += Adjust;
    else
    	Bounds->ymx = SHRT_MAX;
{
#if ENABLETRACE
GSSiExitProg (262);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void ExpandMinMaxL (LPMNMXCORL Bounds, long Adjust)
#if ENABLETRACE
{GSSiEnterProg (262);
#endif
{   
	if ((long)Bounds->xmn - (long) Adjust >= SHRT_MIN)
    	Bounds->xmn -= Adjust; 
    else
    	Bounds->xmn = SHRT_MIN;
	if ((long)Bounds->xmx + (long) Adjust <= SHRT_MAX)
    	Bounds->xmx += Adjust;
    else
    	Bounds->xmx = SHRT_MAX;
	if ((long)Bounds->ymn - (long) Adjust >= SHRT_MIN)
    	Bounds->ymn -= Adjust;
    else
    	Bounds->ymn = SHRT_MIN;
	if ((long)Bounds->ymx + (long) Adjust <= SHRT_MAX)
    	Bounds->ymx += Adjust;
    else
    	Bounds->ymx = SHRT_MAX;
{
#if ENABLETRACE
GSSiExitProg (262);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void TranBounds (HANDLE TranID,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (263);
#endif
{   
	DPOINT	P[4]; 
	short	i;
	
	P[0].x = pBounds->xmn;
	P[0].y = pBounds->ymn;
	P[1].x = pBounds->xmn;
	P[1].y = pBounds->ymx;
	P[2].x = pBounds->xmx;
	P[2].y = pBounds->ymx;
	P[3].x = pBounds->xmx;
	P[3].y = pBounds->ymn;
	DBoundsInit (pBounds);
	for (i=0;i<4;i++) 
	{
		TRANS2 (P[i].x,P[i].y,&P[i].x,&P[i].y,TranID); 
		AddDPointToMinMax (&P[i],pBounds);  
	}
{
#if ENABLETRACE
GSSiExitProg (263);
#endif
	return;    
}
#if ENABLETRACE
}
#endif
}

void ConvertBounds(LPMNMXCORD pBounds, int from, int to)
#if ENABLETRACE
{
	GSSiEnterProg(263);
#endif
	{   
		DPOINT	P[4];
		short	i;

		P[0].x = pBounds->xmn;
		P[0].y = pBounds->ymn;
		P[1].x = pBounds->xmn;
		P[1].y = pBounds->ymx;
		P[2].x = pBounds->xmx;
		P[2].y = pBounds->ymx;
		P[3].x = pBounds->xmx;
		P[3].y = pBounds->ymn;
		DBoundsInit(pBounds);
		for (i = 0; i<4; i++)
		{
			if (!ConvertCoord(&P[i], from, to))
				AddDPointToMinMax(&P[i], pBounds);
		}
		{
#if ENABLETRACE
			GSSiExitProg(263);
#endif
			return;
		}
#if ENABLETRACE
	}
#endif
}

void TranBoundsToRect (HANDLE TranID,LPMNMXCORD pBounds,LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (263);
#endif
{   
	DPOINT	P[4]; 
	short	i;
	
	P[0].x = pBounds->xmn;
	P[0].y = pBounds->ymn;
	P[1].x = pBounds->xmn;
	P[1].y = pBounds->ymx;
	P[2].x = pBounds->xmx;
	P[2].y = pBounds->ymx;
	P[3].x = pBounds->xmx;
	P[3].y = pBounds->ymn;
	for (i=0;i<4;i++) 
		TRANS2 (P[i].x,P[i].y,&P[i].x,&P[i].y,TranID); 
	pRect->left = max (LONG_MIN,min (LONG_MAX,IDNINT(P[0].x)));
	pRect->right = min (LONG_MAX,max (LONG_MIN,IDNINT(P[2].x)));
	pRect->top = max (LONG_MIN,min (LONG_MAX,IDNINT(P[1].y)));
	pRect->bottom = min (LONG_MAX,max (LONG_MIN,IDNINT(P[0].y)));
{
#if ENABLETRACE
GSSiExitProg (263);
#endif
	return;    
}
#if ENABLETRACE
}
#endif
}
double TranAngle (HANDLE TranID,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (264);
#endif
{   
	DPOINT	P[2]; 
	short	i; 
	double	Angle;
	
	P[0].x = pBounds->xmn + (pBounds->xmx-pBounds->xmn)/4;
	P[0].y = (pBounds->ymn + pBounds->ymn)/2;
	P[1].x = pBounds->xmx - (pBounds->xmx-pBounds->xmn)/4;
	P[1].y = P[0].y;
	TRANS2 (P[0].x,P[0].y,&P[0].x,&P[0].y,TranID); 
	TRANS2 (P[1].x,P[1].y,&P[1].x,&P[1].y,TranID);   
	Angle = getazd (&P[0],&P[1]);
{
#if ENABLETRACE
GSSiExitProg (264);
#endif
	return Angle;    
}
#if ENABLETRACE
}
#endif
}

double TranScale (HANDLE TranID,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (265);
#endif
{   
	DPOINT	P[2]; 
	short	i; 
	double	dist1,dist2;
	
	P[0].x = pBounds->xmn + (pBounds->xmx-pBounds->xmn)/4;
	P[0].y = (pBounds->ymn + pBounds->ymn)/2;
	P[1].x = pBounds->xmx - (pBounds->xmx-pBounds->xmn)/4;
	P[1].y = P[0].y;     
	dist1 = ldistp (P[0],P[1]);
	TRANS2 (P[0].x,P[0].y,&P[0].x,&P[0].y,TranID); 
	TRANS2 (P[1].x,P[1].y,&P[1].x,&P[1].y,TranID);   
	dist2 = ldistp (P[0],P[1]);
	if (dist1)
{
#if ENABLETRACE
GSSiExitProg (265);
#endif
		return dist2/dist1;    
}
	else
{
#if ENABLETRACE
GSSiExitProg (265);
#endif
		return 0;
}
#if ENABLETRACE
}
#endif
}

void GSSiGlobFree (LPHANDLE pHandle)
#if ENABLETRACE
{GSSiEnterProg (266);
#endif
{   
	HGLOBAL	st;
	short	ii;  
	UINT	flags;
	short	nLocks;   
	
    if (!*pHandle)
{
#if ENABLETRACE
GSSiExitProg (266);
#endif
    	return;
}
    if (*pHandle < (HANDLE)100)
    	ii=1; 
//    flags = GlobalFlags (*pHandle); 
//    nLocks = flags & GMEM_LOCKCOUNT;
	unsigned int len = GlobalSize(*pHandle);
	TotMemAlloc -= len;
    st = GlobalFree (*pHandle);
    if (st)
    	ii=1;
    *pHandle=0;
{
#if ENABLETRACE
GSSiExitProg (266);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

int GSSiRemove2 (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (267);
#endif
{   
	int	i=0,ii;
	HANDLE	hMem = GSSiGlobAlloc (  84,GMEM_MOVEABLE,256);
	LPSTR	str = GlobalLock (hMem);  
	HFILE	Fid;
	
	_fstrcpy (str,Name);
	ExpandText (str); 
	if (!strstr (str,".tmp"))
		ii=1;
	ConvertToNewLocation (str,TRUE);
	if (*str)
	{
		if ((Fid = FileAlreadyOpen (str,OF_READ,0)) != HFILE_ERROR)
			ActuallyCloseFile (Fid);
		AddFileToUndoFile (str,0,0);
		i=GSSiRemove32 (str); 
		RemoveBMPFromCache32 (str);
		GetLongPathName2 (str,0);
	} 
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (267);
#endif
	return i;
}
#if ENABLETRACE
}
#endif
}

int GSSiRemove (LPSTR Name)
{
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;
	
//	return GSSiRemove2 (Name);
	Fid = GSSiOpenFile (Name,&OFStruct,OF_DELETE);

	if (Fid != HFILE_ERROR)
		return 0;
	return 1;
}

int GSSiRemoveAndClear (LPSTR Name)
{
	int rtn = GSSiRemove (Name);

	*Name = 0;
	return rtn;
}

BOOL GSSiRename (LPSTR FromName, LPSTR ToName)
#if ENABLETRACE
{GSSiEnterProg (268);
#endif
{   
	BOOL rtn;
	char	str1[MAX_PATH],str2[MAX_PATH]; 
	HFILE	Fid;
	
	_fstrcpy (str1,FromName);
	ExpandText (str1);
	if ((Fid = FileAlreadyOpen (str1,OF_READ,0)) != HFILE_ERROR)
		ActuallyCloseFile (Fid);
	_fstrcpy (str2,ToName);
	ExpandText (str2);
	rtn=GSSirenamefile (str1,str2);
{
#if ENABLETRACE
GSSiExitProg (268);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

void GSSiGlobUlFree (LPHANDLE pHandle)
#if ENABLETRACE
{GSSiEnterProg (269);
#endif
{   
	HGLOBAL	st; 
	short	ii;
	
    if (!*pHandle)
{
#if ENABLETRACE
GSSiExitProg (269);
#endif
    	return;
}
    if (GlobalUnlock (*pHandle))
    	ii=1;
	TotMemAlloc -= GlobalSize (*pHandle);
    st = GlobalFree (*pHandle);
    if (st)
    	ii=1;
    *pHandle=0;
{
#if ENABLETRACE
GSSiExitProg (269);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void SECLIN8(const double *X1,const double *Y1,const double *A1,const double *X2,
             const double *Y2,const double *A2,double *X3,double *Y3,short *K)
#if ENABLETRACE
{GSSiEnterProg (270);
#endif
{                    

double Z1, Z2, DM1, DM2;
      *K=0;
      Z1=fabs(cos(*A1));
      Z2=fabs(cos(*A2));
/*C******* SEE if THEY ARE PARALLEL*/
      if(fabs(*A1-*A2) < 1e-5 || fabs(PY - (fabs(*A1 - *A2))) < 1e-5) goto s10;
/*C*/
/*C******* if A2=A1+N*PI, N=...,-3,-2,-1,0,1,2,3,..., THE LINES ARE*/
/*C******* PARALLEL, AND Dfabs(Z1-Z2)=0.*/
/*C******* THE LINES ARE NOT PARALLEL.*/
/*C******* if Z1=0, LINE 1 IS VERTICAL. if Z2=0, LINE 2 IS VERTICAL.*/
      if (min(Z1,Z2) < 1e-5) goto s30;
/*C*/
/*C******* THE LINES ARE NEITHER VERTICAL NOR PARALLEL. THE INTERSECT IN*/
/*C******* EXACTLY ONE PLACE.*/
      DM1 = sin(*A1)/cos(*A1);
      DM2 = sin(*A2)/cos(*A2);
      if(fabs(DM1-DM2)<1e-5) goto s10;
      *X3 = -(*Y1-*Y2-DM1 * *X1+DM2 * *X2)/(DM1-DM2);
      *Y3 = *Y1-DM1*(*X1-*X3);
{
#if ENABLETRACE
GSSiExitProg (270);
#endif
      return;
}
/*C*/
/*C******* THE LINES ARE PARALLEL.*/
/*C******* if THE LINES ARE IDENTICAL, THERE ARE AN INFINITE NUMBER OF*/
/*C******* SOLUTIONS. CHOOSE ONE. if THE LINES ARE DIFFERENT, THERE IS*/
/*C******* NO SOLUTION.*/
 s10: *X3=*X1;
      *Y3=*Y1;
      if (fabs(Z1+Z2) < 1e-5) goto s50;
      if (fabs(*Y1 - *Y2 - sin(*A1) /cos(*A1)*(*X1-*X2)) > 1e-5) *K=1;
{
#if ENABLETRACE
GSSiExitProg (270);
#endif
      return;
}
/*C******* AT LEAST ONE OF THE LINES IS VERTICAL.*/
 s30: if(Z2<2e-5) goto s40;
/*C******* ONLY THE FIRST LINE IS VERTICAL.*/
      *X3=*X1;
      *Y3=sin(*A2)/cos(*A2)*(*X3-*X2)+*Y2;
{
#if ENABLETRACE
GSSiExitProg (270);
#endif
      return;
}
/*C******* THE SECOND LINE IS VERTICAL.*/
 s40: *X3=*X2;
      *Y3=sin(*A1)/cos(*A1)*(*X3-*X1)+*Y1;
{
#if ENABLETRACE
GSSiExitProg (270);
#endif
      return;
}
/*C******* BOTH LINES ARE VERTICAL.*/
 s50: if(fabs(*X1-*X2)>1e-5) *K=1;
{
#if ENABLETRACE
GSSiExitProg (270);
#endif
      return;
}
#if ENABLETRACE
}
#endif
 }
LPSTR LastChr (LPSTR str)
#if ENABLETRACE
{GSSiEnterProg (272);
#endif
{
	LPSTR lc;
	
	lc = _fstrchr (str,0);
	if (lc != str)
		lc--;
{
#if ENABLETRACE
GSSiExitProg (272);
#endif
	return lc;
}
#if ENABLETRACE
}
#endif
}
  
short FillList (HWND hWndDlg,UINT Control,LPSTR file, LPSTR DefaultVal,LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (273);
#endif
{
    HFILE       Fid=HFILE_ERROR;  
    HANDLE		hMem = GSSiGlobAlloc (  85,GMEM_MOVEABLE,4096);
    LPSTR       str = GlobalLock (hMem); 
    LPSTR		str2 = str + 2048;
    LPSTR       lpBar, LastChar, pVal, pComma, pEnd; 
    short       Default=-1, l, i, Index, DefaultIndex=-1;
    int     	TabStops[2]={1000,1100}; 
    BOOL		UseBAR=FALSE, UseFile=FALSE;  
    short		ii, MaxHeight=0;
	int			MaxLen=16, NumItems=0;
    RECT		Rect;
    
    if (!hWndDlg)
    	*DefaultVal = '\0';
    if (hWndDlg)
    {
        SendDlgItemMessage (hWndDlg,Control,LB_SETTABSTOPS,2,(LPARAM)TabStops);    
        SendDlgItemMessage (hWndDlg,Control,LB_RESETCONTENT,0,0); 
    }
    
 	if (*file == '(')
 	{   
 		pVal = file+1;
 		while (*pVal == ',')
 			pVal++;
 		pComma = _fstrchr (pVal,',');
 		pEnd = LastChr (pVal);
         		
 		if (*pEnd == ')')
 			*pEnd = 0;  
		if (pComma)
			*pComma++ = 0; 
		if (*pVal)
			_fstrcpy (str,pVal);
		else
			str = NULL;
 	}
    else
    {
	    Fid=GSSiOpenFile (file,0,OF_READ);  
	    if (Fid == HFILE_ERROR) 
	    	goto Exit; 
	    UseFile = TRUE; 
	    if (!fgetstring (str,2040,Fid))
	    	str = NULL;
		else if (*LastChr (str) == ';')//allows print merge file to be read here
		{
			ProcessText (str);
			fgetstring (str,2040,Fid);
			fgetstring (str,2040,Fid);
		}
	}
    i=0;
    while (str)
    {   
 		if (!_fstrnicmp (str,"$GENLIST(",9))
 		{
 			GenList (hWndDlg,Control,FALSE,FALSE,str,DefaultVal,&UseBAR,&MaxLen,&NumItems);
 		}
 		else
 		{
	        if ((l=_fstrlen(str)))
	        {
	            LastChar = str;
	            LastChar += l;
	            LastChar--;   
	            if (*LastChar == '$')
	            {
	                Default = i;
	                *LastChar = '\0';
	            }
	        }
	        lpBar = _fstrchr(str,'|');
	        if (!lpBar)
	        {
	        	sprintf (str2,"%s|%s",str,str); 
	        	_fstrcpy (str,str2);
		        lpBar = _fstrchr(str,'|');
	        }
	        {   
	        	if (pRect)
	        		MaxLen = max (MaxLen,lpBar-str-1);
	            *lpBar = '\t';
	            if (hWndDlg) 
	            {
	                Index = SendDlgItemMessage (hWndDlg,Control,LB_ADDSTRING,0,(LPARAM)str);  
	                if (pRect)
	                {
	                	SendDlgItemMessage (hWndDlg,Control,LB_GETITEMRECT,Index,(LPARAM)&Rect);
	                	MaxHeight = max (MaxHeight,Rect.bottom - Rect.top);   
	                	NumItems++;
	                }
	                if (i == Default)
	                	DefaultIndex = Index;
	            }
	            else if (i == Default)
	                _fstrcpy (DefaultVal,++lpBar);
	        } 
	    }
        i++; 
        if (UseFile) 
        {
		    if (!fgetstring (str,2040,Fid))
		    	str = NULL;  
		}
		else
		{   
			pVal = pComma;
			if (pVal) 
			{
		 		while (*pVal == ',')
		 			pVal++;
	 			pComma = _fstrchr (pVal,',');
	 		}
			if (pComma)
				*pComma++ = 0; 
			if (pVal)
				_fstrcpy (str,pVal);
			else
				str = NULL;
		}
    }
    MaxLen += 2;//allow for scroll bar 
    if (pRect)
    {   
    	POINT	p;
	   	TEXTMETRIC	TextMet;
		HDC		hDC = GetDC (GetDlgItem (hWndDlg,Control));
		int		cw, ch;

    	GetWindowRect (GetDlgItem (hWndDlg,Control),pRect);
    	p.x = pRect->left;
    	p.y = pRect->top;
    	ScreenToClient (hWndDlg,&p);
    	pRect->left = p.x;
    	pRect->top = p.y;
		GetTextMetrics (hDC,&TextMet);
		cw = LOWORD(GetDialogBaseUnits());
		ch = HIWORD(GetDialogBaseUnits());
    	pRect->right = pRect->left + cw * MaxLen;    
    	pRect->bottom = min (pRect->bottom,pRect->top + ch * (NumItems+1));
		ReleaseDC (hWndDlg,hDC);
    }
    GSSiClose (Fid);
    if (hWndDlg)
    	SendDlgItemMessage (hWndDlg,Control,LB_SETCURSEL,DefaultIndex,0);
Exit:
	GSSiGlobUlFree (&hMem);
	if (!NumItems)
		Default = -2;    
{
#if ENABLETRACE
GSSiExitProg (273);
#endif
    return Default;
}
#if ENABLETRACE
}
#endif
}

short FillCBList (HWND hWndDlg,UINT Control,LPSTR file,short InitVal, LPSTR RtnVal)
#if ENABLETRACE
{GSSiEnterProg (274);
#endif
{
    HFILE       Fid; 
    char        str[132], INITVAL[256]="";
    LPSTR       lpBar, LastChar; 
    short       Default=-1, l, i, Item;
    int      	TabStops[2]={1000,1100}; 
    long		Val;
	OFSTRUCTGM	OFStruct;
    
    if (!hWndDlg) *RtnVal = '\0';
    if (hWndDlg)
    {
        SendDlgItemMessage (hWndDlg,Control,CB_RESETCONTENT,0,0); 
    }

    Fid=GSSiOpenFile (file,&OFStruct,OF_READ);  
    if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (274);
#endif
    	return -1; 
}
    i=0;
    while (fgetstring (str,128,Fid))
    {   
        if ((l=_fstrlen(str)))
        {
            LastChar = str;
            LastChar += l;
            LastChar--;   
            if (*LastChar == '$')
            {
                Default = i;
                *LastChar = '\0';
            }
        }
        lpBar = _fstrchr(str,'|');
        if (lpBar)
        {
            *lpBar++ = 0; 
            Val = atol (lpBar);
            if (Val == InitVal)
            	_fstrcpy (INITVAL,str);
            if (hWndDlg) 
            { 
                Item = SendDlgItemMessage (hWndDlg,Control,CB_ADDSTRING,0,(LPARAM)str);
	         	SendDlgItemMessage (hWndDlg,Control,CB_SETITEMDATA,(WPARAM)Item,(LPARAM)Val);
            }
            else if (Val == Control)
                _fstrcpy (RtnVal,str);
        }
        i++;
    }
    GSSiClose (Fid);
	if (hWndDlg)
		SendDlgItemMessage (hWndDlg,Control,CB_SELECTSTRING,-1,(LPARAM)INITVAL);

    
{
#if ENABLETRACE
GSSiExitProg (274);
#endif
    return Default;
}
#if ENABLETRACE
}
#endif
}  

long GetListNum (LPSTR file, LPSTR Val)
#if ENABLETRACE
{GSSiEnterProg (275);
#endif
{
    HFILE       Fid; 
    char        str[132];
    LPSTR       lpBar; 
	OFSTRUCTGM	OFStruct;
    long		rtn=-1;
    
    Fid=GSSiOpenFile (file,&OFStruct,OF_READ);  
    if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (275);
#endif
    	return FALSE; 
}
    while (fgetstring (str,128,Fid))
    {   
        lpBar = _fstrchr(str,'|');
        if (lpBar)
        {
            *lpBar++ = 0;
            if (!_fstricmp (str,Val)) 
            {
				ExpandText (lpBar);
            	rtn = atol (lpBar);
            	goto Exit;
            }
        }
    } 
Exit:
    GSSiClose (Fid);
    
{
#if ENABLETRACE
GSSiExitProg (275);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL GetListValue (LPSTR file, long ID, LPSTR Val)
#if ENABLETRACE
{GSSiEnterProg (276);
#endif
{
    HFILE       Fid; 
    char        str[132];
    LPSTR       lpBar; 
	OFSTRUCTGM	OFStruct;
    BOOL		rtn=FALSE;
    
    *Val = 0;
    Fid=GSSiOpenFile (file,&OFStruct,OF_READ);  
    if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (276);
#endif
    	return FALSE; 
}
    while (fgetstring (str,128,Fid))
    {   
        lpBar = _fstrchr(str,'|');
        if (lpBar)
        {
            *lpBar++ = 0;
            if (atol (lpBar) == ID) 
            {
            	_fstrcpy (Val,str); 
            	rtn = TRUE;
            	goto Exit;
            }
        }
    } 
Exit:
    GSSiClose (Fid);
    
{
#if ENABLETRACE
GSSiExitProg (276);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

void ShowProcessingMessage (LPSTR Mess)
#if ENABLETRACE
{GSSiEnterProg (277);
#endif
{ 
    
    GetWindowText(GetActiveWindow(),SaveWinText,144);
    SetWindowText (GetActiveWindow(),Mess);
    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
{
#if ENABLETRACE
GSSiExitProg (277);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void ClearProcessingMessage (void)
#if ENABLETRACE
{GSSiEnterProg (278);
#endif
{
    SetWindowText (GetActiveWindow(),SaveWinText);  
    GSSiSetCursor (OldCursor);
{
#if ENABLETRACE
GSSiExitProg (278);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

short DeleteDirAndContents (LPSTR InName)
{ 
	char	Name[260], TempName[MAX_PATH],TempDir[MAX_PATH]; 
	HFILE	Fid;        
	short	l,ii;
	long	TotFiles=0;   
    
    _fstrcpy (Name,InName);
    ExpandText (Name); 
    if (!*Name)
    	return FALSE;
    l = _fstrlen (Name);
    if (!_fstricmp (&Name[l-2],":\\"))
    	return FALSE;
	GetTempPath (MAX_PATH,TempDir);
	GetTempFileName (TempDir,"GMD",0,TempName);
//	GSSiGetTempFileName(0,"gm1",0,TempName);
	Fid =	GSSiOpenFile (TempName,0,OF_CREATE);
	SearchFilesInDir (Name,"*",Fid,&TotFiles,"*",1,TRUE,FALSE);     
	GSSillseek (Fid,0,0);
	while (fgetstring (Name,255,Fid))
	{
		LPSTR pTAB = strchr (Name,'\t');

		if (pTAB)
			*pTAB = 0;
		if (GSSiRemove (Name))
			ii=1;
	}
			//MessageBox (0,Name,"Unable to delete file",MB_ICONEXCLAMATION);
	GSSiClose (Fid); 
	
	Fid =	GSSiOpenFile (TempName,0,OF_CREATE);
    _fstrcpy (Name,InName);
    ExpandText (Name); 
    TotFiles = 0;
	SearchDirectoriesInDir (Name,Fid,&TotFiles,"*.*",1);     
	GSSillseek (Fid,0,0);
	while (fgetstring (Name,255,Fid))
		if (GSSiRemoveDir (Name))
			MessageBox (0,Name,"Unable to delete directory",MB_ICONEXCLAMATION);
	GSSiClose (Fid); 
	GSSiRemove (TempName);
    _fstrcpy (Name,InName);
    ExpandText (Name);
    if (!GSSiRemoveDir (Name))
		return TRUE;
	return FALSE;
}

LPSTR FilePart(LPSTR File,LPSTR Part)
{
	LPSTR rtn = File;
	static char inName[_MAX_PATH];
	static char fullName[_MAX_PATH];
	static char drive[_MAX_DRIVE];
	static char dir[_MAX_DIR];
	static char name[_MAX_FNAME];
	static char ext[_MAX_EXT];

	strcpy(inName,File);
	ExpandText(inName);
	_fullpath(fullName, inName, sizeof(fullName));
	_splitpath(fullName,drive,dir,name,ext);


	if (!stricmp(Part, "ACTUAL"))
	{
		ConvertToNewLocation(File, FALSE);

	}
	else if (!stricmp(Part, "DRIVE"))
		rtn = drive;
	else if (!stricmp(Part, "DRIVEDIR"))
	{
		sprintf(fullName, "%s\\%s", drive, dir);
		rtn = fullName;
	}
	else if (!stricmp(Part, "DIR"))
		rtn = dir;
	else if (!stricmp(Part, "LASTDIR"))
	{
		LPSTR lastdir = strrchr(dir, '\\');
		if (lastdir)
			rtn = ++lastdir;
		else
			rtn = dir;
	}
	else if (!stricmp(Part, "WOLASTDIR"))
	{
		LPSTR lastdir = strrchr(dir, '\\');
		if (lastdir)
			*lastdir = 0;
		rtn = dir;
	}
	else if (!stricmp(Part, "NAME"))
		rtn = name;
	else if (!stricmp(Part, "FULLNAME"))
		rtn = fullName;
	else if (!stricmp(Part, "NAMEEXT"))
	{
		sprintf(fullName, "%s.%s", name, ext);
		rtn = fullName;
	}
	else if (!stricmp(Part, "EXT"))
		rtn = ext;
	else if (!stricmp(Part, "WOEXT"))
	{
		LPSTR lastdir = strrchr(fullName, '\\');
		if (lastdir)
			*lastdir = 0;
		rtn = fullName;
	}
	return rtn;
}

long SearchFilesInDir (LPSTR CurDirIN, LPSTR Ext, HFILE OutFile,LPLONG TotFiles,LPSTR WildCardIn,int Lev,BOOL WantSub,BOOL fileNameOnly)
#if ENABLETRACE
{GSSiEnterProg (280);
#endif
{   
	DWORD	hDir, Type;
	long	NumFilesIn=*TotFiles;
    char    setstr[1024],FileName[256],FullName[256], TestExt[64], CurDir[256], str[2048], WildCard[256];
    short       i, rtn,ii;
    int st;
    BOOL	FirstPass=TRUE, SubDirOnly; 
	LPSTR	lc;
	WIN32_FIND_DATA	FindFileData;
	
	_fstrcpy (WildCard,WildCardIn);
	if (WildCard && !_fstricmp (WildCard,"*.*"))
		*WildCard = 0;
	if (Lev < 0)
	{
		WantSub = FALSE;
		Lev = -Lev;
	}
	_fstrcpy (CurDir,CurDirIN); 
	ExpandText (CurDir);  
	ConvertToNewLocation (CurDir,FALSE);
	lc  = LastChr (CurDir);
    if (*lc == '\\')
    	*lc = 0;
Top: 
	SubDirOnly = FALSE;   
    if (*WildCard && FirstPass)
    {   
    	if (_fstrchr (WildCard,'.'))
    		sprintf (setstr,"%s\\%s",CurDir,WildCard);
    	else
    		sprintf (setstr,"%s\\%s.*",CurDir,WildCard);  
    	_fstrcpy (FileName,setstr);
		hDir = SearchDirectory32 (FileName,0,&Type,&FindFileData);
//	    st = _dos_findfirst (setstr,_A_NORMAL,&FileInfo);
    }
    else 
    {   
    	sprintf (setstr,"%s\\*.*",CurDir);
    	if (*WildCard)
			SubDirOnly = TRUE;   
//    	st = _dos_findfirst (setstr,_A_SUBDIR,&FileInfo);
    	_fstrcpy (FileName,setstr);
		hDir = SearchDirectory32 (FileName,0,&Type,&FindFileData);
	   	FirstPass=FALSE;
    }
    while (hDir)
    {   
        if (FileName[0] != '.')
        {
            sprintf (str,"%s\\%s",CurDir,FileName);
//            if (FileInfo.attrib & _A_SUBDIR)  
			if (Type)
            {   
            	if (WantSub)
					SearchFilesInDir(str, Ext, OutFile, TotFiles, WildCard, Lev + 1, WantSub, fileNameOnly);
            }
            else if (SubDirOnly)
            	goto SkipFile; 
            else
            {   
            	
                _fullpath (FullName,str,sizeof(FullName));
                _splitpath (FullName,0,0,0,TestExt);  
/*                if (WildCard)
                {   short	l;
                	LPSTR	lpast;
                	
                	lpast = _fstrchr (WildCard,'*');
                	if (lpast)
                		l=lpast-WildCard;
                	else
                		l=_fstrlen(WildCard);
                	if (_fstrnicmp(WildCard,FileInfo.name,l))
                		goto SkipFile;
                } 
                if (!_fstricmp (Ext,TestExt))
                {*/
	                (*TotFiles)++;
					//strupr (FullName);
	                if (OutFile != HFILE_ERROR)
					{
						if (fileNameOnly)
	                		fputstring (FullName,OutFile);
						else
						{
							long	createTime = Time64toTime32 (FileTimeToint64(FindFileData.ftCreationTime));
							long	accessTime = Time64toTime32 (FileTimeToint64(FindFileData.ftLastAccessTime));
							long	writeTime = Time64toTime32 (FileTimeToint64(FindFileData.ftLastWriteTime));
							__int64 fLen = HighLowToint64(FindFileData.nFileSizeHigh,FindFileData.nFileSizeLow);

							sprintf (str,"%s\t%i\t%i\t%i\t%I64i",FullName,createTime,accessTime,writeTime,fLen);
	                		fputstring (str,OutFile);
						}
					}
	            //} 
	     SkipFile:;
            }
        }
#if WIN32
//        st = _findnext (st,&FileInfo);
#else
//        st = _dos_findnext (&FileInfo);
#endif
		hDir = SearchDirectory32 (FileName,hDir,&Type,&FindFileData);
    }
    if (FirstPass)
    {
    	FirstPass=FALSE;
    	goto Top;
    }
{
#if ENABLETRACE
GSSiExitProg (280);
#endif
    return (*TotFiles - NumFilesIn);
}
#if ENABLETRACE
}
#endif
} 

short IsValidDir(LPSTR Name)
{
	if (GetPathType (Name) == 2)
		return TRUE;
	return FALSE;
}

long SearchDirectoriesInDir (LPSTR CurDirIN, HFILE OutFile,LPLONG TotFiles,LPSTR WildCardIn,int Lev)
//long SearchFilesInDir (LPSTR CurDirIN, LPSTR Ext, HFILE OutFile,LPLONG TotFiles,LPSTR WildCard,short Lev)
#if ENABLETRACE
{GSSiEnterProg (280);
#endif
{   
	DWORD	hDir, Type;
	long	NumFilesIn=*TotFiles;
    char    setstr[1024],FileName[256],FullName[256], CurDir[256], str[1024], WildCard[256];
    short       i, rtn;
    int st;
    BOOL	FirstPass=TRUE, WantSub=TRUE; 
	LPSTR	lc;
	WIN32_FIND_DATA	FindFileData;
	
	_fstrcpy (WildCard,WildCardIn);
	if (WildCard && !_fstricmp (WildCard,"*.*"))
		*WildCard = 0;
	if (Lev < 0)
	{
		WantSub = FALSE;
		Lev = -Lev;
	}
	_fstrcpy (CurDir,CurDirIN); 
	ExpandText (CurDir);  
	lc  = LastChr (CurDir);
    if (*lc == '\\')
    	*lc = 0;
Top: 
    if (*WildCard && FirstPass)
    {   
    	if (_fstrchr (WildCard,'.'))
    		sprintf (setstr,"%s\\%s",CurDir,WildCard);
    	else
    		sprintf (setstr,"%s\\%s.*",CurDir,WildCard);  
    	_fstrcpy (FileName,setstr);
		hDir = SearchDirectory32 (FileName,0,&Type,&FindFileData);
    }
    else 
    {   
    	sprintf (setstr,"%s\\*.*",CurDir);
    	_fstrcpy (FileName,setstr);
		hDir = SearchDirectory32 (FileName,0,&Type,&FindFileData);
	   	FirstPass=FALSE;
    }
    while (hDir)
    {   
        if (FileName[0] != '.')
        {
            sprintf (str,"%s\\%s",CurDir,FileName);
			if (Type)
            {   
            	if (WantSub)
                	SearchDirectoriesInDir (str,OutFile,TotFiles,WildCard,Lev+1);
                (*TotFiles)++;
                _fullpath (FullName,str,sizeof(FullName));
                if (OutFile != HFILE_ERROR)
                	fputstring (FullName,OutFile);
            }
        }
		hDir = SearchDirectory32 (FileName,hDir,&Type,&FindFileData);
    }
    if (FirstPass)
    {
    	FirstPass=FALSE;
    	goto Top;
    }
{
#if ENABLETRACE
GSSiExitProg (280);
#endif
    return (*TotFiles - NumFilesIn);
}
#if ENABLETRACE
}
#endif
} 

void GSSisplitpath (LPSTR InPath,LPSTR Drive,LPSTR Dir,LPSTR Leaf,LPSTR Ext)
{
	char	Name[256];
	
	_fstrcpy (Name,InPath);
	ExpandText (Name);
	_splitpath (Name,Drive,Dir,Leaf,Ext);
	return;
}


short GetCacheInfo (LPSTR CacheDir)
#if ENABLETRACE
{GSSiEnterProg (283);
#endif
{   
    short   CacheSize;
    HFILE   Fid;   
    char    str[32];
    
    *CacheDir = '\0';
    Fid = GSSiOpenFile ("cache.txt",0,OF_READ);
    if (Fid != HFILE_ERROR)
    {
        fgetstring (CacheDir, 128, Fid);  
        ExpandText (CacheDir);
        fgetstring (str,16,Fid);
        CacheSize = (short)atol (str);
        GSSiClose(Fid);   
{
#if ENABLETRACE
GSSiExitProg (283);
#endif
        return CacheSize;
}
    }
    else
{
#if ENABLETRACE
GSSiExitProg (283);
#endif
        return 0;
}
#if ENABLETRACE
}
#endif
}

short DeleteCacheDir (void)
#if ENABLETRACE
{GSSiEnterProg (284);
#endif
{   
	short	rtn = 0;
	
    FirstCache = TRUE;
	if (!GetCacheInfo (CacheDir))
{
#if ENABLETRACE
GSSiExitProg (284);
#endif
		return -1;
}
//    GSSiRemove ("cache.txt"); 
	if (FileType (CacheDir) == 2)     
		rtn = DeleteDirAndContents (CacheDir);
{
#if ENABLETRACE
GSSiExitProg (284);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}    

int GSSiGetTempFileName (BYTE Drive,LPSTR Pre,UINT Uniquex,LPSTR Name)
{   
	char	Dir[MAX_PATH];
	char	Prefix[8];
	int rtn,ii;
	static	UINT	Unique=1;

	strncpy0 (Prefix,Pre,3);
	strcat (Prefix,"xxx");
	Prefix[3] = 0;
	GetTempDir (Dir);
	rtn = GetTempFileName (Dir,Prefix,Unique++,Name); 
	return rtn;
} 

BOOL GetTempDir (LPSTR Dir)
{   
	LPSTR	pBS;
	static	__time32_t	StartTime=0;
	static	char TempDir[MAX_PATH];
	int		n=0;
	BOOL	rtn;
	
	if (!Dir)
	{
		DeleteDirAndContents (TempDir);
		StartTime = 0;
		return TRUE;
	}
	if (StartTime)
	{
		strcpy (Dir,TempDir);
		return TRUE;
	}
	_time32 (&StartTime);
	do
	{
		GetTempPath (MAX_PATH,TempDir);
		sprintf (strchr(TempDir,0),"gmtemp\\%lx",StartTime++);
	}while (n++<10 && GetPathType (TempDir));

	rtn = makedirectories (TempDir,TRUE,FALSE);
	strcpy (Dir,TempDir);
//	if ((pBS = strrchr (Dir,'\\')))
//		*pBS = 0;
//	GetLongPathName2 (Dir,MAX_PATH);
	return rtn;
} 

BOOL GetCacheDir (LPSTR Dir)
{   
	
	strcpy (Dir,"ALLUSERAPPDATA");
	GM32GetSpecialDirectory (Dir);
	//GetTempPath (MAX_PATH,Dir);
	if (*LastChr (Dir) == '\\')
		strcat (Dir,"gmcache");
	else
		strcat (Dir,"\\gmcache");
	GetLongPathName2 (Dir,MAX_PATH);
	return TRUE;
} 


BOOL GetCacheFile (LPSTR UseFile, LPSTR DiskFile, BOOL Add, HWND StatusWnd)
#if ENABLETRACE
{GSSiEnterProg (285);
#endif
{   
    static  long    MaxCache;
    short       NumFiles, NewNumFiles, st, i;
    long    FreeSpace, MaxFileNum,  CurrentLoc, 
		     NeedLength,  length; 
	HANDLE	hMem = GSSiGlobAlloc (  86,GMEM_MOVEABLE,4096);
    LPSTR    str=GlobalLock (hMem);
    LPSTR	 CacheFile=str+256;
    LPOFSTRUCTGM	pOFStruct = (LPOFSTRUCTGM)(CacheFile + 256);
    LPSTR    lpExt,  pBuffer;
    HFILE    FidIndex=HFILE_ERROR, FidFile;
    HANDLE  hBuffer=0;
    HFILE    Fid;
    CACHEBUF    *pCacheBuf, *pDeleteBuf;   
    BOOL	rtn=FALSE;   
    DWORD	Err;
    
//goto Exit; 
    if (FirstCache)
    {
        FirstCache = FALSE;
        Fid = GSSiOpenFile ("[%DL]cache.txt",pOFStruct,OF_READ);
        if (Fid != HFILE_ERROR)
        {
            fgetstring (CacheDir, 128, Fid);
            ExpandText (CacheDir);
            fgetstring (str,16,Fid);
            MaxCache = atol (str);
            MaxCache *= ((long)1024 * (long)1024);
            GSSiClose(Fid); 
			if (!makedirectories (CacheDir,TRUE,FALSE))
            {
                //if (Err)
                {
                    GSSiMsgBox( GetFocus(), "Unable to create Cache directory",
                    CacheDir, MB_OK,0);
                    goto NoCache;
                }
            } 
            _fstrcpy(CacheIndex,CacheDir);
            _fstrcat(CacheIndex,"\\index"); 
            if (!ExistFile(CacheIndex))
            {   
                ShowProcessingMessage ("Creating Cache Directory");
                FidIndex = GSSiOpenFile(CacheIndex,pOFStruct,OF_CREATE);
                NumFiles = 0;
                FreeSpace = MaxCache;
                MaxFileNum = 0;
                BigWrite (FidIndex,(char *)&NumFiles,2,-1);
                BigWrite (FidIndex,(char *)&MaxCache,4,-1);
                BigWrite (FidIndex,(char *)&FreeSpace,4,-1);
                BigWrite (FidIndex,(char *)&MaxFileNum,4,-1);
                GSSiClose (FidIndex);  
                FidIndex = HFILE_ERROR;
                _fstrcpy(CacheFile,CacheDir);
                _fstrcat(CacheFile,"\\freespac");
                FidFile = GSSiOpenFile(CacheFile,pOFStruct,OF_CREATE);
                {
                    long    FileSize, Done=0;
                    WORD    size;
                    HANDLE  h0s;
                    LPSTR   p0s;
                    
                    h0s = GSSiGlobAlloc (  87,GHND,SHRT_MAX);
                    p0s = GlobalLock (h0s);
                    FileSize = FreeSpace;
                    while (FileSize)
                    {   
                        size = min (FileSize,SHRT_MAX);
                        BigWrite (FidFile,p0s,size,-1);
                        FileSize-=size; 
                        Done+=size;
                        if (StatusWnd)
                            PctBox (StatusWnd, FreeSpace, Done,0);
                    }
                    GSSiGlobUlFree (&h0s);
                }
                GSSiClose(FidFile);
                ClearProcessingMessage();
            }
        }
        else
NoCache:    CacheDir[0]='\0';
    }    
    if (CacheDir[0])
    {   
        /* Open the cache index and determine if file aleady in cache */
        lpExt = _fstrrchr (DiskFile,'.');
        FidIndex = GSSiOpenFile(CacheIndex,pOFStruct,OF_READWRITE);
		if (FidIndex == HFILE_ERROR)
			goto UseDiskFile;
        BigRead (FidIndex,(HPSTR)&NumFiles,2);
        BigRead (FidIndex,(HPSTR)&MaxCache,4);
        BigRead (FidIndex,(HPSTR)&FreeSpace,4);
        BigRead (FidIndex,(HPSTR)&MaxFileNum,4);
        hBuffer = GSSiGlobAlloc (  88,GMEM_MOVEABLE,(long)sizeof(CACHEBUF)*(NumFiles+1));
        pBuffer = GlobalLock (hBuffer);
        pCacheBuf = (CACHEBUF *) pBuffer;
        for (i=0;i<NumFiles;i++,pCacheBuf++)
        {   
            CurrentLoc = GSSillseek (FidIndex,0,1);
            BigRead (FidIndex,(HPSTR)pCacheBuf,sizeof(CACHEBUF));
            if (!_fstricmp(pCacheBuf->FileName,DiskFile))
            {
                sprintf (UseFile,"%s\\%ld%s",CacheDir,pCacheBuf->FileNum,lpExt);    
            /*  sprintf (UseFile,"%s\\%ld",CacheDir,pCacheBuf->FileNum);    */
                pCacheBuf->LastUsed = time(0); 
                GSSillseek (FidIndex,CurrentLoc,0);
                BigWrite (FidIndex,(char *)pCacheBuf,sizeof(CACHEBUF),-1);
                GSSiGlobUlFree(&hBuffer);
                GSSiClose(FidIndex);
                FidIndex = HFILE_ERROR;
                rtn = TRUE;
                goto Exit;
            }
        } 
        
        if (!Add) goto UseDiskFile;
        
        /* Determine length of new files */
        if ((length = GSSiLength(DiskFile)) < 0)
        	goto UseDiskFile;
        
        /* Make space for the new file by taking from freespace and/or 
           deleting existing files */
        if (length > MaxCache) goto UseDiskFile; 
        NeedLength = max(0,length - FreeSpace);
        FreeSpace = max(0,FreeSpace-length);
        while (NeedLength)
        {
            pCacheBuf = (CACHEBUF *) pBuffer;
            pDeleteBuf = (CACHEBUF *)pBuffer;
            for (i=0;i<NumFiles;i++,pCacheBuf++)
            {
                if (pCacheBuf->LastUsed<pDeleteBuf->LastUsed)
                    pDeleteBuf = pCacheBuf;
            }
            pDeleteBuf->LastUsed = LONG_MAX;
            if (pDeleteBuf->FileLength >= NeedLength)
            {
                FreeSpace = pDeleteBuf->FileLength - NeedLength;   
                NeedLength = 0;
            }
            else
                NeedLength = NeedLength - pDeleteBuf->FileLength;
            sprintf (CacheFile,"%s\\%ld",CacheDir,pDeleteBuf->FileNum); 
            GSSiRemove (CacheFile);
            pDeleteBuf->FileNum = 0;
        }
        
        /* Adjust the freespace file */
        _fstrcpy(CacheFile,CacheDir);
        _fstrcat(CacheFile,"\\freespac");
       	CloseFidSmall ();
        FidFile = GSSiOpenFile(CacheFile,pOFStruct,OF_READWRITE);
        st = GSSiChangeLength (FidFile,FreeSpace);
        GSSiClose(FidFile);
        CreateFidSmall ();
        if (st == -1)
        {    
            GSSiMsgBox( GetFocus(), "Unable to adjust Cache directory",
            "Insufficient Space or File Handle Limit", MB_OK,0);
            goto NoCache;
        }
        
        /* Copy the new file to the cache */
        pCacheBuf = (CACHEBUF *)pBuffer;
        pCacheBuf += NumFiles;
        MaxFileNum++;
        pCacheBuf->FileNum = MaxFileNum;
        _fstrcpy (pCacheBuf->FileName,DiskFile);
        sprintf (UseFile,"%s\\%ld%s",CacheDir,pCacheBuf->FileNum,lpExt);    
        /*sprintf (UseFile,"%s\\%ld",CacheDir,pCacheBuf->FileNum);  */
        pCacheBuf->LastUsed = time(0); 
        pCacheBuf->FileLength = length;
        
        sprintf (str,"Copying %s to cache",DiskFile);
        ShowProcessingMessage (str); 
        st = copyfile (UseFile,DiskFile,FALSE,0,0,0,0,0,0); 
        ClearProcessingMessage();
        if (!st) goto UseDiskFile;
        
        /* Update the cache index */
        GSSillseek (FidIndex,0,0);
        BigWrite (FidIndex,(char *)&NumFiles,2,-1);
        BigWrite (FidIndex,(char *)&MaxCache,4,-1);
        BigWrite (FidIndex,(char *)&FreeSpace,4,-1);
        BigWrite (FidIndex,(char *)&MaxFileNum,4,-1);
        NewNumFiles = 0;
        pCacheBuf = (CACHEBUF *)pBuffer;
        NumFiles++;
        for (i=0;i<NumFiles;i++,pCacheBuf++)
        {   
            if (pCacheBuf->FileNum > 0)
            {
                NewNumFiles++;
                BigWrite (FidIndex,(char *)pCacheBuf,sizeof(CACHEBUF),-1);
            }
        }
        GSSillseek (FidIndex,0,0);
        BigWrite (FidIndex,(char *)&NewNumFiles,2,-1);
        
        GSSiGlobUlFree (&hBuffer);
        GSSiClose(FidIndex);
        FidIndex = HFILE_ERROR;  
        rtn = TRUE;
        goto Exit;
    }
    else
    {
UseDiskFile:
        _fstrcpy (UseFile,DiskFile);
        GSSiGlobUlFree (&hBuffer);
        if (FidIndex != HFILE_ERROR)
        	GSSiClose(FidIndex);
        goto Exit;
    }
Exit:          
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (285);
#endif
	return rtn;
}
         
#if ENABLETRACE
}
#endif
} 

BOOL RemoveCacheFile (LPSTR File)
#if ENABLETRACE
{GSSiEnterProg (286);
#endif
{   
	HFILE	FidIndex, FidFile;
	long	MaxCache, FreeSpace, MaxFileNum, CurrentLoc, len, FreeSpaceLoc;
	short	NumFiles, i, st;
    LPSTR   pBuffer, lpExt;
    OFSTRUCTGM    OFStruct; 
    HFILE    Fid;
    CACHEBUF CacheBuf;
    char	CacheFile[132];
	
    if (FirstCache)
{
#if ENABLETRACE
GSSiExitProg (286);
#endif
    	return FALSE;
}
    lpExt = _fstrrchr (File,'.');
    FidIndex = GSSiOpenFile(CacheIndex,&OFStruct,OF_READWRITE);
    if (FidIndex == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (286);
#endif
    	return FALSE;
}
    BigRead (FidIndex,(HPSTR)&NumFiles,2);
    BigRead (FidIndex,(HPSTR)&MaxCache,4);
    FreeSpaceLoc = GSSillseek (FidIndex,0,1);
    BigRead (FidIndex,(HPSTR)&FreeSpace,4);
    BigRead (FidIndex,(HPSTR)&MaxFileNum,4);
    for (i=0;i<NumFiles;i++)
    {   
        CurrentLoc = GSSillseek (FidIndex,0,1);
        BigRead (FidIndex,(HPSTR)&CacheBuf,sizeof(CACHEBUF));
        if (!_fstricmp(CacheBuf.FileName,File))
        {
            sprintf (CacheFile,"%s\\%ld%s",CacheDir,CacheBuf.FileNum,lpExt);
            len = GSSiLength (CacheFile);  
            FreeSpace += len;
            GSSiRemove (CacheFile);   
            CacheBuf.LastUsed = 0;  
            *CacheBuf.FileName = 0;
            GSSillseek (FidIndex,CurrentLoc,0);
            BigWrite (FidIndex,(HPSTR)&CacheBuf,sizeof(CACHEBUF),-1); 
            GSSillseek (FidIndex,FreeSpaceLoc,0);
            BigWrite (FidIndex,(HPSTR)&FreeSpace,4,-1);
            GSSiClose(FidIndex);
            sprintf (CacheFile,"%s\\freespac",CacheDir);
          	CloseFidSmall ();
	        FidFile = GSSiOpenFile(CacheFile,&OFStruct,OF_READWRITE);
	        st = GSSiChangeLength (FidFile,FreeSpace);
	        GSSiClose(FidFile);
          	CreateFidSmall ();
{
#if ENABLETRACE
GSSiExitProg (286);
#endif
            return TRUE;
}
        } 
    } 
        
{
#if ENABLETRACE
GSSiExitProg (286);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

short RemoveTrailingBlanks (LPSTR str)
#if ENABLETRACE
{GSSiEnterProg (287);
#endif
{   short   i;

    i = _fstrlen(str); 
    str += i;
    while (i)
    {
        str--;
        if (*str != ' ')
{
#if ENABLETRACE
GSSiExitProg (287);
#endif
        	return(i);
}
        i--;
        *str = '\0';
    }
{
#if ENABLETRACE
GSSiExitProg (287);
#endif
    return (i);
}
#if ENABLETRACE
}
#endif
}

BOOL ExistFile(LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (288);
#endif
{   
    struct  _stat    buf;
    OFSTRUCTGM OFStruct; 
    short   i;    
	BOOL	rtn=FALSE;
    
	if (!strnicmp("GOOGLE.",Name,7))
		rtn = TRUE;
	else if (RunFromCache)
	{
		char nameTemp[MAX_PATH];

		strcpy (nameTemp,Name);
		ConvertFileNameToCacheFileName (nameTemp);
		if (GSSiOpenFile(nameTemp,&OFStruct,OF_EXIST)!=HFILE_ERROR)
			rtn = TRUE;
	}
    else if (*Name)
    {
		if (strnicmp (Name,"http:",5))
		{
			if (GSSiOpenFile(Name,&OFStruct,OF_EXIST)!=HFILE_ERROR)
				rtn = TRUE;
		}
		else if (GetPathType (Name) == 3)
			rtn = 3;
	}
{
#if ENABLETRACE
GSSiExitProg (288);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

int GetPathType2(LPSTR Name) //returns: 0=not found,1=file, 2=directory,3=http
{
	WIN32_FILE_ATTRIBUTE_DATA	WFAD;
	int		rtn = 0;
	if (*Name)
	{
		if (_fstrnicmp (Name,"http:",5))
		{
			if (GetFileAttributesEx(Name, GetFileExInfoStandard, &WFAD))
			{
				if (WFAD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					rtn = 2;
				else
					rtn = 1;
			}
		}
		else
			rtn = 3;
	}
	return rtn;
}

int GetPathType (LPSTR InName) //returns: 0=not found,1=file, 2=directory,3=http
#if ENABLETRACE
{GSSiEnterProg (288);
#endif
{   
	char	Name[MAX_PATH];
	int		rtn;
    
	strcpy (Name,InName);
	ExpandText (Name);
	ConvertToNewLocation (Name,TRUE);
	rtn = GetPathType2 (Name);
{
#if ENABLETRACE
GSSiExitProg (288);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

short FileType (LPSTR Name)
{
	return GetPathType (Name);
}

short FileType_old(LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (289);
#endif
{   
    OFSTRUCTGM OFStruct;  
    
    if (GSSiOpenFile(Name,&OFStruct,OF_EXIST)==HFILE_ERROR) 
    {
    	if (OFStruct.nErrCode == 5)
{
#if ENABLETRACE
GSSiExitProg (289);
#endif
    		return 2;
}
    	else
{
#if ENABLETRACE
GSSiExitProg (289);
#endif
        	return 0;
}
    }
{
#if ENABLETRACE
GSSiExitProg (289);
#endif
    return 1;
}
#if ENABLETRACE
}
#endif
}

void SetTrace (BOOL On)
#if ENABLETRACE
{GSSiEnterProg (290);
#endif
{   
    if (!On || On == 3)
    {
        if (TraceFid != HFILE_ERROR && TraceOn) 
            _lclose (TraceFid);
        if (TraceOn == 4)
        	TraceInWindow (0);
        TraceOn = FALSE; 
        TraceFid = HFILE_ERROR; 
    }
    else
        TraceOn = On; 
    if (TraceOn == 4)
    	TraceType[0] = 1;
{
#if ENABLETRACE
GSSiExitProg (290);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL TRACEWINDOWMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (291);
#endif
{    
 int    TabStops[2]={4,4};
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (291);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:

         SendDlgItemMessage (hWndDlg,IDC_TRACETEXT,LB_SETTABSTOPS,1,(LPARAM)TabStops);    
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
	     DestroyWindow(hWndDlg);
         hWndTrace = 0;  
         TraceOn = 0;
		 FreeProcInstance(lpfnTRACEWINDOWMsgProc);
         break; /* End of WM_CLOSE                                      */ 
    case WM_DESTROY:
    	 break;
    case WM_COMMAND:
         switch(wParam)
         {  
         	case IDOK:
         		 break;
         		 
           	case IDM_STEPOVER:
                 StepOver = CurTraceLev;
           	case IDM_STEP:
                 NeedStep = FALSE;
           		 break;
           		 
         }
         break;     

    default:
{
#if ENABLETRACE
GSSiExitProg (291);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (291);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

void TraceInWindow (LPSTR str)
#if ENABLETRACE
{GSSiEnterProg (292);
#endif
{ 
	MSG	msg;
	HANDLE	hSTR=0; 
	short	index;
	
    if (!str)
    {   
    	if (hWndTrace)
         	PostMessage(hWndTrace, WM_CLOSE, 0, 0L);
{
#if ENABLETRACE
GSSiExitProg (292);
#endif
    	return;
}
    }
	if (!hWndTrace)
	{ 
	  lpfnTRACEWINDOWMsgProc = MakeProcInstance((DLGPROC)TRACEWINDOWMsgProc, hInst);
	  hWndTrace=CreateDialog(hInst,"TRACEWINDOW",hWndMain, lpfnTRACEWINDOWMsgProc);
	}
	if (CurTraceLev <= 0)
	{
    	SendDlgItemMessage (hWndTrace,IDC_TRACETEXT,LB_RESETCONTENT,0,0); 
    }
    else if (StepOver && CurTraceLev > StepOver)
{
#if ENABLETRACE
GSSiExitProg (292);
#endif
    		return; 
}
    
    else
    {   
    	LPSTR	str2;
    	
    	hSTR = GSSiGlobAlloc (1745,GMEM_MOVEABLE,2048);  
    	str2 = GlobalLock (hSTR);
    	_fmemset (str2,'\t',CurTraceLev);
    	_fstrcpy (&str2[CurTraceLev],str);
    	str = str2;
    }
	index = SendDlgItemMessage (hWndTrace,IDC_TRACETEXT,LB_ADDSTRING,0,(LPARAM)str);
	GSSiGlobUlFree (&hSTR);  
	SendDlgItemMessage (hWndTrace,IDC_TRACETEXT,LB_SETTOPINDEX,(WPARAM)index,(LPARAM)0); 
	NeedStep = TRUE;
	StepOver = FALSE;
	while (NeedStep && hWndTrace && GSSiGetMessage(&msg,0,0,0))
	{
			   
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		
	}
{
#if ENABLETRACE
GSSiExitProg (292);
#endif
	return; 
}
#if ENABLETRACE
}
#endif
}


void GSSiTraceLev (LPSTR str,short lev,short From)
#if ENABLETRACE
{GSSiEnterProg (293);
#endif
{
	if (lev < 0)
		CurTraceLev--;
	GSSiTrace (str,From);
	if (lev > 0)
		CurTraceLev++;
{
#if ENABLETRACE
GSSiExitProg (293);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void debugFile(LPSTR id, LPSTR text)
{
	char str[4096];
	sprintf(str, "%s:%s", id, text);
	AppendFile2("c:/temp/debugfile.txt", str);
}
void GSSiTrace (LPSTR str,short From)
#if ENABLETRACE
{GSSiEnterProg (294);
#endif
{   
	static	BOOL	InTrace=FALSE;
//	if (InTrace || !TraceOn || (TraceOn == 4 && !TraceType[From]))
	if (InTrace || !TraceOn || !TraceType[From])
	{
#if ENABLETRACE
GSSiExitProg (294);
#endif
		return;
}   
	{
	HANDLE	hMem=GSSiGlobAlloc (  89,GMEM_MOVEABLE,1024+4096+128);
    LPSTR   txt = GlobalLock (hMem);
    LPSTR	errmes = txt + 128;
    LPSTR	Spaces = errmes + 128; 
    LPSTR   TraceFile = Spaces + 128;
    LPSTR	pLine = TraceFile + 256;
    LPOFSTRUCTGM    pOFStruct = (LPOFSTRUCTGM) (pLine + 4096); 
    
    InTrace = TRUE;
    switch (TraceOn)
    {
    	case 0:
    		goto Exit;  
    	case 4:
    		TraceInWindow (str);
    		goto Exit;
    }
    
    if (TraceFid == HFILE_ERROR)                                      
    { 
		int saveTraceOn = TraceOn;
		BOOL saveContinueProcessing = ContinueProcessing;
		SetContinueProcessing ( 1);
		TraceOn = 0;
		GetGlobalCVal("[%TRACEFILE]",txt,"c:\\temp\\trace.txt");
		TraceOn = saveTraceOn;
		SetContinueProcessing ( saveContinueProcessing);
        _fullpath(TraceFile,txt,256);
        TraceFid = OpenFileGM (TraceFile,pOFStruct,OF_READWRITE); 
        if (TraceTrace)
        {
            sprintf (txt,"Read trace file %i %i",(short) TraceFid,(short) pOFStruct->nErrCode);
            SetWindowText (hWndMain,txt);
            Sleep(500);
        }
    }
    if (TraceFid == HFILE_ERROR)
    {
        TraceFid = OpenFileGM (TraceFile,pOFStruct,OF_CREATE);  
        if (TraceTrace)
        {
            sprintf (txt,"Read trace file %i %i",(short) TraceFid,(short) pOFStruct->nErrCode);
            SetWindowText (hWndMain,txt);
            Sleep(500);
        }
        if (TraceFid == HFILE_ERROR)
        {   
        
            EnableWindow (hWndMain,FALSE);
            HaltMapDisplay(FALSE,TRUE); 
            TraceOn = FALSE;
            if (!LoadString(hInst, GetOPENERR00()+pOFStruct->nErrCode, errmes, 32)) errmes[0]='\0';
            sprintf (txt,"Cannot create %s: error code is %i (%s)",TraceFile,(short)pOFStruct->nErrCode,errmes);
            GSSiMsgBox( GetFocus(), txt,"", MB_OK|MB_ICONQUESTION,0);
            goto Exit;  
        }
    }
   	_llseek(TraceFid,0,2);
    if (CurTraceLev > 0)
    {
    	_fmemset (Spaces,' ',CurTraceLev);  
    	Spaces[CurTraceLev] = 0;
    } 
    else
    	*Spaces = 0;
    sprintf (pLine,"%s%s",Spaces,str);
//   	fputstring (pLine,TraceFid);
	_lwrite (TraceFid,pLine,strlen(pLine));
	_lwrite (TraceFid,"\r\n",2);
    GSSiGlobUlFree (&hMem);
    if (TraceOn == 2)
    {
        _lclose (TraceFid);
        TraceFid = HFILE_ERROR;
    }
Exit:
	GSSiGlobUlFree (&hMem);  
	InTrace = FALSE;
	}
{
#if ENABLETRACE
GSSiExitProg (294);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}     
           
BOOL atob (LPSTR Value)
#if ENABLETRACE
{GSSiEnterProg (295);
#endif  
{ 
	if (!*Value)
{
#if ENABLETRACE
GSSiExitProg (295);
#endif
	return FALSE;
}
	if (!_fstrcspn (Value," 0FfNn"))
{
#if ENABLETRACE
GSSiExitProg (295);
#endif
		return FALSE;
}
{
#if ENABLETRACE
GSSiExitProg (295);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

void btoa (BOOL Val,LPSTR str)
{
	if (Val)
		*str = '1';
	else
		*str = '0';
	str[1] = 0;
	return;
}

DPOINT atopt (LPSTR Value,LPBOOL pErr)
#if ENABLETRACE
{GSSiEnterProg (296);
#endif
{                 
	DPOINT	DPoint;
	
	if (sscanf (Value,"%lf %lf",&DPoint.x,&DPoint.y) != 2)
	{
		*pErr = TRUE;
		DPoint.x = DPoint.y = 0;
	}
	else
		*pErr = FALSE;
{
#if ENABLETRACE
GSSiExitProg (296);
#endif
	return DPoint;
}
#if ENABLETRACE
}
#endif
} 
 
POINT atopt16 (LPSTR Value,LPBOOL pErr)
#if ENABLETRACE
{GSSiEnterProg (296);
#endif
{           
	POINT	Point;
	DPOINT	DPoint;
	
	if (sscanf (Value,"%lf %lf",&DPoint.x,&DPoint.y) != 2)
	{
		if (pErr)
			*pErr = TRUE;
		Point.x = Point.y = 0;
	}
	else
	{
		Point.x = IDNINT(DPoint.x);
		Point.y = IDNINT(DPoint.y);
		if (pErr)
			*pErr = FALSE;
	}
{
#if ENABLETRACE
GSSiExitProg (296);
#endif
	return Point;
}
#if ENABLETRACE
}
#endif
} 
 
BOOL ValidBounds (LPMNMXCORD Bounds)
{   
	if (Bounds->xmx == Bounds->xmn &&
		Bounds->ymx == Bounds->ymn)
			return FALSE;
	if (Bounds->xmx < Bounds->xmn ||
		Bounds->ymx < Bounds->ymn)
			return FALSE;
	return TRUE;
}

BOOL ValidBounds2 (LPMNMXCORD Bounds)
{   
	if (Bounds->xmx < Bounds->xmn ||
		Bounds->ymx < Bounds->ymn)
			return FALSE;
	return TRUE;
}

MNMXCORD atobounds(LPSTR Value, LPBOOL err)
#if ENABLETRACE
{GSSiEnterProg (297);
#endif
{
	MNMXCORD	Bounds;

	if (sscanf(Value, "%lf %lf %lf %lf", &Bounds.xmn, &Bounds.ymn, &Bounds.xmx, &Bounds.ymx) != 4)
	{
		Bounds.xmn = Bounds.ymn = Bounds.xmx = Bounds.ymx = 0;
		*err = TRUE;
	}
	else
		*err = FALSE;
	{
#if ENABLETRACE
		GSSiExitProg (297);
#endif
		return Bounds;
	}
#if ENABLETRACE
}
#endif
}
MNMXCORD3D atobounds3D(LPSTR Value, LPBOOL err)
#if ENABLETRACE
{GSSiEnterProg (297);
#endif
{
	MNMXCORD3D	Bounds;

	if (sscanf(Value, "%lf %lf %lf %lf %lf %lf", &Bounds.xmn, &Bounds.ymn, &Bounds.zmn, &Bounds.xmx, &Bounds.ymx, &Bounds.zmx) != 6)
	{
		Bounds.xmn = Bounds.ymn = Bounds.zmn = Bounds.xmx = Bounds.ymx = Bounds.zmx = 0;
		*err = TRUE;
	}
	else
		*err = FALSE;
	{
#if ENABLETRACE
		GSSiExitProg(297);
#endif
		return Bounds;
	}
#if ENABLETRACE
}
#endif
}

RECT atorect (LPSTR Value,LPBOOL pErr)
#if ENABLETRACE
{GSSiEnterProg (298);
#endif
{                 
	RECT	Rect;
	
	Rect.left = Rect.right = Rect.bottom = Rect.top = 0;
	if (sscanf(Value, "%i %i %i %i", &Rect.left, &Rect.top, &Rect.right, &Rect.bottom) != 4)
	{	
		*pErr = TRUE;
	}
	else
		*pErr = FALSE;
{
#if ENABLETRACE
GSSiExitProg (298);
#endif
	return Rect;
}
#if ENABLETRACE
}
#endif
}

void recttoa (LPSTR Value,RECT Rect)
#if ENABLETRACE
{GSSiEnterProg (299);
#endif
{
	sprintf (Value,"%i %i %i %i",Rect.left,Rect.top,Rect.right,Rect.bottom);
{
#if ENABLETRACE
GSSiExitProg (299);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void pointstoa (LPSTR OutLoc,int nPoints,HANDLE hPoints,HANDLE hTran)
{
	HPDPOINT pPoint = (HPDPOINT)GlobalLock (hPoints);
	DPOINT	 Point;
	int	n;
	
	*OutLoc = 0;
	for (n=0;n<nPoints;n++)
	{   
		if (n)
			_fstrcat (OutLoc," ");
		Point =  TranPoint (&pPoint[n],hTran);
		dpointtoa (_fstrrchr(OutLoc,0),&Point); 
	}
	GlobalUnlock (hPoints);
	return;
}

void pttoa (LPSTR Value,POINT Point)
#if ENABLETRACE
{GSSiEnterProg (299);
#endif
{
	sprintf (Value,"%i %i",Point.x,Point.y);
{
#if ENABLETRACE
GSSiExitProg (299);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

MNMXCORD lboundstobounds (LPMNMXCORL plbounds)
{
	MNMXCORD bounds={plbounds->xmn,plbounds->ymn,plbounds->xmx,plbounds->ymx};

	return bounds;
}

void lboundstoa (LPSTR Value,LPMNMXCORL Bounds)
{
	sprintf (Value,"%i %i %i %i",Bounds->xmn,Bounds->ymn,Bounds->xmx,Bounds->ymx); 
	return;
}

void boundstoa (LPSTR Value,LPMNMXCORD Bounds)
{
	sprintf (Value,"%.14lg %.14lg %.14lg %.14lg",Bounds->xmn,Bounds->ymn,Bounds->xmx,Bounds->ymx); 
	return;
}

LPSTR ftoa (LPSTR Value,double DVal)
{
	sprintf (Value,"%.14lg",DVal); 
	return Value;
}

void dpointtoa (LPSTR Value,LPDPOINT pPoint)
{
	sprintf (Value,"%.14lg %.14lg",pPoint->x,pPoint->y); 
	return;
}

void dpointtoatrunc (LPSTR Value,LPDPOINT pPoint)
{
	sprintf (Value,"%.9lg %.9lg",pPoint->x,pPoint->y); 
	return;
}

BOOL GSSiChangeLength (HFILE Fid,LONGLONG NewLength) 
{        
	short	st;
	BOOL	rtn = FALSE;

	if (Fid == HFILE_ERROR)
		return FALSE;
	if (JournalFileFid[Fid] != HFILE_ERROR)
		ChangeSizeOfFileInJournal (Fid,NewLength);
	else
	{
		if (NewLength > 0)
			AddFileToUndoFile (0,NewLength+1,OpenFileFid[Fid]);
		else
			NewLength = -NewLength;
		st = _chsize_s (OpenFileFid[Fid],NewLength);
		if (!st)
		{
			OpenFileLength[Fid] = NewLength;
			rtn = TRUE;
		}
	}
	return rtn;
}

  
HFILE GSSiClose2 (LPHFILE pFid)
{
	HFILE rtn = GSSiClose (*pFid);
	
	*pFid = HFILE_ERROR;
	return rtn;
}

HFILE GSSiClose (HFILE Fid)
																							#if ENABLETRACE
																							{GSSiEnterProg (301);
																							#endif
{   HFILE rtn;  
	short	ii; 
	USHORT	i;
	static	USHORT	debugi=12;
	static	HFILE	debugid=35;
	 
	if (Fid == HFILE_ERROR)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (301);
																							#endif
		return 0; 
																							}
	if (Fid <= 0)
		ii=1; 
	if (!InOpenFile && RequestFileClose (Fid))
																							{
																							#if ENABLETRACE
																							GSSiExitProg (301);
																							#endif
		return 0;
																							}
    if (UndoEnabled)
	{
		short	ID = GetUndoFileIDFromFid (Fid);
		
		if (ID > -1)
			UndoFileFid[ID] = -1; 
		for (i=0;i<MAXFILEHANDLES;i++)
			if (OpenFileFid[i] == Fid)
			{
				OpenFileUndoFileID[i] = -1;
				if (i==debugi)
					ii=1; 
				break;
			}
	}
	rtn = ActuallyCloseFile (Fid);																							{

																							#if ENABLETRACE
																							GSSiExitProg (301);
																							#endif
    return rtn;
																							}
																							#if ENABLETRACE
																							}
																							#endif
}

int FlushFile (HFILE Fid)
{
	int	rtn=0;

	if (OpenFileFid[Fid] > -1)
		rtn = _commit (OpenFileFid[Fid]);
	return rtn;
}

int ActuallyCloseFile (HFILE Fid)
{
	int	rtn;
	int	ii;

	if (Fid < 0 || Fid >= MAXFILEHANDLES)
		return 0;
	if (UseMappedFiles && FidIsMapped[Fid])
	{
		BOOL st = UnmapViewOfFile(FidPtr[Fid]);
		st = CloseHandle (FidHandle[Fid]);
		st = CloseHandle (OpenFileHandle[Fid]);
		FidIsMapped[Fid]=FALSE;
		OpenFileLength[Fid] = OriginalFileLength[Fid] = 0;
		rtn = 0;
	}
	if (!JournalIsCompleteFile[Fid])
	{
		if (FidMemLen[Fid])
		{
			if (OpenFileMode[Fid] != OF_READ)
			{
				LPBYTE pMem = GlobalLock (OpenFileHandle[Fid]);
				
				FidMemLen[Fid] = 0;
				GSSillseek (Fid,0,0);
				BigWrite (Fid,pMem,OpenFileLength[Fid],-1);
				GlobalUnlock (OpenFileHandle[Fid]);
			}
			GSSiGlobFree (&OpenFileHandle[Fid]);
			FidMemLen[Fid] = -1;//flags as close of mem file
			rtn =0;
		}
		else if (OpenFileFid[Fid] == HFILE_ERROR)
			rtn = 0;
		else if (OpenFileFid[Fid] != (HFILE)-2)
		{
			int i;
			rtn = _close(OpenFileFid[Fid]);
			//close any other handles on same file if close was requested
			for (i = 0; i < MAXFILEHANDLES; i++)
			{
				if (i != Fid && OpenFileFid[i] > 0 && OpenFileCloseRequested[i] && !stricmp(OpenFileName[i], OpenFileName[Fid]))
				{
					rtn = _close(OpenFileFid[i]);
					CloseJournal(i);
					*OpenFileName[i] = 0;
					LogOpenFilesClose(i);
				}
			}
		}
		else
			rtn = 0;
		if (rtn == HFILE_ERROR || TraceOn) 
		{
			HANDLE	hMem=GSSiGlobAlloc (  90,GMEM_MOVEABLE,128);
			LPSTR	str = GlobalLock (hMem);
			if (rtn) 
			{
	    		sprintf (str,"File close failed for %i\r\n%s",(short)Fid,OpenFileName[Fid]);
	    		GSSiMsgBox (GetFocus(),str,0,MB_ICONEXCLAMATION,0);
			}
			sprintf(str,"Close file: %i %i",Fid,rtn);
			GSSiTrace(str,0);
    		GSSiGlobUlFree (&hMem);
		}
	}
	else
		rtn = 0;
	CloseJournal (Fid);
	*OpenFileName[Fid] = 0;
    LogOpenFilesClose (Fid);
	return rtn;
}

short GetDriveNum (char DriveLetter)
#if ENABLETRACE
{GSSiEnterProg (302);
#endif
{  
	short drivenum;
	
   if (isupper(DriveLetter))
	   drivenum    = DriveLetter - 'A' + 1;
   else
	   drivenum    = DriveLetter - 'a' + 1;  
{
#if ENABLETRACE
GSSiExitProg (302);
#endif
	return drivenum;
}
#if ENABLETRACE
}
#endif
}

HCURSOR GSSiSetCursor (HCURSOR hCursorIn)
#if ENABLETRACE
{GSSiEnterProg (303);
#endif
{   
	HCURSOR	hCur=0; 
	int	ii;
	static	HCURSOR hCurrentCursor=0;
extern	BOOL	ddbug;
	if (ddbug)
		ddbug=FALSE;
	if (!hCursorIn)
	{
		hCursorIn = LoadCursor(0, IDC_ARROW);
		hCurrentCursor = 0;
	}
	if (hCursorIn &&  hCursorIn != hCurrentCursor)
	{
		if (!BackgroundTask)
			hCur = SetCursor(hCursorIn);
		//	else
		//		hCur = hCurrentCursor;
		hCurrentCursor = hCursorIn;
		hCursor = hCursorIn;
	}
{
#if ENABLETRACE
GSSiExitProg (303);
#endif
	return hCur;
}
#if ENABLETRACE
}
#endif
}  

/*void RectToPoints (LPRECT Rect,LPPOINT Points)
{
	Points[0].x = Rect->left;
	Points[0].y = Rect->bottom-1;
	Points[1].x = Rect->left;
	Points[1].y = Rect->top;
	Points[2].x = Rect->right-1;
	Points[2].y = Rect->top;
	Points[3].x = Rect->right-1;
	Points[3].y = Rect->bottom-1; 
	return;
}   */                             

void RectToPoints (LPRECT Rect,LPPOINT Points)
{
	Points[0].x = Rect->left;
	Points[0].y = Rect->bottom;
	Points[1].x = Rect->left;
	Points[1].y = Rect->top;
	Points[2].x = Rect->right;
	Points[2].y = Rect->top;
	Points[3].x = Rect->right;
	Points[3].y = Rect->bottom; 
	return;
}                                

void RectToDPoints (LPRECT Rect,LPDPOINT Points)
{
	Points[0].x = Rect->left;
	Points[0].y = Rect->bottom;
	Points[1].x = Rect->left;
	Points[1].y = Rect->top;
	Points[2].x = Rect->right;
	Points[2].y = Rect->top;
	Points[3].x = Rect->right;
	Points[3].y = Rect->bottom; 
	return;
}

void RectToBounds (LPRECT Rect,LPMNMXCORD pBounds)
{
	pBounds->xmn = Rect->left;
	pBounds->xmx = Rect->right;
	pBounds->ymn = Rect->top;
	pBounds->ymx = Rect->bottom;
	return;
}                                

COLORREF ColorWOWidth (COLORREF InColor)
#if ENABLETRACE
{GSSiEnterProg (304);
#endif
{
	COLORREF OutColor;
	
	OutColor = InColor << 8;
	OutColor = OutColor >> 8;
{
#if ENABLETRACE
GSSiExitProg (304);
#endif
	return OutColor;
}
#if ENABLETRACE
}
#endif
}  

COLORREF ColorWithWidth (COLORREF Color,int Width)
{
	LPBYTE pWidth = (LPBYTE)&Color;
	
	pWidth[3] = Width;
	return Color;
}

void SetFocusAndCursor (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (305);
#endif
{
	RECT	rect;
	short	x,y; 
	
	GetWindowRect(hWnd,&rect);
	x = rect.left + (rect.right - rect.left)/2;
	y = rect.bottom - (rect.bottom - rect.top) / 2;
	SetCursorPos (x,y);
{
#if ENABLETRACE
GSSiExitProg (305);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void SetReplacePath (LPSTR Name,short opt)
#if ENABLETRACE
{GSSiEnterProg (306);
#endif
{
	switch (opt)
	{
		case 1:
 			_fstrcpy (SubstitutePathnameFrom,Name);
 			break;
		case 2:
 			_fstrcpy (SubstitutePathnameTo,Name);
 			break;
		case 3:
		{
			LPSTR	pBeg, pEnd, pSpace;
			char	temp[128];
			short	i;
			
 			if (*Name != '(')
 			{
 				_fstrcpy (CachePathnameFrom[0],Name);
 				NumCachePathnameFrom = 1;
 			}
 			else if ((pEnd = _fstrchr (Name,')')))
 			{
 				*pEnd = 0;
 				_fstrcpy (temp,&Name[1]);
 				*pEnd = ')'; 
 				pBeg = temp;
 				do
 				{
					LPSTR	pLoc = pBeg;

					if (*pBeg == '"')
					{
						pBeg++;
						pLoc = strchr (pBeg,'"');
						if (!pLoc)
							pLoc = pBeg;
					}

 					if ((pSpace = MatchLev (pLoc,' ')))
 						*pSpace++ = 0;
					if (*pLoc == '"')
						*pLoc = 0;
 					_fstrcpy (CachePathnameFrom[NumCachePathnameFrom++],pBeg);
 					pBeg = pSpace;
 				}
 				while (pSpace);
 			} 
//not sure what this is for - removed 2011_07_27
			/*for (i=0;i<NumCachePathnameFrom;i++)
 				if (*LastChr(CachePathnameFrom[i]) == '\\')
 					*LastChr(CachePathnameFrom[i]) = 0;*/
 		}
 			break;
		case 4:
 			_fstrcpy (CachePathnameTo,Name);
			ExpandText (CachePathnameTo);
			//GetLongPathName2 (CachePathnameTo,MAX_PATH);
			if (*CachePathnameTo && *LastChr (CachePathnameTo) != '\\')
				strcat (CachePathnameTo,"\\");
			if (RunFromCache)
			{
				ConvertToNewLocation (0,0);
				GetGlobalCVal ("[%DL]",OriginalDL,0);
				AllowCache = FALSE;
				SetGlobalValue("%DATA_LOC",CachePathnameTo);
				SetGlobalValue ("%DL",CachePathnameTo);
			}
 			break;
 	}
{
#if ENABLETRACE
GSSiExitProg (306);
#endif
 	return;
}
#if ENABLETRACE
}
#endif
}

BOOL IsMemFile (HFILE Fid)
{
	if (Fid >= 0 && Fid < MAXFILEHANDLES && OpenFileFid[Fid] != HFILE_ERROR && FidMemLen[Fid] != 0)
		return TRUE;
	return FALSE;
}

HFILE SetMemFile(HFILE Fid,HANDLE handle,long len)
{ 
	CloseAllRequestedFiles (FALSE);
	FidMemLen[Fid] = len;
	OpenFileLength[Fid] = OriginalFileLength[Fid] = len;
	OpenFilePosition[Fid] = 0;
	OpenFileHandle[Fid] = handle;
	OpenFileFid[Fid] = Fid; 
	return Fid;
}

BOOL ConvertToMemFile (HFILE Fid,int MaxMem)
{
	BOOL	rtn=FALSE;
	LPSTR	pMem;

	if(Fid == HFILE_ERROR)
		return FALSE;
	
	switch (OpenFileMode[Fid])
	{
	case OF_READ:
		OpenFilePosition[Fid] = GSSillseek (Fid,0,1);
		MaxMem = GSSillseek (Fid,0,2);
		OpenFileHandle[Fid] = GSSiGlobAlloc (0,GMEM_MOVEABLE,MaxMem);
		if (!OpenFileHandle[Fid])
		{
			GSSillseek (Fid,OpenFilePosition[Fid],0);
			return FALSE;
		}
		pMem = GlobalLock (OpenFileHandle[Fid]);
		GSSillseek (Fid,0,0);
		BigRead (Fid,pMem,MaxMem);
		GlobalUnlock (OpenFileHandle[Fid]);
		_close (OpenFileFid[Fid]);
		FidMemLen[Fid] = MaxMem;
		break;

	case OF_CREATE:
	case OF_READWRITE:
		OpenFilePosition[Fid] = GSSillseek (Fid,0,1);
		OpenFileHandle[Fid] = GSSiGlobAlloc (0,GMEM_MOVEABLE,MaxMem);
		if (!OpenFileHandle[Fid])
		{
			GSSillseek (Fid,OpenFilePosition[Fid],0);
			return FALSE;
		}
		pMem = GlobalLock (OpenFileHandle[Fid]);
		OpenFileLength[Fid] = GSSillseek (Fid,0,2);
		GSSillseek (Fid,0,0);
		BigRead (Fid,pMem,OpenFileLength[Fid]);
		GlobalUnlock (OpenFileHandle[Fid]);
		FidMemLen[Fid] = MaxMem;
		break;
	}
	return TRUE;
}

HFILE GSSiOpenFileMem (LPSTR InName,UINT mode,UINT maxlen)
{
	HFILE	Fid = GSSiOpenFile (InName,0,mode); 
	long	len;
	HANDLE	handle; 
	HFILE	rtn;
	
	if (Fid == HFILE_ERROR)
		return Fid;
	if (mode == OF_CREATE)
		len = maxlen;
	else
		len = GSSillseek (Fid,0,2); 
	GSSillseek (Fid,0,0);
	handle = GSSiGlobAlloc (1,GMEM_MOVEABLE,len);
	if (mode == OF_CREATE)
	{
		if (!handle)
		{
			GSSiClose (Fid);
			Fid = HFILE_ERROR;
		}
	}
	else if (handle)
	{   
		HPSTR	ptr=GlobalLock (handle);
		short	i;
		
		BigRead (Fid,ptr,len);
		GlobalUnlock (handle);
		GSSiClose (Fid); 
		Fid = SetMemFile (Fid,handle,len);
		if (Fid == HFILE_ERROR)
			GSSiGlobFree (&handle);
	}
	return Fid;
} 

long BigRead (HFILE Fid,LPVOID pBuf,long isize)
#if ENABLETRACE
{GSSiEnterProg (390);
#endif
{   
	static	long ncalls=0;
	long	rtn=0; 
	
	ncalls++;
	if (!isize || Fid == HFILE_ERROR)
		goto Exit;
	if (FidMemLen[Fid])
	{
		rtn = GSSilread (Fid,pBuf,isize);
		goto Exit;
	}
	LastAccessedFid = Fid;
	if ((UseMappedFiles && Fid < MAXFILEHANDLES) && FidIsMapped[Fid])
	{
		rtn = GSSilread (Fid,pBuf,isize);
		goto Exit;
	}
	if (JournalFileFid[Fid] != HFILE_ERROR)
		rtn = ReadWithJournal (Fid,pBuf,isize);
	else
	{
		if (OpenFileFid[Fid] < 0)
			MessageBox(0, "Attemp to read invalid file id", 0, MB_ICONEXCLAMATION);
		else
			rtn = _read(OpenFileFid[Fid], pBuf, isize);
	}
	NumFidRead += isize;
Exit:
{
#if ENABLETRACE
GSSiExitProg (390);
#endif
    return (rtn);
}

#if ENABLETRACE
}
#endif
}

int GSSifstat (int Fid, struct _stati64 * pstat)
{
	int rtn = _fstati64 (OpenFileFid[Fid],pstat);

	return rtn;
}

BOOL GSSifileinfo (int Fid, LPBY_HANDLE_FILE_INFORMATION pstat)
{
	BOOL rtn = GetFileInformationByHandle ((HANDLE)OpenFileFid[Fid],pstat);

	return rtn;
}

HFILE OpenTempNamedFile (void)
{   
	char	File[256];
	HFILE	Fid;
	short	l;
	
	GSSiGetTempFileName (0,"gmd",0,File); 
	Fid = GSSiOpenFile (File,0,OF_CREATE);
	l = _fstrlen (File);
	BigWrite (Fid,(HPSTR)&l,2,-1);
	BigWrite (Fid,(HPSTR)File,l,-1);	
	return Fid;
}   

BOOL CloseAndDeleteFile (LPHFILE pFid)
{   
	char	Name[256]; 
	short	l;
	
	if (*pFid == (HFILE)HFILE_ERROR)
		return FALSE;
	GSSillseek (*pFid,0,0);
	BigRead (*pFid,(HPSTR)&l,2);
	if (l && l<256)
	{
		BigRead (*pFid,Name,l);
		Name[l]=0;
	}
	else
		*Name = 0;
	GSSiClose (*pFid);
	*pFid = HFILE_ERROR;
	GSSiRemove (Name);
	return TRUE;
}
	
short CacheAlreadyChecked (LPSTR Name,int lCacheDir,UINT Mode)
{   
	UINT	l;
	LPSTR	pAlreadyCached,lb; 
	short	rtn=TRUE;
	BOOL	Remove=FALSE; 
	long	lenCACBuf = USHRT_MAX*16L;
	
	if (!UseCacheAlreadyChecked)
		return FALSE; 
	if (!Name)
	{   
		GSSiGlobFree (&hCacheAlreadyChecked);
		GetLongPathFromBuffer (0,0);
		return FALSE;
	}
	if (lCacheDir < 0)
	{
		Remove = TRUE;
		lCacheDir = abs (lCacheDir);
	}
	if (!hCacheAlreadyChecked)
	{
		HFILE	Fid;
		OFSTRUCTGM	OFStruct;
		char	BGCacheList[MAX_PATH];

/*		strcpy (BGCacheList,"[%DL]BACKGROUNDCACHEFILELIST.TXT");
		ExpandText (BGCacheList);
		Fid = OpenFile (BGCacheList,&OFStruct,OF_READ);
		if (Fid != HFILE_ERROR)
		{
			char	bgfile[MAX_PATH+2];

			lenCACBuf = max (lenCACBuf, _llseek (Fid,0,2) * 2);
			_llseek (Fid,0,0);
			hCacheAlreadyChecked = GSSiGlobAlloc (  91,GHND,lenCACBuf);
			pAlreadyCached = GlobalLock (hCacheAlreadyChecked);
			while (fgetstring2 (bgfile,MAX_PATH,Fid))
			{
				strcpy (pAlreadyCached,&bgfile[5]);
				pAlreadyCached = _fstrchr (pAlreadyCached,0);
				pAlreadyCached++;
				*pAlreadyCached++ = CHECKTIMESTAMP; 
			//	*pAlreadyCached++ = 0;
			}
			_lclose (Fid);
			GlobalUnlock (hCacheAlreadyChecked);
		}
		else*/
			hCacheAlreadyChecked = GSSiGlobAlloc (  91,GHND,lenCACBuf);
	}
	lb = pAlreadyCached = GlobalLock (hCacheAlreadyChecked);
	while (*pAlreadyCached)
	{
		if (!_fstricmp (&Name[lCacheDir],pAlreadyCached))
		{
			if (Remove)
			{   
				size_t	movelen;
				LPSTR NextName = _fstrchr (pAlreadyCached,0);
				
				NextName++; 
				movelen = (size_t)((long)lb + lenCACBuf - (long)NextName);
				_fmemmove (pAlreadyCached,NextName,movelen);
			}
			if (Mode == CHECKFOREXIST)
			{
				pAlreadyCached = _fstrchr (pAlreadyCached,0);
				rtn = *(pAlreadyCached+1);
			}
			else if (Mode == CHECKTIMESTAMP)
			{
				pAlreadyCached = _fstrchr (pAlreadyCached,0);
				rtn = *(pAlreadyCached+1);
				if (rtn == DOESEXIST)   
				{
					*(pAlreadyCached+1) = CHECKTIMESTAMP;
					rtn = FALSE;
				}	
			}
			goto Exit;
		}
		pAlreadyCached = _fstrchr (pAlreadyCached,0);
		pAlreadyCached+=2; 
	}
	if (((long)pAlreadyCached - (long)lb) > lenCACBuf - 256) 
	{
		GSSiGlobUlFree (&hCacheAlreadyChecked);
		return FALSE;
	}
	else if (Mode != CHECKFOREXIST)
	{
		_fstrcpy (pAlreadyCached,&Name[lCacheDir]);
		pAlreadyCached = _fstrchr (pAlreadyCached,0);
		pAlreadyCached++;
		*pAlreadyCached++ = Mode; 
		*pAlreadyCached = 0;
	} 
	rtn = FALSE; 
Exit:
	GlobalUnlock (hCacheAlreadyChecked);
	return rtn;
} 

BOOL FileErrMess (HFILE Fid,LPSTR Name,LPOFSTRUCTGM pOFStruct,UINT Mode)
{   
	LPSTR	ErrMess, str;
	HANDLE	hStr;
	char	type[8];
	
	if (Fid != HFILE_ERROR)
		return FALSE;
	hStr = GSSiGlobAlloc (  92,GMEM_MOVEABLE,1024);
	ErrMess = GlobalLock (hStr);
	str = ErrMess + 512;
	
	if (Mode == OF_READ)
		_fstrcpy (type,"open");
	else
		_fstrcpy (type,"create");
	if (pOFStruct->nErrCode)
	{	  
		if (!LoadString(hInst, GetOPENERR00()+pOFStruct->nErrCode, ErrMess, 32))
			*ErrMess = 0;
		sprintf (str,"Cannot %s %s: error code is %i (%s)",type,Name,(short)pOFStruct->nErrCode,ErrMess); 
	}
	else
		sprintf (str,"Cannot %s %s",type,Name); 
	GSSiMsgBox( GetFocus(), str,"", MB_ICONEXCLAMATION,0);    
	GSSiGlobUlFree (&hStr);
	return TRUE;
} 

BOOL ValidPathName (LPSTR InName)
{
	HANDLE	hStr=GSSiGlobAlloc (  93,GMEM_MOVEABLE,1024); 
	LPSTR	pSlash, pDot;
	LPSTR	Err=GlobalLock (hStr);
	LPSTR	str=Err+256;
	LPSTR	Name=str+256;
	
	_fstrcpy (Name,InName);
	pDot=_fstrrchr (Name,'.');	
	if (pDot)
	{
		if (!_fstrchr (pDot,'\\'))
		{
			*pDot++ = 0; 
			if (*pDot)
			{
				if (_fstrlen (pDot) > 3)
				{
					_fstrcpy (Err,"File extension too long");
					goto ShowErr;
				}
			}
			else 
			{
				_fstrcpy (Err,"File extension missing");
				goto ShowErr;
			}
		}
	}
	while ((pSlash = _fstrrchr (Name,'\\')))
	{
		*pSlash++ = 0;
		if (*pSlash)
		{   
			pDot=_fstrrchr (pSlash,'.');	
			if (pDot)
				*pDot++ = 0; 
			if (_fstrlen (pSlash) > 8)
			{
				_fstrcpy (Err,"File or directory name too long");
				goto ShowErr; 
			}
		}
		else if (*Name) 
		{
			_fstrcpy (Err,"File or directory name missing");
			goto ShowErr;
		}
	} 
	GSSiGlobUlFree (&hStr);
	return TRUE;
ShowErr: 
	sprintf (str,"Error in pathname %s: %s",InName,Err);
	GSSiMsgBox (GetFocus(),str,0,MB_ICONEXCLAMATION,0);
	GSSiGlobUlFree (&hStr);
	return FALSE;
}

BOOL ConvertFileNameToCacheFileName (LPSTR FileName)
{
	if (RunFromCache)
	{
		int	icpf, l;
		char	CacheFromName[MAX_PATH];
		char	str[MAX_PATH];
		
		strcpy (str,FileName);

    	for (icpf = 0;icpf < NumCachePathnameFrom; icpf++)
    	{   
    		_fstrcpy (CacheFromName,CachePathnameFrom[icpf]); 
    		ExpandText (CacheFromName);
	    	l = _fstrlen(CacheFromName);
	    	if (!l)
	    		continue;
	    	if (CacheFromName[l-1] != '\\')
	    	{
	    		CacheFromName[l++] = '\\';
	    		CacheFromName[l] = 0;
	    	}
	    	if (!_fstrnicmp (str,CacheFromName,l))
	    	{ 
				char	altDir[MAX_PATH]="";

				if (icpf)
				{
					strcpy (altDir,CacheFromName);
					REPLAC (altDir,":\\","_",MAX_PATH);
				}
				sprintf (FileName,"%s%s%s",CachePathnameTo,altDir,&str[l]);
				return TRUE;
			}
		}
	}
	else if (AllowCache)
	{
		OFSTRUCTGM	OFStruct;

		HFILE Fid = GSSiOpenFile (FileName,&OFStruct,OF_READ);
		
		if (Fid == HFILE_ERROR)
			return FALSE;
		GSSiClose (Fid);
		strcpy (FileName,OFStruct.szPathName);
		return TRUE;
	}

	return FALSE;
}

void ShowOpenFiles (LPSTR Name,UINT Mode)
{   
	UINT	Modes[6]={OF_READ,OF_WRITE,OF_READWRITE,OF_DELETE,OF_CREATE,OF_EXIST};
	char	ModesC[6][10]={"READ","WRITE","READWRITE","DELETE","CREATE","EXIST"}; 
	char	str[256]; 
	char	ModeC[10]="OTHER"; 
	short	i;  
	static	BOOL DisplayInPopup=1;
	
	for (i=0;i<6;i++)
		if (Mode == Modes[i])
			_fstrcpy (ModeC,ModesC[i]);
	sprintf (str,"%s:%s",Name,ModeC);
	if (DisplayInPopup)
	{
		if (GSSiMsgBox (0,str,0,MB_OKCANCEL|MB_APPLMODAL,0) == IDCANCEL)
			DisplayInPopup = 0;    
	}
	else
    	SetWindowText (hWndMain,Name);
	return;
}

int ConvertToNewLocation (LPSTR Path,BOOL DoCopy)
{
	static	char	NewPath[MAX_PATH] = { 0 }, DLPath[MAX_PATH];
	static	int		lDL = 0;
	static	BOOL	Recursive=FALSE;
	int		st, l, ii;
	static	BOOL	First=TRUE;
	static	HANDLE	hConvert=0;
	HFILE	Fid;
	char	str[520], FullPath[MAX_PATH];
	LPSTR	pConvert, pBS;

	if (!Path)
	{
		//MessageBox(0, "Freed", 0, MB_OK);
		GSSiGlobFree (&hConvert);
		First = TRUE;
		return TRUE;
	}
	if (Recursive)
		return FALSE;
	if (*Path == '(')
	{
		LPSTR pEnd = strchr (Path,')');

		if (pEnd)
		{
			char	Drive[8];
			int		ln;
			LPSTR	DriveName=Path+1;

			*pEnd++ = 0;
			ln=strlen (DriveName);
			if (ln && GetDriveLetterFromLocalDriveName (DriveName,Drive))
			{
				int	ln2 = strlen (pEnd);

				strcpy (Path,Drive);
				memmove (strchr (Path,0),pEnd,ln2+1);
			}
		}
	}
	if (First && HaveDL)
	{
		int TotLen=0;
		OFSTRUCTGM	OFStruct;
		char	AutoMoveList[MAX_PATH];
		BOOL	NeedCreateDir=FALSE;

		First = FALSE;
		Recursive = TRUE;
		strcpy (AutoMoveList,AutoMoveFile);
		ExpandText (AutoMoveList);
		//MessageBox(0, AutoMoveList, 0, MB_OK);
		Fid = GSSiOpenFile (AutoMoveList,0,OF_READ);
		if (Fid != HFILE_ERROR)
		{
			//MessageBox(0, "Opened", 0, MB_OK);
			hConvert = GSSiGlobAlloc(1547, GMEM_MOVEABLE, USHRT_MAX);
			pConvert = GlobalLock (hConvert);
			while (fgetstring (str,256,Fid))
			{
				if (!strchr (str,'\t'))
					NeedCreateDir = TRUE;
				ExpandText (str);
				l = strlen (str);
				strcpy (pConvert,str);
				pConvert += l + 1;
				TotLen += l + 1;
			}
			*pConvert = 0;
			TotLen++;
			GSSiClose (Fid);
			GlobalUnlock (hConvert);
			hConvert = GSSiGlobalReAlloc (0,hConvert,TotLen,GMEM_MOVEABLE);
			st = SHGetFolderPath(0, CSIDL_COMMON_APPDATA, 0, SHGFP_TYPE_CURRENT, NewPath);
			/*{
				char mes[512];
				sprintf(mes, "totlen=%i %ld %s", TotLen, (int)hConvert,NewPath);
				MessageBox(0, mes, 0, MB_OK);
			}*/
			strcpy (DLPath,"[%DL]");
			ExpandText (DLPath);
			*LastChr(DLPath) = 0;
			lDL = strlen (DLPath);
			strcpy (str,DLPath);
			pBS = strrchr (str,'\\');
			if (pBS && NeedCreateDir)
			{
				strcat (NewPath,pBS);
				makedirectories (NewPath,TRUE,FALSE);
			}
		}
		Recursive = FALSE;
	}
	if (DebugExistFile)
	{
		_fullpath (FullPath,Path,MAX_PATH);
		strlwr (FullPath);
		if (*FullPath != 's' && *FullPath != 'c')
			MessageBox (0,FullPath,0,MB_OK);
	}
	if (!hConvert)
	{
		Recursive = FALSE;
		return FALSE;
	}
	_fullpath (FullPath,Path,MAX_PATH);
	/*if (dbug)
	{
		MessageBox(0, FullPath,"FullPath", MB_OK);
	}*/
	pConvert = GlobalLock (hConvert);
	while (*pConvert)
	{
		LPSTR	pTab;

		if ((pTab = strchr (pConvert,'\t')))
			*pTab = 0;
		l = strlen (pConvert);
		/*if (dbug)
		{
			MessageBox(0, pConvert, "pConvert", MB_OK);
		}*/

		if (!strnicmp (FullPath,pConvert,l))
		{
			char	MoveToPath[MAX_PATH];

			if (pTab)
			{
				DoCopy = FALSE;
				strcpy (MoveToPath,pTab+1);
				strcat (MoveToPath,&FullPath[l]);
				*pTab = '\t';
			}
			else
			{
				strcpy (MoveToPath,NewPath);
				strcat (MoveToPath,&FullPath[lDL]);
			}
			Recursive = TRUE;
			if (FileType (MoveToPath))
			{
				strcpy (Path,MoveToPath);
				goto Exit;
			}
			if (DoCopy && FileType (FullPath)==1)
			{
				if (!GSSiCopyFile (FullPath,MoveToPath,FALSE))
				{
					MessageBox (0,Path,"Unable to move file",MB_ICONEXCLAMATION|MB_APPLMODAL);
					goto Exit;
				}
			}
			strcpy (Path,MoveToPath);
			goto Exit;
		}
		if (pTab)
			*pTab = '\t';
		pConvert = strchr (pConvert,0);
		pConvert++;
	}
Exit:
	Recursive = FALSE;
	GlobalUnlock (hConvert);
	return TRUE;
}

BOOL PctBox(HWND hWnd, LONGLONG MaxLen, LONGLONG Done, short InFreq)
#if ENABLETRACE
{GSSiEnterProg (387);
#endif
{short  x,y;
 HBRUSH  hRedBrush;
 double         PCTDone,PercentDone; 
 static			double	debugpct=32.55;  
 static			short	LastIPCT=-1;
 char           Text[32];
 static	char	LastText[32]="";
 static	HWND	LasthWnd=0;
 MSG            msg; 
 HDC            hDC; 
 RECT           Rect, Rect2, Rect3;
 short          Twidth, Theight, i=0, Freq,ii; 
 BOOL           ShowNums=FALSE, ShowText=TRUE, Clear=FALSE;
#if WIN32
 SIZE Size;
 LPSIZE lpSize = &Size;
#endif  

 if (Done > MaxLen)
 	ii=1;
 if (InFreq == SHRT_MAX)
 {
 	InFreq = 1;
 	ShowText = FALSE;
 }
 if (InFreq == SHRT_MIN)
 {
 	InFreq = 1; 
 	Done = 0;
 	MaxLen = 1;
 	ShowText = FALSE;
 	Clear = TRUE;
 }
 Freq = abs(InFreq);
 if (Freq && Done < MaxLen)
 {  
    if (Done%Freq)
{
#if ENABLETRACE
GSSiExitProg (387);
#endif
    	return(2);
}
 }
while (DoPCTPeek != (HWND)1 && GSSiPeekMessage(&msg,DoPCTPeek,0,0,PM_REMOVE))
{
#if ENABLETRACE
	SetLastMessage(-1 * (long)msg.message, msg.wParam);
#endif
/*  if (msg.message == WM_QUIT)
  	retrn FALSE;
  if(msg.message == WM_COMMAND && msg.wParam == IDCANCEL)
  {
	Canceled = TRUE;
	retrn FALSE;
  }*/
//  if (msg.message != WM_NCHITTEST)
  {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
  }
}
 if (!hWnd)
{
#if ENABLETRACE
GSSiExitProg (387);
#endif
 	return FALSE;
}
 if (InFreq < 0)
 {
//    MaxLen = -MaxLen;
    ShowNums=TRUE;
 } 
 if (!MaxLen)
{
#if ENABLETRACE
GSSiExitProg (387);
#endif
 	return FALSE;
}
 PercentDone = ((double) max(Done,0))/MaxLen;
 PCTDone = PercentDone*100.0; 
 if (PCTDone >= debugpct)
 	ii=1; 
 if (Done == 7330368)
	 ii=1;
 if (ShowText)
 {
	 if (ShowNums)
	     sprintf(Text, "%.1f%% (%llu of %llu)",PCTDone,Done,MaxLen);
	 else
	     sprintf(Text, "%.2f%%",PCTDone);
	 if (hWnd == LasthWnd && !_fstrcmp (Text,LastText))
{
#if ENABLETRACE
GSSiExitProg (387);
#endif
	 	return TRUE;
}
 }
 LasthWnd = hWnd;
 _fstrcpy (LastText,Text); 
 hDC = GetDC (hWnd);
 if (!hDC)
{
#if ENABLETRACE
GSSiExitProg (387);
#endif
 	return FALSE;
}

 SaveDC (hDC);
 SetMapMode    ( hDC, MM_TEXT );
 SetWindowOrgEx  ( hDC, 0,0, 0 );
 SetViewportOrgEx( hDC, 0, 0, 0 );
 SelectClipRgn (hDC,0);
 GetClientRect (hWnd,&Rect); 
 Rect2 = Rect;  
 Rect3 = Rect;
 //InflateRect(&Rect3,1,1);
 hRedBrush = CreateSolidBrush (RGB(255,0,0));
 if (Clear)  
 	FrameRect (hDC,&Rect3,GetStockObject(LTGRAY_BRUSH));
 else if (InFreq >=0)  
 	FrameRect (hDC,&Rect3,hRedBrush);
 
#if WIN32
 GetTextExtentPoint32 (hDC,Text,strlen(Text),lpSize);
 Twidth = (short)lpSize->cx;
 Theight = (short)lpSize->cy;
#else
 TextExt = GetTextExtent (hDC,Text,_fstrlen(Text));
 Twidth = LOWORD(TextExt);
 Theight = HIWORD(TextExt);
#endif
 
 x=(Rect.left+Rect.right-Twidth)/2;
 y=(Rect.top+Rect.bottom-Theight)/2;
 Rect.right = IDNINT(Rect.right * PercentDone);  
 Rect2.left = Rect.right;
 FillRect (hDC,&Rect2,GetStockObject(WHITE_BRUSH)); 
 if (!Clear)
 	FillRect (hDC,&Rect,hRedBrush); 
 if (ShowText && Done >= 0) 
 	TextOut (hDC,x,y,Text,_fstrlen(Text));
 DeleteObject(hRedBrush);
 RestoreDC (hDC,-1);
 ReleaseDC(hWnd,hDC); 
 if (GetGlobalBVal2("[%IGNORESYSMSG]",FALSE))
 {
 	short iPCT = (short) PCTDone / 10; 
 	
 	if (iPCT != LastIPCT)
 	{   
 		char	Step[64];
 		
 		GetGlobalCVal ("[%STEP]",Step,0);
	    sprintf(Text, "%s (%.1f percent complete)",Step,PCTDone);
// 	    UpdateSysMsgFile (Text);      
 		LastIPCT = iPCT;
 	}
 }
{
#if ENABLETRACE
GSSiExitProg (387);
#endif
 return (TRUE);
}

#if ENABLETRACE
}
#endif
}

BOOL CALLBACK CACHEFILEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
 switch(Message)
   {
    case WM_INITDIALOG:
        cwCenter(hWndDlg, 0);
		SaveFullWindowBitmap (hWndMain);
		SetDlgItemText (hWndDlg,IDC_TITLE,CacheTitle);
        break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
		 DestroyWindow (hWndDlg);
         break; /* End of WM_CLOSE                                      */
	case WM_DESTROY:
		RestoreFullWindowBitmap ();
		break;
    default:
        return FALSE;
   }
 return TRUE;
}

BOOL GSSiCopyFile2 (LPSTR OldNameIN,LPSTR NewNameIN,BOOL Replace)
{
	char	Title[]="Caching files - please wait";
	SHFILEOPSTRUCT SHFO;
	LPSTR	pEnd;
	char	OldName[MAX_PATH+2];
	char	NewName[MAX_PATH+2];

	strcpy (OldName,OldNameIN);
	strcpy (NewName,NewNameIN);

	pEnd=strchr(OldName,0)+1;
	*pEnd = 0;	
	pEnd=strchr(NewName,0)+1;
	*pEnd = 0;	

	memset (&SHFO,0,sizeof(SHFO));
	SHFO.wFunc = FO_COPY;
	SHFO.pFrom = OldName;
	SHFO.pTo   = NewName;
	SHFO.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_SIMPLEPROGRESS ;
	SHFO.lpszProgressTitle = Title;

	if (!SHFileOperation (&SHFO))
		return TRUE;
	if (SHFO.fAnyOperationsAborted)
		GSSiRemove (NewNameIN);
	return FALSE;
}

DWORD CALLBACK CopyToCacheProgressRoutine(
  LARGE_INTEGER TotalFileSize,  // total file size, in bytes
  LARGE_INTEGER TotalBytesTransferred,
                            // total number of bytes transferred
  LARGE_INTEGER StreamSize,  // total number of bytes for this stream
  LARGE_INTEGER StreamBytesTransferred,
                            // total number of bytes transferred for 
                            // this stream
  DWORD dwStreamNumber,     // the current stream
  DWORD dwCallbackReason,   // reason for callback
  HANDLE hSourceFile,       // handle to the source file
  HANDLE hDestinationFile,  // handle to the destination file
  LPVOID lpData             // passed by CopyFileEx
)
{
	long	TotLen = (long)TotalFileSize.LowPart;
	long	CurLoc = (long)TotalBytesTransferred.LowPart;

   	if (hWndCache)
	   PctBox (GetDlgItem(hWndCache,IDC_CACHEPROGRESS),TotLen,CurLoc,0);

	return PROGRESS_CONTINUE;
}

BOOL CopyFileExtended (LPSTR ToFile,LPSTR FromFile)
{
	long	TotLen = 300000000;
	long	CurLoc = 0;
	BOOL	rtn;
	DWORD	ier;
	BOOL	CancelCacheCopy=FALSE;
	
	if (TotLen > DisplayCacheProgressMinFileSize)
	{
		strcpy (CacheTitle,"Caching file ... please wait");
		hWndCache = CreateDialog(hInst, "CACHEFILE", hWndMain, (DLGPROC)CACHEFILEMsgProc); 
		DoPCTPeek = hWndCache;
		SetDlgItemText (hWndCache,IDC_FILEBEINGCACHED,FromFile);
		//rtn = copyfile (ToFile, FromFile,FALSE,0,0,hWndCache,IDC_CACHEPROGRESS,TotLen,&CurLoc);
		if (!CopyFileEx (FromFile,ToFile, CopyToCacheProgressRoutine,0,&CancelCacheCopy,0))
			ier = GetLastError ();
		//GSSiCopyFile (FromFile,ToFile,TRUE);
		DoPCTPeek = 0;
		DestroyWindow (hWndCache);
	}
	else
		//rtn = copyfile (ToFile, FromFile,FALSE,0,0,0,0,0,0);
		rtn = GSSiCopyFile (FromFile,ToFile,TRUE);

	return rtn;
}

BOOL CopyFileToCache (LPSTR ToFileIN, LPSTR FromFileIN)
{
	long	TotLen = GSSiLength (FromFileIN);
	long	CurLoc = 0;
	BOOL	rtn;
	BOOL	CancelCacheCopy=FALSE;
	char	FromFile[MAX_PATH*4], ToFile[MAX_PATH];

	strcpy(FromFile, FromFileIN);
	strcpy(ToFile, ToFileIN);

	ExpandText(FromFile);
	ExpandText(ToFile);
	if (TotLen > DisplayCacheProgressMinFileSize)
	{
		strcpy (CacheTitle,"Caching file ... please wait");
		hWndCache = CreateDialog(hInst, "CACHEFILE", hWndMain,(DLGPROC) CACHEFILEMsgProc); 
		DoPCTPeek = hWndCache;
		SetDlgItemText (hWndCache,IDC_FILEBEINGCACHED,FromFile);
		//rtn = copyfile (ToFile, FromFile,FALSE,0,0,hWndCache,IDC_CACHEPROGRESS,TotLen,&CurLoc);
		rtn = CopyFileEx (FromFile,ToFile, CopyToCacheProgressRoutine,0,&CancelCacheCopy,0);
		//GSSiCopyFile (FromFile,ToFile,TRUE);
		DoPCTPeek = 0;
		DestroyWindow (hWndCache);
	}
	else
	{
		//rtn = copyfile (ToFile, FromFile,FALSE,0,0,0,0,0,0);
		//rtn = GSSiCopyFile (FromFile,ToFile,TRUE);
		rtn = CopyFileEx(FromFile, ToFile, 0, 0, &CancelCacheCopy, 0);
	}
	if (TraceOn)
	{
		HANDLE hMem = GSSiGlobAlloc(0, GMEM_MOVEABLE, 1024);
		LPSTR pMem = (LPSTR)GlobalLock(hMem);
		int st = rtn;
		if (!st)
			st = GetLastError();
		sprintf(pMem, "Copied to cache with status %i:%s %s", st, FromFile, ToFile);
		GSSiTrace(pMem, 0);
		GSSiGlobUlFree(&hMem);
	}
	SetFileAttributes(ToFile, FILE_ATTRIBUTE_NORMAL);

	return rtn;
}

BOOL EditLastTextFile (void)
{
	if (*LastTextFile)
	{
		char file[MAX_PATH];
		if (*LastTextFile == '\'')
			strcpy(file, LastTextFile);
		else
			sprintf(file, "'%s'", LastTextFile);
		GMEdit(hWndMain, file);
	}
	return TRUE;
}
BOOL GetProdNameFromTestName(LPSTR testDir, LPSTR toName, LPSTR fileName)
{
	int ln = strlen(testDir);

	if (strnicmp(fileName, testDir, ln))
		return FALSE;
	sprintf(toName, "[%%DL]%s", &fileName[ln+1]);
	ExpandText(toName);
	return TRUE;
}

int StoreTestToProduction(LPSTR testDirIN, int option)
{
	//returns 0 if successful, 1 if unable to lock file, 2 if unable to rename file, 3 if unable to copy file, 4 failed to restore after unsuccessful store
	char tempName[MAX_PATH];
	char fileName[MAX_PATH + 2];
	char toName[MAX_PATH];
	char deleteName[MAX_PATH];
	HFILE FidTemp;
	int TotFiles = 0;
	HANDLE *fid;
	HANDLE hFid;
	int iFile = 0, lastGoodFile = -1;
	int rtn = 0;
	char testDir[MAX_PATH];

	sprintf(testDir, "[%%DL]TESTENVIRONMENTS\\%s", testDirIN);
	ExpandText(testDir);
	//option 1 tests to see if all files to be replaced can be opened with exclusive write. 2 actually does the store
	EscapeFunction(TRUE);
	AllowCache = FALSE;

	GSSiGetTempFileName(0, "gm", 0, tempName);
	FidTemp = GSSiOpenFile(tempName, 0, OF_CREATE);
	SearchFilesInDir(testDir, "", FidTemp, &TotFiles,"*.*", 1, TRUE, TRUE);
	if (!TotFiles)
		return FALSE;
	GSSillseek(FidTemp, 0, 0);
	hFid = GSSiGlobAlloc(1790, GHND, TotFiles * sizeof(HFILE));
	fid = GlobalLock(hFid);
	while (fgetstring(fileName, MAX_PATH, FidTemp))
	{
		GetProdNameFromTestName(testDir,toName, fileName);
		if (ExistFile(toName))
		{
			fid[iFile] = CreateFile(toName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (fid[iFile] == INVALID_HANDLE_VALUE)
			{
				rtn = 1;
				iFile--;
				while (iFile >= 0)
				{
					if (fid[iFile])
						CloseHandle(fid[iFile]);
					iFile--;
				}
				goto Exit;
			}
		}
		iFile++;
	}
	GSSillseek(FidTemp, 0, 0);
	iFile = 0;
	while (fgetstring(fileName, MAX_PATH, FidTemp))
	{
		GetProdNameFromTestName(testDir, toName, fileName);
		if (fid[iFile])
		{
			CloseHandle(fid[iFile]);
			if (!rtn)
			{
				sprintf(deleteName, "%s.dlt", toName);
				if (ExistFile(deleteName))
					GSSiRemove(deleteName);
				if (!GSSirenamefile(toName, deleteName))
				{
					rtn = 2;
				}
			}
		}
		if (!rtn)
		{
			lastGoodFile = iFile;
			if (!GSSiCopyFile(fileName, toName, FALSE))
			{
				rtn = 3;
			}
		}
		iFile++;
	}
	if (rtn)
	{
		//use lastGoodFile and rtn to undo changes
		GSSillseek(FidTemp, 0, 0);
		iFile = 0;
		while (fgetstring(fileName, MAX_PATH, FidTemp))
		{
			if (iFile <= lastGoodFile)
			{
				GetProdNameFromTestName(testDir, toName, fileName);
				sprintf(deleteName, "%s.dlt", toName);
				if (ExistFile(deleteName))
				{
					GSSiRemove(toName);
					if (!GSSirenamefile(deleteName, toName))
					{
						rtn = 4;
					}
				}
			}
		}
	}
Exit:
	GSSiGlobUlFree(&hFid);
	GSSiClose(FidTemp);
	GSSiRemove(tempName);

	return rtn;
}

void ConvertToTestName (LPSTR Name,UINT Mode)
{
	static int ldlLoc=0;
	static char dlLoc[MAX_PATH]="[%DL]";
	static BOOL inCToTN = FALSE;
	char fullName[MAX_PATH];

	if (inCToTN || !*TestFileLocation)
		return;
	if (HaveDL && !ldlLoc)
	{
		ExpandText (dlLoc);
		ldlLoc = strlen (dlLoc);
		ExpandText (TestFileLocation);
	}
	_fullpath (fullName,Name,MAX_PATH);   
	if (ldlLoc && !strnicmp (fullName,dlLoc,ldlLoc))
	{
		char newName[MAX_PATH];

		strcpy (newName,TestFileLocation);
		strcat (newName,&fullName[ldlLoc]);
		inCToTN = TRUE;
		switch (Mode)
		{
			case OF_READ:
			case OF_READWRITE:
			case OF_EXIST:
				if (ExistFile (newName))
					strcpy (Name,newName);
				break;
			case OF_CREATE:
				strcpy (Name,newName);
				break;
			default:
				break;
		}
		inCToTN = FALSE;
	}
	return;
}
HFILE GSSiOpenFile (LPSTR InName,LPOFSTRUCTGM pOFStruct,UINT Mode)
#if ENABLETRACE
{GSSiEnterProg (307);
#endif
{   HFILE   Fid=HFILE_ERROR; 
 	HANDLE	hSTR=GSSiGlobAlloc (  94,GHND,14*512);
    LPSTR	str=GlobalLock (hSTR);
    LPSTR	SaveName=str+512;
    LPSTR	SaveText=SaveName+512;
    LPSTR	Name=SaveText+512; 
    LPSTR	errmes=Name+512;
    LPSTR	WT=errmes+512;
    LPSTR	CacheFromName = WT+512;
    LPSTR	SaveWT=CacheFromName+512;    
    LPSTR	LongName=SaveWT+512;
    LPSTR	NoCacheVal=LongName+512; 
	LPOFSTRUCTGM	pOFStruct2=(LPOFSTRUCTGM)(NoCacheVal+512);
    
    unsigned    frequency=1000, duration=100; 
    short       NumWait, NumBusyWait,l,ii; 
    clock_t starttime, nms; 
    BOOL	UsingSubName=FALSE; 
	BOOL	UsingCacheName=FALSE;   
	BOOL	NoDelete = FALSE; 
	BOOL	NameExists=FALSE;
 	BOOL	SaveContinueProcessing = ContinueProcessing;
	static	BOOL	First=TRUE;
	int		nAccessErrors=0;
	int		Err;
	
	SetContinueProcessing ( TRUE);
	if (Mode == (OF_CREATE | OF_READWRITE))
		Mode = OF_CREATE;
	if (Mode == OF_EXIST)
		ii=1;
	if (First && AllowCache)
	{
		First = FALSE;
		ExpandText (CachePathnameTo);
	}
    
    if (!pOFStruct)
    	pOFStruct = (LPOFSTRUCTGM) (pOFStruct2 + 1);
    if (DoTime)
	    starttime=GetTickCount(); 
	if (Mode == OF_CREATE_NODELETE)
	{
		NoDelete = TRUE;
		Mode = OF_CREATE;
	}
    NumFilesOpened++;
	if (*InName == '"' && *LastChr (InName) == '"')
	{
		strcpy (Name,&InName[1]);
		*LastChr (Name) = 0;
	}
	else
		strcpy (Name,InName);  
    pOFStruct->nErrCode = 0;                                  
    ExpandText (Name); 
//if in test mode and a file of the same name exists in the test directory use it instead
	ConvertToTestName (Name,Mode);
	if (!InOpenFile && (strstr(Name,".txt") || strstr(Name,".TXT")) && !strstr (Name,"System Message")&& !strstr (Name,"SYSTEM MESSAGE"))
		strcpy (LastTextFile,Name);
    Truncate (Name);
    if (!*Name)
    	goto Exit; 
	GSSiTrace(Name, 0);

	ConvertToNewLocation (Name,TRUE);
	if (fidLogFileUse != INVALID_HANDLE_VALUE)
	{
		char entry[512];
		DWORD dwBytesWritten;

		sprintf(entry, "%s\t%i\r\n", Name, (int)Mode);
		WriteFile(fidLogFileUse, entry, strlen(entry), &dwBytesWritten, NULL);
	}
	if (Mode == OF_CREATE && _fstrlen (Name) < 3)
		ii=1;
	if (*Name == 'l' || *Name == 'L')
		ii = 1;
//	if ((Mode == OF_READWRITE || Mode == OF_CREATE) && (strstr (Name,"HIGHWAYS") || strstr (Name,"highways")))
//		ii=1;
    _fstrcpy (LongName,Name);   
	strcpy (LastPathName,Name);
    if (DisplayFiles==1 || DisplayFiles ==2)
    	SetWindowText (hWndMain,Name);
    else if (DisplayFiles==3)
    	ShowOpenFiles (Name,Mode);   
	if (CheckForValidPath)
	{
	    if (!ValidPathName (Name))
    		goto Exit; 
    }
//    SetWindowText (hWndMain,Name); 
if (_fstrstr (Name,"offensexy.GMD"))
	ii=1;
    if (strnicmp (Name,"HTTP:",5) && !_fstrchr (Name,'\\'))   // change    filex.typ  to .\filex.typ (does this mess up OF_SEARCH?)
    {
        LPSTR s1, s2;
        
        s1 = _fstrchr (Name,'\0');
        s2 = s1 + 2;
        while (s1 != (LPSTR) Name)
            *s2-- = *s1--;
        *s2-- = *s1--;
        *s2-- = '\\';
        *s2 = '.';
    }
    //NameExists = GetLongPathName2 (Name,256); 
    
    if (!InOpenFile && *SubstitutePathnameFrom && Mode == OF_READ)
    {  
    	_fstrcpy (SaveName,Name);
    	_fullpath (str,Name,256);   
    	l = _fstrlen(SubstitutePathnameFrom);
    	if (!_fstrnicmp (str,SubstitutePathnameFrom,l))
    	{ 
    		UsingSubName = TRUE;
    		_fstrcpy (Name,SubstitutePathnameTo);
    		_fstrcat (Name,&str[l]);
    	}
    }    
                                        
    NumWait = 0;  
    NumBusyWait = 0;
	if (!InOpenFile && Mode == OF_READ && (Err=FileAlreadyNotFound (Name,1,0)))
	{
		pOFStruct->nErrCode = Err;
		Fid = HFILE_ERROR;
		goto Exit;
	}

Open2: 
    if (!UsingSubName && !InOpenFile && NumCachePathnameFrom && AllowCache &&
    	(Mode == OF_READ || Mode == OF_DELETE || Mode == OF_WRITE || Mode == OF_READWRITE || Mode == OF_CREATE || Mode == OF_EXIST))
    {  
    	short	icpf,lnoc;
    	LPSTR	pNoCache=NoCacheVal;  
    	LPSTR	pNoCacheEnd, pType;

    	_fstrcpy (SaveName,Name);
    	if (strnicmp (Name,"HTTP:",5))
    		_fullpath (str,Name,256);  
    	_fstrcpy (NoCacheVal,NoCache);  
    	
    	while (!CacheAll && *pNoCache)
    	{   
    		if ((pNoCacheEnd = _fstrchr (pNoCache,'|')))
    			*pNoCacheEnd++=0;
    		else
    			pNoCacheEnd = _fstrchr (pNoCache,0); 
    		if ((pType = _fstrchr (pNoCache,'*')))
    			*pType++ = 0;
    		lnoc = _fstrlen (pNoCache);
    		if (!_fstrnicmp (pNoCache,Name,lnoc)) 
    		{
    			if (pType && stricmp (pType,".*"))
    			{   
    				short lname = _fstrlen (Name);
    				if (lname > 4 && !_fstricmp (pType,&Name[lname-4]))
    					goto Open;
    			}
    			else
    				goto Open;
    		}
    		pNoCache = pNoCacheEnd;
    	}
    	for (icpf = 0;icpf < NumCachePathnameFrom; icpf++)
    	{   
    		_fstrcpy (CacheFromName,CachePathnameFrom[icpf]); 
    		ExpandText (CacheFromName);
	    	l = _fstrlen(CacheFromName);
	    	if (!l)
	    		goto Open;  
	    	if (CacheFromName[l-1] != '\\')
	    	{
	    		CacheFromName[l++] = '\\';
	    		CacheFromName[l] = 0;
	    	}
	    	if (!_fstrnicmp (str,CacheFromName,l))
	    	{ 
				char	altDir[MAX_PATH]="";

	    		UsingCacheName = TRUE;
				if (icpf)
				{
					strcpy (altDir,CacheFromName);
					REPLAC (altDir,":\\","_",MAX_PATH);
				}
				sprintf (Name,"%s%s%s",CachePathnameTo,altDir,&str[l]);

	    		if (_fstrstr (Name,"Configs"))
	    			ii=1;
	    		if (Mode == OF_EXIST)
	    		{   
					short rc = CacheAlreadyChecked (Name,_fstrlen(CachePathnameTo),CHECKFOREXIST);  
					UINT	Code;
					switch (rc)
					{   
						case CHECKTIMESTAMP:
						case DOESEXIST: 
							Fid = MAXFILEHANDLES;
							goto Exit;
						case DONTEXIST:
							Fid = HFILE_ERROR;
							goto Exit;
					}
							
			        Fid = OpenFileGSSi (SaveName,pOFStruct,Mode,0);
			        if (Fid == HFILE_ERROR)
			        {   
			        	if (pOFStruct->nErrCode == 5)
			        		goto Exit;
						GSSiRemove2(Name);
			        	Code = DONTEXIST; 
			        }
			        else
			        	Code = DOESEXIST;
	    			CacheAlreadyChecked (Name,_fstrlen(CachePathnameTo),Code);  
	    			goto Exit;
	    		}
	    		else if (Mode != OF_READ)
	    		{   
	    			CacheAlreadyChecked (Name,-(short)_fstrlen(CachePathnameTo),0); 
	    			GSSiRemove2(Name);
	    			_fstrcpy (Name,SaveName);
	    			goto Open;
	    		}
	    		else
				{    		
					double	dtime;
					struct _stati64	statfrom, statto;  
					HFILE	FidTo;
					
					InOpenFile = TRUE;
					FidTo = GSSiOpenFile (Name,pOFStruct,Mode);
					if (FidTo != HFILE_ERROR) 
					{
						GSSifstat (FidTo,&statto); 
 						if (CacheAlreadyChecked (Name,_fstrlen(CachePathnameTo),CHECKTIMESTAMP))
		    		    {
			   		    	InOpenFile = FALSE;
		    		    	Fid = FidTo;
		    		    	goto Exit;
		    		    } 
						*pOFStruct2 = *pOFStruct;
					}
					Fid = GSSiOpenFile (SaveName,pOFStruct,Mode);   
					if (Fid == HFILE_ERROR) 
					{
			        	if (pOFStruct->nErrCode == 5)
			        		goto Exit;
						CacheAlreadyChecked (Name,_fstrlen(CachePathnameTo),DONTEXIST);
						InOpenFile = FALSE;  
						GSSiClose (FidTo);
		    			GSSiRemove2(Name);
						goto Exit;
					}  
					GSSifstat (Fid,&statfrom);
					GSSiClose (Fid);
					if (!CacheAll && statfrom.st_size > abs (MaxFileSizeToCache) && FidTo == HFILE_ERROR)
					{
						InOpenFile = FALSE;
						strcpy (Name,SaveName);
						goto Open;
					}
					if (FidTo == HFILE_ERROR)
						dtime = 1; 
					else
					{ 
						if (statfrom.st_size != statto.st_size)
							dtime = 1;
						else
							dtime = difftime (statfrom.st_mtime,statto.st_mtime); 
					}
					if (dtime>0 && AllowCache != 2)
					{   
						BOOL	OkToCache=TRUE;
						
						if (FidTo != HFILE_ERROR)
						{
							if (UpdateGMDFromCheckPointLog (FidTo,Name,SaveName))
							{
								InOpenFile = FALSE;
	   		    				break;
							}
							GSSiRemove2(Name);
						} 
						else
							OkToCache = makedirectories (Name,FALSE,FALSE);
						if (OkToCache)
						{
							double FreeSpace = GetDriveFreeSpace (Name); 

							FreeSpace -= statfrom.st_size;
							if (FreeSpace < MinCacheDriveFreeSpace)
								OkToCache = FALSE;
						} 
						if (OkToCache)
						{
							CopyFileToCache (Name,SaveName);
						}
						else
						{
							_fstrcpy (Name,SaveName);
							UsingCacheName = FALSE;
						}
	    		    }       
	    		    else   
	    		    {
		   		    	InOpenFile = FALSE;
	    		    	Fid = FidTo;
						*pOFStruct = *pOFStruct2;
	    		    	goto Exit;
	    		    } 
	   		    	InOpenFile = FALSE;
	   		    	break;
	    		}
	    	}
    	}
    }    
Open: 
	if (Mode == OF_CREATE)
	{
		if (Mode == OF_CREATE && _fstrlen (LongName) < 3)
			ii=1;
		if (!makedirectories (LongName,FALSE,TRUE))
			goto Exit;
		FileWillBeCreated (LongName);
		/*if (!NoDelete) 
		{
			CreateLongNameFile (LongName); 
		    NameExists = GetLongPathName2 (Name,256);
		}*/
	}
	if (NoDelete)
	{   
		UINT	mode2=OF_READWRITE;
		
	    if (DisplayFiles==4)
    		ShowOpenFiles (Name,OF_EXIST);  
		NameExists = GetLongPathName2 (Name,256);
		if (NameExists)
			BlowOut("Trying to create existing file",Name);
	    if (DisplayFiles==4)
    		ShowOpenFiles (Name,Mode);   
//		if (CreateLongNameFile (LongName)) 
//	    	NameExists = GetLongPathName2 (Name,256);
//	    else
	    	mode2 = OF_CREATE;
        Fid = OpenFileGSSi (Name,pOFStruct,mode2,0);
    } 
    else if ((Fid = FileAlreadyOpen (Name,Mode,pOFStruct)) != HFILE_ERROR)
    	goto Exit;
    else if (Mode == OF_READ && ShareEnabled) 
    {
	    if (DisplayFiles==4)
    		ShowOpenFiles (Name,Mode);   
        Fid = OpenFileGSSi (Name,pOFStruct,OF_READ,OF_SHARE_DENY_WRITE);
    }
    else if (Mode == OF_WRITE && ShareEnabled)
    {
	    if (DisplayFiles==4)
    		ShowOpenFiles (Name,Mode);   
        Fid = OpenFileGSSi (Name,pOFStruct,OF_WRITE,OF_SHARE_EXCLUSIVE);
    }
    else if (Mode == OF_READWRITE && ShareEnabled)
    {
	    if (DisplayFiles==4)
    		ShowOpenFiles (Name,Mode);   
        Fid = OpenFileGSSi (Name,pOFStruct,OF_READWRITE,OF_SHARE_EXCLUSIVE);
    }
    else if (Mode == OF_EXIST && ShareEnabled)
    {
	    if (DisplayFiles==4)
    		ShowOpenFiles (Name,Mode);   
        Fid = OpenFileGSSi (Name,pOFStruct,OF_EXIST,0); 
    }
	else if (Mode == OF_DELETE)
	{
		Fid = HFILE_ERROR;
		if (AllowJournal && CurrentCheckPointID)
		{
			if (DeleteFileInJournal (Name))
				Fid = 1;
		}
		else if (!GSSiRemove2(Name))
			Fid = 1;
	}
    else
    {
	    if (DisplayFiles==4)
    		ShowOpenFiles (Name,Mode);   
        Fid = OpenFileGSSi (Name,pOFStruct,Mode,0);
		if (Mode == OF_CREATE && strlen (Name) > 3 && !stricmp (&Name[strlen(Name)-4],".tmp"))
			SetFileAttributes (Name,FILE_ATTRIBUTE_TEMPORARY);
		//pOFStruct->nErrCode = 0;
    }    
    if (Fid == HFILE_ERROR)
    {   
        short   ii;
        
        ii=0;
        if (pOFStruct->nErrCode == 32 && ShareEnabled)
        {   
        	InFileWait = TRUE;
        	if (GetGlobalBVal2 ("[%WAITBEEP]",TRUE))
            	Beep (frequency, duration);  
            GetWindowText(GetActiveWindow(),SaveText,144);
            sprintf (str,"Waiting for %s",Name);
            SetWindowText (GetActiveWindow(),str);
            GSSiTrace(str,0); 
            nms = GetGlobalLVal2 ("[%WAITDURATION]",1000);
            Sleep (nms);
            SetWindowText (GetActiveWindow(),SaveText);
            NumWait++;
		    if (NumWait >= GetGlobalLVal2 ("[%WAITCYCLES]",10))
		    {   
		        sprintf (str,"File %s is in use",Name); 
		        GSSiTrace (str,0);
		        if (!OkToContinue (TRUE))
					BlowOut(0,0);
		        if (GSSiMessageBox (0,"Continue to wait?",str,
		            MB_YESNO|MB_ICONQUESTION|MB_TASKMODAL,0)==IDYES) 
		            NumWait = 0;
		        else
					BlowOut(0,0);
		    }
            goto Open;
        } 
        else if (Mode == OF_READ && pOFStruct->nErrCode==21 && NumBusyWait < 5)
        {
            NumBusyWait++;
            Sleep(2000);
            goto Open;
        }
		else if (!strnicmp (Name,"HTTP:",5) && pOFStruct->nErrCode==22)
			goto Exit;
        else if (pOFStruct->nErrCode == 5 && nAccessErrors++ < 10)
		{
            Sleep (500);
			goto Open;
		}
        else if ((Mode == OF_READ && pOFStruct->nErrCode>3) || 
                 (Mode != OF_READ && pOFStruct->nErrCode>2 && Mode != OF_EXIST))
        {   
        	if (IgnoreFileOpenError)
        		goto Exit; 
            if (!LoadString(hInst, GetOPENERR00()+pOFStruct->nErrCode, errmes, 64))
            	*errmes=0;
            sprintf (str,"Error number %i on open (mode %i): %s\r\n%s",(short)pOFStruct->nErrCode,(short) Mode,errmes,Name);
			if (GetGlobalBVal2 ("[%LogOpenFileErrors]",FALSE))
				AbendWriter (str, Name,0,0);
			else
			{
				HaltMapDisplay(FALSE,FALSE); 
				setDoPaint( FALSE);
				if (GSSiMsgBox( GetFocus(),str, 0, MB_OKCANCEL|MB_ICONEXCLAMATION,0) ==  IDCANCEL)
            		BlowOut(0,0);
			}
        } 
        else if (UsingSubName)
        {
        	UsingSubName = FALSE;  
        	_fstrcpy (Name,SaveName);
        	goto Open2;
        }
        else if (UsingCacheName)
        {
        	UsingCacheName = FALSE;  
        	_fstrcpy (Name,SaveName);
        	goto Open;
        }
    }
    else if (Mode != OF_EXIST && Mode != OF_DELETE)
    {   
    	short ii;
    	
    	if (Fid < FidSmall)
    	{   
    		_close (Fid); 
    		if (Fid ==42)
    			ii=1;
    		CloseFidSmall ();
    		CreateFidSmall (); 
    		goto Open2;
    	}
    	if (!Fid) 
    	{  //this needed in NT environment probably due to a bug in the coord conversion dll - must be closing a FID of 0  
    		_close (Fid);
    		CreateFidSmall (); 
    		goto Open2;
    	}
    }
    if (TraceOn)
    {
        sprintf (str,"Open file: %s  ID: %i %i %i",Name,Fid,(short)Mode,(short)pOFStruct->nErrCode);
        GSSiTrace(str,0); 
        _fullpath (str,Name,256);
        GSSiTrace(str,0);
    }
    if (DoTime)
    	OpenFileTime+=(GetTickCount()-starttime);   
    InFileWait = FALSE;
	if (Mode == OF_WRITE || Mode == OF_READWRITE)
		FileOpenForUpdate (Name,Fid,Mode);
	else if (Mode == OF_READ && AllowJournal && CurrentCheckPointID)
		OpenJournal (Name,Fid,Mode);
Exit: 
	GSSiGlobUlFree (&hSTR);  
	if (Fid == HFILE_ERROR)
		NumNullOpen++;
	else if (Mode != OF_EXIST)
		NumFileOpen++;
	SetContinueProcessing ( SaveContinueProcessing);
{
#if ENABLETRACE
GSSiExitProg (307);
#endif
	return (Fid);
}
#if ENABLETRACE
}
#endif
}

BOOL copyfile (LPSTR ToFileIn, LPSTR FromFile,short AppendOrReplace,long BeginPos,long EndPos,HWND hWndDlg,UINT StatusCntl,long TotLen,LPLONG pCurLoc)
#if ENABLETRACE
{GSSiEnterProg (308);
#endif
{   UINT nRead;
    short FidFrom, FidTo;
    DWORD maxread;
    OFSTRUCTGM    fStruct;
    LPOFSTRUCTGM  pStruct = &fStruct;
    HANDLE  hBuffer;
    LPSTR   pBuffer;
    HCURSOR hcurSave, hCursor;
    char	ToFile[MAX_PATH]; 
    long	zeros[64], TotToRead=-1, TotRead=0;
    
    _fstrcpy (ToFile,ToFileIn);
    ExpandText (ToFile);
    if (EndPos > 0)
    	TotToRead = EndPos - BeginPos;

    maxread = SHRT_MAX;
    hBuffer = GSSiGlobAlloc (  95,GMEM_MOVEABLE,maxread);
    pBuffer = GlobalLock (hBuffer);
    FidFrom = GSSiOpenFile (FromFile,pStruct,OF_READ);
    if (FidFrom == HFILE_ERROR)
    	goto RtnFalse;    
    if (BeginPos > 0)
    	GSSillseek (FidFrom,BeginPos,0);	
    if (AppendOrReplace == TRUE)
    {
    	FidTo   = GSSiOpenFile (ToFile,pStruct,OF_READWRITE);  
    	if (FidTo != HFILE_ERROR)
    	{
    		GSSillseek (FidTo,0,2);
    		goto Start;
    	}
    } 
    if (AppendOrReplace == -1)
    {
    	if (ExistFile (ToFile))
    	{   
    		int	DoReplace;
    		
    		sprintf (pBuffer,"Do you wish to replace existing file: %s",ToFile);
			DoReplace = GSSiMsgBox (GetFocus(),pBuffer,"Verify File Replace",MB_YESNO|MB_ICONQUESTION,0); 
			if (DoReplace != IDYES)
		    {
		        GSSiClose(FidFrom);
		        goto RtnFalse;
		    } 
    	}
    }
	makedirectories (ToFile,FALSE,FALSE); 
   	FidTo = GSSiOpenFile (ToFile,0,OF_CREATE); 
//    FidTo   = GSSiOpenFile (ToFile,pStruct,OF_CREATE);
    if (FidTo == HFILE_ERROR)
    {
        GSSiClose(FidFrom);
        goto RtnFalse;
    } 
Start:
    hCursor = LoadCursor (0,IDC_WAIT);
    hcurSave = GSSiSetCursor (hCursor);
    nRead = BigRead (FidFrom,pBuffer,(UINT) maxread);
    while (nRead)
    {
//   _lwrite (FidTo,zeros,256);   
	    if (nRead == (UINT)HFILE_ERROR) 
    	{
		    GSSiClose (FidFrom);
		    GSSiClose (FidTo);
		    GSSiSetCursor (hcurSave); 
    		goto RtnFalse;
    	}
	    TotRead += nRead; 
	    if (TotToRead > 0 && TotRead > TotToRead)
	    	nRead -= (TotRead - TotToRead);
        if (BigWrite(FidTo,pBuffer,nRead,-1) != nRead) 
    	{
		    GSSiClose (FidFrom);
		    GSSiClose (FidTo);
		    GSSiSetCursor (hcurSave); 
    		goto RtnFalse;
    	}
	    if (TotToRead > 0 && TotRead >= TotToRead)
	    	break;
    	if (hWndDlg)
    	{
    		(*pCurLoc) += nRead;
	        PctBox (GetDlgItem(hWndDlg,StatusCntl),TotLen,*pCurLoc,0);
	    }
    	 
//   _lwrite (FidTo,zeros,256);
        nRead = BigRead (FidFrom,pBuffer,(UINT) maxread);
    }
    GSSiClose (FidFrom);
    GSSiClose (FidTo);
    GSSiGlobUlFree (&hBuffer);
    GSSiSetCursor (hcurSave); 
{
#if ENABLETRACE
GSSiExitProg (308);
#endif
    return (TRUE);
}
RtnFalse:
    GSSiGlobUlFree (&hBuffer);
{
#if ENABLETRACE
GSSiExitProg (308);
#endif
    return (FALSE);
}
#if ENABLETRACE
}
#endif
}

LPSTR FirstNonBlank(LPSTR Text)
#if ENABLETRACE
{GSSiEnterProg (309);
#endif
{   LPSTR rtn;

    rtn = Text;
    while (*rtn == ' '|| *rtn=='\t') rtn++;
{
#if ENABLETRACE
GSSiExitProg (309);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

LPSTR NextBlank(LPSTR Text)
#if ENABLETRACE
{GSSiEnterProg (310);
#endif
{   LPSTR rtn;

    rtn = Text;
    if (!*rtn)
{
#if ENABLETRACE
GSSiExitProg (310);
#endif
    	return 0;
}
    while (*rtn && *rtn != ' '&& *rtn!='\t') rtn++;
{
#if ENABLETRACE
GSSiExitProg (310);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

LPSTR LastNonBlank(LPSTR Text)
#if ENABLETRACE
{GSSiEnterProg (311);
#endif
{   LPSTR rtn;

    rtn = Text;
    while (*Text)
    {
        if (*Text!=' ' && *Text!='\t') rtn=Text;
        Text++;
    }
{
#if ENABLETRACE
GSSiExitProg (311);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
} 

LPSTR PadString (LPSTR str,char padchr,int padlen)
#if ENABLETRACE
{GSSiEnterProg (312);
#endif
{
	short	l=_fstrlen (str);
	
	while (l < padlen)
	{
		str[l] = padchr;
		l++;
	}       
	str[l]=0;
{
#if ENABLETRACE
GSSiExitProg (312);
#endif
	return str; 
}
#if ENABLETRACE
}
#endif
}

LPSTR Truncate(LPSTR Text)
#if ENABLETRACE
{GSSiEnterProg (313);
#endif
{   LPSTR rtn, EndChar;

    rtn = Text; 
    EndChar = _fstrchr (Text,0);
    while (EndChar > Text)
    {                    
    	EndChar--;
        if (*EndChar != ' ')
        	goto Exit;
        *EndChar = 0;
    } 
Exit:
{
#if ENABLETRACE
GSSiExitProg (313);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}    
LPSTR Truncate2(LPSTR Text,char c)
#if ENABLETRACE
{GSSiEnterProg (313);
#endif
{   LPSTR rtn, EndChar;

rtn = Text;
EndChar = _fstrchr(Text, 0);
while (EndChar > Text)
{
	EndChar--;
	if (*EndChar != c)
		goto Exit;
	*EndChar = 0;
}
Exit:
{
#if ENABLETRACE
	GSSiExitProg(313);
#endif
	return (rtn);
}
#if ENABLETRACE
}
#endif
}

LPSTR TruncateAt (LPSTR Text,LPSTR EndChars)
#if ENABLETRACE
{GSSiEnterProg (314);
#endif
{   LPSTR rtn;
	size_t	i;

    rtn = Text;
    i = _fstrcspn (Text,EndChars);
    Text+=i;
    *Text=0; 
{
#if ENABLETRACE
GSSiExitProg (314);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}   

LPSTR FirstNonInt (LPSTR Text)
#if ENABLETRACE
{GSSiEnterProg (315);
#endif
{ 

	while (*Text)
	{
		if (!isdigit (*Text))
{
#if ENABLETRACE
GSSiExitProg (315);
#endif
			return Text; 
}
		Text++;
	}
{
#if ENABLETRACE
GSSiExitProg (315);
#endif
	return Text;
}
#if ENABLETRACE
}
#endif
} 

LPSTR FirstAlpha (LPSTR Text)
#if ENABLETRACE
{GSSiEnterProg (315);
#endif
{ 

	while (*Text)
	{
		if (isalpha (*Text))
{
#if ENABLETRACE
GSSiExitProg (315);
#endif
			return Text; 
}
		Text++;
	}
{
#if ENABLETRACE
GSSiExitProg (315);
#endif
	return Text;
}
#if ENABLETRACE
}
#endif
} 

DPOINT dnewpt (DPOINT OldPoint, double AZM, double DIS)
#if ENABLETRACE
{GSSiEnterProg (316);
#endif
{   DPOINT NewPoint;

      NewPoint.x=OldPoint.x+DIS*cos(AZM);
      NewPoint.y=OldPoint.y+DIS*sin(AZM);
{
#if ENABLETRACE
GSSiExitProg (316);
#endif
      return (NewPoint);
}
#if ENABLETRACE
}
#endif
}

void LNEWPT (double X,double Y,LPDOUBLE NewX,LPDOUBLE NewY, double AZM, double DIS)
#if ENABLETRACE
{GSSiEnterProg (317);
#endif
{  
      *NewX=X+DIS*cos(AZM);
      *NewY=Y+DIS*sin(AZM); 
{
#if ENABLETRACE
GSSiExitProg (317);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}

void RestoreScreenRect (HDC hDC, HBITMAP hSavedBM, RECT Rect, RECT NewRect)
#if ENABLETRACE
{GSSiEnterProg (318);
#endif
{   HDC     hdcMem;
    HBITMAP hbmPrev; 
    short       i;

    SaveDC (hDC);
	SetDisplayMode (hDC, GF_SCREENMODE); 
/*    SetMapMode    ( hDC, MM_TEXT );
    SetWindowOrgEx  ( hDC, 0,   0,0 );
    SetViewportOrgEx( hDC, 0, 0,0 );*/ 
    SelectClipRgn (hDC,0);
    
    hdcMem = CreateCompatibleDC(hDC);
    hbmPrev = SelectObject(hdcMem, hSavedBM);

    i=StretchBlt (hDC, NewRect.left,NewRect.top,
                NewRect.right-NewRect.left+1,
                NewRect.bottom-NewRect.top+1,
                hdcMem, 0, 0,
                Rect.right-Rect.left+1,
                Rect.bottom-Rect.top+1,
                SRCCOPY);

    SelectObject(hdcMem, hbmPrev);
    DeleteDC(hdcMem);
    RestoreDC (hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (318);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}

void TRAN (void *source, void *dest, size_t count)
#if ENABLETRACE
{GSSiEnterProg (319);
#endif
{

    _fmemmove(dest,source,count);
{
#if ENABLETRACE
GSSiExitProg (319);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

void STRIPR(LPSTR stuff, int *NLEN,LPSTR out_with,  int llen) 
{
	LPSTR pLoc, pLoc2;
	
Top:
	pLoc = strstr (stuff,out_with);
	if (!pLoc)
		return;
	pLoc2 = pLoc + llen;
	while (*pLoc2)
		*pLoc++ = *pLoc2++;
	*pLoc = 0;
	*NLEN -= llen;
	goto Top;
}


void  STRIPR_old(LPSTR stuff, int *NLEN,LPSTR out_with, int llen) //strips leading characters??
#if ENABLETRACE
{GSSiEnterProg (320);
#endif
{
LPSTR  loc_it, the_end;
int    whats_left,idiff,i;
     
       
      the_end =  stuff + _fstrlen(stuff); 
      
r10:  loc_it = strstr (stuff, out_with);
 
      while (loc_it)
      { 
          whats_left = the_end - loc_it;
          i = whats_left-llen;
          if (i <= 0)
          {
          	*NLEN = 0;
{
#if ENABLETRACE
GSSiExitProg (320);
#endif
          	return;
}
          }
          _fmemmove (loc_it,loc_it+llen,(size_t)(whats_left-llen));
           the_end -= llen;
		   *the_end = 0;
          goto r10;  
      } 
      *NLEN = _fstrlen(stuff);     
{
#if ENABLETRACE
GSSiExitProg (320);
#endif
return;
}
#if ENABLETRACE
}
#endif
} 

  long  IREAD (LPSTR incoming, long NUMDIG, long *IRC )
#if ENABLETRACE
{GSSiEnterProg (321);
#endif
{

long	rtn;
char	savechr;
LPSTR	ptr;
short i;
*IRC = 0;
  if (NUMDIG > 31)
  {
    *IRC = -1;
{
#if ENABLETRACE
GSSiExitProg (321);
#endif
    return 0;
}
  }  
  ptr = incoming;
  i = NUMDIG;
  while (i--)
  {
    if(*ptr < '0' || *ptr++ > '9')
    {
      *IRC = -1;
{
#if ENABLETRACE
GSSiExitProg (321);
#endif
      return 0;
}
    }
  }    
 
 ptr = incoming + NUMDIG;
 savechr = *ptr;
 *ptr = 0; 
 rtn = atol(incoming); 
 *ptr = savechr;
{
#if ENABLETRACE
GSSiExitProg (321);
#endif
 return rtn;
}
#if ENABLETRACE
}
#endif
 }

BOOL IWRITE (long ival,LPSTR str ,short len)
#if ENABLETRACE
{GSSiEnterProg (322);
#endif
{
	char	buf[16];
	UINT	ns, l;
	
	ltoa (ival,buf,10);
	l = _fstrlen (buf);
	if (l > len)
{
#if ENABLETRACE
GSSiExitProg (322);
#endif
		return FALSE;
}
	ns = len - l;
	while (ns--)
		*str++ = ' ';
	_fmemmove (str,buf,l);
{
#if ENABLETRACE
GSSiExitProg (322);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL IWRITEZ (long ival,LPSTR str ,short len)
#if ENABLETRACE
{GSSiEnterProg (323);
#endif
{
	char	buf[16];
	UINT	ns, l;
	
	ltoa (ival,buf,10);
	l = _fstrlen (buf);
	if (l > len)
{
#if ENABLETRACE
GSSiExitProg (323);
#endif
		return FALSE;
}
	ns = len - l;
	while (ns--)
		*str++ = '0';
	_fmemmove (str,buf,l);
{
#if ENABLETRACE
GSSiExitProg (323);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

LPSTR REPLAC (LPSTR STRING, LPSTR OLD, LPSTR NEW, int MAXLEN)//neg maxlen implies ignore case
#if ENABLETRACE
{GSSiEnterProg (324);
#endif
{
    LPSTR   IBEG, loc, new;
    
      short  STRLEN, CURLEN, OLDLEN, NEWLEN, IDIFF;
      long  MOVELEN; 
	  BOOL ignoreCase = FALSE;
      
	  if (MAXLEN < 0)
	  {
		  ignoreCase = TRUE;
		  MAXLEN = -MAXLEN;
	  }
      OLDLEN = _fstrlen (OLD);
	  if (OLDLEN)
	  {
		  NEWLEN = _fstrlen(NEW);
		  STRLEN = _fstrlen(STRING);
		  IBEG = STRING;
		  IDIFF = NEWLEN - OLDLEN;
		  while (*IBEG)
		  {
			  if (ignoreCase)
			  {
				  int lold = strlen(OLD);
				  int lbeg = strlen(IBEG);
				  LPSTR upOLD = malloc(lold + 4);
				  LPSTR upBEG = malloc(lbeg + 4);
				  LPSTR newBEG;
				  strcpy(upOLD, OLD);
				  strupr(upOLD);
				  strcpy(upBEG, IBEG);
				  strupr(upBEG);
				  newBEG = strstr(upBEG, upOLD);
				  if (newBEG)
				  {
					  int inc = newBEG - upBEG;
					  IBEG = IBEG + inc;
				  }
				  else
					  IBEG = 0;
				  free(upOLD);
				  free(upBEG);
			  }
			  else
				  IBEG = _fstrstr(IBEG, OLD);
			  if (!IBEG) goto Exit;
			  if (!IDIFF)
			  {
				  new = NEW;
				  while (*new) *IBEG++ = *new++;
			  }
			  else if (IDIFF < 0)
			  {
				  loc = IBEG + OLDLEN;
				  new = NEW;
				  while (*new) *IBEG++ = *new++;
				  new = IBEG;
				  while (*loc) *new++ = *loc++;
				  *new = 0;
				  STRLEN += IDIFF;
			  }
			  else
			  {
				  CURLEN = IBEG - STRING;
				  MOVELEN = STRLEN - CURLEN;
				  STRLEN += IDIFF;
				  if (STRLEN > MAXLEN)
				  {
					  STRLEN = MAXLEN;
					  goto Exit;
				  }
				  TRAN(IBEG + OLDLEN, IBEG + NEWLEN, (size_t)MOVELEN);
				  new = NEW;
				  while (*new) *IBEG++ = *new++;
			  }
		  }
	  Exit:
		  loc = STRING + STRLEN;
		  *loc = '\0';
	  }
{
#if ENABLETRACE
GSSiExitProg (324);
#endif
      return (STRING);
}
#if ENABLETRACE
}
#endif
}
    
void Sound (short Type)
#if ENABLETRACE
{GSSiEnterProg (325);
#endif
{
   switch (Type)
   {
        case GOOD_SOUND:
            MessageBeep (MB_OK);
			Sleep (1000);
            break;
        case BAD_SOUND:
            MessageBeep (MB_ICONEXCLAMATION);
			Sleep (1000);
            break;
   }
{
#if ENABLETRACE
GSSiExitProg (325);
#endif
   return; 
}
#if ENABLETRACE
}
#endif
}

/* Sounds the speaker for a time specified in microseconds by duration
 * at a pitch specified in hertz by frequency.
 */
#ifndef WIN32
void Beep( unsigned frequency, unsigned duration )
#if ENABLETRACE
{GSSiEnterProg (326);
#endif
{
    short control;

    /* If frequency is 0, Beep doesn't try to make a sound. */
    if( frequency )
    {
        /* 75 is about the shortest reliable duration of a sound. */
        if( duration < 75 )
            duration = 75;

        /* Prepare timer by sending 10111100 to port 43. */
        _outp( 0x43, 0xb6 );

        /* Divide input frequency by timer ticks per second and
         * write (byte by byte) to timer.
         */
        frequency = (unsigned)(1193180L / frequency);
        _outp( 0x42, (char)frequency );
        _outp( 0x42, (char)(frequency >> 8) );

        /* Save speaker control byte. */
        control = _inp( 0x61 );

        /* Turn on the speaker (with bits 0 and 1). */
        _outp( 0x61, control | 0x3 );
    }

    Sleep( (clock_t)duration );

    /* Turn speaker back on if necessary. */
    if( frequency )
        _outp( 0x61, control );
#if ENABLETRACE
}
#endif
}

/* Pauses for a specified number of milliseconds. */
void Sleepx( clock_t _wait )
#if ENABLETRACE
{GSSiEnterProg (327);
#endif
{
    clock_t goal;

    goal = _wait + GetTickCount();
    while( goal > GetTickCount() )
        ;
#if ENABLETRACE
}
#endif
}

#endif

HBITMAP SaveScreen (HDC hDC, RECT Rect)
#if ENABLETRACE
{GSSiEnterProg (328);
#endif
{   HDC hdcMem;
    HBITMAP hbmPrev, hNewBM;
    short   i,ii; 
    long	size;
	int		w=RECTWIDTH(&Rect), h=RECTHEIGHT(&Rect);
    
//    InflateRect (&Rect,2,2);
    SaveDC (hDC);
	SetGraphicsMode(hDC, GM_COMPATIBLE);
//	SetDisplayMode (hDC, GF_SCREENMODE); 
    SetWindowOrgEx  ( hDC, 0, 0,0 );
    SetViewportOrgEx( hDC, 0, 0,0 );    
    SetMapMode    ( hDC, MM_TEXT );
  	SelectClipRgn ( hDC,0);
    hdcMem = CreateCompatibleDC(hDC);
    hNewBM = CreateCompatibleBitmap(hDC,w,h);
    if (hNewBM)
    {
		BITMAP	bm;

		GetObject(hNewBM, sizeof(bm), (LPSTR)&bm);
        hbmPrev = SelectObject(hdcMem, hNewBM);
    
        i=BitBlt(hdcMem, 0, 0, w,h,
               hDC, Rect.left,Rect.top, SRCCOPY);
    
        if (hbmPrev)
            SelectObject(hdcMem, hbmPrev);
    }
    else
        ii=0;
    DeleteDC(hdcMem);
    RestoreDC (hDC,-1); 
{
#if ENABLETRACE
GSSiExitProg (328);
#endif
    return (hNewBM);
}
#if ENABLETRACE
}
#endif
}

void RestoreScreen (HDC hDC, HBITMAP hSavedBM, RECT Rect)
#if ENABLETRACE
{GSSiEnterProg (329);
#endif
{   HDC     hdcMem;
    HBITMAP hbmPrev;
    short       i;

    if (!hSavedBM)
{
#if ENABLETRACE
GSSiExitProg (329);
#endif
    	return;
}
    SaveDC (hDC);
	SetDisplayMode (hDC, GF_SCREENMODE); 
 /*   SetWindowOrgEx  ( hDC, 0, 0,0 );
    SetViewportOrgEx( hDC, 0, 0,0 );    
    SetMapMode    ( hDC, MM_TEXT );
  	SelectClipRgn ( hDC,0);*/
    hdcMem = CreateCompatibleDC(hDC);
    hbmPrev = SelectObject(hdcMem, hSavedBM);

    i=BitBlt(hDC, Rect.left,Rect.top,
                Rect.right-Rect.left+1,
                Rect.bottom-Rect.top+1,
           hdcMem, 0,0,  SRCCOPY);

    SelectObject(hdcMem, hbmPrev);
    DeleteDC(hdcMem);
    RestoreDC (hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (329);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void dumpmemdc(HDC hdc)
{
	HBITMAP hbm = CreateCompatibleBitmap(hdc, 1, 1);
	HBITMAP hBM = SelectObject(hdc, hbm);
	HDIB hDib = BitmapToDIB(hBM, 0, 0);
	SaveDIB(hDib, "c:\\temp\\dump.bmp");
	SelectObject (hdc,hBM);
	DeleteObject(hbm);
	return;
}
HANDLE SaveScreen2 (HWND hWnd,HDC hDC, RECT Rect, LPVOID pVP,LPLONG pID)
#if ENABLETRACE
{GSSiEnterProg (330);
#endif
{   
	HANDLE	handle=GSSiGlobAlloc (  96,GHND,sizeof(SAVESCREEN));
	LPSAVESCREEN	pSaveScreen=(LPSAVESCREEN)GlobalLock (handle);
	RECT winRect;
	BOOL	dbug=FALSE;

	if (pID)
	{
		*pID = NextScreenID; 
		pSaveScreen->ID = NextScreenID++; 
	}
	else
		pSaveScreen->ID = 0;
	pSaveScreen->hWnd = hWnd;
	if (hWnd != (HWND)-1)
	{
		GetClientRect(hWnd, &winRect);
		IntersectRect(&Rect, &Rect, &winRect);
	}
	pSaveScreen->Rect = Rect;
	pSaveScreen->hBM = SaveScreen (hDC,Rect);
	/*if (dbug)
	{
		HDIB hDib=BitmapToDIB (pSaveScreen->hBM, 0,0);
		SaveDIB (hDib,"c:\\temp\\temp.bmp");
	}*/
	if (!pSaveScreen->hBM)
		GSSiGlobUlFree (&handle);
	else
	{
		pSaveScreen->pVP = pVP;
		GlobalUnlock (handle);
		RegisterSavedScreen (handle); 
	} 
{
#if ENABLETRACE
GSSiExitProg (330);
#endif
    return (handle);
}
#if ENABLETRACE
}
#endif
}

BOOL ScreenIsRegistered (HANDLE hSavedScreen,long ID)
#if ENABLETRACE
{GSSiEnterProg (331);
#endif
{   
	UINT	i;   
	BOOL	rtn;
	
    if (!hSavedScreen)
{
#if ENABLETRACE
GSSiExitProg (331);
#endif
    	return FALSE; 
}
    for (i=0;i<nSavedScreens;i++)
    	if (hSavedScreen == hSavedScreens[i]) 
    	{
			LPSAVESCREEN	pSaveScreen=(LPSAVESCREEN)GlobalLock (hSavedScreen);  
			rtn = (ID == pSaveScreen->ID);
			GlobalUnlock (hSavedScreen);
    		
{
#if ENABLETRACE
GSSiExitProg (331);
#endif
    		return rtn;
} 
		}
{
#if ENABLETRACE
GSSiExitProg (331);
#endif
    return FALSE;
}
#if ENABLETRACE
}
#endif
}

void RestoreScreen2 (HDC hDC, HANDLE hSavedScreen,long ID,BOOL Clip)
#if ENABLETRACE
{GSSiEnterProg (332);
#endif
{
	LPSAVESCREEN	pSaveScreen;
	HDC     hdcMem;
    HBITMAP hbmPrev;
    short       i;
    
	if (!hSavedScreen)
		return;
    if (!ScreenIsRegistered(hSavedScreen,ID))
{
#if ENABLETRACE
GSSiExitProg (332);
#endif
    	return;
}
	if (debugvalue < 0)
		debugvalue = 0;
	pSaveScreen=(LPSAVESCREEN)GlobalLock (hSavedScreen); 
	if (!pSaveScreen)
{
#if ENABLETRACE
GSSiExitProg (332);
#endif
	    return;
}
	if (!pSaveScreen->hBM)
		goto Exit;
	if (ID && pSaveScreen->ID != ID)
		goto Exit;
    SaveDC (hDC);
	SetDisplayMode (hDC, GF_SCREENMODE); 
// 	SetGraphicsMode(hDC, GM_COMPATIBLE);
    SetMapMode    ( hDC, MM_TEXT );
    SetWindowOrgEx  ( hDC, 0, 0,0 );
    SetViewportOrgEx( hDC, 0, 0,0 );
  	if (!Clip)
  		SelectClipRgn ( hDC,0);
    hdcMem = CreateCompatibleDC(hDC); 
    if (hdcMem)
    {
	    hbmPrev = SelectObject(hdcMem, pSaveScreen->hBM);
	    	
	    i=BitBlt(hDC, pSaveScreen->Rect.left,pSaveScreen->Rect.top,
					  RECTWIDTH(&pSaveScreen->Rect),RECTHEIGHT(&pSaveScreen->Rect),
	                //pSaveScreen->Rect.right-pSaveScreen->Rect.left+1,
	                //pSaveScreen->Rect.bottom-pSaveScreen->Rect.top+1,
	           hdcMem, 0, 0,  SRCCOPY);
	
	    SelectObject(hdcMem, hbmPrev);
	    DeleteDC(hdcMem); 
	}
    RestoreDC (hDC,-1);
Exit: 
    GlobalUnlock (hSavedScreen);
{
#if ENABLETRACE
GSSiExitProg (332);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

void DestroySavedScreen (LPHANDLE phSavedScreen,long ID)
#if ENABLETRACE
{GSSiEnterProg (333);
#endif
{
	LPSAVESCREEN	pSaveScreen; 
	short	i;

    if (!ScreenIsRegistered(*phSavedScreen,ID))
    {
		*phSavedScreen = 0;
{
#if ENABLETRACE
GSSiExitProg (333);
#endif
    	return; 
}
    }
	pSaveScreen=(LPSAVESCREEN)GlobalLock (*phSavedScreen);
	if (ID && pSaveScreen->ID != ID)
	{
		GlobalUnlock (*phSavedScreen);
{
#if ENABLETRACE
GSSiExitProg (333);
#endif
		return;
}
	}
	if (pSaveScreen->hBM) 
		DeleteObject(pSaveScreen->hBM); 
	UnRegisterSavedScreen (*phSavedScreen);
	GSSiGlobUlFree (phSavedScreen);
{
#if ENABLETRACE
GSSiExitProg (333);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

BOOL WriteSavedScreen (LPSTR File,HANDLE hSavedScreen,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (334);
#endif
{
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;
	HANDLE		handle;
	long		SaveSize,ii;
	LPSAVESCREEN pSaveScreen;
	HPSTR	pBM;
	HDIB	hDIB;
	BOOL	rtn=FALSE;
	
	if (!*File)
{
#if ENABLETRACE
GSSiExitProg (334);
#endif
		return FALSE;
}
	makedirectories (File,FALSE,FALSE);
	pSaveScreen = (LPSAVESCREEN)GlobalLock (hSavedScreen); 
	hDIB = BitmapToDIB (pSaveScreen->hBM, 0,0);
	SaveDIB (hDIB,File);
	DestroyDIB (hDIB); 
	Fid = GSSiOpenFile (File,&OFStruct,OF_READWRITE);  
	if (Fid == HFILE_ERROR)
		goto Exit;
    ii=GSSillseek (Fid,0,2);
	BigWrite (Fid,(HPSTR)&pSaveScreen->Rect,sizeof(RECT),-1);
	BigWrite (Fid,(HPSTR)pBounds,sizeof(MNMXCORD),-1);
	GSSiClose (Fid);
Exit:
	GlobalUnlock (hSavedScreen);
{
#if ENABLETRACE
GSSiExitProg (334);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

HANDLE	ReadSavedScreen (LPSTR File,LPLONG pUpdateTime,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (335);
#endif
{
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;
	HANDLE		handle;
	long		SaveSize,ii;
	LPSAVESCREEN pSaveScreen;
	HPSTR	pBM;  
	HDIB	hDIB; 
	HPALETTE	hPal;
	struct _stati64	Stat;
	
	if (!*File)
{
#if ENABLETRACE
GSSiExitProg (335);
#endif
		return 0;
}
	Fid = GSSiOpenFile (File,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (335);
#endif
		return FALSE;
}
	GSSifstat (Fid,&Stat);
	*pUpdateTime = Stat.st_mtime;
	handle = GSSiGlobAlloc (  97,GMEM_MOVEABLE,sizeof(SAVESCREEN));  
	pSaveScreen = (LPSAVESCREEN)GlobalLock (handle);
	ii=GSSillseek (Fid,0,2);                
	ii=GSSillseek (Fid,ii-(sizeof(RECT)+sizeof(MNMXCORD)),0);                
	BigRead (Fid,(HPSTR)&pSaveScreen->Rect,sizeof(RECT));
	BigRead (Fid,(HPSTR)pBounds,sizeof(MNMXCORD));
	GSSiClose (Fid);
	hDIB = LoadDIB (File); 
	hPal = CreateDIBPalette (hDIB);
	pSaveScreen->hBM = DIBToBitmap (hDIB,hPal); 
	pSaveScreen->ID = NextScreenID++; 
	if (hPal)
    	DeleteObject (hPal); 
	DestroyDIB (hDIB); 
	GlobalUnlock (handle);
	RegisterSavedScreen (handle);  
{
#if ENABLETRACE
GSSiExitProg (335);
#endif
	return handle;
}
#if ENABLETRACE
}
#endif
} 
long DecompressBinaryRecordUnsafe (LPBYTE pDecompressedRec,LPBYTE pCompressedRec,DWORD CompressedLen)
{
	int		DCLen;
	int		rtn;

	if (InitLZO())
	{    
		rtn = lzo1x_decompress(pCompressedRec,(lzo_uint) CompressedLen,pDecompressedRec,(lzo_uint *) &DCLen,NULL);
		if (rtn == 0)
        	return DCLen;
	}
	return 0;
}

void RegisterSavedScreen (HANDLE hSavedScreen)
#if ENABLETRACE
{GSSiEnterProg (336);
#endif
{	
	short	i;

	if (nSavedScreens == MAXSAVEDSCREENS)
{
#if ENABLETRACE
GSSiExitProg (336);
#endif
		return; 
}
	for (i=0;i<nSavedScreens;i++)
		if (hSavedScreen == hSavedScreens[i])
{
#if ENABLETRACE
GSSiExitProg (336);
#endif
			return;
}
	hSavedScreens[nSavedScreens++] = hSavedScreen;
{
#if ENABLETRACE
GSSiExitProg (336);
#endif
	return;		
}
#if ENABLETRACE
}
#endif
}   

void ReduceSavedScreens (void)
#if ENABLETRACE
{GSSiEnterProg (337);
#endif
{   
	short	i,n=0;
	
	for (i=0;i<nSavedScreens;i++)
		if (hSavedScreens[i]) 
			hSavedScreens[n++] = hSavedScreens[i];
	nSavedScreens = n;
	
{
#if ENABLETRACE
GSSiExitProg (337);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void UnRegisterSavedScreen (HANDLE hSavedScreen)
#if ENABLETRACE
{GSSiEnterProg (338);
#endif
{	
	short	i;

	for (i=0;i<nSavedScreens;i++)
		if (hSavedScreen == hSavedScreens[i]) 
			hSavedScreens[i] = 0; 
	ReduceSavedScreens ();
{
#if ENABLETRACE
GSSiExitProg (338);
#endif
	return;		
}
#if ENABLETRACE
}
#endif
}
   
void ClearSavedScreens(HWND hWnd,LPVOID pVP, LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (339);
#endif
{
	LPSAVESCREEN	pSaveScreen; 
	RECT	IntRect;
	short	i;

	for (i=0;i<nSavedScreens;i++)
	{
		pSaveScreen=(LPSAVESCREEN)GlobalLock (hSavedScreens[i]);
		if (pSaveScreen)
		{
			if (hWnd == (HWND)-1 || hWnd == pSaveScreen->hWnd)
			{
				if (pSaveScreen->pVP == pVP || !pVP)
				{
					DeleteObject(pSaveScreen->hBM); 
					GSSiGlobUlFree (&hSavedScreens[i]);
				}
				else if ((!pSaveScreen->pVP || !pVP) && pRect && IntersectRect (&IntRect,&pSaveScreen->Rect,pRect))
				{
					DeleteObject(pSaveScreen->hBM); 
					GSSiGlobUlFree (&hSavedScreens[i]);
				}
				else
					GlobalUnlock (hSavedScreens[i]);
			}
			else
				GlobalUnlock (hSavedScreens[i]);
		}
		else
			hSavedScreens[i] = 0;
	}
	ReduceSavedScreens ();
{
#if ENABLETRACE
GSSiExitProg (339);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ResaveSavedScreens(void)
#if ENABLETRACE
{GSSiEnterProg (340);
#endif
{   int	iscreen;

/*	for (iscreen=0;iscreen<nSavedScreens;iscreen++)
	{
		if (*SavedScreens[iscreen])
		{
			DeleteObject(*SavedScreens[iscreen]);
			*SavedScreens[iscreen]=SaveScreen (CurView->hDC,SavedRects[iscreen]);
		}
	}*/
#if ENABLETRACE
}
#endif
} 

BOOL    GetBit (int ibit, LPSTR lpBytes)
#if ENABLETRACE
{GSSiEnterProg (341);
#endif
{   int       bit, byte;

    byte = ibit/8;
    bit  = ibit%8;
    lpBytes += byte;
    if (Mask[bit] & *lpBytes)
{
#if ENABLETRACE
GSSiExitProg (341);
#endif
        return 1;
}
    else
{
#if ENABLETRACE
GSSiExitProg (341);
#endif
        return 0;
}
#if ENABLETRACE
}
#endif
}

void SetBit (int ibit, LPSTR lpBytes, BOOL setto)
#if ENABLETRACE
{GSSiEnterProg (342);
#endif
{   int       bit, byte;

    byte = ibit/8;
    bit  = ibit%8;
    lpBytes += byte;
    if (setto)
        *lpBytes = Mask[bit] | *lpBytes;
    else
        if (Mask[bit] & *lpBytes)
        	*lpBytes = Mask[bit] ^ *lpBytes;

{
#if ENABLETRACE
GSSiExitProg (342);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

void SetBit2 (int ibit, LPSTR lpBytes, BOOL setto)
#if ENABLETRACE
{GSSiEnterProg (343);
#endif
{   int       bit, byte;

    byte = ibit/8;
    bit  = 7-ibit%8;
    lpBytes += byte;
    if (setto)
        *lpBytes = Mask[bit] | *lpBytes;
    else
        if (Mask[bit] & *lpBytes) *lpBytes = Mask[bit] ^ *lpBytes;

{
#if ENABLETRACE
GSSiExitProg (343);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

BYTE ComputeCheckSum (LPBYTE rec,DWORD l)
{
	USHORT	chksum=0;
	BYTE	csum;	
	
	while(l--)
		chksum += *rec++;
	csum = 256-(chksum&0xff);
	return csum; 
}

long GSSiLength (LPSTR File)
#if ENABLETRACE
{GSSiEnterProg (344);
#endif
{
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;
	long		l;
	
	Fid = GSSiOpenFile (File,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (344);
#endif
		return -1;
}
	l = GSSifilelength (Fid);
	GSSiClose(Fid);
{
#if ENABLETRACE
GSSiExitProg (344);
#endif
	return l;
}
#if ENABLETRACE
}
#endif
}

long GSSifilelength (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (345);
#endif
{                
    long    CurLoc, Len;
    
	if (Fid < 0)
		Len = -1;
	else
	{
		CurLoc = GSSillseek (Fid,0,1);
		Len = GSSillseek (Fid,0,2);
		GSSillseek (Fid,CurLoc,0);
	}
{
#if ENABLETRACE
GSSiExitProg (345);
#endif
    return (Len);
}
#if ENABLETRACE
}
#endif
}

/************************************************************************/
/*  GWCheckMenuItem Function                                            */
/*                                                                      */
/*  checks and unchecks menu items based on their current state         */
/*  retrns the new state                                                                    */
/************************************************************************/

BOOL GWCheckMenuItem(HWND hWnd, int wItem)
#if ENABLETRACE
{GSSiEnterProg (346);
#endif
{
 HANDLE     hMenu = GetMenu(hWnd);
 WORD       wState;
 
 if (!hMenu)
{
#if ENABLETRACE
GSSiExitProg (346);
#endif
 	return FALSE;
}
 wState = GetMenuState(hMenu, wItem, MF_BYCOMMAND);
 if(wState == (wState | MF_CHECKED))
 {
   CheckMenuItem(hMenu, wItem, MF_BYCOMMAND | MF_UNCHECKED);
{
#if ENABLETRACE
GSSiExitProg (346);
#endif
   return (FALSE);
}
 }
 else
   CheckMenuItem(hMenu, wItem, MF_BYCOMMAND | MF_CHECKED);
{
#if ENABLETRACE
GSSiExitProg (346);
#endif
 return (TRUE);
}

#if ENABLETRACE
}
#endif
}



LPSTR StrEnd (LPSTR str)
/*  Retrns pointer to first space after last non-space or end of string */
#if ENABLETRACE
{GSSiEnterProg (347);
#endif
{   LPSTR rtn;

    rtn = 0;
    while (*str)
    {   if (*str == ' ')
            if (!rtn) rtn = str;
        else
            rtn = 0;
        str++;
    }
    if (!rtn) rtn = str;
{
#if ENABLETRACE
GSSiExitProg (347);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

short EndStr (LPSTR str)
/*  Retrns length to first space after last non-space or end of string */
#if ENABLETRACE
{GSSiEnterProg (348);
#endif
{   short rtn, i;

    rtn = 0;
    i=0;
    while (*str)
    {   if (*str == ' ')
        {
            if (!rtn) rtn = i;
        }
        else
            rtn =  0;
        str++;
        i++;
    }
    if (!rtn) rtn = i;
{
#if ENABLETRACE
GSSiExitProg (348);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL WriteErrorHandler (HFILE Fid,long ln,long loc)
#if ENABLETRACE
{GSSiEnterProg (349);
#endif
{
    short	i;
    char	mess[256];

	if (GetGlobalBVal2 ("[%LOGWRITEERRORS]",FALSE))
	{
		char	file[MAX_PATH];

		if ((i=GetOpenFileID (Fid)) >= 0) 
		{
			sprintf (mess,"Unable to write %ld bytes to FID %ld(%ld)- probable disk full",ln,(long)Fid,OpenFileFid[i]);
			if (!OpenFileMode[i])
				sprintf (mess,"WRITE attempted on file open for READ:%s", OpenFileStruct[i].szPathName); 
		} 
		else
		{
			sprintf (mess,"Unable to write %ld bytes to FID %ld - invalid Fid",ln,(long)Fid);
		}
		GetGlobalCVal ("[%WRITEERRORLOGFILE]",file,"C:\\temp\\gmwriteerrorlog.txt");
		AppendFile2 (file,mess);
	}
	else
	{
		if ((i=GetOpenFileID (Fid)) >= 0) 
		{
			sprintf (mess,"Unable to write %ld bytes to FID %ld(%ld)- probable disk full",ln,(long)Fid,OpenFileFid[i]);
			if (!OpenFileMode[i])
				BlowOut ("WRITE attempted on file open for READ", OpenFileStruct[i].szPathName); 
			else
				BlowOut (mess, OpenFileStruct[i].szPathName);
		} 
		else
		{
			sprintf (mess,"Unable to write %ld bytes to FID %ld - probable disk full",ln,(long)Fid);
			BlowOut (mess, ""); 
		}
	}
{
#if ENABLETRACE
GSSiExitProg (349);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL fputstring(LPSTR lpStr, HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (350);
#endif
{   
    UINT    len;
	int		rtn=0;
    
	if (Fid != HFILE_ERROR)
	{
		len=_fstrlen(lpStr);
		if (len)
			rtn = BigWrite (Fid,(char *)lpStr,len,-1); 
		rtn += BigWrite (Fid,"\r\n",2,-1);
	}
{
#if ENABLETRACE
GSSiExitProg (350);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL fputstring2(LPSTR lpStr, HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (350);
#endif
{   
    UINT    len; 
    
    len=_fstrlen(lpStr);
    if (len)
        _lwrite (Fid,(char *)lpStr,len); 
    _lwrite (Fid,"\r\n",2);
{
#if ENABLETRACE
GSSiExitProg (350);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetOpenFilePathname (HFILE Fid,LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (351);
#endif
{   
	short	i;
	
	if ((i=GetOpenFileID (Fid)) >= 0)
	{
		_fstrcpy (Name,OpenFileStruct[i].szPathName); 
{
#if ENABLETRACE
GSSiExitProg (351);
#endif
		return TRUE;
}
	}
{
#if ENABLETRACE
GSSiExitProg (351);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

LPSTR fgetstring (LPSTR lpStr, int lenIN, HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (352);
#endif
{   UINT   lrec;
    LPSTR lpEnd; 
    DWORD   loc; 
	int len = abs(lenIN);
    
    *lpStr = 0;            
    loc = GSSillseek (Fid,0,1); 
	LastFGSLoc = loc;    
    lrec = BigRead (Fid,lpStr,len+2);
    if (!lrec)// || lrec == (UINT)HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (352);
#endif
    	return 0; 
}
    lpEnd = lpStr;
    while (lrec--)
    {
		if (lenIN < 0)
		{
			if (*lpEnd == '\r' && *(lpEnd + 1) != '\n')
				*lpEnd = ' ';
		}
    	if (*lpEnd == '\r' || *lpEnd == '\n')
    		break;
    	lpEnd++;
    }
    *lpEnd++ = 0; 
    if (*lpEnd == '\n')
		lpEnd++;
    lrec =  lpEnd - lpStr;
    LastFGSlRec = lrec;
    GSSillseek (Fid,loc,0);
    GSSillseek (Fid,lrec,1);
{
#if ENABLETRACE
GSSiExitProg (352);
#endif
    return lpStr;
}
#if ENABLETRACE
}
#endif
}

LPSTR fgetstring2 (LPSTR lpStr, int len, HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (352);
#endif
{   UINT   lrec;
    LPSTR lpEnd; 
    DWORD   loc; 
    
    *lpStr = 0;            
    loc = _llseek (Fid,0,1); 
	LastFGSLoc = loc;    
    lrec = _lread (Fid,lpStr,len+2);
    if (!lrec)// || lrec == (UINT)HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (352);
#endif
    	return 0; 
}
    lpEnd = lpStr;
    while (lrec--)
    {
    	if (*lpEnd == '\r' || *lpEnd == '\n')
    		break;
    	lpEnd++;
    }
    *lpEnd++ = 0; 
    if (*lpEnd == '\n')
		lpEnd++;
    lrec =  lpEnd - lpStr;
    LastFGSlRec = lrec;
    _llseek (Fid,loc,0);
    _llseek (Fid,lrec,1);
{
#if ENABLETRACE
GSSiExitProg (352);
#endif
    return lpStr;
}
#if ENABLETRACE
}
#endif
}

void fputss (LPSTR lpStr, FILE *Fid)
#if ENABLETRACE
{GSSiEnterProg (353);
#endif
{
    fputs (lpStr,Fid);
    fputs ("\r\n",Fid);
{
#if ENABLETRACE
GSSiExitProg (353);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}
LPSTR fgetss (LPSTR lpStr, short len, FILE *Fid)
#if ENABLETRACE
{GSSiEnterProg (354);
#endif
{   LPSTR i, r;

    r = fgets (lpStr,len, Fid);
    if (r)
    {
        i = _fstrchr (lpStr,'\r');
        if (i) *i = '\0';
        i = _fstrchr (lpStr,'\n');
        if (i) *i = '\0';
    }
{
#if ENABLETRACE
GSSiExitProg (354);
#endif
    return (r);
}
#if ENABLETRACE
}
#endif
}

BOOL NextLine (LPSTR *lpText,LPSTR lpLine,short nAutoLines)
#if ENABLETRACE
{GSSiEnterProg (365);
#endif
{   LPSTR	loc; 
	short	inc=1;

    if (!**lpText)
{
#if ENABLETRACE
GSSiExitProg (365);
#endif
    	return (FALSE);
}
    _fstrcpy(lpLine,*lpText);
    if (nAutoLines)
    {   
    	short	ltxt = _fstrlen (lpLine);
    	LPSTR	lpSpace = lpLine; 
    	LPSTR	pEnd =  lpLine + IDNINT(1.5 * (ltxt/nAutoLines));
    	
    	while (lpSpace && lpSpace < pEnd) 
    	{
    		loc = lpSpace;
    		lpSpace = _fstrchr (lpSpace+1,' ');
    	} 
    	inc = 0;
    }
    else
    	loc = _fstrchr(lpLine,'\r');
    if (loc && loc != lpLine)
    {   
    	long	inc2 = (long)loc - (long)lpLine + inc;
    	
    	if (*(loc+1) == '\n')
    		inc2++;
        (*lpText) += inc2;
        *loc = '\0';
    }
    else if (loc)
    {
    	*loc = 0;
    	(*lpText)++;
    	if (*(loc+1) == '\n')
	    	(*lpText)++;
    }
    else
        *lpText += _fstrlen(*lpText);
    if (!_fstrlen(lpLine))
    	_fstrcpy (lpLine," ");
{
#if ENABLETRACE
GSSiExitProg (365);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

short   Signof (double val)
#if ENABLETRACE
{GSSiEnterProg (366);
#endif
{
    if (val < 0)
{
#if ENABLETRACE
GSSiExitProg (366);
#endif
        return -1;
}
    if (val > 0)
{
#if ENABLETRACE
GSSiExitProg (366);
#endif
        return 1;
}
{
#if ENABLETRACE
GSSiExitProg (366);
#endif
    return 0;
}
#if ENABLETRACE
}
#endif
}

int NumCharInString (LPSTR str,char chr)
{
	int	n=0;
	
	while (*str)
		if (*str++ == chr)
			n++;
	return n;
}

short   lSignof (long val)
#if ENABLETRACE
{GSSiEnterProg (367);
#endif
{
    if (val < 0)
{
#if ENABLETRACE
GSSiExitProg (367);
#endif
        return -1;
}
    if (val > 0)
{
#if ENABLETRACE
GSSiExitProg (367);
#endif
        return 1;
}
{
#if ENABLETRACE
GSSiExitProg (367);
#endif
    return 0;
}
#if ENABLETRACE
}
#endif
}

double   dSignof (double val)
#if ENABLETRACE
{GSSiEnterProg (368);
#endif
{
    if (val < 0)
{
#if ENABLETRACE
GSSiExitProg (368);
#endif
        return (double)-1.0;
}
    if (val > 0)
{
#if ENABLETRACE
GSSiExitProg (368);
#endif
        return (double)1.0;
}
{
#if ENABLETRACE
GSSiExitProg (368);
#endif
    return 0;
}
#if ENABLETRACE
}
#endif
}

double  AZDF(double AZC, double AZP, double RL)
#if ENABLETRACE
{GSSiEnterProg (369);
#endif
{   
	double	rtn=LTWOPI(DSIGN(LTWOPI(AZC-AZP), RL));
	
{
#if ENABLETRACE
GSSiExitProg (369);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
};

double DSIGN(const double Value, const double Sign)
#if ENABLETRACE
{GSSiEnterProg (370);
#endif
{ 
	double rtn;
	
	rtn = (Sign >= 0e0) ? fabs(Value) : -fabs(Value);
{
#if ENABLETRACE
GSSiExitProg (370);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}
	
POINT newpt(POINT OldPoint, double AZM, double DIS)
#if ENABLETRACE
{GSSiEnterProg (371);
#endif
{   POINT NewPoint;

NewPoint.x = IDNINT((OldPoint.x + DIS*cos(AZM)));
NewPoint.y = IDNINT((OldPoint.y + DIS*sin(AZM)));
{
#if ENABLETRACE
	GSSiExitProg (371);
#endif
	return (NewPoint);
}
#if ENABLETRACE
}
#endif
}

FPOINT newptF(FPOINT OldPoint, double AZM, double DIS)
#if ENABLETRACE
{GSSiEnterProg (371);
#endif
{  FPOINT NewPoint;

NewPoint.x = OldPoint.x + DIS*cos(AZM);
NewPoint.y = OldPoint.y + DIS*sin(AZM);
{
#if ENABLETRACE
	GSSiExitProg(371);
#endif
	return (NewPoint);
}
#if ENABLETRACE
}
#endif
}

POINT newptscreen(POINT OldPoint, double AZM, double DIS)
#if ENABLETRACE
{GSSiEnterProg (371);
#endif
{   POINT NewPoint;

      NewPoint.x= IDNINT((OldPoint.x+DIS*cos(AZM)));
      NewPoint.y= IDNINT((OldPoint.y+DIS*sin(AZM)));
      NewPoint.y = OldPoint.y - (NewPoint.y - OldPoint.y);
{
#if ENABLETRACE
GSSiExitProg (371);
#endif
      return (NewPoint);
}
#if ENABLETRACE
}
#endif
}
double getaz(POINT Point1, POINT Point2)
#if ENABLETRACE
{GSSiEnterProg(372);
#endif
{   
	double	rtn = LTWOPI(atan2(((double)Point2.y - (double)Point1.y), ((double)Point2.x - (double)Point1.x)));
	{
#if ENABLETRACE
		GSSiExitProg(372);
#endif
		return rtn;
	}
#if ENABLETRACE
}
#endif
}


double getazF (FPOINT Point1, FPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (372);
#endif
{   
	double	rtn=LTWOPI(atan2(((double)Point2.y-(double)Point1.y),((double)Point2.x-(double)Point1.x)));
{
#if ENABLETRACE
GSSiExitProg (372);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

float fgetaz (FLTPOINT Point1, FLTPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (372);
#endif
{   
	float	rtn=LTWOPI(atan2(((double)Point2.y-(double)Point1.y),((double)Point2.x-(double)Point1.x)));
{
#if ENABLETRACE
GSSiExitProg (372);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

double getazl (LPOINT Point1, LPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (372);
#endif
{   
	double	rtn=LTWOPI(atan2(((double)Point2.y-(double)Point1.y),((double)Point2.x-(double)Point1.x)));
{
#if ENABLETRACE
GSSiExitProg (372);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

double getazd (HPDPOINT Point1, HPDPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (373);
#endif
{   
	double 	rtn=LTWOPI(atan2((Point2->y-Point1->y),(Point2->x-Point1->x)));
{
#if ENABLETRACE
GSSiExitProg (373);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

double get2dazfrom3d (HPDPOINT3D Point1, HPDPOINT3D Point2)
#if ENABLETRACE
{GSSiEnterProg (373);
#endif
{   
	double 	rtn=LTWOPI(atan2((Point2->y-Point1->y),(Point2->x-Point1->x)));
{
#if ENABLETRACE
GSSiExitProg (373);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

double LGETAZ(double X1, double Y1, double X2, double Y2)
#if ENABLETRACE
{GSSiEnterProg (374);
#endif
{   
	double	rtn=LTWOPI(atan2(Y2 - Y1,X2 - X1));
{
#if ENABLETRACE
GSSiExitProg (374);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

double LTWOPI (double AZ1)
#if ENABLETRACE
{GSSiEnterProg (375);
#endif
{
      if (AZ1<0)
{
#if ENABLETRACE
GSSiExitProg (375);
#endif
      	return (TWOPI + fmod(AZ1,TWOPI));
}
      if (AZ1 == 0)
{
#if ENABLETRACE
GSSiExitProg (375);
#endif
      	return (AZ1);
}
{
#if ENABLETRACE
GSSiExitProg (375);
#endif
      return (fmod(AZ1,TWOPI));
}
#if ENABLETRACE
}
#endif
}

POINT MidPoint(POINT Point1, POINT Point2)
#if ENABLETRACE
{GSSiEnterProg (376);
#endif
{   POINT point;

point.x = ((long)Point1.x + (long)Point2.x) / 2;
point.y = ((long)Point1.y + (long)Point2.y) / 2;
{
#if ENABLETRACE
	GSSiExitProg (376);
#endif
	return (point);
}

#if ENABLETRACE
}
#endif
}

FPOINT MidPointF(FPOINT Point1, FPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (376);
#endif
{   FPOINT point;

point.x = (Point1.x + Point2.x) / 2;
point.y = (Point1.y + Point2.y) / 2;
{
#if ENABLETRACE
	GSSiExitProg(376);
#endif
	return (point);
}

#if ENABLETRACE
}
#endif
}

DPOINT MidPointD(DPOINT Point1, DPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (377);
#endif
{   DPOINT point;

    point.x = (Point1.x + Point2.x)/2;
    point.y = (Point1.y + Point2.y)/2;
{
#if ENABLETRACE
GSSiExitProg (377);
#endif
    return (point);
}
#if ENABLETRACE
}
#endif
}

DPOINT3D MidPoint3D (DPOINT3D Point1, DPOINT3D Point2)
#if ENABLETRACE
{GSSiEnterProg (377);
#endif
{   DPOINT3D point;

    point.x = (Point1.x + Point2.x)/2;
    point.y = (Point1.y + Point2.y)/2;
    point.z = (Point1.z + Point2.z)/2;
{
#if ENABLETRACE
GSSiExitProg (377);
#endif
    return (point);
}

#if ENABLETRACE
}
#endif
}

double idist(POINT Point1, POINT Point2)
#if ENABLETRACE
{GSSiEnterProg (378);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (378);
#endif
    return (sqrt (pow((double)Point1.x-(double)Point2.x,2)
            +pow((double)Point1.y-(double)Point2.y,2)));
}
#if ENABLETRACE
}
#endif
}
double ldistp(DPOINT Point1, DPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (379);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (379);
#endif
    return (sqrt (pow(Point1.x-Point2.x,2)
            +pow(Point1.y-Point2.y,2)));
}
#if ENABLETRACE
}
#endif
}
double l2ddistfrom3d (DPOINT3D Point1, DPOINT3D Point2)
#if ENABLETRACE
{GSSiEnterProg (379);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (379);
#endif
    return (sqrt (pow(Point1.x-Point2.x,2)
            +pow(Point1.y-Point2.y,2)));
}
#if ENABLETRACE
}
#endif
}
double ldistpp(LPDPOINT Point1, LPDPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (379);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (379);
#endif
    return (sqrt (pow(Point1->x-Point2->x,2)
            +pow(Point1->y-Point2->y,2)));
}
#if ENABLETRACE
}
#endif
}
LPSTR IADDR (LPSTR InAdd, long Offset)
#if ENABLETRACE
{GSSiEnterProg (380);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (380);
#endif
    return (InAdd + (Offset-1));
}
#if ENABLETRACE
}
#endif
}

long HADDR (HPSTR InAddress, long Offset)
#if ENABLETRACE
{GSSiEnterProg (380);
#endif
{
	long	InAdd = (long)InAddress;
{
#if ENABLETRACE
GSSiExitProg (380);
#endif
    return (InAdd + (Offset-1));
}
#if ENABLETRACE
}
#endif
}

short IDNSHRT (double X)
{   
	long	i=IDNINT (X);
	
	return min (max(i,SHRT_MIN),SHRT_MAX);
}

void GSSiERROR (HWND hWnd, short ErrorNum, LPSTR ErrorMessage)
#if ENABLETRACE
{GSSiEnterProg (381);
#endif
{
    GSSiMsgBox( hWnd, ErrorMessage,"GSSi Error Message", MB_OK,0);
{
#if ENABLETRACE
GSSiExitProg (381);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL MemError ()
#if ENABLETRACE
{GSSiEnterProg (382);
#endif
{
    GSSiMsgBox( GetFocus(), "Insufficient memory for this operation","Fatal Error", MB_OK,0);
{
#if ENABLETRACE
GSSiExitProg (382);
#endif
    return(FALSE);
}
#if ENABLETRACE
}
#endif
}  
  
BOOL IsReal (LPSTR str)
{
	LPSTR pEnd;
	double	val=strtod (str,&pEnd);
	
	if (*pEnd)
		return FALSE;
	return TRUE;
}
	
BOOL IsInteger(LPSTR str)
{   
	short	lstr;
	LPSTR	instr;
	
	if (!*str)
		return FALSE;
	while (*str == ' ')
		str++;
	if (*str == '-')
		str++; 
	instr = str;
	lstr = _fstrlen (str);
	if (lstr > 10)
		return FALSE;
    while (*str)
    {
        if (!isdigit(*str++))
            return FALSE;
    }
    if (lstr == 10)
    {
    	if (atof (instr) > LONG_MAX)
    		return FALSE;
    }
    return TRUE;
}

BOOL CheckForContinue(BOOL QuitOnEscapeOnly, LPBOOL pQuitProcessing)
#if ENABLETRACE
{GSSiEnterProg (384);
#endif
{
 MSG            msg; 

 GdiFlush ();
 if (pQuitProcessing)
	 *pQuitProcessing = FALSE;
 while (GSSiPeekMessage(&msg,0,0,0,PM_NOREMOVE))
 {
																							#if ENABLETRACE
	 SetLastMessage(-1 * (long)msg.message, msg.wParam);
																							#endif
	if (msg.message == WM_QUIT)
{
																							#if ENABLETRACE
																							GSSiExitProg (384);
																							#endif
	if (pQuitProcessing)
		*pQuitProcessing = TRUE;
	return FALSE;
}
//	if (msg.message == WM_PAINT)
//    	rturn TRUE;
	if (msg.message == WM_COMMAND && msg.wParam == IDCANCEL)
{
																							#if ENABLETRACE
																							GSSiExitProg (384);
																							#endif
	if (pQuitProcessing)
		*pQuitProcessing = TRUE;
	return FALSE;
}
	if (msg.message == WM_KEYDOWN && (!QuitOnEscapeOnly || (msg.wParam == 27)))
{
																							#if ENABLETRACE
																							GSSiExitProg (384);
																							#endif
		if (msg.wParam == 27 && pQuitProcessing)
			*pQuitProcessing = TRUE;
																							
		return FALSE;  
}
	if (msg.message == WM_LBUTTONDOWN ||
		msg.message == WM_RBUTTONDOWN)
   {
#if ENABLETRACE
		GSSiExitProg(384);
#endif

		return FALSE;
	}
	GSSiPeekMessage(&msg, 0, 0, 0, PM_REMOVE);
	TranslateMessage(&msg);
	DispatchMessage(&msg);
   

 }
{
																							#if ENABLETRACE
																							GSSiExitProg (384);
																							#endif
 return TRUE;
}
																							#if ENABLETRACE
																							}
																							#endif
}  

BOOL GetOrPeekMessage (MSG FAR *msg,HWND hWnd , UINT MinFilter, UINT MaxFilter, UINT Remove, BOOL UseGetMessage)
{
	if (UseGetMessage)
		return (GSSiGetMessage (msg, hWnd, MinFilter, MaxFilter));
	return (GSSiPeekMessage(msg, hWnd, MinFilter, MaxFilter,Remove));
}

BOOL WaitForKeystroke (BOOL UseGetMessage)
#if ENABLETRACE
{GSSiEnterProg (385);
#endif
{
 MSG            msg; 

 while (GetOrPeekMessage(&msg,0,0,0,PM_REMOVE,UseGetMessage))
 {
#if ENABLETRACE
	 SetLastMessage(-1 * (long)msg.message, msg.wParam);
#endif
	if (msg.message == WM_QUIT)
{
#if ENABLETRACE
GSSiExitProg (385);
#endif
    	return FALSE;
}
	if (msg.message == WM_PAINT)
	{
	  TranslateMessage(&msg);
	  DispatchMessage(&msg);
	}
	if(msg.message == WM_KEYDOWN)
{
#if ENABLETRACE
GSSiExitProg (385);
#endif
        return msg.wParam;
}
 }
{
#if ENABLETRACE
GSSiExitProg (385);
#endif
 return FALSE;
}
#if ENABLETRACE
}
#endif
}

/****************************************************************************

    FUNCTION: GetPrinterDC()

    PURPOSE:  Get hDc for current device on current output port according to
              info in WIN.INI.

    COMMENTS:

        Searches WIN.INI for information about what printer is connected, and
        if found, creates a DC for the printer.

        retrns
            hDC > 0 if success
            hDC = 0 if failure

****************************************************************************/

HANDLE GetPrinterDC()
#if ENABLETRACE
{GSSiEnterProg (388);
#endif
{
    char pPrintInfo[80];
    LPSTR lpTemp;
    LPSTR lpPrintType;
    LPSTR lpPrintDriver;
    LPSTR lpPrintPort;

    if (!GetProfileString("windows", "Device", (LPSTR)"", pPrintInfo, 80))
{
#if ENABLETRACE
GSSiExitProg (388);
#endif
        return (0);
}
    lpTemp = lpPrintType = pPrintInfo;
    lpPrintDriver = lpPrintPort = 0;
    while (*lpTemp) {
        if (*lpTemp == ',') {
            *lpTemp++ = 0;
            while (*lpTemp == ' ')
                lpTemp = AnsiNext(lpTemp);
            if (!lpPrintDriver)
                lpPrintDriver = lpTemp;
            else {
                lpPrintPort = lpTemp;
                break;
            }
        }
        else
            lpTemp = AnsiNext(lpTemp);
    }

{
#if ENABLETRACE
GSSiExitProg (388);
#endif
    return (CreateDC(lpPrintDriver, lpPrintType, lpPrintPort, 0));
}
#if ENABLETRACE
}
#endif
} 

void farfree(void far *block)
#if ENABLETRACE
{GSSiEnterProg (391);
#endif
{
#if WIN32
    HANDLE h = GlobalHandle(block);
#else
    HANDLE h = GlobalHandle(FP_SEG(block));
#endif
    if (! h)
{
#if ENABLETRACE
GSSiExitProg (391);
#endif
    	return;
}
    GSSiGlobUlFree (&h);
#if ENABLETRACE
}
#endif
}

void CenterRectOnPoint(LPRECT pRect, POINT center)
{
	RECT outRect;

	outRect.left = center.x - RECTWIDTH(pRect) / 2;
	outRect.right = center.x + RECTWIDTH(pRect) / 2;
	outRect.top = center.y - RECTHEIGHT (pRect) / 2;
	outRect.bottom = center.y + RECTHEIGHT(pRect) / 2;
	*pRect = outRect;
	return;
}

POINT RectMid (LPRECT rect)
#if ENABLETRACE
{GSSiEnterProg (396);
#endif
{
	POINT	p;
	
	p.x = rect->left + (rect->right - rect->left)/2;
	p.y = rect->top + (rect->bottom - rect->top)/2;
{
#if ENABLETRACE
GSSiExitProg (396);
#endif
	return p;
}
#if ENABLETRACE
}
#endif
}

DPOINT RectMidD (LPRECT rect)
#if ENABLETRACE
{GSSiEnterProg (396);
#endif
{
	DPOINT	p;
	
	p.x = (double)rect->left + ((double)(rect->right - rect->left))/2;
	p.y = (double)rect->top +  ((double)(rect->bottom - rect->top))/2;
{
#if ENABLETRACE
GSSiExitProg (396);
#endif
	return p;
}
#if ENABLETRACE
}
#endif
}

/*
 *  Calculate the CRC for a buffer "icp", of "icnt" length, based upon
 *  initial CRC "icrc".  For XMODEM-style CRC calculation, initialize
 *  icrc to 0; for CCITT-recommended CRC calculation, initialize icrc
 *  to -1.
 */

WORD ECRCMem(WORD *icrc, BYTE *icp, WORD icnt)
/* icrc = initial crc */
/* icp = buffer */
/* icnt = buffer len */
#if ENABLETRACE
{GSSiEnterProg (401);
#endif
    {
    register unsigned short crc = *icrc;
    BYTE W_FAR *cp = (BYTE W_FAR *) icp;
    register WORD cnt = icnt;

    /*
     *  If CRC lookup table has not already been built, callocate memory
     *  and build table.
     */
    if (have_crc_table == 0)
        {
        register short i;
        WORD b, v;  
        have_crc_table = 1;                        
        for (b = 0;  b <= (1 << 8) - 1;  ++b)
            {
            for (v = b << 8,  i = 8;  --i >= 0;  )
                v = v & 0x8000 ? (v << 1) ^ 0x1021 : v << 1;
            crc_table[b] = v;
            }
        }

    while (cnt--)
        crc = (crc << 8) ^ crc_table[(crc >> 8) ^ *cp++];

    *icrc = crc;
{
#if ENABLETRACE
GSSiExitProg (401);
#endif
    return 0;   // all ok
}
#if ENABLETRACE
}
#endif
    }   // ECrcBuf

DWORD CRC16 (LPSTR Array, short nc)
#if ENABLETRACE
{GSSiEnterProg (402);
#endif
{

    UINT    CRC=0; 
    short       i;

    while (0 < nc--)
    {
        CRC = CRC ^ (short)*Array++ << 8;
        for (i=0; i<8; i++)  
        {
            if (CRC & 0x8000)
                CRC = CRC << 1 ^ 0x1021;
            else
                CRC = CRC << 1;
        }
    }
{
#if ENABLETRACE
GSSiExitProg (402);
#endif
    return ((DWORD)CRC * (DWORD) CRC);
}
#if ENABLETRACE
}
#endif
}
short DrawCircle (HDC hDC,POINT Point,double Radius,short Width,COLORREF Color)
#if ENABLETRACE
{GSSiEnterProg (403);
#endif
{   
    short   npts; 
    DPOINT  Newpt, Dpoint;  
    HPEN    OldPen, Pen; 
    double  dinc, az;
                        
    Pen = CreatePen (PS_SOLID,Width,Color);
    OldPen = SelectObject (hDC,Pen);
    Dpoint.x = Point.x;
    Dpoint.y = Point.y;
    
    npts = (short)(2 * PY * Radius);
    dinc = 1.0/Radius;    
    az=0;
    Newpt = dnewpt (Dpoint,az,Radius);
    MoveToEx (hDC,(int)IDNINT(Newpt.x),(int)IDNINT(Newpt.y),0);   
    while (npts--) 
    {
        az+=dinc;
        Newpt = dnewpt (Dpoint,az,Radius);
        LineTo (hDC,(int)IDNINT(Newpt.x),(int)IDNINT(Newpt.y));   
    } 
    SelectObject (hDC,OldPen);
    DeleteObject (Pen);
{
#if ENABLETRACE
GSSiExitProg (403);
#endif
    return 0;
}
#if ENABLETRACE
}
#endif
}

BOOL SamePoint (POINT p1, POINT p2)
#if ENABLETRACE
{GSSiEnterProg (405);
#endif
{
	if (p1.x == p2.x && p1.y == p2.y)
{
#if ENABLETRACE
GSSiExitProg (405);
#endif
		return TRUE;
}
{
#if ENABLETRACE
GSSiExitProg (405);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}  

BOOL SamePointS (POINTS p1, POINTS p2)
#if ENABLETRACE
{GSSiEnterProg (405);
#endif
{
	if (p1.x == p2.x && p1.y == p2.y)
{
#if ENABLETRACE
GSSiExitProg (405);
#endif
		return TRUE;
}
{
#if ENABLETRACE
GSSiExitProg (405);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}  

BOOL SameLPoint (LPOINT p1, LPOINT p2)
#if ENABLETRACE
{GSSiEnterProg (405);
#endif
{
	if (p1.x == p2.x && p1.y == p2.y)
{
#if ENABLETRACE
GSSiExitProg (405);
#endif
		return TRUE;
}
{
#if ENABLETRACE
GSSiExitProg (405);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}  

BOOL PolylineF(HDC hDC, LPFPOINT pt, int npt)
{
	HANDLE hPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, npt * sizeof(POINT)+4);
	LPPOINT Points = GlobalLock(hPoints);
	BOOL rtn;

	for (int i = 0; i < npt; i++)
		Points[i] = FPointToPoint(pt[i]);
	rtn = Polyline(hDC, Points, npt);
	GSSiGlobUlFree(&hPoints);
	return rtn;
}

BOOL PolygonF(HDC hDC, LPFPOINT pt, int npt)
{
	HANDLE hPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, npt * sizeof(POINT)+4);
	LPPOINT Points = GlobalLock(hPoints);
	BOOL rtn;

	for (int i = 0; i < npt; i++)
		Points[i] = FPointToPoint(pt[i]);
	rtn = Polygon(hDC, Points, npt);
	GSSiGlobUlFree(&hPoints);
	return rtn;
}

BOOL SameDPoint(LPDPOINT p1, LPDPOINT p2)
#if ENABLETRACE
{GSSiEnterProg (406);
#endif
{
	if (LDIST(p1->x, p1->y, p2->x, p2->y) <= P_TOL)
	{
#if ENABLETRACE
		GSSiExitProg (406);
#endif
		return TRUE;
	}
	{
#if ENABLETRACE
		GSSiExitProg (406);
#endif
		return FALSE;
	}
#if ENABLETRACE
}
#endif
}

BOOL SameFPoint(FPOINT p1,FPOINT p2)
#if ENABLETRACE
{GSSiEnterProg (406);
#endif
{
	if (LDIST(p1.x, p1.y, p2.x,p2.y) <= P_TOL)
	{
#if ENABLETRACE
		GSSiExitProg(406);
#endif
		return TRUE;
	}
	{
#if ENABLETRACE
		GSSiExitProg(406);
#endif
		return FALSE;
	}
#if ENABLETRACE
}
#endif
}

void flip(LPSTR In, short n)
#if ENABLETRACE
{GSSiEnterProg (408);
#endif
{ 
    char Out[32];
    LPSTR SaveIn=In;
    short     num=n, i=0;
    
    while (num--)
        Out[num]=*In++;
    _fmemmove (SaveIn,Out,n);
{
#if ENABLETRACE
GSSiExitProg (408);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}   
    


void SIOOpenError (LPSTR Port,int Error)
#if ENABLETRACE
{GSSiEnterProg (413);
#endif
{       
	char	title[144],mess[256];
	
	switch (Error)
	{
   		
		case IE_BADID:
			_fstrcpy(mess,"The device identifier is invalid or unsupported.");
			break;
		case IE_BAUDRATE:
			_fstrcpy(mess,"The device's baud rate is unsupported.");
			break;
		case IE_BYTESIZE:
			_fstrcpy(mess,"The specified byte size is invalid.");
			break;
		case IE_DEFAULT:
			_fstrcpy(mess,"The default parameters are in error."); 
			break;
		case IE_HARDWARE:
			_fstrcpy(mess,"The hardware is not available (is locked by another device).");
			break;
		case IE_MEMORY:
			_fstrcpy(mess,"The function cannot allocate the queues."); 
			break;
		case IE_NOPEN:
			_fstrcpy(mess,"The device is not open."); 
			break;
		case IE_OPEN:
			_fstrcpy(mess,"The device is already open."); 
            break;
  	} 
  	sprintf(title,"Error opening port %s",Port);
  	GSSiMsgBox (GetFocus(),mess,title,MB_ICONEXCLAMATION,0);
{
#if ENABLETRACE
GSSiExitProg (413);
#endif
  	return;
}
#if ENABLETRACE
}
#endif
}

void GMEditReturn(void)
{
	EscapeFunction(FALSE);
	return;
}

void GetNonCachedFile(LPSTR file)
{
	int icpf=0, l;
	char CacheFromName[MAX_PATH];
	char nonCachedFile[MAX_PATH];

	l = strlen(CachePathnameTo);
	if (!_fstrnicmp(file, CachePathnameTo, l))
	{
		if (file[l] == '#')
		{
			icpf = atoi(&file[l + 1]);
			l += 2;
		}
		strcpy(nonCachedFile, CachePathnameFrom[icpf]);
		strcat(nonCachedFile, &file[l]);
		ExpandText(nonCachedFile);
		strcpy(file, nonCachedFile);
	}
	return;
}

BOOL GMEdit (HWND hWnd, LPSTR File)
{
	BOOL rtn=TRUE;
	char str[MAX_PATH+32];
	char nonCachedFile[MAX_PATH];

	strcpy(nonCachedFile, File);
	GetNonCachedFile(nonCachedFile);
	CloseAllRequestedFiles(FALSE);
	sprintf(str, "$SESSION(CREATE,GMEdit /GMEdit %s /W %ld)", nonCachedFile, (DWORD)hWnd);
	ProcessText (str);
	return rtn;
}

BOOL EditTextFile (HWND hWnd,LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (415);
#endif
{
	char	Editor[256], mess[128];
	DWORD	WVer;
	int		WinVer, DosVer;
	UINT	ierr;
	BOOL	rtn = TRUE;

    if (ExistFile(Name))
    {							      	 
		 sprintf (Editor,"notepad.exe %s",Name); 
		 ExpandText (Editor);
/*		 WVer = GetVersion ();
		 WinVer = HIBYTE(LOWORD(WVer)); 
				 WinVer=10;
		 if (WinVer <95)
		 {
			 if ((ierr = WinExecEx (Editor,SW_SHOWMAXIMIZED)) < 32)
			 {  
			 	rtn = FALSE;
			 	sprintf (mess,"Error loading editor: %i",(int) ierr);
			 	GSSiMsgBox (GetFocus(),mess,0,0);  
			 }
		 }
		 else
		 {*/
			 if ((ierr = WinExec (Editor,SW_SHOW)) < 32)
			 {  
			 	rtn = FALSE;
			 	sprintf (mess,"Error loading editor: %i",(int) ierr);
			 	GSSiMsgBox (GetFocus(),mess,0,0,0);  
			 }
//			 else if (hWnd)
//			 	ShowWindow (hWnd,SW_MINIMIZE);
//		} 
		CacheAlreadyChecked (0,0,0);
	 }
{
#if ENABLETRACE
GSSiExitProg (415);
#endif
	 return rtn;
}
#if ENABLETRACE
}
#endif
}  

void ClearFullWindowBitmap (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (417);
#endif
{
	if (hWnd)
	{
		HDC	hDC = GetDC (hWnd);
		ClearMeterPrompts (hDC);
		ReleaseDC (hWnd,hDC);
	}
	if (hFullWindowBitMap && (int)hFullWindowBitMap != -1)
	{
		GSSiDeleteObject (&hFullWindowBitMap);
		GdiFlush ();
		RedisplayLastPrompt (); 
		NotifyFunction((LPVIEWPORT)-1, GF_REDRAW);

	}
{
#if ENABLETRACE
GSSiExitProg (417);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ShowCheck (HDC hDC,LPRECT pRect,BOOL Checked,LPRECT pOutRect)
{
	HBITMAP	hBmp;
	char	BMName[10];

	if (Checked)
		strcpy (BMName,"CHECKED");
	else
		strcpy (BMName,"UNCHECKED");

	if ((hBmp = LoadBitmap (hInst,BMName)))
    {
		HDIB	hDIB = BitmapToDIB (hBmp, 0,0);

		DeleteObject (hBmp);
		//SetDisplayMode (hDC, GF_SCREENMODE); 
		/*SetWindowOrgEx  ( hDC, 0, 0,0 );
		SetViewportOrgEx( hDC, 0, 0,0 );    
		SetMapMode    ( hDC, MM_TEXT );*/
		//SelectClipRgn (lpdis->hDC,0);
		//FillRect (hDC,pRect,GetStockObject(WHITE_BRUSH));
		DisplayBMInRect2 (hDC,hDIB, *pRect,0,0,0,pOutRect);
		DestroyDIB (hDIB); 
	}
	return;
}



HANDLE	EnterBlockingWindow (HWND hWndDlg)
#if ENABLETRACE
{GSSiEnterProg (418);
#endif
{
	HANDLE	hBM=0;  
	RECT	Rect;
	HDC		hDC;
	
	if (HaveBlockingWindow || !hWndMain)
{
#if ENABLETRACE
GSSiExitProg (418);
#endif
		return hBM;
}
	if (LastVP)
    	NotifyFunction (LastVP,GF_EXIT_VIEWPORT);
    LastVP = 0;  
	HaltMapDisplay (FALSE,FALSE);
	GetClientRect(hWndMain, &Rect); 
	hDC = GetDC (hWndMain);   
	hBM = SaveScreen2 (hWndMain,hDC,Rect,0,0);
	ReleaseDC (hWndMain,hDC);    
	HaveBlockingWindow = TRUE;
{
#if ENABLETRACE
GSSiExitProg (418);
#endif
	return hBM;
}
#if ENABLETRACE
}
#endif
}   

BOOL MinimizeWindowIfOverMain (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (419);
#endif
{   
	RECT	WindRect, MainRect, IntRect;
	
	GetWindowRect (hWnd,&WindRect);
	GetWindowRect (hWndMain,&MainRect);  
	if (IntersectRect (&IntRect,&WindRect,&MainRect))
		ShowWindow (hWnd,SW_MINIMIZE);
	else
{
#if ENABLETRACE
GSSiExitProg (419);
#endif
		return FALSE;
}
{
#if ENABLETRACE
GSSiExitProg (419);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void LeaveBlockingWindow(HANDLE hSavedScreen)
#if ENABLETRACE
{GSSiEnterProg (420);
#endif
{   
	RECT	Rect;
	HDC		hDC=GetDC (hWndMain);
    
    if (hSavedScreen)
    {
		RestoreScreen2 (hDC,hSavedScreen,0,FALSE);
		DestroySavedScreen (&hSavedScreen,0);
		HaveBlockingWindow = FALSE;
		IgnoreLbutton = TRUE;  
	}
	ReleaseDC (hWndMain,hDC);
{
#if ENABLETRACE
GSSiExitProg (420);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

void GSSiEndDialog(HWND hWndDlg,BOOL rtn,HANDLE hSavedScreen)
#if ENABLETRACE
{GSSiEnterProg (421);
#endif
{   
	RECT	Rect;
	HDC		hDC=GetDC (hWndMain);

	EndDialog(hWndDlg,rtn);
	RestoreScreen2 (hDC,hSavedScreen,0,FALSE); 
	if (hSavedScreen)
	{
		SaveFullWindowBitmap (hWndMain);
		DestroySavedScreen (&hSavedScreen,0);
	}
	HaveBlockingWindow = FALSE;
	IgnoreLbutton = TRUE;  
	ReleaseDC (hWndMain,hDC);
{
#if ENABLETRACE
GSSiExitProg (421);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

void FixRect (LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (422);
#endif
{          
	if (pRect->top < 0)
	{
		pRect->bottom += pRect->top;
		pRect->top = 0;
	}
	if (pRect->left < 0)
	{
		pRect->right += pRect->left;
		pRect->left = 0;
	}
{
#if ENABLETRACE
GSSiExitProg (422);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}   


MNMXCORD FactorBounds (LPMNMXCORD pRect,double Factor)
{
	MNMXCORD OutRect;  
	DPOINT	MidPoint=MinMaxMidPointD(pRect);
	double	w = (pRect->xmx - pRect->xmn) * Factor/2;
	double	h = (pRect->ymx - pRect->ymn) * Factor/2;        
	
	OutRect.xmn = MidPoint.x - w;
	OutRect.xmx = MidPoint.x + w;
	OutRect.ymn = MidPoint.y - h;
	OutRect.ymx = MidPoint.y + h;
	return OutRect;
} 
   
RECT FactorRect (LPRECT pRect,double Factor)
{
	RECT OutRect;  
	POINT	MidPoint=RectMid (pRect);
	long	w = IDNINT((pRect->right - pRect->left) * Factor/2);
	long	h = IDNINT((pRect->bottom - pRect->top) * Factor/2);        
	
	OutRect.left = MidPoint.x - w;
	OutRect.right = MidPoint.x + w;
	OutRect.top = MidPoint.y - h;
	OutRect.bottom = MidPoint.y + h;
	return OutRect;
} 
   

BOOL NormalRect (LPRECT Rect)
#if ENABLETRACE
{GSSiEnterProg (423);
#endif
{   
	long	w=(long)Rect->right - (long)Rect->left;
	long	h=(long)Rect->bottom - (long)Rect->top;
	
	if (w > INT_MAX || h > INT_MAX)
{
#if ENABLETRACE
GSSiExitProg (423);
#endif
		return FALSE;
}
	Rect->left = Rect->top = 0;
	Rect->right = w;
	Rect->bottom = h;
	
{
#if ENABLETRACE
GSSiExitProg (423);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL WindowBelongsToViewport (HWND hWnd)
{
	int i;
	
	for (i=0;i<*pNumViewports;i++)  
	{
		if (pViewports[i]->hWndDlg == hWnd)
			return TRUE;
	}
	return FALSE;
}

BOOL Is64BitMachine(void)
{
	BOOL isWOW64=TRUE;
	HANDLE hProcess = GetCurrentProcess();

	if (!IsWow64Process(hProcess, &isWOW64))
		isWOW64 = FALSE;
	return isWOW64;
}

BOOL WindowIsCovered (HWND hWnd,short opt)
#if ENABLETRACE
{GSSiEnterProg (424);
#endif
{ 
	RECT	rMyRect, rOtherRect, rDestRect;  
	HWND	hPrevWnd, hNextWnd; 
	POINT	Point1, Point2;  
	short	ii;
	
	if (opt == 1)
	{
		OSVERSIONINFOEX verinfo;
		BOOL isWOW64;
		HANDLE hProcess = GetCurrentProcess();

		if (!IsWow64Process(hProcess, &isWOW64))
			isWOW64 = FALSE;

		verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		GetVersionEx((LPOSVERSIONINFO)&verinfo);
	//	if (verinfo.dwMajorVersion > 5) //dont care if covered if Vista or higher
		if (isWOW64)
			return FALSE;
		GetClientRect (hWnd,&rMyRect); 
		Point1.x = rMyRect.left;
		Point1.y = rMyRect.top;
		Point2.x = rMyRect.right;
		Point2.y = rMyRect.bottom;
		ClientToScreen (hWnd,&Point1);
		ClientToScreen (hWnd,&Point2);
		rMyRect.left = Point1.x;
		rMyRect.top = Point1.y;
		rMyRect.right = Point2.x;
		rMyRect.bottom = Point2.y;
	}
	else
	{
		GetWindowRect(hWnd, &rMyRect); 
		FixRect (&rMyRect);
	} 
      /*  Start from the current window and use the GetWindow()
       *  function to move through the previous window handles.
       */
  	for (hPrevWnd = hWnd;
      	(hNextWnd = GetWindow(hPrevWnd, GW_HWNDPREV)) != NULL;
      	 hPrevWnd = hNextWnd)
    {
      /*  Get the window rectangle dimensions of the window that
       *  is higher Z-Order than the application's window.
       */
    	GetWindowRect(hNextWnd, &rOtherRect);   
        FixRect (&rOtherRect);
      /*  Check to see if this window is visible and if intersects
       *  with the rectangle of the application's window. If it does,
       *  call MessageBeep(). This intersection is an area of this
       *  application's window that is not visible.
       */
		if (!IsRectEmpty(&rOtherRect) && IsWindowVisible(hNextWnd))
		{
			HWND Owner = GetWindow(hNextWnd, GW_OWNER);
			if (IntersectRect(&rDestRect, &rMyRect, &rOtherRect))
			{
				char txt[128];
				GetWindowText(hNextWnd, txt,120);
				BOOL wbtv = WindowBelongsToViewport(hWnd);
				if ((Owner != hWnd) || (opt == 1 && !wbtv))
				{
#if ENABLETRACE
					GSSiExitProg(424);
#endif
					return TRUE;
				}
			}
		}
	}
{
#if ENABLETRACE
GSSiExitProg (424);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

__int64 llFileSeek (HANDLE hf, __int64 distance, DWORD MoveMethod)
{
   LARGE_INTEGER li;

   li.QuadPart = distance;

   li.LowPart = SetFilePointer (hf, li.LowPart, &li.HighPart, MoveMethod);

   if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR)
   {
      li.QuadPart = -1;
   }

   return li.QuadPart;
}

LONG GSSillseek (HFILE Fid, LONG loc, int opt)
{
	if (Fid < 0 || Fid >= MAXFILEHANDLES)
		return -1;
	if (FidMemLen[Fid])
	{   
		switch (opt)
		{
			case 0:
				OpenFilePosition[Fid] = loc;
			break;
			case 1:
				OpenFilePosition[Fid] += loc;
			break;
			case 2:
				OpenFilePosition[Fid] = OpenFileLength[Fid] + loc;
			break;
		}
		OpenFilePosition[Fid] = max (0,OpenFilePosition[Fid]);
		return OpenFilePosition[Fid];
	}
	if (JournalFileFid[Fid] != HFILE_ERROR)
	{   
		switch (opt)
		{
			case 0:
				OpenFilePosition[Fid] = loc;
			break;
			case 1:
				OpenFilePosition[Fid] += loc;
			break;
			case 2:
				OpenFilePosition[Fid] = OpenFileLength[Fid] + loc;
			break;
		}				
		return OpenFilePosition[Fid];
	}
	if ((UseMappedFiles && Fid < MAXFILEHANDLES) && FidIsMapped[Fid])
	{
		switch (opt)
		{
			case 0:
				OpenFilePosition[Fid] = loc;
			break;
			case 1:
				OpenFilePosition[Fid] += loc;
			break;
			case 2:
				OpenFilePosition[Fid] = OpenFileLength[Fid] + loc;
			break;
		}
		if (OpenFilePosition[Fid] < 0)
			OpenFilePosition[Fid] = 0;
		return OpenFilePosition[Fid];
	}
	else if (OpenFileFid[Fid] != HFILE_ERROR)
	{
		int ii,pos=_lseek (OpenFileFid[Fid],loc,opt);
		if (pos<0)
			ii=_llseek (OpenFileFid[Fid],loc,opt);
		ii=1;
		return pos;
	}
	else
		return -1;
}
 
LONGLONG GSSillseek2 (HFILE Fid, LONGLONG loc, int opt)
{   
	LONGLONG	rtnloc;
	
	if (OpenFileFid[Fid] == HFILE_ERROR)
		return 0;
	rtnloc = _lseeki64 (OpenFileFid[Fid],loc,opt); 
	return rtnloc;
}
 
long  GSSilread(HFILE Fid, void _huge* ptr, long len)
{   
	
	if (FidMemLen[Fid])
	{
		HPSTR	pFile=GlobalLock (OpenFileHandle[Fid]);
		long	endpos = min (OpenFileLength[Fid],OpenFilePosition[Fid]+len);
		
		len = max (0,endpos - OpenFilePosition[Fid]);
		if (len)
		{     
			memmove (ptr,(HPSTR)(pFile+OpenFilePosition[Fid]),len);
			OpenFilePosition[Fid] = endpos;
		}
		GlobalUnlock (OpenFileHandle[Fid]);
		NumMemRead += len;
		return len;
	}   
	if (FidIsMapped[Fid])
	{
		HPSTR	pFile=(HPSTR)FidPtr[Fid];
		long	endpos = min (OpenFileLength[Fid],OpenFilePosition[Fid]+len);
		
		len = endpos - OpenFilePosition[Fid];
		if (len > 0)
		{     
			memmove (ptr,(HPSTR)(pFile+OpenFilePosition[Fid]),len);
			OpenFilePosition[Fid] = endpos;
		}
		else
			len = 0;
		NumMemRead += len;
		return len;
	}   
	return BigRead (Fid,ptr,len);
}

UINT  WINAPI GSSilwrite(HFILE Fid, HPSTR ptr, UINT len)
{
	return BigWrite (Fid,ptr,len,-1);
}  

long NumRowsInTxtFile (HFILE Fid)
{   
    HANDLE	hSTR = GSSiGlobAlloc ( 141,GMEM_MOVEABLE,4096);
    LPSTR	Value = GlobalLock (hSTR);  
	long	rtn=0;
	long	Offset=GSSillseek (Fid,0,1);
	
	if (Fid != HFILE_ERROR)	     
	while (fgetstring (Value,4090,Fid))
		rtn++;
	GSSillseek (Fid,Offset,0); 
	GSSiGlobUlFree (&hSTR);  
	return rtn;
}

int GSSiMsgBox (HWND hWnd, LPSTR MessIn, LPSTR TitleIn, UINT Flag,LPSTR Position)
{   
	//HANDLE	hMem=GSSiGlobAlloc (1545,GMEM_MOVEABLE,4096*2+256+512);
	LPSTR	Title=malloc (4096*2+256+512);
	LPSTR	Mess=Title+4096;
	LPSTR	File=Mess+4096,Line=File+256;
	HFILE		Fid; 
	int	irc;

//	AppendFile2 ("c:\\messagelog.txt",MessIn);
//	AppendFile2 ("c:\\messagelog.txt",TitleIn);
//	SetWindowText (hWndMain,MessIn);
	if (!Position || !*Position)
		Flag = Flag|MB_TASKMODAL;
	if (TitleIn)
	{
		_fstrcpy (Title,TitleIn);
//		ExpandText (Title); 
	}
	else
		_fstrcpy (Title,"Error");
	if (MessIn)
	{
		_fstrcpy (Mess,MessIn);
		ExpandText (Mess); 
	}
	else
		*Mess = 0;
	if (BackgroundTask)
	{   
		OFSTRUCTGM	OFStruct;
		BOOL		SaveCP = ContinueProcessing;

		SetContinueProcessing ( TRUE);

		GetGlobalCVal ("[%BACKGROUNDLOGFILE]",File,"bkglog.txt");
		//GetShortPathName2 (File,128);
		Fid = OpenFileGM (File,&OFStruct,OF_READWRITE);
		if (Fid == HFILE_ERROR)
			Fid = OpenFileGM (File,&OFStruct,OF_CREATE);
		if (Fid != HFILE_ERROR) 
		{
			int	len;
			char	DateTime[64]="$CAL([%SYS_CLOCK])";

			ExpandText (DateTime);
			_llseek (Fid,0,2); 
			sprintf (Line,"%s Message:%s Title:%s",DateTime,Mess,Title);
		    len=_fstrlen(Line);
			if (len)
				_lwrite (Fid,(char *)Line,len); 
			_lwrite (Fid,"\r\n",2);
			_lclose (Fid);  
		}
		free (Title);
		SetContinueProcessing ( SaveCP);
		return 0;
	}
	else 
	{
		BOOL	SaveHaveBlockingWindow = HaveBlockingWindow;
		HaveBlockingWindow = TRUE;
		if (!Position || !*Position)
			irc = MessageBox (hWnd,Mess,Title,Flag); 
		else
			irc = MessageBoxAtPosition (hWnd,Mess,Title,Flag,Position);
		HaveBlockingWindow = SaveHaveBlockingWindow;
		free (Title);
		return irc;
	}
}

/************************************************************************/
/*  cwCenter Function                                                   */
/*                                                                      */
/*  centers a window based on the client area of its parent             */
/*  top >  0         - adjust top up                                    */
/*  top == 0         - center in parent                                 */
/*  top == SHRT_MAX  - center in desktop                                */
/*  top == -1		 - left justifies window in parent on cursor        */
/*  top == -2		 - center on cursor in parent						*/
/*  top == -3        - center at bottom - 16                            */
/*  top == -4        - center at bottom - 16 of CurView                 */
/************************************************************************/

void cwCenter(HWND hWnd, int top)
#if ENABLETRACE
{GSSiEnterProg (452);
#endif
{
 POINT      pt;
 RECT       swp;
 RECT       rParent;
 int        iwidth;
 int        iheight; 
 HWND		hPWnd;  
 BOOL		IsClient=FALSE;

 /* get the rectangles for the parent and the child                     */
 if (!GetWindowRect(hWnd, &swp))
	 return;

begin:

 if (!hWndMain || top == SHRT_MAX)
 {
 	top = 0;
 	hPWnd = GetDesktopWindow();  
 	IsClient = FALSE;
 }
 else
 	hPWnd = hWndMain;
// GetClientRect(hPWnd, &rParent);
 GetWindowRect(hPWnd, &rParent);

 /* calculate the height and width for MoveWindow                       */
 iwidth = swp.right - swp.left;
 iheight = swp.bottom - swp.top;

 if (top == -3)//center at bottom
 {
	 pt.y = rParent.bottom - iheight - 16;
	 if (!IsRectEmpty(&PromptRect))
		 pt.y -= RECTHEIGHT(&PromptRect);
	 pt.x = RECTWIDTH(&rParent) / 2 - iwidth / 2;
	 goto Exit;
 }
 if (top == -4)//center at bottom of vp
 {
	 pt.y = CurView->DrawRect.top + CurView->DrawRect.bottom - iheight - 16;
	 pt.x = CurView->DrawRect.left + RECTWIDTH(&CurView->DrawRect) / 2 - iwidth / 2;
	 goto Exit;
 }
 else if (top<0)
{
	GetCursorPos (&pt);
	if (top==-1)
		pt.x=0;
}
else
{
	 /* find the center point and convert to screen coordinates             */
	 pt.x = (rParent.right + rParent.left) / 2;
	 pt.y = (rParent.bottom + rParent.top) / 2; 
	 if (IsClient)
	 	ClientToScreen(hWndMain, &pt);
} 

 /* calculate the new x, y starting point                               */
 pt.x = max (0,pt.x - (iwidth / 2));
 pt.y = max (0,pt.y - (iheight / 2));

 /* top will adjust the window position, up or down                     */
 if(top>0)
   pt.y = pt.y + top;
 else
 {
	 if (pt.x + iwidth > rParent.right)
		 pt.x = rParent.right - iwidth;
	 if (pt.y < rParent.top)
		 pt.y = rParent.top;
	 if (pt.x < 0 || pt.y < 0)
	 {
		 top = SHRT_MAX;
		 goto begin;
	 }
 }

 Exit:
 /* move the window                                                     */
 MoveWindow(hWnd, pt.x, pt.y, iwidth, iheight, FALSE);
 //SetWindowPos (hWnd,0,pt.x, pt.y, iwidth, iheight,SWP_NOZORDER|SWP_NOOWNERZORDER);
{
#if ENABLETRACE
GSSiExitProg (452);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}


/************************************************************************/
/*  CwUnRegisterClasses Function                                        */
/*                                                                      */
/*  Deletes any refrences to windows resources created for this         */
/*  application, frees memory, deletes instance, handles and does       */
/*  clean up prior to exiting the window                                */
/*                                                                      */
/************************************************************************/

void CwUnRegisterClasses(void)
#if ENABLETRACE
{GSSiEnterProg (453);
#endif
{
 WNDCLASS   wndclass;    /* struct to define a window class             */
 _fmemset(&wndclass, 0x00, sizeof(WNDCLASS));

 UnregisterClass(szAppName, hInst);
 // added by lda then removed when we started using ODBC
 //UnregisterClass("ClientDDEWndClass", hInst);
 //UnregisterClass("ServerWClass", hInst);
{
#if ENABLETRACE
GSSiExitProg (453);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}    /* End of CwUnRegisterClasses                                      */

BOOL GSSiPeekMessage(
    __out LPMSG lpMsg,
    __in_opt HWND hWnd,
    __in UINT wMsgFilterMin,
    __in UINT wMsgFilterMax,
    __in UINT wRemoveMsg)
{
	BOOL	rtn = PeekMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax,wRemoveMsg);
	int		ii;

	if (TraceWnd2)
		ii=1;
	return rtn;
}

BOOL GSSiGetMessage(
    __out LPMSG lpMsg,
    __in_opt HWND hWnd,
    __in UINT wMsgFilterMin,
    __in UINT wMsgFilterMax)
{
	BOOL	rtn = GetMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax);
	int	ii;

	if (TraceWnd2)
		ii=1;
	return rtn;
}
#define OBS_COMPLETELYCOVERED       0
#define OBS_PARTIALLYVISIBLE        1
#define OBS_COMPLETELYVISIBLE       2

int GetClientObscuredness(HWND hwnd)
{
    HDC hdc;
    RECT rc, rcClient;
    int iType;

    hdc = GetDC(hwnd);
    iType = GetClipBox(hdc, &rc);

    ReleaseDC(hwnd, hdc);

    if (iType == NULLREGION)
        return OBS_COMPLETELYCOVERED;
    if (iType == COMPLEXREGION)
        return OBS_PARTIALLYVISIBLE;

    GetClientRect(hwnd, &rcClient);
    if (EqualRect(&rc, &rcClient))
        return OBS_COMPLETELYVISIBLE;

    return OBS_PARTIALLYVISIBLE;
}
#pragma optimize( "", on )

