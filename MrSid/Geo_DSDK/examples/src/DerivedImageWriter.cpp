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


// This demonstrates how to derive your own image writer.  This example
// class just writes raw (BIP) files.


#include "support.h"

#include "lt_fileSpec.h"
#include "lti_bbbImageReader.h"
#include "lti_geoFileImageWriter.h"
#include "lti_sceneBuffer.h"
#include "lti_pixel.h"
#include "lti_utils.h"

LT_USE_NAMESPACE(LizardTech);


//---------------------------------------------------------------------------
// simple BIP (raw) writer
//---------------------------------------------------------------------------

class MyWriter : public LTIGeoFileImageWriter
{
public:
   MyWriter();
   ~MyWriter();
   LT_STATUS initialize(LTIImageStage* image);

   LT_STATUS writeBegin(const LTIScene& scene);
   LT_STATUS writeStrip(LTISceneBuffer& stripBuffer, const LTIScene& stripScene);

private:
   lt_uint32 m_fullWidth;
   lt_uint32 m_fullHeight;
   lt_uint32 m_curRow;

   // nope
   MyWriter(MyWriter&);
   MyWriter& operator=(const MyWriter&);
};



MyWriter::MyWriter(void) :
   LTIGeoFileImageWriter(true),
   m_fullWidth(0),
   m_fullHeight(0),
   m_curRow(0)
{
   return;
}


MyWriter::~MyWriter()
{
   return;
}


LT_STATUS
MyWriter::initialize(LTIImageStage* image)
{
   LT_STATUS sts = LTIGeoFileImageWriter::init(image);
   if (!LT_SUCCESS(sts)) return sts;
 
   return LT_STS_Success;
}


LT_STATUS
MyWriter::writeBegin(const LTIScene &scene)
{
   LT_STATUS sts = LT_STS_Uninit;

   sts = LTIGeoFileImageWriter::writeBegin(scene);
   if (!LT_SUCCESS(sts)) return sts;

   m_fullWidth = scene.getNumCols();
   m_fullHeight = scene.getNumRows();
   m_curRow = 0;

   return LT_STS_Success;
}



LT_STATUS
MyWriter::writeStrip(LTISceneBuffer& stripBuffer,
                     const LTIScene& stripScene)
{
   (void)stripScene;
   LT_STATUS sts = LT_STS_Uninit;

   if(LTIUtils::needsSwapping(stripBuffer.getPixelProps().getDataType(), LTI_ENDIAN_LITTLE))
      stripBuffer.byteSwap();

   LTIOStreamInf& stream = *getStream();
   sts = stripBuffer.exportDataBIP(stream);
   if (!LT_SUCCESS(sts)) return sts;

   return LT_STS_Success;
}


//---------------------------------------------------------------------------

void
DerivedImageWriter()
{
   LT_STATUS sts = LT_STS_Uninit;

   // read in a raw rgb image, and write it out as grayscale

   // make the reader
   const LTFileSpec fileSpec("data/meg.bip");
   LTIBBBImageReader *reader = LTIBBBImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec);
   ASSERT(LT_SUCCESS(sts));

   // make the raw writer
   MyWriter writer;
   sts = writer.initialize(reader);
   ASSERT(LT_SUCCESS(sts));

   // set up the output file
   sts = writer.setOutputFileSpec("meg.bip");
   ASSERT(LT_SUCCESS(sts));
   
   const LTIScene scene(0, 0, 640, 480, 1.0);

   // write the scene to the file   
   sts = writer.write(scene);
   ASSERT(LT_SUCCESS(sts));

   // verify we got the right output
   ASSERT(Compare("meg.bip", "data/meg.bip"));
   Remove("meg.bip");

   reader->release();
   reader = NULL;

   return;
}
