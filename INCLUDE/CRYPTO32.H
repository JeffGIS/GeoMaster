/* -Crypto++ SDK Copyright(c) 1997-1998 Sampson Multimedia (r)- */
/* ------------------------------------------------------------ */
//#ifdef __cplusplus 
//  extern "C" {
//#endif

int  CreateControlFile(void);
int  ResetControlFile(void);
int  GetEncryptedInfo(LPSTR c1, LPSTR c2, LPSTR c3, LPSTR c4, LPSTR c5, LPSTR c6);
DWORD  DecryptUserInfo(DWORD skey, LPSTR cc1, LPSTR cc2, LPSTR cc3,
								 LPSTR cc4, LPSTR cc5, LPSTR cc6, int f);
int  GetActivationCode(DWORD akey, LPSTR c1, LPSTR c2, LPSTR c3,
								 LPSTR c4, LPSTR c5, LPSTR c6, DWORD v1,
								 DWORD v2, DWORD v3, DWORD v4, DWORD v5,
								 DWORD v6);
DWORD  GetRunsCount(void);
DWORD  GetDateCount(void);
int  SetActivationCode(LPSTR a1, LPSTR a2, LPSTR a3, LPSTR a4, LPSTR a5, LPSTR a6);
int  DecrementRunsCount(void);
int  LicenseIntegrityCheck(void);
int  ControlFileExists(void);
DWORD  GetUserTime(void);
int  ActivationMask(void);
int  GetDemoMask(void);
int  SetDemoMask(void);
int  SetRunsCount(DWORD akey, DWORD count);
int  SetDateCount(DWORD akey, DWORD count);
int  SetActualUsersCount(DWORD count);
DWORD  GetActualUsersCount(void);
DWORD  GetUsersCount(void);
int  SetUsersCount(DWORD akey, DWORD count);
int  UsersCountValidate(void);
int  TransferHardwareID(void);
int  SetTransferCode(DWORD x);
int  GetTransferCode(void);

int  UpdateRandom(void);
int  TimeUpdate(void);

int  CheckModuleMask(DWORD m);
int  SetModuleMask(DWORD akey, DWORD m);
int  ResetModuleMask(DWORD m);
int  ResetAllModuleMask(void);

int  SetGlobals(DWORD s_key, DWORD a_key, LPSTR archi);

DWORD  Crypto(DWORD re, DWORD rn, DWORD da, DWORD us, DWORD mr,
						DWORD md, DWORD ne, DWORD sc, DWORD ac, LPSTR cf,
						LPSTR ci, int df);


//#ifdef __cplusplus  
//  }
//#endif


