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
//    FixedPoint.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    12/22/2008
//
//  DESCRIPTION:
//    Data and functions to use for fixed point numbers.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "FixedPoint.h"
#include "ClAssert.h"

/*
  Look-up table of sine values.

  DELTA_THETA is the difference in radians between two locations in the 
  sine table.  DELTA_THETA is multiplied by 256 to reduce the loss in precision
  when used with DivFx().
*/
#define SIN_TABLE_LEN 512 // FxSin and FxCos expect this number to be 2**N.
#define DELTA_THETA  ( 2 * Divfx( FX_PI << 8, itofx( SIN_TABLE_LEN ) ) )
#define SIN_TABLE_INDEX(angle) ( fxtoi( Divfx( angle << 8, DELTA_THETA) ) )
static fixed m_sinTable[SIN_TABLE_LEN] = 
{
  /*
	  0 radians to 0.1104 radians
	  0 degrees to 6.3281 degrees
  */
  0, 3, 6, 9, 12, 15, 18, 21, 25, 28,
  /*
	  0.1227 radians to 0.2332 radians
	  7.0312 degrees to 13.3594 degrees
  */
  31, 34, 37, 40, 43, 46, 49, 53, 56, 59,
  /*
	  0.2454 radians to 0.3559 radians
	  14.0625 degrees to 20.3906 degrees
  */
  62, 65, 68, 71, 74, 77, 80, 83, 86, 89,
  /*
	  0.3682 radians to 0.4786 radians
	  21.0938 degrees to 27.4219 degrees
  */
  92, 95, 97, 100, 103, 106, 109, 112, 115, 117,
  /*
	  0.4909 radians to 0.6013 radians
	  28.125 degrees to 34.4531 degrees
  */
  120, 123, 126, 128, 131, 134, 136, 139, 142, 144,
  /*
	  0.6136 radians to 0.724 radians
	  35.1562 degrees to 41.4844 degrees
  */
  147, 149, 152, 155, 157, 159, 162, 164, 167, 169,
  /*
	  0.7363 radians to 0.8468 radians
	  42.1875 degrees to 48.5156 degrees
  */
  171, 174, 176, 178, 181, 183, 185, 187, 189, 191,
  /*
	  0.859 radians to 0.9695 radians
	  49.2187 degrees to 55.5469 degrees
  */
  193, 195, 197, 199, 201, 203, 205, 207, 209, 211,
  /*
	  0.9817 radians to 1.0922 radians
	  56.25 degrees to 62.5781 degrees
  */
  212, 214, 216, 217, 219, 221, 222, 224, 225, 227,
  /*
	  1.1045 radians to 1.2149 radians
	  63.2812 degrees to 69.6094 degrees
  */
  228, 230, 231, 232, 234, 235, 236, 237, 238, 239,
  /*
	  1.2272 radians to 1.3376 radians
	  70.3125 degrees to 76.6406 degrees
  */
  241, 242, 243, 244, 244, 245, 246, 247, 248, 249,
  /*
	  1.3499 radians to 1.4603 radians
	  77.3437 degrees to 83.6719 degrees
  */
  249, 250, 251, 251, 252, 252, 253, 253, 254, 254,
  /*
	  1.4726 radians to 1.5831 radians
	  84.375 degrees to 90.7031 degrees
  */
  254, 255, 255, 255, 255, 255, 255, 255, 256, 255,
  /*
	  1.5953 radians to 1.7058 radians
	  91.4062 degrees to 97.7344 degrees
  */
  255, 255, 255, 255, 255, 255, 254, 254, 254, 253,
  /*
	  1.7181 radians to 1.8285 radians
	  98.4375 degrees to 104.7656 degrees
  */
  253, 252, 252, 251, 251, 250, 249, 249, 248, 247,
  /*
	  1.8408 radians to 1.9512 radians
	  105.4687 degrees to 111.7969 degrees
  */
  246, 245, 244, 244, 243, 242, 241, 239, 238, 237,
  /*
	  1.9635 radians to 2.0739 radians
	  112.5 degrees to 118.8281 degrees
  */
  236, 235, 234, 232, 231, 230, 228, 227, 225, 224,
  /*
	  2.0862 radians to 2.1967 radians
	  119.5312 degrees to 125.8594 degrees
  */
  222, 221, 219, 217, 216, 214, 212, 211, 209, 207,
  /*
	  2.2089 radians to 2.3194 radians
	  126.5625 degrees to 132.8906 degrees
  */
  205, 203, 201, 199, 197, 195, 193, 191, 189, 187,
  /*
	  2.3317 radians to 2.4421 radians
	  133.5937 degrees to 139.9219 degrees
  */
  185, 183, 181, 178, 176, 174, 171, 169, 167, 164,
  /*
	  2.4544 radians to 2.5648 radians
	  140.625 degrees to 146.9531 degrees
  */
  162, 159, 157, 155, 152, 149, 147, 144, 142, 139,
  /*
	  2.5771 radians to 2.6875 radians
	  147.6562 degrees to 153.9844 degrees
  */
  136, 134, 131, 128, 126, 123, 120, 117, 115, 112,
  /*
	  2.6998 radians to 2.8103 radians
	  154.6875 degrees to 161.0156 degrees
  */
  109, 106, 103, 100, 97, 95, 92, 89, 86, 83,
  /*
	  2.8225 radians to 2.933 radians
	  161.7187 degrees to 168.0469 degrees
  */
  80, 77, 74, 71, 68, 65, 62, 59, 56, 53,
  /*
	  2.9452 radians to 3.0557 radians
	  168.75 degrees to 175.0781 degrees
  */
  49, 46, 43, 40, 37, 34, 31, 28, 25, 21,
  /*
	  3.068 radians to 3.1784 radians
	  175.7812 degrees to 182.1094 degrees
  */
  18, 15, 12, 9, 6, 3, 0, -3, -6, -9,
  /*
	  3.1907 radians to 3.3011 radians
	  182.8125 degrees to 189.1406 degrees
  */
  -12, -15, -18, -21, -25, -28, -31, -34, -37, -40,
  /*
	  3.3134 radians to 3.4238 radians
	  189.8437 degrees to 196.1719 degrees
  */
  -43, -46, -49, -53, -56, -59, -62, -65, -68, -71,
  /*
	  3.4361 radians to 3.5466 radians
	  196.875 degrees to 203.2031 degrees
  */
  -74, -77, -80, -83, -86, -89, -92, -95, -97, -100,
  /*
	  3.5588 radians to 3.6693 radians
	  203.9062 degrees to 210.2344 degrees
  */
  -103, -106, -109, -112, -115, -117, -120, -123, -126, -128,
  /*
	  3.6816 radians to 3.792 radians
	  210.9375 degrees to 217.2656 degrees
  */
  -131, -134, -136, -139, -142, -144, -147, -149, -152, -155,
  /*
	  3.8043 radians to 3.9147 radians
	  217.9687 degrees to 224.2969 degrees
  */
  -157, -159, -162, -164, -167, -169, -171, -174, -176, -178,
  /*
	  3.927 radians to 4.0374 radians
	  225 degrees to 231.3281 degrees
  */
  -181, -183, -185, -187, -189, -191, -193, -195, -197, -199,
  /*
	  4.0497 radians to 4.1602 radians
	  232.0312 degrees to 238.3594 degrees
  */
  -201, -203, -205, -207, -209, -211, -212, -214, -216, -217,
  /*
	  4.1724 radians to 4.2829 radians
	  239.0625 degrees to 245.3906 degrees
  */
  -219, -221, -222, -224, -225, -227, -228, -230, -231, -232,
  /*
	  4.2951 radians to 4.4056 radians
	  246.0937 degrees to 252.4219 degrees
  */
  -234, -235, -236, -237, -238, -239, -241, -242, -243, -244,
  /*
	  4.4179 radians to 4.5283 radians
	  253.125 degrees to 259.4531 degrees
  */
  -244, -245, -246, -247, -248, -249, -249, -250, -251, -251,
  /*
	  4.5406 radians to 4.651 radians
	  260.1563 degrees to 266.4844 degrees
  */
  -252, -252, -253, -253, -254, -254, -254, -255, -255, -255,
  /*
	  4.6633 radians to 4.7737 radians
	  267.1875 degrees to 273.5156 degrees
  */
  -255, -255, -255, -255, -256, -255, -255, -255, -255, -255,
  /*
	  4.786 radians to 4.8965 radians
	  274.2188 degrees to 280.5469 degrees
  */
  -255, -255, -254, -254, -254, -253, -253, -252, -252, -251,
  /*
	  4.9087 radians to 5.0192 radians
	  281.25 degrees to 287.5781 degrees
  */
  -251, -250, -249, -249, -248, -247, -246, -245, -244, -244,
  /*
	  5.0315 radians to 5.1419 radians
	  288.2813 degrees to 294.6094 degrees
  */
  -243, -242, -241, -239, -238, -237, -236, -235, -234, -232,
  /*
	  5.1542 radians to 5.2646 radians
	  295.3125 degrees to 301.6406 degrees
  */
  -231, -230, -228, -227, -225, -224, -222, -221, -219, -217,
  /*
	  5.2769 radians to 5.3873 radians
	  302.3438 degrees to 308.6719 degrees
  */
  -216, -214, -212, -211, -209, -207, -205, -203, -201, -199,
  /*
	  5.3996 radians to 5.5101 radians
	  309.375 degrees to 315.7031 degrees
  */
  -197, -195, -193, -191, -189, -187, -185, -183, -181, -178,
  /*
	  5.5223 radians to 5.6328 radians
	  316.4063 degrees to 322.7344 degrees
  */
  -176, -174, -171, -169, -167, -164, -162, -159, -157, -155,
  /*
	  5.645 radians to 5.7555 radians
	  323.4375 degrees to 329.7656 degrees
  */
  -152, -149, -147, -144, -142, -139, -136, -134, -131, -128,
  /*
	  5.7678 radians to 5.8782 radians
	  330.4688 degrees to 336.7969 degrees
  */
  -126, -123, -120, -117, -115, -112, -109, -106, -103, -100,
  /*
	  5.8905 radians to 6.0009 radians
	  337.5 degrees to 343.8281 degrees
  */
  -97, -95, -92, -89, -86, -83, -80, -77, -74, -71,
  /*
	  6.0132 radians to 6.1237 radians
	  344.5313 degrees to 350.8594 degrees
  */
  -68, -65, -62, -59, -56, -53, -49, -46, -43, -40,
  /*
	  6.1359 radians to 6.2464 radians
	  351.5625 degrees to 357.8906 degrees
  */
  -37, -34, -31, -28, -25, -21, -18, -15, -12, -9,
  /*
	  6.2586 radians to 6.2709 radians
	  358.5938 degrees to 359.2969 degrees
  */
  -6, -3
};


