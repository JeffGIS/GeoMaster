#include "graphint.h" 

#include "gmextern.h"

BOOL CopyMapFile (LPSTR Name, LPSTR FromName,long Offset,FILEINDEXENTRY CurFileIndexEntry)
{   
	char	str[256],FullName[128], Dir[150], FName[16], Ext[8], NewName[128], FileName[128],IndexName[128], FromFileName[128]; 
	char	DataLoc[MAX_PATH]="[%DL]test.txt", Drive[16], DLDrive[16], DLDir[128];
	LPSTR	StartDir, EndDir, pDir;  
	OFSTRUCT	OFStruct;
	BOOL	Replace;      
	HFILE	FromFid, GCIFid=HFILE_ERROR, FidIndex;
	FILEINDEXENTRY	IndexEntry, IndexEntry2; 
	LPFILEINDEX		lpFI;
	LPFILEINDEXENTRY pEntry=&IndexEntry2;                
    LPFILEINDEX lpIndex;
	char	CurFileName[128];   
    MNMXCORD	FileBounds; 
    BOOL	CurHeaderWritten;  
	HANDLE	hlpFI;  
	short	lhead; 
	char	biheadspace[2048];
	LPBITMAPINFOHEADER	biHead=(LPBITMAPINFOHEADER)biheadspace;
    short     Version=2, NumLevs=1, CurLev=0, n;
    long    Signature=80251, FirstHeaderOffset, LastHeaderOffset, CurOffset=0;
    long	TotCopyLen; 
    DWORD	Err;
	
	ExpandText (DataLoc);
	switch (DoMapCopy)
	{
		case 0:
			return FALSE;
		case 1: 
			BigWrite (CopyFID,(HPSTR)Name,128,-1);
			BigWrite (CopyFID,(HPSTR)FromName,128,-1);
			BigWrite (CopyFID,(HPSTR)&Offset,4,-1);
			BigWrite (CopyFID,(HPSTR)&CurFileIndexEntry,sizeof(FILEINDEXENTRY),-1);
			return TRUE;
		case 2:
			Replace=TRUE; 
			break;
		case 3:
			Replace=FALSE;   
		case 4:
			NumCopyFiles++;
			sprintf (str,"%s\r%ld files",PltName,NumCopyFiles);  
	   		StatusWindowUpdate (NULL,str, 1,0);
			return TRUE;
		break;
	}
	
	CopyFID = GSSiOpenFile ("copylist.txt",&OFStruct,OF_READ);
	TotCopyLen = GSSifilelength (CopyFID);
	
	while (ContinueProcessing && BigRead (CopyFID,FileName,128))
	{   
		BigRead (CopyFID,(HPSTR)FromFileName,128);
		BigRead (CopyFID,(HPSTR)&Offset,4);
		BigRead (CopyFID,(HPSTR)&IndexEntry,sizeof(FILEINDEXENTRY)); 
		if (Offset < 0 || StringEndsWith (FileName,"index"))
			_fstrcpy (FileName,FromFileName); 
		_fstrcpy (Dir,FileName); 
		if (_fstricmp (FileName,CurFileName))
		{   
			if (GCIFid != HFILE_ERROR) 
			{
				GSSiChangeLength (FidIndex,CurOffset);
				GSSillseek (FidIndex,0,2);
				BigWrite(FidIndex,(HPSTR)&Signature,4,-1);
				BigWrite(FidIndex,(HPSTR)&Version,2,-1);
				GSSillseek (FidIndex,0,0);
				BigWrite (FidIndex,(HPSTR)&FileBounds,sizeof(MNMXCORD),-1); 
				if (!CurHeaderWritten)
				{ 
					 GSSillseek (FidIndex,LastHeaderOffset,0);
					 BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
				}
				GSSillseek (FidIndex,FirstHeaderOffset,0);
				BigRead (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH);
				lpFI->EndOffset = CurOffset;
				GSSillseek (FidIndex,FirstHeaderOffset,0);
				BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
				GSSiGlobUlFree (&hlpFI);
				GSSiClose (GCIFid); 
				GCIFid = HFILE_ERROR;
				GSSiClose (FidIndex);
			}
			if (!_fstricmp (FileName,"-DONE-"))
				goto Exit;
			ExpandText (Dir);
			_fullpath (FullName,Dir,sizeof(FullName));
			EndDir = _fstrchr (FullName,0);
			EndDir++;
			*EndDir = 0; 
			_splitpath (DataLoc,DLDrive,DLDir,NULL,NULL);
			_splitpath (FullName,Drive,Dir,FName,Ext);     
			n = _fstrlen (DLDir);
			pDir = Dir;
			if (!_fstricmp (DLDrive,Drive) && !_fstrnicmp (DLDir,Dir,n))
				pDir += n;
			_fstrcpy (NewName,MapCopyPath);
			makedirectories (NewName,TRUE,FALSE);
			StartDir = pDir;
			while (*StartDir)
			{   
				short	inc = 1;
				
				if (!_fstricmp (StartDir,"\\"))
					inc = 0;
				EndDir = _fstrchr ((LPSTR)(StartDir+inc),'\\');
				if (!EndDir)
					EndDir = _fstrchr (StartDir,0); 
				*EndDir = 0;
				if (*StartDir && *StartDir != '\\')
					_fstrcat (NewName,"\\");
				_fstrcat (NewName,StartDir);
				GSSiMakeDir (NewName,&Err);
				StartDir = ++EndDir;
			} 
			_fstrcat (NewName,"\\");
			_fstrcpy (IndexName,NewName);
			_fstrcat (IndexName,"index"); 
			if (!_fstricmp (FName,"filelist") && !_fstricmp (Ext,".txt"))
				_fstrcat (NewName,"orthos.gci");
			else
			{
				_fstrcat (NewName,FName);
				_fstrcat (NewName,Ext);
			} 
			if (Offset > 0)
			{
				GCIFid = GSSiOpenFile (NewName,&OFStruct,OF_CREATE);
				FidIndex = GSSiOpenFile (IndexName,&OFStruct,OF_CREATE);
				FromFid = GSSiOpenFile (FromFileName,&OFStruct,OF_READ);
				BigRead (FromFid,(HPSTR)&lhead,2);
				BigWrite (GCIFid,(HPSTR)&lhead,2,-1);
				BigRead (FromFid,(HPSTR)biHead,lhead);
				BigWrite (GCIFid,(HPSTR)biHead,lhead,-1);
				BigRead (FromFid,(HPSTR)&lhead,2);
				BigWrite (GCIFid,(HPSTR)&lhead,2,-1);
				BigRead (FromFid,(HPSTR)biHead,lhead);
				BigWrite (GCIFid,(HPSTR)biHead,lhead,-1);
	            GSSiClose (FromFid);
				hlpFI = GSSiGlobAlloc(1743,GHND,sizeof(FILEINDEX)+sizeof(FILEINDEXENTRY)); 
				lpFI = (LPFILEINDEX) GlobalLock(hlpFI);    
				lpFI->CurrentEntry=(FILEINDEXENTRY *) &lpFI->FirstIndex;
				lpFI->Type = 5;   
				lpFI->UsesTimes = FALSE;     
	                 
				DBoundsInit (&FileBounds);
				BigWrite (FidIndex,(HPSTR)&FileBounds,sizeof(MNMXCORD),-1);  
				FirstHeaderOffset = LastHeaderOffset = GSSillseek (FidIndex,0,1);    
                DBoundsInit (&lpFI->Bounds);   
				lpFI->NumFiles=0;
				lpFI->Length=0; 
				BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1); 
				CurHeaderWritten = TRUE; 
			}
		}
		_fstrcpy (CurFileName,FileName);	
		if (Offset > 0)		
        {
            long	TotLen=0, CurLoc=1, FileListLen, Frame, lRec;
			HANDLE	hData;
			HPSTR	pData;  
			ULONG	NextLen;
					 
            Frame = GSSillseek (GCIFid,0,1);   
			FromFid = GSSiOpenFile (FromFileName,&OFStruct,OF_READ);
            GSSillseek (FromFid,Offset,0);
            BigRead (FromFid,(HPSTR)&lRec,4);
            NextLen = (ULONG)Frame + (ULONG)lRec + (ULONG)1024;  
            if (NextLen > (ULONG)LONG_MAX)
            { 
				GSSiClose (FromFid);
                MessageBox(0,"Maximum file size exceeded",
                           0,MB_OK|MB_ICONQUESTION|MB_TASKMODAL);
				return FALSE;
			} 
    		*pEntry = IndexEntry;                  
            AddMinMaxD (&lpFI->Bounds,&IndexEntry.Bounds);
           	sprintf (_fstrrchr(pEntry->Name,'@'),"@%ld",Frame);
            pEntry->Len = _fstrlen (pEntry->Name) + 1 +
                            sizeof(FILEINDEXENTRY) - sizeof(pEntry->Name);
            lpFI->Length += pEntry->Len;
            if (lpFI->Length >(USHRT_MAX - sizeof(FILEINDEXENTRY)))
            { 
				GSSiClose (FromFid); 
                MessageBox(0,"Too many files in index - create sub indexes",
                           0,MB_OK|MB_ICONQUESTION|MB_TASKMODAL);
                return FALSE;
            }
            lpFI->NumFiles++;
            BigWrite (FidIndex,(HPSTR)pEntry,pEntry->Len,-1);       
                                     
			BigWrite (GCIFid,(HPSTR)&lRec,4,-1);  
			hData = GSSiGlobAlloc (1234,GMEM_MOVEABLE,lRec+1024);
			pData = GlobalLock (hData);
			BigRead (FromFid,pData,lRec); 
			BigWrite (GCIFid,(HPSTR)pData,lRec,-1);
			GSSiClose (FromFid); 
            GSSiGlobUlFree (&hData);      
                  
            AddMinMaxD (&FileBounds,&lpFI->Bounds); 
        	CurOffset = GSSillseek (FidIndex,0,1);
            if (lpFI->NumFiles > 255)
            {   
            	lpFI->NextHeaderOffset = CurOffset;
            	GSSillseek (FidIndex,LastHeaderOffset,0);
				BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
				GSSillseek (FidIndex,CurOffset,0);
				DBoundsInit (&lpFI->Bounds);   
				lpFI->NumFiles=0;
				lpFI->Length=0; 
            	LastHeaderOffset = CurOffset;
				BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
				CurHeaderWritten = TRUE;
            }
            else
				CurHeaderWritten = FALSE;
        }
	    else if (Replace || !ExistFile(NewName))  
		{
			copyfile (NewName,FullName,FALSE,0,0,0,0,0,0);
			ConvertFileCoordinates (NewName,0,0,1); 
			if (!ContinueProcessing) 
			{
				GSSiRemove (NewName);
				goto Exit;
			}
		}
		StatusWindowUpdate (NULL,FromFileName, TotCopyLen, GSSillseek (CopyFID,0,1));
	}
Exit: 
	ContinueProcessing = TRUE;
	GSSiClose (CopyFID);
	
	return TRUE;
} 

HANDLE SortAreasBySize (HPSTR *rec,long len)
{
	return 0;
}


