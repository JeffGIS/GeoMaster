#include <windows.h>
#include "shr.h"
#include <strsafe.h>
#include <wininet.h>



__int64 HighLowToint64(DWORD HighPart,DWORD LowPart);
__int64 FileTimeToint64(FILETIME ft);
long Time64toTime32 (time_t time64);

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "user32.lib")

#define  FTP_FUNCTIONS_BUFFER_SIZE          MAX_PATH+8

/*HINTERNET InternetOpen(
  _In_  LPCTSTR lpszAgent,
  _In_  DWORD dwAccessType,
  _In_  LPCTSTR lpszProxyName,
  _In_  LPCTSTR lpszProxyBypass,
  _In_  DWORD dwFlags
);*/

__int64 FTPGetFileSize (HANDLE hConnect)
{
	DWORD fileSizeHigh;
	DWORD fileSizeLow = FtpGetFileSize(hConnect,&fileSizeHigh);
	__int64 fileSize64 = HighLowToint64(fileSizeHigh,fileSizeLow);

	return fileSize64;
}

BOOL SetInternetErrorVar (LPSTR errorVarName)
{
	int ierr = GetLastError (), lnerr;
	DWORD internetErr;
	char errDesc[4096];
	DWORD lenErrDesc=4000;
	BOOL rc;
	GetSystemErrMessage(GetLastError(), errDesc);
	lnerr = strlen(errDesc);
	rc = InternetGetLastResponseInfo(&internetErr, strchr(errDesc,0), &lenErrDesc);
	if (errorVarName && *errorVarName)
	{
		if (rc && lenErrDesc)
		{
			errDesc[min(1023,lnerr+lenErrDesc)] = 0;
			SetGlobalValue (errorVarName,errDesc); 
		}
		else if (lnerr)
			SetGlobalValue(errorVarName, errDesc);
		else
			SetGlobalValue (errorVarName,"No error description available"); 
	}
	return rc;
}

BOOL FTPClose (HANDLE h)
{
	BOOL rtn = InternetCloseHandle (h);

	return rtn;
}

HANDLE FTPOpen (LPCSTR lpszServerName,LPCSTR lpszUsername,LPCSTR lpszPassword,LPCSTR directory,LPSTR errorVarName)
{
	HINTERNET hInternet;
    INTERNET_PORT nServerPort = INTERNET_DEFAULT_FTP_PORT;
    DWORD dwService = INTERNET_SERVICE_FTP;
    DWORD dwFlags = INTERNET_FLAG_PASSIVE;
    DWORD_PTR dwContext = 0;
	HINTERNET ic = NULL;

	hInternet = InternetOpen ("GeoMaster",INTERNET_OPEN_TYPE_DIRECT,NULL,NULL,0);

	if (hInternet)
	{

			ic = InternetConnect (hInternet,
								  lpszServerName,
								  nServerPort,
								  lpszUsername,
								  lpszPassword,
								  dwService,
								  dwFlags,
								  dwContext);
			if (ic && directory && *directory)
			{
				if (!FtpSetCurrentDirectory(ic,directory))
				{
					SetInternetErrorVar (errorVarName);
					InternetCloseHandle (ic);
					ic = NULL;
				}
			}
			else
				SetInternetErrorVar (errorVarName);

	}
	return ic;
}

