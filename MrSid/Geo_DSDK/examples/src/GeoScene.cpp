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


// This demonstrates how to use geographic coordinate information
// to make decode requests.  We will write the middle of the image
// out to a raw file.

#include "support.h"

#include "lt_fileSpec.h"
#include "lti_navigator.h"
#include "MrSIDImageReader.h"
#include "lti_rawImageWriter.h"

LT_USE_NAMESPACE(LizardTech);

void
GeoScene()
{
   LT_STATUS sts = LT_STS_Uninit;

   // make the image reader
   const LTFileSpec fileSpec("data/meg_cr20.sid");
   MrSIDImageReader *reader = MrSIDImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec);
   ASSERT(LT_SUCCESS(sts));

   // create a navigator; initially it is set to the whole image
   LTINavigator nav(*reader);

   const double pixelWidth = reader->getWidth();
   const double pixelHeight = reader->getHeight();
   const double halfPixelWidth = pixelWidth / 2.0;
   const double halfPixelHeight = pixelHeight / 2.0;
   const double quarterPixelWidth = pixelWidth / 4.0;
   const double quarterPixelHeight = pixelHeight / 4.0;

   // set up a scene in the middle of the image, using geo coordinates
   const LTIGeoCoord& geo = reader->getGeoCoord();
   double geoCenterX = geo.getX() + (geo.getXRes() * halfPixelWidth);
   double geoCenterY = geo.getY() + (geo.getYRes() * halfPixelHeight);
   double ulx = geoCenterX - (geo.getXRes() * quarterPixelWidth);
   double uly = geoCenterY - (geo.getYRes() * quarterPixelHeight);
   double lrx = geoCenterX + (geo.getXRes() * quarterPixelWidth);
   double lry = geoCenterY + (geo.getYRes() * quarterPixelHeight);
   
   sts = nav.setSceneAsGeoULLR(ulx, uly, lrx, lry, 2.0);
   ASSERT(LT_SUCCESS(sts));
   
   const LTIScene& scene = nav.getScene();
   
   // make the raw writer
   LTIRawImageWriter writer;
   sts = writer.initialize(reader);
   ASSERT(LT_SUCCESS(sts));

   // set up the output file
   sts = writer.setOutputFileSpec("x.raw");
   ASSERT(LT_SUCCESS(sts));
   
   // write the scene to the file   
   sts = writer.write(scene);
   ASSERT(LT_SUCCESS(sts));

   ASSERT(Compare("x.raw", "data/meg_geo.raw"));   

   Remove("x.raw");
   
   reader->release();
   reader = NULL;
      
   return;
}
