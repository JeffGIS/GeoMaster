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


// This demonstrates how to decode a scene from a MrSID file into a
// byte array in memory, by directly performing a decode() operation
// on a MrSIDImageReader object.


#include "support.h"

#include "lt_fileSpec.h"
#include "lti_scene.h"
#include "lti_sceneBuffer.h"
#include "MrSIDImageReader.h"

LT_USE_NAMESPACE(LizardTech);

void
DecodeMrSIDToMemory()
{
   LT_STATUS sts = LT_STS_Uninit;

   // make the image reader
   const LTFileSpec fileSpec("data/meg_cr20.sid");
   MrSIDImageReader *reader = MrSIDImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec);
   ASSERT(LT_SUCCESS(sts));
   
   // decode the whole image at full resolution
   const lt_uint32 w = 640;
   const lt_uint32 h = 480;
   const LTIScene scene(0, 0, w, h, 1.0);
   
   // construct the buffer we're decoding into
   // note we choose to allocate our own buffer, rather than let
   // LTISceneBuffer implicitly allocate one for us
   const lt_uint32 siz = w * h * 1;
   lt_uint8* membuf = new lt_uint8[siz*3];
   void* bufs[3] = { membuf+siz*0, membuf+siz*1, membuf+siz*2 };
   LTISceneBuffer bufData(reader->getPixelProps(), w, h, bufs);
   
   // perform the decode
   sts = reader->read(scene, bufData);
   ASSERT(LT_SUCCESS(sts));

   // verify we got the right output
   FILE* fp=fopen("x.raw","w+b");
   ASSERT(fp!=NULL);
   lt_uint32 i=0;
   lt_uint32 j=0;
   for (j=0; j<siz; j++)
   {
      lt_uint32 cnt = 0;
      cnt = (lt_uint32)fwrite((lt_uint8*)(bufs[0]) + j,1,1,fp);
      ASSERT(cnt==1);
      cnt = (lt_uint32)fwrite((lt_uint8*)(bufs[1]) + j,1,1,fp);
      ASSERT(cnt==1);
      cnt = (lt_uint32)fwrite((lt_uint8*)(bufs[2]) + j,1,1,fp);
      ASSERT(cnt==1);
   }
   fclose(fp);
   ASSERT(Compare("x.raw", "data/meg_cr20.raw"));

   delete[] membuf;
   
   Remove("x.raw");
   
   
   reader->release();
   reader = NULL;

   return;
}
