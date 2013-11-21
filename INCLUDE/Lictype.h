// This is a part of the Sheriff System Development Kit.
// Copyright (C) 1997-1998 Acudata Limted.
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Sheriff System Development Kit and related
// electronic documentation provided with the SDK.

#ifndef LICTYPE_H
#define LICTYPE_H

//state of licence
#define	SLS_STATE_UNDEFINED	0x0000	//Licence type and contents to be specified
#define	SLS_STATE_BAD		0x0001	//Licence has being illegally modified
#define SLS_STATE_EXPIRED	0x0002	//Licence has expired
#define SLS_STATE_EXHAUSTED	0x0003	//Licence units has run out
#define SLS_STATE_NOCOUSER	0x0004	//Licence user limit is 0
#define SLS_STATE_OK		0x0008	//Licence is valid

//type of licence
#define	SLS_TYPE_UNDEFINED		0x0000	//Undefined
#define SLS_TYPE_UNIT_METER		0x0001	//Unit Metering
#define SLS_TYPE_TIME_METER		0x0002	//Time Metering
#define SLS_TYPE_EXPIRATION		0x0004	//Expiry Control
#define SLS_TYPE_CONCURRENCY	0x0008	//Concurrency Control

#define SLS_TYPE_LIFETIME_KEY	0x0100	//Life time key, it is now obsolete and renamed as SLS_TYPE_REUSABLE_KEY
#define SLS_TYPE_REUSABLE_KEY	0x0100	//Reusable key
#define SLS_TYPE_LIFETIME_REF	0x0200	//Life time reference, it is now obsolete and renamed as SLS_TYPE_REUSABLE_REF
#define SLS_TYPE_REUSABLE_REF	0x0200	//Reuable reference
#define SLS_TYPE_UNEXPORTABLE	0x0400	//Export Disabled
#define SLS_TYPE_STANDALONE		0x0800	//Standalone Licence

#pragma pack(1)

typedef struct _SLS_DATE
{
	DWORD  Year,Month,Day;
} SLS_DATE;

typedef struct _SLS_DATE_TIME
{
	DWORD  Year,Month,Day;
	DWORD  Hour,Minute,Second;
} SLS_DATE_TIME;

typedef struct _SLS_LICENCE
{
	DWORD		Type;		//type of licence
	DWORD		Meter;		//for metering, e.g. Unit, Time mertering
	SLS_DATE	EndDate;	//for expiration date
	DWORD		CoUsers;	//Concurrent users
	DWORD		AccessKey;	//Access control key
} SLS_LICENCE;

typedef struct _SLS_REQUEST
{
	DWORD	UnitsReserved;
} SLS_REQUEST;

typedef struct _SLS_PERMIT
{
	//WORD	State;		//state of licence permit
	DWORD	UnitsGranted;
	DWORD	AccessKey;
} SLS_PERMIT;

typedef struct _SLS_UPDATE
{
	DWORD	UnitsReserved;
	DWORD	UnitsConsumed;
} SLS_UPDATE;

typedef struct _SLS_RELEASE
{
	DWORD	UnitsConsumed;
} SLS_RELEASE;

//types for challenging
typedef struct _SLS_MSG_DIGEST
{
	char MessageDigest[16];	//binary data
} SLS_MSG_DIGEST; 

typedef struct _SLS_CHALLDATA 
{  
	DWORD SecretIndex; 
	DWORD Random; 
	SLS_MSG_DIGEST MsgDigest;
} SLS_CHALLDATA; 

typedef struct _SLS_CHALLENGE
{  
	DWORD Protocol; 
	DWORD Size; 
	SLS_CHALLDATA ChallengeData; 
} SLS_CHALLENGE; 

//Indentity used by challenge only
typedef struct _SLS_IDENTITY
{
	TCHAR Name[128];
	TCHAR Version[32];
	TCHAR Publisher[256];
} SLS_IDENTITY;

//Secret used for challenge
typedef struct _SLS_SECRET
{
	unsigned char Secret[32];	//null-terminate string
} SLS_SECRET;

typedef struct _SLS_MAC	//Message Authentication Code
{
	char  MessageDigest[16];	//binary data
} SLS_MAC;

//types for suit implementation
typedef struct _SLS_PRODUCT
{
	char  ProductID[32];
	TCHAR ProductName[128];
} SLS_PRODUCT;

typedef struct _SLS_SUIT
{
	DWORD NumberOfProducts;
	SLS_PRODUCT *pProducts;
} SLS_SUIT;

//Licence Handle
typedef DWORD SLS_HANDLE;

//SLS_QueryLicenceInfo
typedef struct _SLS_LICENCE_INFO
{
	//Licence Part
	DWORD		Type;		//type of licence
	DWORD		Meter;		//Units or Days limit
	SLS_DATE	EndDate;	//Expiration date
	DWORD		CoUsers;	//Concurrent users
	DWORD		AccessKey;	//Access control key
	//Usage Part
	DWORD		State;		//State of licence
	DWORD		MeterUsage;	//for metering only(units used, days used)
	SLS_DATE	StartDate;	//Start date
	DWORD		ActiveUsers;//Number of active users
} SLS_LICENCE_INFO;

//SLS_QueryUserInfo
typedef struct _SLS_USER_INFO
{
	char			UserName[32];
	DWORD			UserID;
	SLS_DATE_TIME	LoginTime;
	SLS_DATE_TIME	LastUpdateTime;	
} SLS_USER_INFO;

//SLS_SetOptions
typedef struct _SLS_OPTIONS
{
	DWORD HeartbeatTime;	//heartbeat time 
	DWORD ReclaimTime;		//reclaim time
	DWORD Reserved1;		//reserved, should be set to 0
	DWORD Reserved2;		//reserved, should be set to 0
} SLS_OPTIONS;

#pragma pack()

#endif  //LICTYPE_H

