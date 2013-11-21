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


// This demonstrates how to decode a scene from a NITF file to a
// BBB (raw) file.  Note the use of the NITFImageManager class.

#include "support.h"

#include "lt_fileSpec.h"
#include "lti_scene.h"
#include "NITFImageManager.h"
#include "NITFImageReader.h"
#include "lti_bbbImageWriter.h"

LT_USE_NAMESPACE(LizardTech);

void
DecodeNITFToBBB()
{
   LT_STATUS sts = LT_STS_Uninit;

   // make the image reader
   const LTFileSpec fileSpec("data/meg.ntf");
   NITFImageManager *manager = NITFImageManager::create();
   ASSERT(manager != NULL);

   sts = manager->initialize(fileSpec);
   ASSERT(LT_SUCCESS(sts));

   NITFImageReader *reader = NULL;
   sts = manager->createReader(reader, 1);  // first image segment
   ASSERT(reader != NULL);
   ASSERT(LT_SUCCESS(sts));

   // make the BBB writer
   LTIBBBImageWriter writer;
   sts = writer.initialize(reader);
   ASSERT(LT_SUCCESS(sts));

   // set up the output file
   sts = writer.setOutputFileSpec("x.bip");
   ASSERT(LT_SUCCESS(sts));
   
   // we will decode the whole image
   const LTIScene scene(0, 0, 640, 480, 1.0);

   // write the scene to the file   
   sts = writer.write(scene);
   ASSERT(LT_SUCCESS(sts));

   // verify we got the right output
   ASSERT(Compare("x.bip", "data/meg.bip"));
   Remove("x.bip");
   Remove("x.hdr");

   reader->release();
   reader = NULL;

   manager->release();
   manager = NULL;
   
   return;
}
