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
//    FixedPoint.h
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    12/22/2008
//
//  DESCRIPTION:
//    Data types, macros, and functions to use for fixed point numbers.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifndef __FIXED_POINT_H_
#define __FIXED_POINT_H_

#include "types.h"
#include "ClAssert.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
  Fixed point values use the signed 24.8 format.
*/
typedef long fixed;												// Our new fixed point type
typedef unsigned long ufixed;

#define FX_BIT_COUNT       ( sizeof(fixed) * 8 )
#define FX_FRAC_BIT_COUNT  (8)
#define FX_INT_BIT_COUNT   ( FX_BIT_COUNT - FX_FRAC_BIT_COUNT )
#define FX_FRAC_MASK       ( ( 1L << FX_FRAC_BIT_COUNT ) - 1 )
#define FX_INT_MASK        ( ~FX_FRAC_MASK )
#define FX_MAX_FRAC        ( ( 1L << FX_FRAC_BIT_COUNT ) - 1 )
/*
  constants
*/
#define f_1   ( 1L << FX_FRAC_BIT_COUNT ) // 1 in fixed point format. Sometimes used to convert constants to fixed point.
#define f_0_5 ( f_1 >> 1 ) // .5 in fixed point format.
#define FX_PI	( (long)(3.1415926f * f_1 ) )		// Value of PI


/*
  Conversion macros.
*/
#define itofx(x) ((long)(x) << FX_FRAC_BIT_COUNT)										// Integer to fixed point
#define ftofx(x) (long)((x) * f_1)								// Float to fixed point
#define dtofx(x) (long)((x) * f_1)								// Double to fixed point
#define fxtoi(x) ((x) >> FX_FRAC_BIT_COUNT)		// Fixed point to integer
#define fxtof(x) ((float) (x) / f_1)							// Fixed point to float
#define fxtod(x) ((double)(x) / f_1)							// Fixed point to double

/*
  Rounding macros
*/
#define fxceiling(x) ( ( (x) + FX_MAX_FRAC ) & FX_INT_MASK )
#define fxfloor(x) ( (x) & FX_INT_MASK )
#define fxround(x) ( ( (x) + f_0_5 ) & FX_INT_MASK )

/*
  Fixed Point parts
*/
#define fxfract(x) ( (x) & FX_FRAC_MASK )

/*
  Arithmetic operations.
*/
#define Mulfx(x,y) (((x) * (y)) >> FX_FRAC_BIT_COUNT)							// Multiply a fixed by a fixed

#define Divfx(x,y) (((x) << FX_FRAC_BIT_COUNT) / (y))							// Divide a fixed by a fixed

#if defined(FX_DEBUG) && !defined(HUMMINBIRD)

#define FX_SHIFTED_BITS_MASK (-1L << (FX_BIT_COUNT - ( FX_FRAC_BIT_COUNT + 1 )))

// Stop execution if there is an overflow.
__inline fixed Divfx_Debug( fixed x, fixed y) 
{ 
  ClAssert( 
    !(  x < 0 &&  ~x & FX_SHIFTED_BITS_MASK ||
        x > 0 && x & FX_SHIFTED_BITS_MASK  ||
        y == 0 
    )
  );

  return Divfx(x,y);
}
// Replace the macro with the debug function.
#undef Divfx
#define Divfx Divfx_Debug

#endif

/*
  Trig functions.
*/
fixed FxSin( fixed degrees );
fixed FxCos( fixed degrees );

fixed FxSqrt(fixed x);

#ifdef __cplusplus
}
#endif


#endif //__FIXED_POINT_H_