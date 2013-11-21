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


// This demonstrates how to derive your own interrupt delegate and use it
// inside of a decode request.

#include "support.h"

#include "lt_fileSpec.h"
#include "lti_delegates.h"
#include "lti_scene.h"
#include "lti_sceneBuffer.h"
#include "MrSIDImageReader.h"

LT_USE_NAMESPACE(LizardTech);

// This class will simulate an interrupt event after N calls by the decoder
// to query the interrupt status.  In a real system, the if-test would really
// be checking some external event, e.g. a ^C or button-press.
class MyInterrupt : public LTIInterruptDelegate
{
public:
   MyInterrupt(int n) : 
      LTIInterruptDelegate(),
      m_cnt(0),
      m_max(n)
   {
      return;
   }

   LT_STATUS getInterruptStatus()
   {
      printf("interrupt called %d times\n", m_cnt);
      if (m_cnt == 10) return 999;
      ++m_cnt;
      return LT_STS_Success;
   }

public:
   int m_cnt;   // public so we can query from the test routine

private:
   const int m_max;
};


void
InterruptDelegate()
{
   LT_STATUS sts = LT_STS_Uninit;

   // stop after 10 calls
   MyInterrupt interrupt(10);

   // make the image reader
   const LTFileSpec fileSpec("data/meg_cr20.sid");
   MrSIDImageReader *reader = MrSIDImageReader::create();
   ASSERT(reader != NULL);

   sts = reader->initialize(fileSpec);
   ASSERT(LT_SUCCESS(sts));
   
   reader->setInterruptDelegate(&interrupt);

   const lt_uint32 w = 640;
   const lt_uint32 h = 480;
   const LTIScene scene(0, 0, w, h, 1.0);
   const lt_uint32 siz = w * h * 1;
   lt_uint8* membuf = new lt_uint8[siz*3];
   void* bufs[3] = { membuf+siz*0, membuf+siz*1, membuf+siz*2 };
   LTISceneBuffer bufData(reader->getPixelProps(), w, h, bufs);
   
   // the decode will fail, return 999 from within the delegate
   sts = reader->read(scene, bufData);
   ASSERT(sts == 999);

   // verify the interrupt handler was called several times
   ASSERT(interrupt.m_cnt == 10);
   
   reader->release();
   reader = NULL;
   
   return;
}
