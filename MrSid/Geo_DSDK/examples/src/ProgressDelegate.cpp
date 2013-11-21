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


// This demonstrates how to derive your own progress meter class.
// This example class just prints out the current percent-complete
// each time it is called.

#include "support.h"

#include "lt_fileSpec.h"
#include "lti_delegates.h"
#include "lti_scene.h"
#include "lti_sceneBuffer.h"
#include "MrSIDImageReader.h"

LT_USE_NAMESPACE(LizardTech);

class MyProgress : public LTIProgressDelegate
{
public:
   MyProgress() :
      LTIProgressDelegate(),
      m_cnt(0)
   {
      return;
   }

   LT_STATUS setProgressStatus(float x)
   {
      printf("%d: %f\n", m_cnt, x);
      ++m_cnt;
      return LT_STS_Success;
   }

public:
   int m_cnt;
};


void
ProgressDelegate()
{
   LT_STATUS sts = LT_STS_Uninit;

   // 
   MyProgress progress;

   // make the image reader
   const LTFileSpec fileSpec("data/meg_cr20.sid");
   MrSIDImageReader *reader = MrSIDImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec);
   ASSERT(LT_SUCCESS(sts));
   
   reader->setProgressDelegate(&progress);

   const lt_uint32 w = 640;
   const lt_uint32 h = 480;
   const LTIScene scene(0, 0, w, h, 1.0);
   const lt_uint32 siz = w * h * 1;
   lt_uint8* membuf = new lt_uint8[siz*3];
   void* bufs[3] = { membuf+siz*0, membuf+siz*1, membuf+siz*2 };
   LTISceneBuffer bufData(reader->getPixelProps(), w, h, bufs);
   
   // the decode will fail, return 999 from within the delegate
   sts = reader->read(scene, bufData);
   ASSERT(sts == LT_STS_Success);

   // verify the interrupt handler was called several times
   ASSERT(progress.m_cnt == 30);

   reader->release();
   reader = NULL;
   
   return;
}