//**********************************************************************
//  FUNCTION:
//    FxSin
//
//  DESCRIPTION:	
//    Computes the fixed point sine of an angle.
//
//  Inputs:
//    fixed radians
//      The angle.
//
//  Outputs:
//    Returns the sine.
//
//  GLOBALS:
//    DELTA_THETA
//    m_sinTable
//**********************************************************************
fixed FxSin( fixed radians )
{
  int nIndex = (int)SIN_TABLE_INDEX(radians);
  return m_sinTable[nIndex & ( SIN_TABLE_LEN - 1 )];
}

//**********************************************************************
//  FUNCTION:
//    FxCos
//
//  DESCRIPTION:	
//    Computes the fixed point cosine of an angle.
//
//  Inputs:
//    fixed radians
//      The angle.
//
//  Outputs:
//    Returns the sine.
//
//  GLOBALS:
//    DELTA_THETA
//    m_sinTable
//**********************************************************************
fixed FxCos( fixed radians )
{
  int nIndex = (int)SIN_TABLE_INDEX(radians);
  nIndex += SIN_TABLE_LEN / 4;
  return m_sinTable[nIndex & ( SIN_TABLE_LEN - 1 )];
}


//**********************************************************************
//  FUNCTION:
//    FxSqrt
//
//  DESCRIPTION:	
//    Computes the square roots of 32 bit fixed point numbers.
//
//  Inputs:
//    fixed64 x
//      The radicand.
//
//  Outputs:
//    Returns the square root of x.
//
//  GLOBALS:
//
//**********************************************************************
fixed FxSqrt(fixed x)
{

  ufixed root, remHi, remLo, testDiv, count;

  ClAssert( x >= 0L );

  root = 0; /* Clear root */
  remHi = 0; /* Clear high part of partial remainder */
  remLo = x; /* Get argument into low part of partial remainder */
  count = 32 >> 1; /* Load loop counter.  Counts pairs of bits.*/

  do {
    remHi = (remHi<<2) | (remLo >> ( 32 - 2 ) ); remLo <<= 2; /* get 2 bits of arg */
    root <<= 1; /* Get ready for the next bit in the root */
    testDiv = (root << 1) + 1; /* Test radical */
    if (remHi >= testDiv) {
      remHi -= testDiv;
      root++;
    }
  } while (--count != 0);

  root <<= (8 >> 1);

  return(root);
}


#ifdef __cplusplus
}
#endif