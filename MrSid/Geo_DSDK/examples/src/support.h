/* $Id$ */
/* //////////////////////////////////////////////////////////////////////////
//                                                                         //
// This code is Copyright (c) 2008 LizardTech, Inc, 1008 Western Avenue,   //
// Suite 200, Seattle, WA 98104.  Unauthorized use or distribution         //
// prohibited.  Access to and use of this code is permitted only under     //
// license from LizardTech, Inc.  Portions of the code are protected by    //
// US and foreign patents and other filings. All Rights Reserved.          //
//                                                                         //
////////////////////////////////////////////////////////////////////////// */
/* PUBLIC */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ASSERT(b) Assert(__FILE__, __LINE__, #b, (b))
extern void Remove(const char* file);
extern void Assert(const char* file, int line, const char* str, int cond);
extern int Compare(const char* file1,
                   const char* file2);
extern int GetNumDiffs(const char* file1,
                       const char* file2);
extern void checkCwd();
