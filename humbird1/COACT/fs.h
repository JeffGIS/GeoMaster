//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// NNN     NN  PPPPPPP  EEEEEE
// NN NN   NN  PP    PP EE
// NN  NN  NN  PPPPPPP  EEEEE
// NN   NN NN  PP       EE
// NN     NNN  PP       EEEEEEE
//
// Copyright © 2008 by North Pole Engineering, Inc.  All rights reserved.
// Printed in the United States of America.  Except as permitted under the
// United States Copyright Act of 1976, no part of this software may be
// reproduced or distributed in any form or by any means, without the prior
// written permission of North Pole Engineering, Inc., unless such copying is
// expressly permitted by federal copyright law.
//
// Address copying inquires to:
// North Pole Engineering, Inc.
// Attn: Joe Meyer
// 221 North 1st Street Suite 310
// Minneapolis, Minnesota 55401
//
// Information contained in this software has been created or obtained by North
// Pole Engineering, Inc. from sources believed to be reliable. However, North
// Pole Engineering, Inc. does not guarantee the accuracy or completeness of the
// information published herein nor shall North Pole Engineering, Inc. be liable
// for any errors, omissions, or damages arising from the use of this software.
//
//
//	MODULE:
//    fs.h
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    06/24/2009
//
//  DESCRIPTION:
//    File system functions
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifndef _FS_H
#define _FS_H

#include <stdio.h>

#if defined(HUMMINBIRD)
#include "fs_api.h"
#if !defined(HBIRDDEMO)
#define HBIRDDEMO 1
#endif
#endif

//**********************************************************************
// Parameters
//**********************************************************************
#define	CL_MAX_PATH	128

//**********************************************************************
// File IO function aliases
//**********************************************************************
#define CHART_ROOT "temp\\hbtest\\"
#if defined(HUMMINBIRD)
typedef FS_FILE CLFILE;
#define cl_fopen FS_FOpen
#define cl_fread FS_FRead
#define cl_fclose FS_FClose
#define cl_fseek FS_FSeek
#define cl_ftell FS_FTell
#define CHART_PATH CHART_ROOT
#else
typedef FILE      CLFILE;
#define cl_fopen  fopen
#define cl_fread  fread
#define cl_fclose fclose
#define cl_fseek  fseek
#define cl_ftell  ftell
#define CHART_PATH "C:\\"CHART_ROOT
#endif



#endif //  _FS_H
