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
//    FixedPoint64.h
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    05/14/2009
//
//  DESCRIPTION:
//    Data types, macros, and functions to use for 64 bit fixed point numbers.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifndef __FIXED_POINT64_H_
#define __FIXED_POINT64_H_

#include "ClAssert.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
  Fixed point values use the signed 40.24 format.
*/
typedef long long fixed64;
typedef unsigned long long ufixed64; // Only to be used internally by FixedPoint64.c.

#define BIT_COUNT       ( sizeof(fixed64) * 8 )
#define FRAC_BIT_COUNT  (28)
#define INT_BIT_COUNT   ( BIT_COUNT - FRAC_BIT_COUNT )
#define FRAC_MASK       ( ( 1 << FRAC_BIT_COUNT ) - 1 )
#define INT_MASK        ( ~FRAC_MASK )
#define MAX_FRAC        ( ( 1LL << FRAC_BIT_COUNT ) - 1 )

/*
  constants
*/
#define f64_1   ( 1LL << FRAC_BIT_COUNT ) // 1 in fixed point format. Sometimes used to convert constants to fixed point.
#define f64_0_5 (f64_1>>1) // .5 in fixed point format.
#define PI 3.14159265358979323e0
#define PI64	( (fixed64)(PI * f64_1 ) )		// Value of PI


/*
  Conversion macros.
*/
#define itofx64(x) ((fixed64)(x) << FRAC_BIT_COUNT)		// Integer to fixed point
#define ftofx64(x) (fixed64)((x) * f64_1)							// Float to fixed point
#define dtofx64(x) (fixed64)((x) * f64_1)							// Double to fixed point
#define fx64toi(x) ((long)((x) >> FRAC_BIT_COUNT))  	// Fixed point to integer
#define fx64toi64(x) ((long long)((x) >> FRAC_BIT_COUNT))// Fixed point to integer
#define fx64tof(x) ((float) (x) / f64_1)							// Fixed point to float
#define fx64tod(x) ((double)(x) / f64_1)							// Fixed point to double

/*
  Rounding macros
*/
#define fx64ceiling(x) ( ( (x) + ( f64_1 - 1 ) ) & INT_MASK )
#define fx64floor(x) ( (x) & INT_MASK )
#define fx64round(x) ( ( (x) + f64_0_5 ) & INT_MASK )

/*
  Arithmetic operations.
*/
#define Mulfx64(x,y) (((x) * (y)) >> FRAC_BIT_COUNT)  // Multiply a fixed by a fixed
#define MulLargefx64(x,xShft,y,yShft) \
  (((((x)>>(xShft) ) * ((y)>>(yShft)))) >> (FRAC_BIT_COUNT - ((xShft) + (yShft))))  // Multiply a fixed by a fixed

#define Divfx64(x,y) (((x) << FRAC_BIT_COUNT) / (y))  // Divide a fixed by a fixed

#if defined(FX_DEBUG) && !defined(HUMMINBIRD)

#define SHIFTED_BITS_MASK ( -1LL << ( BIT_COUNT - ( FRAC_BIT_COUNT + 1 ) ) )

__inline fixed64 Divfx64_Debug( fixed64 x, fixed64 y) 
{ 
  ClAssert( 
    !(  x < 0 &&  ~x & SHIFTED_BITS_MASK ||
        x > 0 && x & SHIFTED_BITS_MASK  ||
        y == 0 
    )
  );

  return  Divfx64(x,y);
}
// Replace the macro with the debug function.
#undef Divfx64
#define Divfx64 Divfx64_Debug
#endif

#define Fabs64(x) ( ( (x) >= 0 ) ? (x) : -(x) )

#define fx64int(x) ( (x)&INT_MASK )
#define fx64fract(x) ( (x)&FRAC_MASK )

/*
  Trig functions.
*/
fixed64 Fx64Sin( fixed64 degrees );
fixed64 Fx64Cos( fixed64 degrees );
fixed64 Fx64Tan( fixed64 radians );
fixed64 FxSqrt64(fixed64 x);


#ifdef __cplusplus
}
#endif


#endif //__FIXED_POINT64_H_