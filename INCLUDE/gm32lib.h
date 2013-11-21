

/*
HFILE GSSiOpenFile (LPSTR Name,LPOFSTRUCT pOFStruct,UINT Mode);
HFILE GSSiClose (HFILE Fid);
HFILE GSSiClose2 (LPHFILE pFid);
LONG GSSillseek (HFILE Fid, LONG loc, int opt); 
DWORD GSSillseek2 (HFILE Fid, DWORD loc, int opt);
long  GSSilread(HFILE Fid, void _huge* ptr, long len);
LPSTR fgetstring (LPSTR lpStr, USHORT len, HFILE Fid);
BOOL fputstring(LPSTR lpStr, HFILE Fid);
void BufWrite (HPSTR *pBuf,LPLONG plBuf,HPSTR data,long ldata);
HFILE SetMemFile (HANDLE handle,long len);
HFILE FileAlreadyOpen (LPSTR Name,UINT Mode,LPOFSTRUCT pOFStruct);
DWORD GM32StretchDIBits (DWORD hDC16,DWORD destX,DWORD destY,DWORD destW,DWORD destH,DWORD xoff,
						 DWORD yoff,DWORD bmWidth,DWORD bmHeight,
						 DWORD pDibInfoD, DWORD pImageD,DWORD ColorType,DWORD RastOpts,DWORD pFactorD);
HFILE DecompressCfgFile (HFILE FidConfig);
DWORD GM32DecompressBinaryRecord (LPBYTE pDecompressedRec,LPBYTE pCompressedRec,DWORD CompressedLen);
BOOL  DisplayBMInRect2 (HDC hDC, HANDLE hBM, RECT Rect, double Factor,double VPct, double HPct);
short  DisplayBMInRect (HDC hDC,LPBITMAPINFOHEADER pDibInfo,LPSTR pImage, RECT Rect, short MaintainAspect);
WORD FAR PaletteSize(LPSTR lpDIB);
long    IDNINT (double X);
void GSSiGlobFree (LPHANDLE pHandle);
long    IDNINT (double X);
void GSSiGlobFree (LPHANDLE pHandle);
void GSSiGlobUlFree (LPHANDLE pHandle);
HGLOBAL GSSiGlobAlloc (USHORT From,UINT fuAlloc, long cbAlloc);
void BlowOut (LPSTR Message, LPSTR Title);
long BigWrite (HFILE Fid,HPSTR pMF,DWORD isize,long loc);
BOOL GetOpenFilePathname (HFILE Fid,LPSTR Name);
DWORD GM32StretchDIBits (DWORD hDC16,DWORD destX,DWORD destY,DWORD destW,DWORD destH,DWORD xoff,
						 DWORD yoff,DWORD bmWidth,DWORD bmHeight,
						 DWORD pDibInfoD, DWORD pImageD,DWORD ColorType,DWORD RastOpts,DWORD pFactorD);

*/



