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


// This demonstrates how to differentiate between MrSID Raster files and those containing LiDAR data

#include "support.h"

#include "lt_fileSpec.h"
#include "lti_scene.h"
#include "MrSIDImageReader.h"

LT_USE_NAMESPACE(LizardTech);

void
DecodeMrSIDLidar()
{
   LT_STATUS sts = LT_STS_Uninit;

   const LTFileSpec fileSpec("data/mg4lidar.sid");

   lt_uint8 gen = 0;
   bool raster = true;

   // call this to determine if a MrSID file contains LiDAR data
   sts = MrSIDImageReaderInterface::getMrSIDGeneration(fileSpec, gen, raster);
   ASSERT(LT_SUCCESS(sts));
   ASSERT(raster == false);
   
   // if we try to open this MrSID lidar file, it should fail
   MrSIDImageReader *reader = MrSIDImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec);
   ASSERT(LT_FAILURE(sts));

   reader->release();
   reader = NULL;
   
   return;
}
