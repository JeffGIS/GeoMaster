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


// This example shows some of the basic operations supported by
// the LTIOStreamInf class.

#include "support.h"

#include "lt_ioFileStream.h"
#include "lt_ioSubStream.h"

LT_USE_NAMESPACE(LizardTech);

void
UsingStreams()
{
   LT_STATUS sts = LT_STS_Uninit;

   // make and open a file-based stream
   LTIOFileStream stream;
   sts = stream.initialize("data/meg.hdr", "r");
   ASSERT(LT_SUCCESS(sts));
   sts = stream.open();
   ASSERT(LT_SUCCESS(sts));

   // read the first five bytes
   lt_uint8 buf[5];
   lt_uint32 cnt = stream.read(buf, 5);
   ASSERT(cnt == 5);
   ASSERT(strncmp((char*)buf, "NROWS", 5)==0);

   // seek ahead two bytes
   sts = stream.seek(2, LTIO_SEEK_DIR_CUR);
   ASSERT(LT_SUCCESS(sts));

   const lt_int64 pos = stream.tell();
   ASSERT(pos==7);

   // read two more bytes
   cnt = stream.read(buf, 2);
   ASSERT(cnt == 2);
   ASSERT(strncmp((char*)buf, "80", 2)==0);

   // all done
   sts = stream.close();
   ASSERT(LT_SUCCESS(sts));

   //
   // now test the substream class
   //
   {
      LTIOFileStream stream1;
      sts = stream1.initialize("data/meg.hdr", "r");
      ASSERT(LT_SUCCESS(sts));
      sts = stream1.open();
      ASSERT(LT_SUCCESS(sts));

      LTIOSubStream stream2;
      sts = stream2.initialize(&stream1, 7, 9, false);
      ASSERT(LT_SUCCESS(sts));
      sts = stream2.open();
      ASSERT(LT_SUCCESS(sts));

      lt_uint8 buf[2];
      const lt_int32 cnt = stream2.read(buf, 2);
      ASSERT(cnt == 2);
      ASSERT(strncmp((char*)buf, "80", 2)==0);
      
      lt_int64 pos1 = stream1.tell();
      lt_int64 pos2 = stream2.tell();
      ASSERT(pos1 == 9);
      ASSERT(pos2 == 2);

      stream2.close();
      stream1.close();
   }

   return;
}
