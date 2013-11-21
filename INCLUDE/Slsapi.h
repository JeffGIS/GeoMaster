// This is a part of the Sheriff System Development Kit.
// Copyright (C) 1997-1999 Acudata Limted.
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Sheriff System Development Kit and related
// electronic documentation provided with the SDK.

#ifndef SLSAPI_H
#define SLSAPI_H

#include "licdefs.h"
#include "lictype.h"
#include "licscode.h"    

#ifdef __cplusplus 
extern "C"
{
#endif

HRESULT WINAPI SLS_Register(
					LPCSTR pszProductID,
					LPCSTR pszProductName,
					LPCSTR pszLicencePath);

HRESULT WINAPI SLS_License(
					LPCSTR pszProductID,
					SLS_LICENCE *pLicence,
					SLS_CHALLENGE *pChallenge);

HRESULT WINAPI SLS_Request(
					LPCSTR pszProductID,
					LPCSTR pszUserName,
					SLS_REQUEST *pRequest,
					SLS_PERMIT *pPermit,
					SLS_HANDLE *plLicenceHandle,
					SLS_CHALLENGE *pChallenge);

HRESULT WINAPI SLS_Update(
					LPCSTR pszProductID,
					SLS_HANDLE lLicenceHandle,
					SLS_UPDATE *pUpdate,
					SLS_PERMIT *pPermit,
					SLS_CHALLENGE *pChallenge);

HRESULT WINAPI SLS_Release(
					LPCSTR pszProductID,
					SLS_HANDLE lLicenceHandle,
					SLS_RELEASE *pRelease,
					SLS_CHALLENGE *pChallenge);

HRESULT WINAPI SLS_GetReference(
					LPCSTR pszProductID,
					LPSTR  pszReference);

HRESULT WINAPI SLS_SetLicence(
					LPCSTR pszProductID,
					LPCSTR pszReference,
					LPCSTR pszLicenceKey);

HRESULT WINAPI SLS_QueryLicenceInfo(LPCSTR pszProductID,
									 SLS_LICENCE_INFO *pLicenceInfo);

//Challenge
HRESULT WINAPI SLS_CreateChallenge(
					 SLS_SECRET *pSecretArray,
					 int nSecrectSize,
					 const BYTE *pbStream,
					 int nStreamSize,
					 SLS_CHALLENGE *pChallenge);

HRESULT WINAPI SLS_VerifyChallenge(
					 SLS_SECRET *pSecretArray,
					 int nSecrectSize,
					 const BYTE *pbStream,
					 int nStreamSize,
					 SLS_CHALLENGE *pChallenge);

//SLS_GetErrorMessage
//Get Error Mesaage. pszErrorMessage points to buffer allocated by caller, 
//the minimum length of the buffer is 256 bytes
HRESULT WINAPI SLS_GetErrorMessage(HRESULT hr,LPCSTR pszErrorMessage);

//Utilities
HRESULT WINAPI SLS_IsProductInstalled(LPCSTR pszProductID);
HRESULT WINAPI SLS_IsProductRegistered(LPCSTR pszProductID);
HRESULT WINAPI SLS_IsProductLicensed(LPCSTR pszProductID);


//SLS_SetReclaimTime is now obsolte and replaced by SLS_SetOptions
HRESULT WINAPI SLS_SetReclaimTime(int nMinutes);
HRESULT WINAPI SLS_SetOptions(
					LPCSTR pszProductID,
					SLS_HANDLE lLicenceHandle,
					SLS_OPTIONS *pOptions,
					SLS_CHALLENGE *pChallenge);

//Terminate
HRESULT WINAPI SLS_Terminate(
					LPCSTR pszProductID,
					LPSTR  pszTerminationCode);
//Remove
HRESULT WINAPI SLS_Remove(
					LPCSTR pszProductID,
					DWORD  dwRemoveOption,
					SLS_CHALLENGE *pChallenge);

//Current Licence Status
HRESULT WINAPI SLS_GetStatusCode(
					LPCSTR pszProductID,
					LPSTR  pszStatusCode);

//Export Licence
HRESULT WINAPI SLS_ExportLicence(
					LPCSTR pszProductID,
					LPCSTR pszReference,
					SLS_LICENCE *pLicence,
					LPSTR  pszLicenceKey);

HRESULT WINAPI SLS_ImportLicence(
					LPCSTR pszProductID,
					LPCSTR pszReference,
					LPCSTR pszLicenceKey);

//Extended Function from 1.70
//Publiser data
HRESULT WINAPI SLS_SetPublisherData(LPCSTR pszProductID,LPCSTR lpszPublisherData);
HRESULT WINAPI SLS_GetPublisherData(LPCSTR pszProductID,LPCSTR lpszPublisherData);

HRESULT WINAPI SLS_RequestEx(
					LPCSTR  pszProductID,
					LPCSTR pszUserName,
					DWORD   dwUserID,
					SLS_REQUEST *pRequest,
					SLS_PERMIT *pPermit,
					SLS_HANDLE *plLicenceHandle,
					SLS_CHALLENGE *pChallenge);

HRESULT WINAPI SLS_GetUserCount(LPCSTR  pszProductID,DWORD *pdwUserCount);

HRESULT WINAPI SLS_QueryUserInfo(
					LPCSTR  pszProductID,
					DWORD	dwUserIndex,
					SLS_USER_INFO *pUserInfo);

#ifdef __cplusplus  
}
#endif


#endif
