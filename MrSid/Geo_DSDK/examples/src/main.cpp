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


#include "support.h"
#include "main.h"


int
main()
{
   checkCwd();

   DecodeJP2ToBBB();
   DecodeJP2ToMemory();
   DecodeMrSIDToMemory();
   DecodeMrSIDLidar();
   DecodeMrSIDToRaw();
   DecodeNITFToBBB();
   DecodeMrSIDToTIFF();
   DecodeJP2ToJPG();
   DerivedImageFilter();  
   DerivedImageReader();
   DerivedImageWriter();
   DerivedStream();
   ErrorHandling();
   GeoScene();
   ImageInfo();
   InterruptDelegate();
   MetadataDump();
   Pipeline();
   ProgressDelegate();
   SceneBuffer();
   UsingCInterface();
   UsingCStream();
   UsingStreams();

   UserTest();

   printf("passed DSDK tests\n");
   
   return 0;
}
