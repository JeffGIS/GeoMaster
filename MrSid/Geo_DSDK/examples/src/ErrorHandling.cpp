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


// This demonstrates how to handle errors and convert them into strings.

#include "support.h"

#include "lt_fileSpec.h"
#include "lt_utilStatusStrings.h"
#include "MrSIDImageReader.h"

LT_USE_NAMESPACE(LizardTech);

void
ErrorHandling()
{
   LT_STATUS sts = LT_STS_Uninit;

   // initialize error string system (required for formatted strings)
   sts = initializeStatusStrings();
   ASSERT(LT_SUCCESS(sts));

   // make the image reader: we will use an invalid file
   const LTFileSpec fileSpec("data/meg.hdr");
   MrSIDImageReader* reader = MrSIDImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec);
   ASSERT(!LT_SUCCESS(sts));

   // retrieve the formatted string
   const char* str = getLastStatusString(sts);
   ASSERT(strcmp(str,"invalid mrsid file format [50607]")==0);

   // close up the error string system
   sts = terminateStatusStrings();
   ASSERT(LT_SUCCESS(sts));

   reader->release();
   reader = NULL;
   return;
}
