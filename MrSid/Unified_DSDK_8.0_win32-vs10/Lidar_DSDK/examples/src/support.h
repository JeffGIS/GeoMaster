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


#include "lidar/Error.h"
#include "lidar/PointSource.h"

LT_USE_LIDAR_NAMESPACE

#define ASSERT(b) do { if(!(b)) THROW_LIBRARY_ERROR(-1)(#b); } while(0)

void removeFile(const char* file);
void compareFiles(const char* file1, const char* file2);
void compareTXTFiles(const char* file1, const char* file2);
void checkCwd(void);

