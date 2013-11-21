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
//    FixedPoint64.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    05/14/2009
//
//  DESCRIPTION:
//    Data and functions to use for 64 bit fixed point numbers.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "FixedPoint64.h"
#include "ClAssert.h"

/*
  Look-up table of sine values.

  DELTA_THETA is the difference in radians between two locations in the 
  sine table.  DELTA_THETA is multiplied by 8 to reduce 
  the loss in precision when used with DivFx().
*/
#define SIN_TABLE_LEN 512 // FxSin and FxCos expect this number to be 2**N.
#define DELTA_THETA  ( 2 * Divfx64( PI64 << 3, itofx64( SIN_TABLE_LEN ) ) )
//#define SIN_TABLE_INDEX(angle) ( fx64toi( Divfx64( angle << 3, DELTA_THETA) ) )
#define SIN_TABLE_INDEX(angle) ( Divfx64( angle << 3, DELTA_THETA) )
#define INDEX_90_DEG  ( SIN_TABLE_LEN >> 2 )
static fixed64 m_sinTable[SIN_TABLE_LEN] = 
{
  /*
	  Array Index: 0 to 9
	  0 radians to 0.1104 radians
	  0 degrees to 6.3281 degrees
  */
  0LL, 3294115LL, 6587735LL, 9880363LL, 13171503LL, 16460659LL, 19747337LL, 23031040LL, 26311275LL, 29587548LL,
  /*
	  Array Index: 10 to 19
	  0.1227 radians to 0.2332 radians
	  7.0312 degrees to 13.3594 degrees
  */
  32859365LL, 36126233LL, 39387661LL, 42643158LL, 45892232LL, 49134395LL, 52369159LL, 55596036LL, 58814541LL, 62024188LL,
  /*
	  Array Index: 20 to 29
	  0.2454 radians to 0.3559 radians
	  14.0625 degrees to 20.3906 degrees
  */
  65224495LL, 68414979LL, 71595160LL, 74764559LL, 77922699LL, 81069104LL, 84203301LL, 87324816LL, 90433181LL, 93527927LL,
  /*
	  Array Index: 30 to 39
	  0.3682 radians to 0.4786 radians
	  21.0938 degrees to 27.4219 degrees
  */
  96608588LL, 99674700LL, 102725801LL, 105761432LL, 108781136LL, 111784458LL, 114770946LL, 117740150LL, 120691622LL, 123624918LL,
  /*
	  Array Index: 40 to 49
	  0.4909 radians to 0.6013 radians
	  28.125 degrees to 34.4531 degrees
  */
  126539598LL, 129435220LL, 132311350LL, 135167555LL, 138003404LL, 140818470LL, 143612330LL, 146384561LL, 149134748LL, 151862476LL,
  /*
	  Array Index: 50 to 59
	  0.6136 radians to 0.724 radians
	  35.1562 degrees to 41.4844 degrees
  */
  154567334LL, 157248914LL, 159906814LL, 162540632LL, 165149972LL, 167734441LL, 170293650LL, 172827213LL, 175334749LL, 177815881LL,
  /*
	  Array Index: 60 to 69
	  0.7363 radians to 0.8468 radians
	  42.1875 degrees to 48.5156 degrees
  */
  180270234LL, 182697439LL, 185097130LL, 187468946LL, 189812531LL, 192127530LL, 194413595LL, 196670383LL, 198897553LL, 201094769LL,
  /*
	  Array Index: 70 to 79
	  0.859 radians to 0.9695 radians
	  49.2187 degrees to 55.5469 degrees
  */
  203261701LL, 205398023LL, 207503413LL, 209577553LL, 211620132LL, 213630842LL, 215609379LL, 217555447LL, 219468752LL, 221349005LL,
  /*
	  Array Index: 80 to 89
	  0.9817 radians to 1.0922 radians
	  56.25 degrees to 62.5781 degrees
  */
  223195924LL, 225009231LL, 226788652LL, 228533919LL, 230244770LL, 231920947LL, 233562198LL, 235168275LL, 236738936LL, 238273946LL,
  /*
	  Array Index: 90 to 99
	  1.1045 radians to 1.2149 radians
	  63.2812 degrees to 69.6094 degrees
  */
  239773072LL, 241236089LL, 242662778LL, 244052922LL, 245406312LL, 246722745LL, 248002023LL, 249243953LL, 250448347LL, 251615025LL,
  /*
	  Array Index: 100 to 109
	  1.2272 radians to 1.3376 radians
	  70.3125 degrees to 76.6406 degrees
  */
  252743810LL, 253834533LL, 254887030LL, 255901141LL, 256876715LL, 257813604LL, 258711667LL, 259570769LL, 260390781LL, 261171579LL,
  /*
	  Array Index: 110 to 119
	  1.3499 radians to 1.4603 radians
	  77.3437 degrees to 83.6719 degrees
  */
  261913046LL, 262615069LL, 263277543LL, 263900369LL, 264483453LL, 265026706LL, 265530047LL, 265993400LL, 266416696LL, 266799870LL,
  /*
	  Array Index: 120 to 129
	  1.4726 radians to 1.5831 radians
	  84.375 degrees to 90.7031 degrees
  */
  267142865LL, 267445630LL, 267708118LL, 267930290LL, 268112113LL, 268253559LL, 268354608LL, 268415243LL, 268435456LL, 268415243LL,
  /*
	  Array Index: 130 to 139
	  1.5953 radians to 1.7058 radians
	  91.4062 degrees to 97.7344 degrees
  */
  268354608LL, 268253559LL, 268112113LL, 267930290LL, 267708118LL, 267445630LL, 267142865LL, 266799870LL, 266416696LL, 265993400LL,
  /*
	  Array Index: 140 to 149
	  1.7181 radians to 1.8285 radians
	  98.4375 degrees to 104.7656 degrees
  */
  265530047LL, 265026706LL, 264483453LL, 263900369LL, 263277543LL, 262615069LL, 261913046LL, 261171579LL, 260390781LL, 259570769LL,
  /*
	  Array Index: 150 to 159
	  1.8408 radians to 1.9512 radians
	  105.4687 degrees to 111.7969 degrees
  */
  258711667LL, 257813604LL, 256876715LL, 255901141LL, 254887030LL, 253834533LL, 252743810LL, 251615025LL, 250448347LL, 249243953LL,
  /*
	  Array Index: 160 to 169
	  1.9635 radians to 2.0739 radians
	  112.5 degrees to 118.8281 degrees
  */
  248002023LL, 246722745LL, 245406312LL, 244052922LL, 242662778LL, 241236089LL, 239773072LL, 238273946LL, 236738936LL, 235168275LL,
  /*
	  Array Index: 170 to 179
	  2.0862 radians to 2.1967 radians
	  119.5312 degrees to 125.8594 degrees
  */
  233562198LL, 231920947LL, 230244770LL, 228533919LL, 226788652LL, 225009231LL, 223195924LL, 221349005LL, 219468752LL, 217555447LL,
  /*
	  Array Index: 180 to 189
	  2.2089 radians to 2.3194 radians
	  126.5625 degrees to 132.8906 degrees
  */
  215609379LL, 213630842LL, 211620132LL, 209577553LL, 207503413LL, 205398023LL, 203261701LL, 201094769LL, 198897553LL, 196670383LL,
  /*
	  Array Index: 190 to 199
	  2.3317 radians to 2.4421 radians
	  133.5937 degrees to 139.9219 degrees
  */
  194413595LL, 192127530LL, 189812531LL, 187468946LL, 185097130LL, 182697439LL, 180270234LL, 177815881LL, 175334749LL, 172827213LL,
  /*
	  Array Index: 200 to 209
	  2.4544 radians to 2.5648 radians
	  140.625 degrees to 146.9531 degrees
  */
  170293650LL, 167734441LL, 165149972LL, 162540632LL, 159906814LL, 157248914LL, 154567334LL, 151862476LL, 149134748LL, 146384561LL,
  /*
	  Array Index: 210 to 219
	  2.5771 radians to 2.6875 radians
	  147.6562 degrees to 153.9844 degrees
  */
  143612330LL, 140818470LL, 138003404LL, 135167555LL, 132311350LL, 129435220LL, 126539598LL, 123624918LL, 120691622LL, 117740150LL,
  /*
	  Array Index: 220 to 229
	  2.6998 radians to 2.8103 radians
	  154.6875 degrees to 161.0156 degrees
  */
  114770946LL, 111784458LL, 108781136LL, 105761432LL, 102725801LL, 99674700LL, 96608588LL, 93527927LL, 90433181LL, 87324816LL,
  /*
	  Array Index: 230 to 239
	  2.8225 radians to 2.933 radians
	  161.7187 degrees to 168.0469 degrees
  */
  84203301LL, 81069104LL, 77922699LL, 74764559LL, 71595160LL, 68414979LL, 65224495LL, 62024188LL, 58814541LL, 55596036LL,
  /*
	  Array Index: 240 to 249
	  2.9452 radians to 3.0557 radians
	  168.75 degrees to 175.0781 degrees
  */
  52369159LL, 49134395LL, 45892232LL, 42643158LL, 39387661LL, 36126233LL, 32859365LL, 29587548LL, 26311275LL, 23031040LL,
  /*
	  Array Index: 250 to 259
	  3.068 radians to 3.1784 radians
	  175.7812 degrees to 182.1094 degrees
  */
  19747337LL, 16460659LL, 13171503LL, 9880363LL, 6587735LL, 3294115LL, 0LL, -3294115LL, -6587735LL, -9880363LL,
  /*
	  Array Index: 260 to 269
	  3.1907 radians to 3.3011 radians
	  182.8125 degrees to 189.1406 degrees
  */
  -13171503LL, -16460659LL, -19747337LL, -23031040LL, -26311275LL, -29587548LL, -32859365LL, -36126233LL, -39387661LL, -42643158LL,
  /*
	  Array Index: 270 to 279
	  3.3134 radians to 3.4238 radians
	  189.8437 degrees to 196.1719 degrees
  */
  -45892232LL, -49134395LL, -52369159LL, -55596036LL, -58814541LL, -62024188LL, -65224495LL, -68414979LL, -71595160LL, -74764559LL,
  /*
	  Array Index: 280 to 289
	  3.4361 radians to 3.5466 radians
	  196.875 degrees to 203.2031 degrees
  */
  -77922699LL, -81069104LL, -84203301LL, -87324816LL, -90433181LL, -93527927LL, -96608588LL, -99674700LL, -102725801LL, -105761432LL,
  /*
	  Array Index: 290 to 299
	  3.5588 radians to 3.6693 radians
	  203.9062 degrees to 210.2344 degrees
  */
  -108781136LL, -111784458LL, -114770946LL, -117740150LL, -120691622LL, -123624918LL, -126539598LL, -129435220LL, -132311350LL, -135167555LL,
  /*
	  Array Index: 300 to 309
	  3.6816 radians to 3.792 radians
	  210.9375 degrees to 217.2656 degrees
  */
  -138003404LL, -140818470LL, -143612330LL, -146384561LL, -149134748LL, -151862476LL, -154567334LL, -157248914LL, -159906814LL, -162540632LL,
  /*
	  Array Index: 310 to 319
	  3.8043 radians to 3.9147 radians
	  217.9687 degrees to 224.2969 degrees
  */
  -165149972LL, -167734441LL, -170293650LL, -172827213LL, -175334749LL, -177815881LL, -180270234LL, -182697439LL, -185097130LL, -187468946LL,
  /*
	  Array Index: 320 to 329
	  3.927 radians to 4.0374 radians
	  225 degrees to 231.3281 degrees
  */
  -189812531LL, -192127530LL, -194413595LL, -196670383LL, -198897553LL, -201094769LL, -203261701LL, -205398023LL, -207503413LL, -209577553LL,
  /*
	  Array Index: 330 to 339
	  4.0497 radians to 4.1602 radians
	  232.0312 degrees to 238.3594 degrees
  */
  -211620132LL, -213630842LL, -215609379LL, -217555447LL, -219468752LL, -221349005LL, -223195924LL, -225009231LL, -226788652LL, -228533919LL,
  /*
	  Array Index: 340 to 349
	  4.1724 radians to 4.2829 radians
	  239.0625 degrees to 245.3906 degrees
  */
  -230244770LL, -231920947LL, -233562198LL, -235168275LL, -236738936LL, -238273946LL, -239773072LL, -241236089LL, -242662778LL, -244052922LL,
  /*
	  Array Index: 350 to 359
	  4.2951 radians to 4.4056 radians
	  246.0937 degrees to 252.4219 degrees
  */
  -245406312LL, -246722745LL, -248002023LL, -249243953LL, -250448347LL, -251615025LL, -252743810LL, -253834533LL, -254887030LL, -255901141LL,
  /*
	  Array Index: 360 to 369
	  4.4179 radians to 4.5283 radians
	  253.125 degrees to 259.4531 degrees
  */
  -256876715LL, -257813604LL, -258711667LL, -259570769LL, -260390781LL, -261171579LL, -261913046LL, -262615069LL, -263277543LL, -263900369LL,
  /*
	  Array Index: 370 to 379
	  4.5406 radians to 4.651 radians
	  260.1563 degrees to 266.4844 degrees
  */
  -264483453LL, -265026706LL, -265530047LL, -265993400LL, -266416696LL, -266799870LL, -267142865LL, -267445630LL, -267708118LL, -267930290LL,
  /*
	  Array Index: 380 to 389
	  4.6633 radians to 4.7737 radians
	  267.1875 degrees to 273.5156 degrees
  */
  -268112113LL, -268253559LL, -268354608LL, -268415243LL, -268435456LL, -268415243LL, -268354608LL, -268253559LL, -268112113LL, -267930290LL,
  /*
	  Array Index: 390 to 399
	  4.786 radians to 4.8965 radians
	  274.2188 degrees to 280.5469 degrees
  */
  -267708118LL, -267445630LL, -267142865LL, -266799870LL, -266416696LL, -265993400LL, -265530047LL, -265026706LL, -264483453LL, -263900369LL,
  /*
	  Array Index: 400 to 409
	  4.9087 radians to 5.0192 radians
	  281.25 degrees to 287.5781 degrees
  */
  -263277543LL, -262615069LL, -261913046LL, -261171579LL, -260390781LL, -259570769LL, -258711667LL, -257813604LL, -256876715LL, -255901141LL,
  /*
	  Array Index: 410 to 419
	  5.0315 radians to 5.1419 radians
	  288.2813 degrees to 294.6094 degrees
  */
  -254887030LL, -253834533LL, -252743810LL, -251615025LL, -250448347LL, -249243953LL, -248002023LL, -246722745LL, -245406312LL, -244052922LL,
  /*
	  Array Index: 420 to 429
	  5.1542 radians to 5.2646 radians
	  295.3125 degrees to 301.6406 degrees
  */
  -242662778LL, -241236089LL, -239773072LL, -238273946LL, -236738936LL, -235168275LL, -233562198LL, -231920947LL, -230244770LL, -228533919LL,
  /*
	  Array Index: 430 to 439
	  5.2769 radians to 5.3873 radians
	  302.3438 degrees to 308.6719 degrees
  */
  -226788652LL, -225009231LL, -223195924LL, -221349005LL, -219468752LL, -217555447LL, -215609379LL, -213630842LL, -211620132LL, -209577553LL,
  /*
	  Array Index: 440 to 449
	  5.3996 radians to 5.5101 radians
	  309.375 degrees to 315.7031 degrees
  */
  -207503413LL, -205398023LL, -203261701LL, -201094769LL, -198897553LL, -196670383LL, -194413595LL, -192127530LL, -189812531LL, -187468946LL,
  /*
	  Array Index: 450 to 459
	  5.5223 radians to 5.6328 radians
	  316.4063 degrees to 322.7344 degrees
  */
  -185097130LL, -182697439LL, -180270234LL, -177815881LL, -175334749LL, -172827213LL, -170293650LL, -167734441LL, -165149972LL, -162540632LL,
  /*
	  Array Index: 460 to 469
	  5.645 radians to 5.7555 radians
	  323.4375 degrees to 329.7656 degrees
  */
  -159906814LL, -157248914LL, -154567334LL, -151862476LL, -149134748LL, -146384561LL, -143612330LL, -140818470LL, -138003404LL, -135167555LL,
  /*
	  Array Index: 470 to 479
	  5.7678 radians to 5.8782 radians
	  330.4688 degrees to 336.7969 degrees
  */
  -132311350LL, -129435220LL, -126539598LL, -123624918LL, -120691622LL, -117740150LL, -114770946LL, -111784458LL, -108781136LL, -105761432LL,
  /*
	  Array Index: 480 to 489
	  5.8905 radians to 6.0009 radians
	  337.5 degrees to 343.8281 degrees
  */
  -102725801LL, -99674700LL, -96608588LL, -93527927LL, -90433181LL, -87324816LL, -84203301LL, -81069104LL, -77922699LL, -74764559LL,
  /*
	  Array Index: 490 to 499
	  6.0132 radians to 6.1237 radians
	  344.5313 degrees to 350.8594 degrees
  */
  -71595160LL, -68414979LL, -65224495LL, -62024188LL, -58814541LL, -55596036LL, -52369159LL, -49134395LL, -45892232LL, -42643158LL,
  /*
	  Array Index: 500 to 509
	  6.1359 radians to 6.2464 radians
	  351.5625 degrees to 357.8906 degrees
  */
  -39387661LL, -36126233LL, -32859365LL, -29587548LL, -26311275LL, -23031040LL, -19747337LL, -16460659LL, -13171503LL, -9880363LL,
  /*
	  Array Index: 510 to 511
	  6.2586 radians to 6.2709 radians
	  358.5938 degrees to 359.2969 degrees
  */
  -6587735LL, -3294115LL
};


