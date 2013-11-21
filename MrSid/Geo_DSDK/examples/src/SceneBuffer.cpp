/* $Id$ */
/* //////////////////////////////////////////////////////////////////////////
//                                                                         //
// This code is Copyright (c) 2004 LizardTech, Inc, 1008 Western Avenue,   //
// Suite 200, Seattle, WA 98104.  Unauthorized use or distribution         //
// prohibited.  Access to and use of this code is permitted only under     //
// license from LizardTech, Inc.  Portions of the code are protected by    //
// US and foreign patents and other filings. All Rights Reserved.          //
//                                                                         //
////////////////////////////////////////////////////////////////////////// */
/* PUBLIC */


// This demonstrates some of the features of the LTISceneBuffer class,
// by decoding an image into a larger "frame" colored grey.

#include "support.h"

#include "lt_fileSpec.h"
#include "lt_ioFileStream.h"
#include "lti_scene.h"
#include "lti_sceneBuffer.h"
#include "lti_bbbImageReader.h"

LT_USE_NAMESPACE(LizardTech);

void
SceneBuffer()
{
   LT_STATUS sts = LT_STS_Uninit;

   // make the image reader
   const LTFileSpec fileSpec("data/meg_cr20.bip");
   LTIBBBImageReader *reader = LTIBBBImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec);
   ASSERT(LT_SUCCESS(sts));

   reader->setStripHeight(200);

   // Construct the buffer we're decoding into: we are going to
   // use a buffer that is 150x150 and put the image into the
   // middle of it.  Note we let the buffer allocate its own
   // storage.

   LTISceneBuffer sceneBufferOuter(reader->getPixelProps(),
                                   150, 150,    // window size
                                   NULL);

   {
      // some sanity checks
      const lt_uint32 wco = sceneBufferOuter.getWindowColOffset();
      const lt_uint32 wro = sceneBufferOuter.getWindowRowOffset();
      const lt_uint32 wnc = sceneBufferOuter.getWindowNumCols();
      const lt_uint32 wnr = sceneBufferOuter.getWindowNumRows();
      const lt_uint32 tnc = sceneBufferOuter.getTotalNumCols();
      const lt_uint32 tnr = sceneBufferOuter.getTotalNumRows();
      ASSERT(wco==0);
      ASSERT(wro==0);
      ASSERT(wnc==150);
      ASSERT(wnr==150);
      ASSERT(tnc==150);
      ASSERT(tnr==150);
   }

   {
      // make a pink pattern
      lt_uint8* p = new lt_uint8[150*150*3];
      lt_uint32 i=0;
      for (i=0; i<150*150*3; i+=3)
      {
         p[i]=255;
         p[i+1]=128;
         p[i+2]=128;
      }

      // import the pink pattern into the scene buffer
      sts = sceneBufferOuter.importDataBIP(p);
      ASSERT(LT_SUCCESS(sts));

      delete[] p;
   }

   // decode a 100x100 scene from the middle of the 640x480 image
   const LTIScene scene(270, 190, 100, 100, 1.0);

   // make a window into the larger buffer, at offset (0,0)
   LTISceneBuffer sceneBufferInner(sceneBufferOuter, 25, 25, 100, 100);

   {
      // some more sanity checks
      const lt_uint32 wco = sceneBufferInner.getWindowColOffset();
      const lt_uint32 wro = sceneBufferInner.getWindowRowOffset();
      const lt_uint32 wnc = sceneBufferInner.getWindowNumCols();
      const lt_uint32 wnr = sceneBufferInner.getWindowNumRows();
      const lt_uint32 tnc = sceneBufferInner.getTotalNumCols();
      const lt_uint32 tnr = sceneBufferInner.getTotalNumRows();
      ASSERT(wco==25);
      ASSERT(wro==25);
      ASSERT(wnc==100);
      ASSERT(wnr==100);
      ASSERT(tnc==150);
      ASSERT(tnr==150);
   }

   // perform the decode
   sts = reader->read(scene, sceneBufferInner);
   ASSERT(LT_SUCCESS(sts));

   // export the image in BSQ form to disk
   {
      LTIOFileStream stream;
      sts = stream.initialize("x.raw", "wb");
      ASSERT(LT_SUCCESS(sts));
      sts = stream.open();
      ASSERT(LT_SUCCESS(sts));
      sts = sceneBufferOuter.exportDataBSQ(stream);
      ASSERT(LT_SUCCESS(sts));
      sts = stream.close();
      ASSERT(LT_SUCCESS(sts));
   }
   
   ASSERT( Compare("x.raw", "data/meg_framed_bsq.raw") );
   
   Remove("x.raw");

   reader->release();
   reader = NULL;
   
   return;
}
