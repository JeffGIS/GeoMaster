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


// This example shows the use of a complex image pipeline: we will take an
// RGB image, convert it to greyscale, crop it, and write it out to disk.
// The input and output formats are both raw.

#include "support.h"

#include "lti_pixel.h"
#include "lti_scene.h"
#include "lti_rawImageReader.h"
#include "lti_colorTransformer.h"
#include "lti_cropFilter.h"
#include "lti_rawImageWriter.h"

LT_USE_NAMESPACE(LizardTech);

void
Pipeline()
{
   LT_STATUS sts = LT_STS_Uninit;

   // make the raw image reader
   const LTIPixel inputProps(LTI_COLORSPACE_RGB, 3, LTI_DATATYPE_UINT8);
   LTIRawImageReader *reader = LTIRawImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize("data/meg.bip", inputProps, 640, 480);
   ASSERT(LT_SUCCESS(sts));

   // make the RGB -> greyscale filter
   LTIColorTransformer *colorFilter = LTIColorTransformer::create();
   ASSERT(colorFilter != NULL);

   const LTIPixel grayPixel(LTI_COLORSPACE_GRAYSCALE, 1, reader->getDataType());
   sts = colorFilter->initialize(reader, grayPixel);
   ASSERT(LT_SUCCESS(sts));

   // prove the color transform worked
   ASSERT(colorFilter->getColorSpace() == LTI_COLORSPACE_GRAYSCALE);

   // crop it to remove the outer 20 pixels on each edge
   LTICropFilter *cropFilter = LTICropFilter::create();
   ASSERT(cropFilter != NULL);

   sts = cropFilter->initialize(colorFilter, 20, 20, 600, 440);
   ASSERT(LT_SUCCESS(sts));

   // prove the crop worked
   ASSERT(cropFilter->getWidth() == 600);
   ASSERT(cropFilter->getHeight() == 440);
   
   // make the raw writer
   LTIRawImageWriter writer;
   sts = writer.initialize(cropFilter);
   ASSERT(LT_SUCCESS(sts));

   // set up the output file
   sts = writer.setOutputFileSpec("x.raw");
   ASSERT(LT_SUCCESS(sts));
   
   // we will use the whole (cropped) image
   const LTIScene scene(0, 0, 600, 440, 1.0);

   // write the scene to the file   
   sts = writer.write(scene);
   ASSERT(LT_SUCCESS(sts));

   // verify we got the right output
   ASSERT(Compare("x.raw", "data/meg_filter.raw"));

   Remove("x.raw");

   cropFilter->release();
   cropFilter = NULL;

   colorFilter->release();
   colorFilter = NULL;

   reader->release();
   reader = NULL;
   
   return;
}