//**********************************************************************
//  FUNCTION:
//    Fx64Sin
//
//  DESCRIPTION:	
//    Computes the 64 bit fixed point sine of an angle.
//
//  Inputs:
//    fixed64 radians
//      The angle.
//
//  Outputs:
//    Returns the sine.
//
//  GLOBALS:
//    DELTA_THETA
//    m_sinTable
//**********************************************************************
fixed64 Fx64Sin( fixed64 radians )
{
//  int nIndex = (int)SIN_TABLE_INDEX(radians);
//  return m_sinTable[nIndex & ( SIN_TABLE_LEN - 1 )];

  fixed64 fxIndex0 = SIN_TABLE_INDEX(radians);
  int nIndex0 = fx64toi(fxIndex0) & ( SIN_TABLE_LEN - 1 );
  int nIndex1 = ( nIndex0 + 1 ) & ( SIN_TABLE_LEN - 1 );
  fixed64 fxDeltaSin = m_sinTable[ nIndex1 ] - m_sinTable[ nIndex0 ];
  fixed64 fxSin = Mulfx64( fxDeltaSin, fx64fract(fxIndex0) ) + m_sinTable[nIndex0];

  return fxSin;

}

//**********************************************************************
//  FUNCTION:
//    Fx64Cos
//
//  DESCRIPTION:	
//    Computes the 64 bit fixed point cosine of an angle.
//
//  Inputs:
//    fixed radians
//      The angle.
//
//  Outputs:
//    Returns the cosine.
//
//  GLOBALS:
//    DELTA_THETA
//    m_sinTable
//**********************************************************************
fixed64 Fx64Cos( fixed64 radians )
{
//  int nIndex = (int)SIN_TABLE_INDEX(radians);
//  nIndex += INDEX_90_DEG;
//  return m_sinTable[nIndex & ( SIN_TABLE_LEN - 1 )];
  fixed64 r = PI64 >> 1;

  return Fx64Sin( radians + (PI64 >> 1) );
}

