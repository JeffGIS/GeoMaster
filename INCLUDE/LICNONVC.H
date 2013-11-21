// This is a part of the Sheriff System Development Kit.
// Copyright (C) 1997-1998 Acudata Limted.
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Sheriff System Development Kit and related
// electronic documentation provided with the SDK.

#ifndef LICNOVC_H
#define LICNOVC_H

#ifndef HRESULT
#define HRESULT	long
#endif

#ifndef S_OK
#define S_OK	((HRESULT)0x00000000L)
#endif

#ifndef SUCCEEDED
#define SUCCEEDED(Status) ((HRESULT)(Status) >= 0) 
#endif

#ifndef FAILED
#define FAILED(Status) ((HRESULT)(Status)<0) 
#endif

#ifndef MAKE_SCODE
#define MAKE_SCODE(sev,fac,code) \
    ((SCODE) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
#endif

#ifndef SEVERITY_SUCCESS
#define SEVERITY_SUCCESS    0
#endif

#ifndef SEVERITY_ERROR
#define SEVERITY_ERROR      1
#endif


#endif
