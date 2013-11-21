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
//    ClMalloc.h
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    05/22/2008
//
//  DESCRIPTION:
//    Memory allocation functions.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifndef __CL_MALLOC_H
#define __CL_MALLOC_H

#include "ChartLib.h"

//**********************************************************************
// Data Types
//**********************************************************************

//**********************************************************************
// Function prototypes
//**********************************************************************

unsigned long ClFreeMemorySize( void );

#if defined(DEBUG_MEM)

#if !defined(HUMMINBIRD)
void ClDumpFreeList( void );
#endif // !defined(HUMMINBIRD)

/*
  Define DEBUG_MEM when you want to have rudimentry bounds checking and allocation tracking.
*/
void *cl_malloc_debug( unsigned long nbytes, char *psFile, int nLine, char *psFunction );
void cl_free_debug( void *p, char *psFile, int iLine, char *psFunc );

#undef cl_malloc
#undef cl_free

#if defined(HUMMINBIRD)

#define cl_malloc( size ) \
  cl_malloc_debug( (size), __FILE__, __LINE__, NULL )

#define cl_free( p ) \
  cl_free_debug( (p), __FILE__, __LINE__, NULL )

#else

#define cl_malloc( size ) \
  cl_malloc_debug( (size), __FILE__, __LINE__, __FUNCTION__ )

#define cl_free( p ) \
  cl_free_debug( (p), __FILE__, __LINE__, __FUNCTION__ )

#endif // HUMMINBIRD


#else

void *cl_malloc(unsigned long nbytes );
void *lm_malloc(unsigned long nbytes );
int cl_chkmalloc (unsigned long nbytes );
void cl_free( void *p );

#endif


#endif // __CL_MALLOC_H