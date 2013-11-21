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


// This example shows the basic image information available from an LTIImageStage.


#include "support.h"

#include "lt_fileSpec.h"
#include "lti_geoCoord.h"
#include "lti_utils.h"
#include "MrSIDImageReader.h"
#include "lti_pixel.h"
#include "lti_sample.h"

LT_USE_NAMESPACE(LizardTech);

void
ImageInfo()
{
   LT_STATUS sts = LT_STS_Uninit;
   
   // make the image reader
   const LTFileSpec fileSpec("data/meg_cr20.sid");
   MrSIDImageReader *reader = MrSIDImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec);
   ASSERT(LT_SUCCESS(sts));

   // get some information about the image
   ASSERT(reader->getWidth() == 640);
   ASSERT(reader->getHeight() == 480);
   ASSERT(reader->getColorSpace() == LTI_COLORSPACE_RGB);
   ASSERT(reader->getNumBands() == 3);
   ASSERT(reader->getDataType() == LTI_DATATYPE_UINT8);
   
   ASSERT(reader->getNominalImageSizeWithAlpha() == 640 * 480 * 3 * 1);
   
   ASSERT(reader->getMinMagnification() == 0.0625);
   ASSERT(LTIUtils::magToLevel(0.0625) == 4);
   ASSERT(reader->getMaxMagnification() == 1048576);
   ASSERT(LTIUtils::magToLevel(1048576) == -20); // (an arbitrarily large value)

   const LTIGeoCoord& geo = reader->getGeoCoord();
   ASSERT(geo.getX()==0.0);
   ASSERT(geo.getY()==479.0);
   ASSERT(geo.getXRes()==1.0);
   ASSERT(geo.getYRes()==-1.0);

   const LTIPixel *nd = reader->getNoDataPixel();
   if (nd)
      for (lt_uint16 b = 0; b < nd->getNumBands(); b++)
         ASSERT(nd->getSample(b).getValueAsDouble() == 0); // black

   reader->release();
   reader = NULL;
   
   return;
}