//**********************************************************************
//  FUNCTION:
//    Fx64Tan
//
//  DESCRIPTION:	
//    Computes the 64 bit fixed point tangent of an angle.
//
//  Inputs:
//    fixed radians
//      The angle.
//
//  Outputs:
//    Returns the tangent.
//
//  GLOBALS:
//    DELTA_THETA
//    m_sinTable
//**********************************************************************
fixed64 Fx64Tan( fixed64 radians )
{
  return Divfx64( Fx64Sin( radians ), Fx64Cos( radians ) );
}

//**********************************************************************
//  FUNCTION:
//    FxSqrt64
//
//  DESCRIPTION:	
//    Computes the square roots of 64 bit fixed point numbers.
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
fixed64 FxSqrt64(fixed64 x)
{

  ufixed64 root, remHi, remLo, testDiv, count;

  ClAssert( x >= 0LL );

  root = 0; /* Clear root */
  remHi = 0; /* Clear high part of partial remainder */
  remLo = x; /* Get argument into low part of partial remainder */
  count = BIT_COUNT >> 1; /* Load loop counter.  Counts pairs of bits.*/

  do {
    remHi = (remHi<<2) | (remLo >> ( BIT_COUNT - 2 ) ); remLo <<= 2; /* get 2 bits of arg */
    root <<= 1; /* Get ready for the next bit in the root */
    testDiv = (root << 1) + 1; /* Test radical */
    if (remHi >= testDiv) {
      remHi -= testDiv;
      root++;
    }
  } while (--count != 0);

  root <<= (FRAC_BIT_COUNT >> 1);

  return(root);
}

#ifdef __cplusplus
}
#endif