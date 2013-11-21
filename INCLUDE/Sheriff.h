// This is a part of the Sheriff System Development Kit.
// Copyright (C) 1997-1998 Acudata Limted.
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Sheriff System Development Kit and related
// electronic documentation provided with the SDK.
#define LPCTSTR	LPSTR
#ifndef SHERIFF_H
#define SHERIFF_H

#include "slsapi.h"

//class CSheriff
//{
//private:
/*	SLS_HANDLE	m_hLicence;			//Licence Handle
	HRESULT		m_lLastError;		//Last Error Code
	CString		m_strUserName;
	CString		m_strProductID;
	SLS_SECRET	m_arySecrets[4];
	BOOL		m_bSecretsSet; */

//public:
	CSheriff(LPCTSTR lpszProductID,LPCTSTR lpszUserName);
/*	~CSheriff();

	void	SetSecrets(SLS_SECRET *parySecrets,int nSizeSecrets=4);
	BOOL	Succeeded()	{ return SUCCEEDED(m_lLastError); }
	HRESULT	GetLastError() { return m_lLastError; }
	void	GetLastErrorMessage(CString &strError);  */

//public:
	BOOL QueryLicenceInfo(SLS_LICENCE_INFO &licInfo);
	BOOL GetReference(CString &strReference);
	BOOL SetLicence(LPCSTR pszReference,LPCTSTR lpszLicenceKey);

	//Interface
	BOOL Register(LPCTSTR lpszProductName,
				  LPCTSTR lpszLicencePath);	
	BOOL License(SLS_LICENCE Licence);
	BOOL Request(SLS_REQUEST Request,SLS_PERMIT &Permit);
	BOOL Update(SLS_UPDATE Update,SLS_PERMIT &Permit);
	BOOL Release(SLS_RELEASE Release);

	//Attribute
	BOOL IsProductInstalled();
	BOOL IsProductRegistered();
	BOOL IsProductLicensed();

	BOOL IsLicenceDefined();
	BOOL IsLicenceValid();
	BOOL IsLicenceExportable();

	BOOL IsLifetimeKey();
	BOOL IsLifetimeRef();
	BOOL IsStandaloneKey();

	//Options
	BOOL SetOptions(SLS_OPTIONS Options);

	//Utilities
	BOOL Export(SLS_LICENCE Licence,LPCTSTR lpszExportRef,CString &strExportKey);
	BOOL Import(LPCSTR pszReference,LPCTSTR lpszLicenceKey);

	DWORD GetUserCount();
	BOOL  QueryUserInfo(DWORD dwUserIndex,SLS_USER_INFO &userInfo);
//};

#endif
