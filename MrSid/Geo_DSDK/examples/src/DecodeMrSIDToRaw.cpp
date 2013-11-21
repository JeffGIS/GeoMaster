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


// This demonstrates how to decode a scene from a MrSID file to a
// raw file, by constructing a pipeline consisting of a MrSIDImageReader
// which feeds into an LTIRawImageWriter.

#include "support.h"

#include "lt_fileSpec.h"
#include "lti_scene.h"
#include "MrSIDImageReader.h"
#include "lti_rawImageWriter.h"

LT_USE_NAMESPACE(LizardTech);

void
DecodeMrSIDToRaw()
{
   LT_STATUS sts = LT_STS_Uninit;

   // make the image reader
   const LTFileSpec fileSpec("data/meg_cr20.sid");
   MrSIDImageReader *reader = MrSIDImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec);
   ASSERT(LT_SUCCESS(sts));

   // make the raw writer
   LTIRawImageWriter writer;
   sts = writer.initialize(reader);
   ASSERT(LT_SUCCESS(sts));

   // set up the output file
   sts = writer.setOutputFileSpec("x.raw");
   ASSERT(LT_SUCCESS(sts));
   
   // we will decode the whole image at half resolution
   lt_uint32 w=0, h=0;
   sts = reader->getDimsAtMag(0.5, w, h);
   ASSERT(LT_SUCCESS(sts));
   const LTIScene scene(0, 0, w, h, 0.5);

   // write the scene to the file   
   sts = writer.write(scene);
   ASSERT(LT_SUCCESS(sts));

   // verify we got the right output
   ASSERT(Compare("x.raw", "data/meg_cr20_magP5_sid.bip"));

   Remove("x.raw");
   
   reader->release();
   reader = NULL;
   
   return;
}
