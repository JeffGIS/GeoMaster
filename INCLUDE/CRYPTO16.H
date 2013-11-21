/* -Crypto++ SDK Copyright(c) 1997-1998 Sampson Multimedia (r)- */
/* ------------------------------------------------------------ */
#ifdef __cplusplus 
  extern "C" {
#endif

int FAR PASCAL CreateControlFile(void);
int FAR PASCAL ResetControlFile(void);
int FAR PASCAL GetEncryptedInfo(LPSTR c1, LPSTR c2, LPSTR c3, LPSTR c4, LPSTR c5, LPSTR c6);
DWORD FAR PASCAL DecryptUserInfo(DWORD skey, LPSTR cc1, LPSTR cc2, LPSTR cc3,
								 LPSTR cc4, LPSTR cc5, LPSTR cc6, int f);
int FAR PASCAL GetActivationCode(DWORD akey, LPSTR c1, LPSTR c2, LPSTR c3,
								 LPSTR c4, LPSTR c5, LPSTR c6, DWORD v1,
								 DWORD v2, DWORD v3, DWORD v4, DWORD v5,
								 DWORD v6);
DWORD FAR PASCAL GetRunsCount(void);
DWORD FAR PASCAL GetDateCount(void);
int FAR PASCAL SetActivationCode(LPSTR a1, LPSTR a2, LPSTR a3, LPSTR a4, LPSTR a5, LPSTR a6);
int FAR PASCAL DecrementRunsCount(void);
int FAR PASCAL LicenseIntegrityCheck(void);
int FAR PASCAL ControlFileExists(void);
DWORD FAR PASCAL GetUserTime(void);
int FAR PASCAL ActivationMask(void);
int FAR PASCAL GetDemoMask(void);
int FAR PASCAL SetDemoMask(void);
int FAR PASCAL SetRunsCount(DWORD akey, DWORD count);
int FAR PASCAL SetDateCount(DWORD akey, DWORD count);
int FAR PASCAL SetActualUsersCount(DWORD count);
DWORD FAR PASCAL GetActualUsersCount(void);
DWORD FAR PASCAL GetUsersCount(void);
int FAR PASCAL SetUsersCount(DWORD akey, DWORD count);
int FAR PASCAL UsersCountValidate(int inst);
int FAR PASCAL TransferHardwareID(void);
int FAR PASCAL SetTransferCode(DWORD x);
int FAR PASCAL GetTransferCode(void);

int FAR PASCAL UpdateRandom(void);
int FAR PASCAL TimeUpdate(void);

int FAR PASCAL CheckModuleMask(DWORD m);
int FAR PASCAL SetModuleMask(DWORD akey, DWORD m);
int FAR PASCAL ResetModuleMask(DWORD m);
int FAR PASCAL ResetAllModuleMask(void);

int FAR PASCAL SetGlobals(DWORD s_key, DWORD a_key, LPSTR archi);


#ifdef __cplusplus  
  }
#endif


