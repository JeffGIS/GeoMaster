/* - Crypto++ SDK 2005 Copyright(c) 1997-2005 Sampson Multimedia - */
/* --------------------------------------------------------------- */
#ifdef __cplusplus
  extern "C" {
#endif

int WINAPI ResetControlFile(void);
int WINAPI GetEncryptedInfo(LPSTR c1);
DWORD WINAPI DecryptUserInfo(DWORD pkey, LPSTR cc1, int f);
int WINAPI GetActivationCode(DWORD akey, LPSTR c1, DWORD v1, DWORD v2,
				DWORD v3, DWORD v4, DWORD v5, DWORD v6);
DWORD WINAPI GetRuns(void);
int WINAPI SetActivationCode(LPSTR a1);
int WINAPI TransferHardwareID(void);
int WINAPI GetTransferCode(void);
int WINAPI SetTransferCode(DWORD x);
int WINAPI CheckModuleMask(DWORD m);
int WINAPI SetModuleMask(DWORD m);
int WINAPI ResetModuleMask(DWORD m);
int WINAPI ResetAllModuleMask(void);
int WINAPI SetGlobals(DWORD pkey, LPSTR str);
DWORD WINAPI Crypto(DWORD re, DWORD rn, DWORD da, DWORD us, DWORD ne,
				DWORD pk, LPSTR cf,	LPSTR ci, int df);
int WINAPI InetSendCode(LPSTR url, LPSTR path, int port, LPSTR code, LPSTR pass);
int WINAPI InetGetCode(LPSTR inetcode);
DWORD WINAPI GetDaysLeft(void);
int WINAPI UsersUpdate(void);
int WINAPI SetTrialSettings(DWORD rn, DWORD da, DWORD us, DWORD ne);
int WINAPI LicenseCheck(DWORD ne);
int WINAPI IsLicensed(DWORD ne);
int WINAPI UsersCheck(void);
int WINAPI CheckActivationCode(DWORD ne);


#ifdef __cplusplus  
  }
#endif