HANDLE ListFtpDir(HANDLE hConnection,HANDLE hFind,LPSTR pWildCard,
				  DWORD dwFindFlags,LPSTR fileName,
				  LPDWORD pFileSize,LPDWORD pLastUpdateTime,LPBOOL pisDirectory,
				  LPSTR errorVarName)
{
  LPWIN32_FIND_DATA pdirInfo = calloc (1,sizeof(WIN32_FIND_DATA)*8);
  DWORD           dwError;
  BOOL            retVal = FALSE;
  TCHAR           szMsgBuffer[FTP_FUNCTIONS_BUFFER_SIZE];
  TCHAR           szFName[FTP_FUNCTIONS_BUFFER_SIZE];
  DWORD_PTR		  dwContext=0;
  __int64	fileSize64;
  
  if (fileName)
	  *fileName = 0;
  if (pFileSize)
	  *pFileSize = 0;
  if (pLastUpdateTime)
	  *pLastUpdateTime = 0;
  if (pisDirectory)
	  *pisDirectory = FALSE;
  if (!hFind && pWildCard && *pWildCard)
  {
	  hFind = FtpFindFirstFile( hConnection, pWildCard,pdirInfo, dwFindFlags,0 );
	  if (!hFind)
	  {
		  if (GetLastError() != ERROR_NO_MORE_FILES)
			SetInternetErrorVar(errorVarName);
		  free (pdirInfo);
		  return NULL;
	  }
  }
  else
  {
	  if (!InternetFindNextFile( hFind,pdirInfo ))
	  {
		free (pdirInfo);
		InternetCloseHandle(hFind);
		return NULL;
	  }
  }

  strcpy (fileName,pdirInfo->cFileName);
  if (pFileSize)
  {
	  fileSize64 = HighLowToint64(pdirInfo->nFileSizeHigh,pdirInfo->nFileSizeLow);
	  *pFileSize = fileSize64;
  }
  if (pLastUpdateTime)
  {
	  __int64 itime = FileTimeToint64 (pdirInfo->ftLastWriteTime);
	  *pLastUpdateTime = Time64toTime32 (itime);
  }

  if (pisDirectory && pdirInfo->dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	  *pisDirectory = TRUE;
  free (pdirInfo);
  return hFind;
}

BOOL FTPGetFile(HANDLE hConnect,LPCTSTR lpszRemoteFile,LPCTSTR lpszNewFile,BOOL replace,BOOL showStatus,LPSTR errorVarName)
{
	BOOL rtn=FALSE;
	
	if (showStatus)
	{
		HANDLE handle = FtpOpenFile (hConnect,lpszRemoteFile,GENERIC_READ,FTP_TRANSFER_TYPE_BINARY,0);
		if (handle)
		{
			__int64 size = FTPGetFileSize (handle);
			DWORD Tot = (DWORD)size;
			DWORD Done = 0;
			LPSTR Title, Mess;
			DWORD dwNumberOfBytesToRead=USHRT_MAX;
			DWORD numBytesRead;
			HFILE Fid;
			HANDLE hBuffer;
			LPSTR pBuffer, leafName;

			if (!(leafName = strrchr (lpszRemoteFile,'/')))
				leafName=(LPSTR)lpszRemoteFile;
			Fid = GSSiOpenFile ((LPSTR)lpszNewFile,0,OF_CREATE);
			if (Fid == HFILE_ERROR)
			{
				if (errorVarName && *errorVarName)
					SetGlobalValue (errorVarName,"Unable to create output file"); 
				InternetCloseHandle (handle);
				return FALSE;
			}
			hBuffer = GSSiGlobAlloc (0,GMEM_MOVEABLE,dwNumberOfBytesToRead+32);
			pBuffer = GlobalLock (hBuffer);
			rtn = TRUE;
			CreateStatusWind (0,1,leafName);
			while (rtn && Done < Tot &&	StatusWindowUpdate (leafName,0,Tot,Done))
			{
				rtn = InternetReadFile(handle,pBuffer,dwNumberOfBytesToRead,&numBytesRead);
				Done += numBytesRead;
				BigWrite (Fid,pBuffer,numBytesRead,-1);
			};
			StatusWindowUpdate(leafName, 0,Tot, Tot);
			DestroyStatusWindow (0);
			GSSiClose (Fid);
			GSSiGlobUlFree (&hBuffer);
			InternetCloseHandle (handle);
		}
	}
	else
	{
		makedirectories((LPSTR)lpszNewFile, FALSE, FALSE);
		rtn = FtpGetFile(hConnect, lpszRemoteFile, lpszNewFile, !replace,
			FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_BINARY, 0);
	}
	if (!rtn)
		SetInternetErrorVar (errorVarName);
	return rtn;
}

BOOL FTPPutFile(HANDLE hConnect,LPCTSTR lpszRemoteFile,LPCTSTR lpszLocalfile,BOOL replace,BOOL showStatus,LPSTR errorVarName)
{
	BOOL rtn=FALSE;
	char localPath[MAX_PATH];

	strcpy(localPath, lpszLocalfile);
	ExpandText(localPath);

	if (!replace)
	{
		//check for existing file
	}
	if (showStatus)
	{
		HANDLE handle = FtpOpenFile(hConnect, lpszRemoteFile, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY, 0);
		if (handle)
		{
			__int64 size = GSSiLength((LPSTR)lpszLocalfile);
			DWORD Tot = (DWORD)size;
			DWORD Done = 0;
			LPSTR Title, Mess;
			DWORD dwNumberOfBytesToRead = USHRT_MAX;
			DWORD numBytesRead;
			DWORD dwNumberOfBytesToWrite, numBytesWritten;
			HFILE Fid;
			HANDLE hBuffer;
			LPSTR pBuffer, leafName;

			if (!(leafName = strrchr(lpszRemoteFile, '/')))
				leafName = (LPSTR)lpszRemoteFile;
			Fid = GSSiOpenFile((LPSTR)lpszLocalfile, 0, OF_READ);
			if (Fid == HFILE_ERROR)
			{
				if (errorVarName && *errorVarName)
					SetGlobalValue(errorVarName, "Unable to open source file");
				InternetCloseHandle(handle);
				return FALSE;
			}
			hBuffer = GSSiGlobAlloc(0, GMEM_MOVEABLE, dwNumberOfBytesToRead + 32);
			pBuffer = GlobalLock(hBuffer);
			rtn = TRUE;
			CreateStatusWind(0, 1, leafName);
			while (rtn && Done < Tot &&	StatusWindowUpdate(leafName, 0, Tot, Done))
			{
				dwNumberOfBytesToWrite = numBytesRead = BigRead(Fid, pBuffer, dwNumberOfBytesToRead);
				rtn = InternetWriteFile(handle, pBuffer, dwNumberOfBytesToWrite, &numBytesWritten);
				if (numBytesRead != dwNumberOfBytesToWrite)
					rtn = 0;
				Done += numBytesRead;
			};
			StatusWindowUpdate(leafName, 0,Tot, Tot);
			DestroyStatusWindow(0);
			GSSiClose(Fid);
			GSSiGlobUlFree(&hBuffer);
			InternetCloseHandle(handle);
		}
	}
	else
	{
		rtn = FtpPutFile(hConnect, localPath, lpszRemoteFile, FTP_TRANSFER_TYPE_BINARY, 0);
	}
	if (!rtn)
		SetInternetErrorVar (errorVarName);
	return rtn;
}
BOOL FTPDeleteFile(HANDLE hConnect,LPCTSTR lpszRemoteFile,LPSTR errorVarName)
{
	BOOL rtn=FALSE;
	
	rtn = FtpDeleteFile(hConnect,lpszRemoteFile);
	if (!rtn)
		SetInternetErrorVar (errorVarName);
	return rtn;
}

BOOL FTPCreateDirectory(HANDLE hConnect,LPCTSTR lpszRemoteDir,LPSTR errorVarName)
{
	BOOL rtn=FALSE;
	
	rtn = FtpCreateDirectory (hConnect,lpszRemoteDir);
	if (!rtn)
		SetInternetErrorVar (errorVarName);
	return rtn;
}
BOOL FTPGetDirectory(HANDLE hConnect,LPSTR lpszRemoteDir,LPSTR errorVarName)
{
	BOOL rtn=FALSE;
	DWORD lnDir=MAX_PATH;

	rtn = FtpGetCurrentDirectory (hConnect,lpszRemoteDir,&lnDir);
	if (!rtn)
		SetInternetErrorVar (errorVarName);
	return rtn;
}

BOOL FTPSetDirectory(HANDLE hConnect,LPCTSTR lpszRemoteDir,LPSTR errorVarName)
{
	BOOL rtn=FALSE;

	rtn = FtpSetCurrentDirectory (hConnect,lpszRemoteDir);
	if (!rtn)
		SetInternetErrorVar (errorVarName);
	return rtn;
}


/*BOOL FtpCommand(
  _In_   HINTERNET hConnect,
  _In_   BOOL fExpectResponse,
  _In_   DWORD dwFlags,
  _In_   LPCTSTR lpszCommand,
  _In_   DWORD_PTR dwContext,
  _Out_  HINTERNET *phFtpCommand
);*/

