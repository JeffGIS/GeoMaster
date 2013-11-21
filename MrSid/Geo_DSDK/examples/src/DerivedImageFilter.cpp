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


// This demonstrates how to derive your own image filter from LTIImageFilter.
// For this example, we make a filter which simply converts RGB images to
// grayscale.  (For clarity of presentation, this filter does not update
// the metadata to reflect the new colorspace, it only works on 8 bit
// samples, and it does it modify the background and nodata pixels to be
// grayscale data.)


#include "support.h"

#include "lt_fileSpec.h"
#include "lti_imageFilter.h"
#include "lti_pixel.h"
#include "lti_scene.h"
#include "lti_sceneBuffer.h"
#include "lti_bbbImageReader.h"
#include "lti_bbbImageWriter.h"

LT_USE_NAMESPACE(LizardTech);


//---------------------------------------------------------------------------
// RGB to grayscale filter
//---------------------------------------------------------------------------

class MyFilter : public LTIOverridePixelProps<LTIImageFilter>
{
   LTI_REFERENCE_COUNTED_BOILERPLATE(MyFilter);
public:
   LT_STATUS initialize(LTIImageStage* sourceImage);

   // LTIImageStage override
   virtual lt_uint32 getModifications(const LTIScene &scene) const;

   LT_STATUS decodeBegin(const LTIScene& scene);
   LT_STATUS decodeEnd();
   LT_STATUS decodeStrip(LTISceneBuffer& stripBuffer, const LTIScene& stripScene);

private:
   lt_uint8** m_bandsBuffer;
};



MyFilter::MyFilter(void) :
   m_bandsBuffer(NULL)
{
}


MyFilter::~MyFilter()
{
   delete[] m_bandsBuffer[0];
   delete[] m_bandsBuffer[1];
   delete[] m_bandsBuffer[2];
   delete[] m_bandsBuffer;
   return;
}

MyFilter *MyFilter::create(void)
{
   return new MyFilter;
}


LT_STATUS
MyFilter::initialize(LTIImageStage* sourceImage)
{
   LT_STATUS sts = LTIImageFilter::init(sourceImage);
   if (!LT_SUCCESS(sts))
      return sts;

   const LTIImageStage& prev = *getPreviousStage();

   if (prev.getColorSpace() != LTI_COLORSPACE_RGB ||
       prev.getDataType() != LTI_DATATYPE_UINT8)
      return LT_STS_Failure;

   LTIPixel props(LTI_COLORSPACE_GRAYSCALE, 1, prev.getDataType());
   sts = setPixelProps(props);
   if (!LT_SUCCESS(sts)) 
      return sts;

   m_bandsBuffer = new lt_uint8*[3];
   m_bandsBuffer[0] = NULL;
   m_bandsBuffer[1] = NULL;
   m_bandsBuffer[2] = NULL;

   return LT_STS_Success;
}


lt_uint32
MyFilter::getModifications(const LTIScene &scene) const
{
   lt_uint32 mods = LTI_MODIFICATION_UNKNOWN;
   if (getPreviousStage())
      mods = getPreviousStage()->getModifications(scene);
   mods |= LTI_MODIFICATION_CHANGEDCOLORSPACE;
   return mods;
}

LT_STATUS
MyFilter::decodeBegin(const LTIScene& fullScene)
{
   // set up work buffer
   lt_uint32 width = fullScene.getNumCols();
   lt_uint32 height = getPreviousStage()->getStripHeight();
   
   delete[] m_bandsBuffer[0];
   m_bandsBuffer[0] = new lt_uint8[width*height];
   delete[] m_bandsBuffer[1];
   m_bandsBuffer[1] = new lt_uint8[width*height];
   delete[] m_bandsBuffer[2];
   m_bandsBuffer[2] = new lt_uint8[width*height];

   return getPreviousStage()->readBegin(fullScene);
}


LT_STATUS
MyFilter::decodeEnd()
{     
   // clean up work buffer
   delete[] m_bandsBuffer[0];
   m_bandsBuffer[0] = NULL;
   delete[] m_bandsBuffer[1];
   m_bandsBuffer[1] = NULL;
   delete[] m_bandsBuffer[2];
   m_bandsBuffer[2] = NULL;
   
   return getPreviousStage()->readEnd();
}


LT_STATUS
MyFilter::decodeStrip(LTISceneBuffer& stripBuffer, const LTIScene& stripScene)
{
   LT_STATUS sts = LT_STS_Uninit;
   
   const lt_int32 width = stripScene.getNumCols();
   const lt_int32 height = stripScene.getNumRows();
         
   // make a buffer to hold the rgb data
   LTISceneBuffer srcData(getPreviousStage()->getPixelProps(),
                          width, height, (void **)m_bandsBuffer);
   
   // read the RGB data from the previous stage
   sts = getPreviousStage()->readStrip(srcData, stripScene);
   if (!LT_SUCCESS(sts))
      return sts;
   
   // copy the data from the RGB buffer into our grayscale buffer
   lt_uint8 *red = m_bandsBuffer[0];
   lt_uint8 *green = m_bandsBuffer[1];
   lt_uint8 *blue = m_bandsBuffer[2];
   lt_uint8 *gray = (lt_uint8 *)stripBuffer.getWindowBandData(0);
   for (lt_int32 r = 0; r < height; r++)
   {
      for (lt_int32 c = 0; c < width; c++)
      {
         gray[c] = (lt_uint8)(0.3f * red[c] + 0.6f * green[c] + 0.1f * blue[c]);
      }
      red += width;
      green += width;
      blue += width;
      gray += stripBuffer.getTotalNumCols();
   }
   
   return LT_STS_Success;
}

//---------------------------------------------------------------------------

void
DerivedImageFilter()
{
   LT_STATUS sts = LT_STS_Uninit;

   // read in a raw rgb image, and write it out as grayscale

   // make the reader
   const LTFileSpec fileSpec("data/meg.bip");
   LTIBBBImageReader *reader = LTIBBBImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec);
   ASSERT(LT_SUCCESS(sts));

   // connect up the filter
   MyFilter *filter = MyFilter::create();
   ASSERT(filter != NULL);

   sts = filter->initialize(reader);
   ASSERT(LT_SUCCESS(sts));

   ASSERT(reader->getColorSpace() == LTI_COLORSPACE_RGB);
   ASSERT(filter->getColorSpace() == LTI_COLORSPACE_GRAYSCALE);

   // make the BBB writer
   LTIBBBImageWriter writer;
   sts = writer.initialize(filter);
   ASSERT(LT_SUCCESS(sts));

   // set up the output file
   sts = writer.setOutputFileSpec("meg_gray.bip");
   ASSERT(LT_SUCCESS(sts));
   
   const LTIScene scene(0, 0, 640, 480, 1.0);

   // write the scene to the file   
   sts = writer.write(scene);
   ASSERT(LT_SUCCESS(sts));

   // verify we got the right output
   ASSERT(Compare("meg_gray.bip", "data/meg_gray.bip"));
   Remove("meg_gray.bip");
   Remove("meg_gray.hdr");

   filter->release();
   filter = NULL;
   reader->release();
   reader = NULL;
   
   return;
}
