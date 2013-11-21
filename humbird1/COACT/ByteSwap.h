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
//    ByteSwap.h
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    6/28/2008
//
//  DESCRIPTION:
//    Functions for byte swapping.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// Function prototypes.
//**********************************************************************
int ByteSwapInt( int i );
CL_BOOL cl_ByteSwapNeeded(void);
short cl_freadS16(CLFILE *pFID);
long cl_freadS32(CLFILE *pFID);
double cl_freadF64(CLFILE *pFID);
CLFILE *db_fopen( const char *filename, const char *mode );
int db_fread(void * pbuf, int size,int count, CLFILE * fid);
int	db_fseek (CLFILE * fid,int loc,int pos);
int db_ftell (CLFILE * fid);
int db_fclose (CLFILE * fid);
int GetClockTicks (void);



