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
 * This demonstrates how to use the C-callable stream class.  We provide
 * functions which implement a FILE*-based stream, similar in spirit to
 * the example class derived in DerivedStream.cpp.  Note also that the
 * test workflow in this example is the same as in UsingStreams.cpp.
 */

#include "support.h"

#include "lt_ioCStream.h"

#undef ASSERT
#define ASSERT(x) if (!(x)) exit(1);

#ifdef LT_CPLUSPLUS
extern "C" {
#endif


/*---------------------------------------------------------------------------
 * user-level object storing stream data
 *-------------------------------------------------------------------------*/

typedef struct StreamData
{
   FILE* fp;
   char* name;
   char* mode;
} StreamData;

static StreamData*
createStreamData(const char* name, const char* mode)
{
   StreamData* streamData = (StreamData*)malloc(sizeof(StreamData));
   
   streamData->fp = NULL;
   
   streamData->name = (char*)malloc(strlen(name)+1);
   strcpy(streamData->name,name);
   
   streamData->mode = (char*)malloc(strlen(mode)+1);
   strcpy(streamData->mode,mode);
   
   return streamData;
}

void
deleteStreamData(StreamData* streamData)
{
   free(streamData->name);
   free(streamData->mode);
   free(streamData);
}


/*---------------------------------------------------------------------------
 * user functions for C stream
 *-------------------------------------------------------------------------*/

static LT_STATUS
myOpen(void* user)
{
   StreamData* streamData = (StreamData*)user;
   streamData->fp = fopen(streamData->name, streamData->mode);
   return LT_STS_Success;
}

static LT_STATUS
myClose(void* user)
{
   StreamData* streamData = (StreamData*)user;
   fclose(streamData->fp);
   streamData->fp = NULL;
   return LT_STS_Success;
}

static lt_uint32
myRead(void* user, lt_uint8* buf, lt_uint32 len)
{
   StreamData* streamData = (StreamData*)user;
   lt_uint32 cnt = (lt_uint32)fread(buf, 1, len, streamData->fp);
   return cnt;
}

static lt_uint32
myWrite(void* user, const lt_uint8* buf, lt_uint32 len)
{
   StreamData* streamData = (StreamData*)user;
   lt_uint32 cnt = (lt_uint32)fwrite(buf, 1, len, streamData->fp);
   return cnt;
}

static LT_STATUS
mySeek(void* user, lt_int64 pos, LTIOSeekDir dir)
{
   StreamData* streamData = (StreamData*)user;
   const long lpos = (long)pos;
   int stat = 0;
   int mydir=0;

   switch (dir)
   {
   case LTIO_SEEK_DIR_BEG: mydir = SEEK_SET; break;
   case LTIO_SEEK_DIR_CUR: mydir = SEEK_CUR; break;
   case LTIO_SEEK_DIR_END: mydir = SEEK_END; break;
   };

   stat = fseek(streamData->fp, lpos, mydir);
   if (stat != 0) return LT_STS_Failure;
   return LT_STS_Success;
}

static lt_int64
myTell(void* user)
{
   StreamData* streamData = (StreamData*)user;
   long pos = ftell(streamData->fp);
   return pos;
}

static lt_uint8
myIsEOF(void* user)
{
   StreamData* streamData = (StreamData*)user;
   return (0 != feof(streamData->fp));
}

static lt_uint8
myIsOpen(void* user)
{
   StreamData* streamData = (StreamData*)user;
   return (streamData->fp != NULL);
}

static void*
myDuplicate(void* user)
{
   /* this will leak -- proper implementation would keep a list of
    * allocated StreamData objects...
    */
   StreamData* streamData = (StreamData*)user;
   StreamData* newStreamData = createStreamData(streamData->name, streamData->mode);
   return newStreamData;
}


/*---------------------------------------------------------------------------
 * show use of C streams
 *-------------------------------------------------------------------------*/

void
UsingCStream()
{
   LT_STATUS sts = LT_STS_Uninit;
   lt_uint8 buf[5];
   lt_uint32 cnt = 0;
   LTIOStreamH stream = 0;
   lt_int64 pos = 0;

   /* make and open a file-based stream */
   StreamData* streamData = createStreamData("data/meg.hdr", "r");

   stream = lt_ioCallbackStreamCreate(myOpen, myClose,
                                      myRead, myWrite,
                                      mySeek, myTell,
                                      myIsEOF, myIsOpen,
                                      myDuplicate,
                                      streamData);
   ASSERT(stream!=NULL);

   sts = lt_ioCStreamOpen(stream);
   ASSERT(LT_SUCCESS(sts));
   ASSERT(streamData->fp!=NULL);

   /* read the first five bytes */
   cnt = lt_ioCStreamRead(stream,buf, 5);
   ASSERT(cnt == 5);
   ASSERT(strncmp((char*)buf, "NROWS", 5)==0);

   /* seek ahead two bytes */
   sts = lt_ioCStreamSeek(stream, 2, LTIO_SEEK_DIR_CUR);
   ASSERT(LT_SUCCESS(sts));

   pos = lt_ioCStreamTell(stream);
   ASSERT(pos==7);

   /* read two more bytes */
   cnt = lt_ioCStreamRead(stream,buf, 2);
   ASSERT(cnt == 2);
   ASSERT(strncmp((char*)buf, "80", 2)==0);

   {
      /* duplicate the stream */
      LTIOStreamH stream2 = lt_ioCStreamDuplicate(stream);

      sts = lt_ioCStreamOpen(stream2);
      ASSERT(LT_SUCCESS(sts));

      cnt = lt_ioCStreamRead(stream2,buf, 5);
      ASSERT(cnt == 5);
      ASSERT(strncmp((char*)buf, "NROWS", 5)==0);

   }

   /* all done */
   sts = lt_ioCStreamClose(stream);
   ASSERT(LT_SUCCESS(sts));

   return;
}


#ifdef LT_CPLUSPLUS
}
#endif
