// This is a part of the Sheriff System Development Kit.
// Copyright (C) 1997-1998 Acudata Limted.
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Sheriff System Development Kit and related
// electronic documentation provided with the SDK.

#ifndef LICDEFS_H
#define LICDEFS_H

//Challenge protocol
#define SLS_NO_PROTOCOL		0x00000000
#define SLS_BASIC_PROTOCOL	0x00000001

//Remove Options
#define SLS_REMOVE_FILES	0x0001	//remove licence files
#define SLS_REMOVE_REGISTRY	0x0002	//remove licence registry
#define SLS_REMOVE_HISTORY	0x0004	//remove licence history
#define SLS_REMOVE_ALL		0x0007	//remove all of above

#endif
