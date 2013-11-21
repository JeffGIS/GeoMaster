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


/*
 * This demonstrates how to use the C API to decode a scene from a
 * MrSID image.
 */

#include "support.h"

#include "ltic_api.h"

#undef ASSERT
#define ASSERT(x) if (!(x)) exit(1);

#ifdef LT_CPLUSPLUS
extern "C" {
#endif


void
UsingCInterface()
{
   LT_STATUS sts = LT_STS_Uninit;
   LTICImageH image = 0;
   void** bufs = (void**)malloc(sizeof(char*) * 3);
   char buf0[4];
   char buf1[4];
   char buf2[4];
   bufs[0] = buf0;
   bufs[1] = buf1;
   bufs[2] = buf2;

   sts = ltic_openMrSIDImageFile(&image, "data/meg_cr20.sid");
   ASSERT(LT_SUCCESS(sts));

   ASSERT(ltic_getWidth(image) == 640);
   ASSERT(ltic_getHeight(image) == 480);
   ASSERT(ltic_getNumBands(image) == 3);
   ASSERT(ltic_getColorSpace(image) == LTI_COLORSPACE_RGB);
   ASSERT(ltic_getDataType(image) == LTI_DATATYPE_UINT8);

   /* just read the upper-left 4 pixels */
   sts = ltic_decode(image, 0, 0, 2, 2, 1.0, bufs);
   ASSERT(LT_SUCCESS(sts));

   ASSERT(buf0[0] == 17);
   ASSERT(buf0[1] == 18);
   ASSERT(buf0[2] == 16);
   ASSERT(buf0[3] == 17);
   ASSERT(buf1[0] == 29);
   ASSERT(buf1[1] == 28);
   ASSERT(buf1[2] == 26);
   ASSERT(buf1[3] == 26);
   ASSERT(buf2[0] == 20);
   ASSERT(buf2[1] == 20);
   ASSERT(buf2[2] == 18);
   ASSERT(buf2[3] == 19);

   sts = ltic_closeImage(image);
   ASSERT(LT_SUCCESS(sts));

   free(bufs);

   return;
}


#ifdef LT_CPLUSPLUS
}
#endif
